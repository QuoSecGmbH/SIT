/*
  Utilities for AFF4 imaging. These are mostly high level utilities used by the
  command line imager.
*/

#include "aff4/libaff4.h"
#include "aff4/aff4_imager_utils.h"
#include "aff4/rdf.h"
#include <iostream>
#include <fstream>
#include <streambuf>
#include <string>
#include <time.h>
#include <memory>
#include <algorithm>
#include "spdlog/spdlog.h"

namespace aff4 {

   // bool metadataDone; // CUSTOM signals that specified metadataFiles should be ignored by input processing
   // char* metadataFile; // CUSTOM metadataFile name as defined in -m -> what about path??

AFF4Status ExtractStream(
    DataStore& resolver, VolumeGroup &volumes,
    URN input_urn,
    std::string filename,
    bool truncate) {

    AFF4Flusher<AFF4Stream> in_stream;
    AFF4Flusher<FileBackedObject> out_stream;

    // Use the VolumeGroup to figure out which stream to get.
    RETURN_IF_ERROR(volumes.GetStream(input_urn, in_stream));

    resolver.logger->info("Extracting {} to {} ({})",
                          input_urn, filename, in_stream->Size());

    RETURN_IF_ERROR(NewFileBackedObject(
                        &resolver, filename, truncate ? "truncate" : "append",
                        out_stream));

    DefaultProgress progress(&resolver);
    progress.length = in_stream->Size();

    return out_stream->WriteStream(in_stream.get(), &progress);
}


// A Progress indicator which keeps tabs on the size of the output volume.
// This is used to support output volume splitting.
bool VolumeManager::Report(aff4_off_t readptr) {
    if (!MaybeSwitchVolumes())
        return false;

    return DefaultProgress::Report(readptr);
}

void VolumeManager::ManageStream(AFF4Stream *stream) {
    streams.push_back(stream);
}

bool VolumeManager::MaybeSwitchVolumes() {
    if (imager->max_output_volume_file_size == 0)
        return true;

    // We are only allowed to switch the volume at valid
    // checkpoints.
    for (auto stream: streams) {
        if (!stream->CanSwitchVolume())
            return true;  // Just keep reporting - cant do
        // anything now.
    }

    // Is the output volume still ok?
    AFF4Volume *volume;
    imager->GetCurrentVolume(&volume);
    if (imager->max_output_volume_file_size > volume->Size()) {
        return true;
    }

    // Make a new volume.
    AFF4Volume *new_volume;
    imager->GetNextPart();
    imager->GetCurrentVolume(&new_volume);
    for (auto stream: streams) {
        stream->SwitchVolume(new_volume);
    }



    return true;
}


AFF4Status BasicImager::Run(int argc, char** argv)  {
    AFF4Status res = Initialize();
    if (res != STATUS_OK) {
        return res;
    }

    RegisterArgs();

    TCLAP::CmdLine cmd(GetName(), ' ', GetVersion());

    for (auto it = args.rbegin(); it != args.rend(); it++) {
        cmd.add(it->get());
    }

    try {
        cmd.parse(argc, argv);
        res =  ParseArgs();
    } catch(const TCLAP::ArgException& e) {
        resolver.logger->error("Error {} {}", e.error(), e.argId());
        return GENERIC_ERROR;
    }

    if (res == CONTINUE) {
        res = ProcessArgs();
    }

    return res;
}

AFF4Status BasicImager::ParseArgs() {
    AFF4Status result = handle_logging();

    if (Get("threads")->isSet()) {
        int threads = GetArg<TCLAP::ValueArg<int>>("threads")->getValue();
        resolver.logger->info("Will use {} threads.", threads);
        resolver.pool.reset(new ThreadPool(threads));
    }

    // Check for incompatible commands.
    if (Get("export")->isSet() && Get("input")->isSet()) {
        resolver.logger->critical(
            "The --export and --input flags are incompatible. "
            "Please select only one.");
        return INCOMPATIBLE_TYPES;
    }

    if (result == CONTINUE && Get("metadata")->isSet()) {
        result = parse_metadataFiles();
    }
    
    if (result == CONTINUE) {
        result = parse_input();
    }

    if (result == CONTINUE && Get("compression")->isSet()) {
        result = handle_compression();
    }

    if (result == CONTINUE && Get("aff4_volumes")->isSet()) {
        result = handle_aff4_volumes();
    }

    if (result == CONTINUE && Get("split")->isSet()) {
        max_output_volume_file_size = GetArg<TCLAP::SizeArg>(
            "split")->getValue();
        resolver.logger->info("Output volume will be limited to {} bytes",
                              max_output_volume_file_size);
    }

    return result;
}

AFF4Status BasicImager::ProcessArgs() {
    AFF4Status result = CONTINUE;

    if (Get("list")->isSet()) {
        result = handle_list();
    }

    if (result == CONTINUE && Get("view")->isSet()) {
        result = handle_view();
    }

    if (result == CONTINUE && Get("export")->isSet()) {
        result = handle_export();
    }

    if (result == CONTINUE && inputs.size() > 0) {
        result = process_input();
    }

    if (result == CONTINUE && metadataFiles.size() > 0) {
        result = process_metadataFiles();
    }

    return result;
}


AFF4Status BasicImager::handle_logging() {
    if (Get("logfile")->isSet()) {
        std::vector<spdlog::sink_ptr> sinks = resolver.logger->sinks();

        auto new_sink = std::make_shared<spdlog::sinks::simple_file_sink_mt>(
            GetArg<TCLAP::ValueArg<std::string>>(
                "logfile")->getValue());
        sinks.push_back(new_sink);

        resolver.logger = std::make_shared<spdlog::logger>(
                     "", sinks.begin(), sinks.end());
    }

    int level = GetArg<TCLAP::MultiSwitchArg>("debug")->getValue();
    switch(level) {
    case 0:
        resolver.logger->set_level(spdlog::level::err);
        break;

    case 1:
        resolver.logger->set_level(spdlog::level::warn);
        break;

    case 2:
        resolver.logger->set_level(spdlog::level::info);
        break;

    case 3:
        resolver.logger->set_level(spdlog::level::debug);
        break;

    default:
        resolver.logger->set_level(spdlog::level::trace);
        break;
    }


    resolver.logger->set_pattern("%Y-%m-%d %T %L %v");

    return CONTINUE;
}

AFF4Status BasicImager::handle_aff4_volumes() {
    auto volumes = GetArg<TCLAP::UnlabeledMultiArg<std::string>>(
        "aff4_volumes")->getValue();

    for (std::string glob : volumes) {
        for (const auto &volume_to_load:  GlobFilename(glob)) {
            // Currently we support AFF4Directory and ZipFile. If the
            // directory does not already exist, and the argument ends
            // with a / then we create a new directory.
            if (AFF4Directory::IsDirectory(volume_to_load, /* must_exist= */ false)) {
                AFF4Flusher<AFF4Directory> volume;
                RETURN_IF_ERROR(AFF4Directory::OpenAFF4Directory(
                                    &resolver, volume_to_load, volume));

                volume_objs.AddVolume(AFF4Flusher<AFF4Volume>(volume.release()));

            } else {
                AFF4Flusher<FileBackedObject> backing_stream;
                RETURN_IF_ERROR(NewFileBackedObject(
                                    &resolver, volume_to_load,
                                    "read", backing_stream));

                AFF4Flusher<ZipFile> volume;
                RETURN_IF_ERROR(ZipFile::OpenZipFile(
                                    &resolver,
                                    AFF4Flusher<AFF4Stream>(backing_stream.release()),
                                    volume));

                volume_objs.AddVolume(AFF4Flusher<AFF4Volume>(volume.release()));
            }
        }
    }

    return CONTINUE;
}

AFF4Status BasicImager::handle_list() {
    URN image_type(AFF4_IMAGE_TYPE);
    for (const auto& subject: resolver.Query(
             URN(AFF4_TYPE), &image_type)) {
        std::cout << subject.SerializeToString() << "\n";
    }

    return STATUS_OK;
}


AFF4Status BasicImager::handle_view() {
    resolver.Dump(GetArg<TCLAP::SwitchArg>("verbose")->getValue());

    // After running the View command we are done.
    return STATUS_OK;
}

AFF4Status BasicImager::parse_input() {
    for (const auto& input: GetArg<TCLAP::MultiArgToNextFlag>("input")->getValue()) {
        // Read input files from stdin - this makes it easier to support
        // files with spaces and special shell chars in their names.

        
        if (input == "@") {
            for(std::string line;;) {
                std::getline (std::cin, line);
                if (line.size() == 0) break;
                inputs.push_back(line);
            }
        } else {
            std::cout << "parse_input() input file to consider for filter:  " << input << "\n";
            
            inputs.push_back(input);
        }
    }

    return CONTINUE;
}

AFF4Status BasicImager::process_input() {
    genericMetafilename = "genericMETA.turtle";
    std::string inputForwardSlash; // a temporary string to compare paths independently of slash changes by GlobFilename()
  
    for (std::string glob : inputs) {
            
        std::cout << "process_input() input glob before GlobFilename:  " << glob << "\n";
        
        for (std::string input : GlobFilename(glob)) {
            
            std::cout << "process_input() input file to consider for filter:  " << input << "\n";
                  
            inputForwardSlash = input;
            // Replace backslashes with forward slashes
            std::replace(inputForwardSlash.begin(), inputForwardSlash.end(), '\\', '/');

            std::cout << "process_input() inputForwardSlash file to consider for filter:  " << inputForwardSlash << "\n";

            // Appends genericMetafilename to metadataFiles if found in input and not already in metadataFiles -> process_metadataFiles() will run next
            if ((inputForwardSlash.find(genericMetafilename) != std::string::npos) ) { // TODO && (std::find(metadataFiles.begin(), metadataFiles.end(), genericMetafilename) != metadataFiles.end()) == false
                metadataFiles.push_back(inputForwardSlash); // What if genericMetafilename specified by -m ?? TODO FIX !!
            }

            if (((std::find(metadataFiles.begin(), metadataFiles.end(), inputForwardSlash) != metadataFiles.end())==false) && (inputForwardSlash.find(genericMetafilename)!= std::string::npos)==false )
            {  // TODO if genericMETA.rdf without -m -> call metaHandler for this input, -> see above!
                
                std::cout << "process_input() input file not filtered:  " << input << "\n";
                
                // Check if the volume needs to be split.
                VolumeManager(&resolver, this).MaybeSwitchVolumes();

                // Get the output volume.
                AFF4Volume* volume;
                RETURN_IF_ERROR(GetCurrentVolume(&volume));

                AFF4Flusher<FileBackedObject> input_stream;

                resolver.logger->debug("Will add file {}", input);
                RETURN_IF_ERROR(NewFileBackedObject(
                    &resolver, input, "read",
                    input_stream));
                resolver.logger->info("Adding {} as {}", input, input_stream->urn);

                URN image_urn;

                // Create a new AFF4Image in this volume.
                if (GetArg<TCLAP::SwitchArg>("relative")->getValue()) {
                    char cwd[PATH_MAX];
                    if (getcwd(cwd, PATH_MAX) == NULL) {
                        return IO_ERROR;
                    }

                    URN current_dir_urn = URN::NewURNFromFilename(cwd, false);
                    image_urn.Set(volume->urn.Append(current_dir_urn.RelativePath(
                        input_stream->urn)));
                }
                else {
                    image_urn.Set(volume->urn.Append(input_stream->urn.Path()));

                    // Store the original filename.
                    resolver.Set(image_urn, AFF4_STREAM_ORIGINAL_FILENAME,
                        new XSDString(input));
                }

                // For very small streams, it is more efficient to just store them without
                // compression. Also if the user did not ask for compression, there is no
                // advantage in storing a Bevy based image, just store it in one piece.
                if (compression == AFF4_IMAGE_COMPRESSION_ENUM_STORED ||
                    (input_stream->Size() > 0 && input_stream->Size() < 10 * 1024 * 1024)) {

                    AFF4Flusher<AFF4Stream> segment;
                    RETURN_IF_ERROR(volume->CreateMemberStream(image_urn, segment));

                    // If the underlying stream supports compression, lets do that.
                    segment->compression_method = AFF4_IMAGE_COMPRESSION_ENUM_DEFLATE;

                    // This doesnt do anything anyway because segments can
                    // not be split across volumes.
                    VolumeManager progress(&resolver, this);
                    progress.ManageStream(segment.get());

                    // Copy the input stream to the output stream.
                    RETURN_IF_ERROR(segment->WriteStream(input_stream.get(), &progress));

                    // Make this stream as an Image (Should we have
                    // another type for a LogicalImage?
                    
                    resolver.Set(segment->urn, AFF4_TYPE, new URN(AFF4_IMAGE_TYPE),
                        /* replace = */ false);

                    // We need to explicitly check the abort status here.
                    if (should_abort || aff4_abort_signaled) {
                        return ABORTED;
                    }

                    // Otherwise use an AFF4Image.
                }
                else {
                    AFF4Flusher<AFF4Image> image_stream;
                    RETURN_IF_ERROR(
                        AFF4Image::NewAFF4Image(
                            &resolver, image_urn, volume, image_stream));

                    // Set the output compression according to the user's wishes.
                    image_stream->compression = compression;

                    // Copy the input stream to the output stream.
                    VolumeManager progress(&resolver, this);
                    progress.ManageStream(image_stream.get());

                    RETURN_IF_ERROR(image_stream->WriteStream(input_stream.get(), &progress));

                    // Make this stream as an Image (Should we have
                    // another type for a LogicalImage?
                    resolver.Set(image_urn, AFF4_TYPE, new URN(AFF4_IMAGE_TYPE),
                        /* replace = */ false);
                }
            }
        }
    }

    actions_run.insert("input");

    return CONTINUE;
}

AFF4Status BasicImager::parse_metadataFiles() {
    printf("enter parse_metadataFiles()\n");
    for (const auto& input : GetArg<TCLAP::MultiArgToNextFlag>("metadata")->getValue()) {
        // Read input files from stdin - this makes it easier to support
        // files with spaces and special shell chars in their names.
        // This would need to be separate from stdin input e.g --input @
        /*if (metadataFiles == "@") {
            for (std::string line;;) {
                std::getline(std::cin, line);
                if (line.size() == 0) break;
                inputs.push_back(line);
            }
        }*/
        //else {
        //Conversion to string
        std::string tmp = input;
        // Appends file format 
        
        //tmp = GlobFilename(tmp);

        //printf("parse_metadataFile: String input: %s \n",input); // TODO remove
        std::cout << "parse_metadataFile: String input: " << input << "\n";
            metadataFiles.push_back(tmp);


        //}
    }

    return CONTINUE;
}

AFF4Status BasicImager::process_metadataFiles() {
    
    // Set if unsuccessful processing of metadataFile. Will cause the metadataFile to be included in image as file.
    bool error = false;

    std::cout << "enter process_metadataFiles()\n";
    for (std::string glob : metadataFiles) {
        
        std::cout << "enter process_metadataFiles(), outer for loop: String glob: " << glob << "\n";
        for (std::string metadataFile : GlobFilename(glob)) {
            
            std::cout << "enter process_metadataFiles(), inner for loop: String metadataFile: " << metadataFile << "\n";
            
            //bool verbose;
            std::string metadataFileContent; // might be needed instead of normal string

            // Reads in metadataFile as String
            std::ifstream t(metadataFile);
            std::string metadataFileString((std::istreambuf_iterator<char>(t)),
                std::istreambuf_iterator<char>());
                                             
            // TODO REVIEW INPUT SECURITY E.G STRINGS              

           std::cout << "process_metadataFiles read rdf attempt: \n" << metadataFileString <<"\n";            

           // Serialize metadataFileString and set in information.turtle
                      
           unsigned int prefixCounter = 0;


           std::string header;
           size_t position;
           
           //Check for "@id <aff4:metadata> ."
           header = "@id <aff4:metadata> .";
           position = metadataFileString.find(header);
           if (position != std::string::npos) {
               std::cout << "found @id" << '\n';
               prefixCounter++;
               metadataFileString.erase(position, header.length());
           }
           //Check for "@prefix rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> ."
           header = "@prefix rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> .";
           position = metadataFileString.find(header);
           if (position != std::string::npos) {
               std::cout << "found @prefix rdf" << '\n';
               prefixCounter++;
               metadataFileString.erase(position, header.length());
           }
           //Check for "@prefix aff4: <http://aff4.org/Schema#> ."
           header = "@prefix aff4: <http://aff4.org/Schema#> .";
           position = metadataFileString.find(header);
           if (position != std::string::npos) {
               std::cout << "found @prefix aff4" << '\n';
               prefixCounter++;
               metadataFileString.erase(position, header.length());
           }
           //Check for "@prefix xsd: <http://www.w3.org/2001/XMLSchema#> ."
           header = "@prefix xsd: <http://www.w3.org/2001/XMLSchema#> .";
           position = metadataFileString.find(header);
           if (position != std::string::npos) {
               std::cout << "found @prefix xsd" << '\n';
               prefixCounter++;
               metadataFileString.erase(position, header.length());
           }
           
           if (prefixCounter ==4) std::cout << "all headers correct" << '\n';

           //Remove empty lines
           metadataFileString.erase(std::unique(metadataFileString.begin(), metadataFileString.end(),
               [](char a, char b) {return a == '\n' && b == '\n';}),
               metadataFileString.end());

           std::string eof_signal = "eof-metadataFileString"; // Maybe unique string type for this?
           std::string filePath;
           std::string startDelimiter = "<"; 
           std::string endDelimiter = ">";
           std::string metadataSection;
           std::string metadataSectionRemover; // For deleting this substring from metadataFileString
           size_t startDelimiterPos;
           size_t endDelimiterPos;
           bool warningArtificalDelimiter = false; // Set if artifical delimiter is set
           URN imageURN;
           URN attributeURN;
           URN testURN; // TODO remove
           
           //Check for </path/to/file> 
                // Append metadata attributes to each file in information.turtle
           while (metadataFileString.find(eof_signal) == std::string::npos) {
               std::cout << "process_metadataFiles entering while loop " <<  "\n";
               
               // Check for <?> statement
               
               startDelimiterPos = metadataFileString.find("<"); 
               
               if (startDelimiterPos != std::string::npos) {
                   std::cout << "process_metadataFiles first if " << "\n";
                   endDelimiterPos = metadataFileString.find(">");
                   
                   if (endDelimiterPos != std::string::npos) {
                       std::cout << "process_metadataFiles second if " << "\n";
                       
                       // Selects filePath
                       filePath = metadataFileString.substr(startDelimiterPos+1, endDelimiterPos - startDelimiterPos -1);
                       std::cout << "process_metadataFiles filePath: " << filePath << "\n";
                       
                       // Checks if filePath empty 
                       if (filePath.length() > 0) {
                           std::cout << "process_metadataFiles third if " << "\n";                
                          
                           std::cout << "process_metadataFiles metadataFileString before creating sections, before removing path: " << metadataFileString << "\n";
                           // Remove filePath and its Delimiters
                           metadataFileString.erase(startDelimiterPos, filePath.length()+2);
                           std::cout << "process_metadataFiles startDelimiterPos: " << startDelimiterPos << " endDelimiterPos: "<< endDelimiterPos <<"\n";
                           

                           std::cout << "process_metadataFiles appending MetadataFilePath: " << filePath << "\n";

                           //Appending metadata from filePath
                           
                           // Check for next filePath delimiter as end limit for this metadata section
                           std::cout << "process_metadataFiles metadataFileString before creating sections, after removing path: " << metadataFileString << "\n";

                           // if no startDelimiter left in file, append artifical one TODO remove
                           startDelimiterPos = metadataFileString.find(startDelimiter);
                           if (startDelimiterPos == std::string::npos) {
                               metadataFileString.append(startDelimiter);
                               warningArtificalDelimiter = true;

                           }

                           std::cout << "process_metadataFiles before section creation: startDelimiterPos: " << startDelimiterPos << " endDelimiterPos: " << endDelimiterPos << "\n";
                           //std::cout << "process_metadataFiles delimiterCheck :  endDelimiter" << filePath << "\n";
                           metadataSection = metadataFileString.substr(0, startDelimiterPos - 1);
                                                     
                           metadataSectionRemover = metadataSection; // Copy to allow removal of substring metadataSection 

                           std::cout << "process_metadataFiles metadata Section for: " << filePath << "\n" << metadataSection << "\n";

                           AFF4Volume* volume;
                           RETURN_IF_ERROR(GetCurrentVolume(&volume));

                           imageURN = volume->urn;
                           attributeURN = imageURN;
                           attributeURN.Append(filePath);
                           testURN.Set(volume->urn.Append(filePath));
                           
                           // Appends metadata if information.turtle includes the required file
                           if (resolver.HasURN(attributeURN)) {
                               
                               while (metadataSection.find("aff4:") != std::string::npos) {
                                   size_t startPosition; // Used to store startPositions for substr() usage
                                   size_t RDFdelimiter; // Used to store either ";" or "."
                                   std::string AFF4Type; // Used to store the AFF4Type that is currently being checked 
                                   
                                   // Contains the metadataSection substring beginning with AFF4Type
                                   std::string metadataSubstr;

                                   // Contains the metadata
                                   std::string metadata;
                                   std::string metadataFiltered;                                   
                                   
                                   AFF4Type = "aff4:timestamp";
                                   if (metadataSection.find(AFF4Type) != std::string::npos) {

                                       startPosition = metadataSection.find(AFF4Type);
                                                                              
                                       // Contains metadataSection substring beginning with AFF4Type 
                                       
                                       std::cout << "process_metadataFiles metadata Section for: " << filePath << "\n" << metadataSection << "\n";
                                       std::cout << "process_metadataFiles startPosition: " << startPosition << "\n";

                                       metadataSubstr = metadataSection.substr(startPosition);
                                       std::cout << "process_metadataFiles metadataSubstr: " << "\n" << metadataSubstr << "\n";
                                       
                                       // Contains the metadata
                                       startPosition = metadataSubstr.find(AFF4Type)+ AFF4Type.length();
                                       if (metadataSubstr.find(";") != std::string::npos) {
                                          RDFdelimiter = metadataSubstr.find(";");
                                          std::cout << "process_metadataFiles startPosition: " << startPosition << "\n";
                                          std::cout << "process_metadataFiles RDFdelimiter: " << RDFdelimiter << "\n";
                                          std::cout << "process_metadataFiles metadataSubstr.length(): " << metadataSubstr.length() << "\n";
                                           metadata = metadataSubstr.substr(startPosition, RDFdelimiter-startPosition+1);
                                           std::cout << "process_metadataFiles metadata.length(): " << metadata.length() << "\n";
                                       }
                                            
                                       else {
                                           RDFdelimiter = metadataSubstr.find(".");
                                           metadata = metadataSubstr.substr(startPosition, RDFdelimiter - startPosition+1);
                                       }
                                       // Remove aff4:Type + metadata
                                       metadataSection.erase(metadataSection.find(AFF4Type), AFF4Type.length() + metadata.length());
                                       std::cout << "process_metadataFiles metadataSection after removing AFF4Type + metadata: " << "\n" << metadataSection << "\n";


                                       std::cout << "process_metadataFiles metadata: " << "\n"  << metadata << "\n";

                                       // Filter metadata 

                                       metadataFiltered = metadata.substr(metadata.find('"')+1); // Start at first "
                                       metadataFiltered = metadataFiltered.substr(0, metadataFiltered.find('"'));
                                       std::cout << "process_metadataFiles metadataFiltered: " << "\n" << metadataFiltered << "\n";
                                       // Check for input type
                                       if (metadata.find("^^xsd:dateTime") != std::string::npos)
                                       {
                                           resolver.Set(testURN, AFF4_STREAM_TIMESTAMP, new XSDString(metadataFiltered), true);
                                       }
                                       /* if (metadata.find("^^xsd:int")!= std::string::npos) 
                                       {
                                           resolver.Set(testURN, AFF4_STREAM_TIMESTAMP, new XSDLong(metadataFiltered), true);
                                       }
                                       if (metadata.find("^^xsd:long") != std::string::npos) {
                                               resolver.Set(testURN, AFF4_STREAM_TIMESTAMP, new XSDInteger(metadataFiltered), true); // TODO find XSDTIMEDATE conversion
                                       }*/
                                       else {
                                           resolver.Set(testURN, AFF4_STREAM_TIMESTAMP, new XSDString(metadataFiltered), true);
                                       }
                                       
                                   }

                                   AFF4Type = "aff4:examiner";
                                   if (metadataSection.find(AFF4Type) != std::string::npos) {

                                       startPosition = metadataSection.find(AFF4Type);

                                       // Contains metadataSection substring beginning with AFF4Type 

                                       std::cout << "process_metadataFiles metadata Section for: " << filePath << "\n" << metadataSection << "\n";
                                       std::cout << "process_metadataFiles startPosition: " << startPosition << "\n";

                                       metadataSubstr = metadataSection.substr(startPosition);
                                       std::cout << "process_metadataFiles metadataSubstr: " << "\n" << metadataSubstr << "\n";

                                       // Contains the metadata
                                       startPosition = metadataSubstr.find(AFF4Type) + AFF4Type.length();
                                       if (metadataSubstr.find(";") != std::string::npos) {
                                           RDFdelimiter = metadataSubstr.find(";");
                                           std::cout << "process_metadataFiles startPosition: " << startPosition << "\n";
                                           std::cout << "process_metadataFiles RDFdelimiter: " << RDFdelimiter << "\n";
                                           std::cout << "process_metadataFiles metadataSubstr.length(): " << metadataSubstr.length() << "\n";
                                           metadata = metadataSubstr.substr(startPosition, RDFdelimiter - startPosition + 1);
                                           std::cout << "process_metadataFiles metadata.length(): " << metadata.length() << "\n";
                                       }

                                       else {
                                           RDFdelimiter = metadataSubstr.find(".");
                                           metadata = metadataSubstr.substr(startPosition, RDFdelimiter - startPosition + 1);
                                       }
                                       // Remove aff4:Type + metadata
                                       metadataSection.erase(metadataSection.find(AFF4Type), AFF4Type.length() + metadata.length());
                                       std::cout << "process_metadataFiles metadataSection after removing AFF4Type + metadata: " << "\n" << metadataSection << "\n";


                                       std::cout << "process_metadataFiles metadata: " << "\n" << metadata << "\n";

                                       // Filter metadata 

                                       metadataFiltered = metadata.substr(metadata.find('"') + 1); // Start at first "
                                       metadataFiltered = metadataFiltered.substr(0, metadataFiltered.find('"'));
                                       std::cout << "process_metadataFiles metadataFiltered: " << "\n" << metadataFiltered << "\n";
                                       // Check for input type
                                       if (metadata.find("^^xsd:dateTime") != std::string::npos)
                                       {
                                           resolver.Set(testURN, AFF4_STREAM_EXAMINER, new XSDString(metadataFiltered), true);
                                       }
                                       /* if (metadata.find("^^xsd:int")!= std::string::npos)
                                       {
                                           resolver.Set(testURN, AFF4_STREAM_TIMESTAMP, new XSDLong(metadataFiltered), true);
                                       }
                                       if (metadata.find("^^xsd:long") != std::string::npos) {
                                               resolver.Set(testURN, AFF4_STREAM_TIMESTAMP, new XSDInteger(metadataFiltered), true); // TODO find XSDTIMEDATE conversion
                                       }*/
                                       else {
                                           resolver.Set(testURN, AFF4_STREAM_EXAMINER, new XSDString(metadataFiltered), true);
                                       }

                                   }
                                   
                                   else {
                                       // Instead add custom type "aff4:notes"  TODO what if multiple unknow aff4:types? How to append notes.
                                       AFF4Type = "aff4:"; // Generic identifier
                                       size_t AFF4TypeLength = AFF4Type.length(); // Needs to be increased for unknown AFF4Type
                                       std::string tmp; // Used for determining TypeLength
                                           startPosition = metadataSection.find(AFF4Type);

                                           // Contains metadataSection substring beginning with AFF4Type 

                                           std::cout << "process_metadataFiles metadata Section ELSE for: " << filePath << "\n" << metadataSection << "\n";
                                           std::cout << "process_metadataFiles startPosition ELSE: " << startPosition << "\n";

                                           metadataSubstr = metadataSection.substr(startPosition);
                                           tmp = metadataSubstr.substr(AFF4TypeLength, metadataSubstr.find('"') - AFF4TypeLength - 1); // Length of Type with Spaces up to first "
                                           std::cout << "process_metadataFiles generic Type: " << "\n" << tmp << "\n";

                                           AFF4TypeLength += tmp.length(); // AFF4TypeLength now the actual length of unknown AFF4Type

                                           std::cout << "process_metadataFiles metadataSubstr: " << "\n" << metadataSubstr << "\n";

                                           // Contains the metadata
                                           startPosition = metadataSubstr.find(AFF4Type) + AFF4TypeLength; // Doesnt work with unknown type length
                                           
                                           if (metadataSubstr.find(";") != std::string::npos) {
                                               RDFdelimiter = metadataSubstr.find(";");
                                               std::cout << "process_metadataFiles startPosition: " << startPosition << "\n";
                                               std::cout << "process_metadataFiles RDFdelimiter: " << RDFdelimiter << "\n";
                                               std::cout << "process_metadataFiles metadataSubstr.length(): " << metadataSubstr.length() << "\n";
                                               metadata = metadataSubstr.substr(startPosition, RDFdelimiter - startPosition + 1);
                                               std::cout << "process_metadataFiles metadata.length(): " << metadata.length() << "\n";
                                           }

                                           else {
                                               RDFdelimiter = metadataSubstr.find(".");
                                               metadata = metadataSubstr.substr(startPosition, RDFdelimiter - startPosition + 1);
                                           }

                                           // Remove aff4:Type + metadata
                                           metadataSection.erase(metadataSection.find(AFF4Type), AFF4TypeLength + metadata.length());
                                           std::cout << "process_metadataFiles metadataSection after removing AFF4Type + metadata: " << "\n" << metadataSection << "\n";

                                           std::cout << "process_metadataFiles metadata: " << "\n" << metadata << "\n";

                                           // Filter metadata 

                                           metadataFiltered = metadata.substr(metadata.find('"') + 1); // Start at first "
                                           metadataFiltered = metadataFiltered.substr(0, metadataFiltered.find('"'));
                           
                                           std::cout << "process_metadataFiles metadataFiltered: " << "\n" << metadataFiltered << "\n";
                                           // Check for input type
                                           if (metadata.find("^^xsd:dateTime") != std::string::npos)
                                           {
                                               resolver.Set(testURN, AFF4_STREAM_NOTES, new XSDString(metadataFiltered), true);
                                           }
                                           /* if (metadata.find("^^xsd:int")!= std::string::npos)
                                           {
                                                resolver.Set(testURN, AFF4_STREAM_TIMESTAMP, new XSDLong(metadataFiltered), true);
                                            }
                                            if (metadata.find("^^xsd:long") != std::string::npos) {
                                               resolver.Set(testURN, AFF4_STREAM_TIMESTAMP, new XSDInteger(metadataFiltered), true); // TODO find XSDTIMEDATE conversion
                                            }*/
                                           else {
                                               resolver.Set(testURN, AFF4_STREAM_UNKNOWN, new XSDString(metadataFiltered), true);
                                           }

                                           tmp.erase(remove_if(tmp.begin(), tmp.end(), isspace), tmp.end()); // TODO include source

                                           // Saves the original unknown metadata type. 
                                           resolver.Set(testURN, AFF4_STREAM_UNKNOWN_ORIGINAL_TYPE, new XSDString("aff4:"+tmp), true);                                                                              
                                   }                                                                                               
                               
                               }
                                                                                                                            
                               // if false go to next loop and add metadata file as independent image.
                               std::cout << "process_metadataFiles Try setting custom attributes:  " << "\n";
                               
                               //resolver.Set(testURN, AFF4_STREAM_TIMESTAMP, new XSDString("d:/hidden/path/wow"), true);
                           }
                           
                           else { 
                               // File from metadataFile was not found in information.turtle
                               error = true;
                           }
                           
                           
                           // remove section from metadataFileString -> Replace with attribute remove loop 
                           startDelimiterPos = metadataFileString.find("<");
                           metadataFileString.erase(1, startDelimiterPos - 1);
                           
                           // Remove artifical delimiter
                           startDelimiterPos = metadataFileString.find("<");
                           if (warningArtificalDelimiter) {
                               metadataFileString.erase(startDelimiterPos, 1);
                               warningArtificalDelimiter = false;
                           }
                           
                           //Remove empty lines
                           metadataFileString.erase(std::unique(metadataFileString.begin(), metadataFileString.end(),
                               [](char a, char b) {return a == '\n' && b == '\n';}),
                               metadataFileString.end());
                       }
                       // Reset filePath
                       filePath = "";
                   }
               }
               else {
                   std::cout << "process_metadataFiles eof " << "\n";
                   // All metadata successfully? appended to information.turtle
                   metadataFileString.append(eof_signal); // Append to filePath? More elegant solution?
               }
           }
           
           std::cout << "process_metadataFiles read rdf attempt after serializing: \n" << metadataFileString << "\n";
        }
    }

    //TODO insert metadataString as aff4 image if invalid rdf file
    if (error) {
        // TODO insert metadataString as aff4 image if invalid rdf file
    }

    return CONTINUE;
}


AFF4Status BasicImager::handle_export() {
    if (Get("output")->isSet()) {
        resolver.logger->error(
            "Cannot specify an export and an output volume at the same time "
            "(did you mean --export_dir).");
        return INVALID_INPUT;
    }

    std::string export_dir = GetArg<TCLAP::ValueArg<std::string>>(
        "export_dir")->getValue();
    std::vector<URN> urns;
    std::string export_pattern = GetArg<TCLAP::ValueArg<std::string>>(
        "export")->getValue();

    // A pattern of @ means to read all subjects from stdin.
    if (export_pattern == "@") {
        for(std::string line;;) {
            std::getline (std::cin, line);
            if (line.size() == 0) break;
            urns.push_back(line);
            resolver.logger->info("Found image {}", line);
        }
    } else {
        // These are all acceptable stream types.
        for (const URN image_type : std::vector<URN>{
                URN(AFF4_IMAGE_TYPE),
                    URN(AFF4_MAP_TYPE)
                    }) {
            for (const URN& image: resolver.Query(AFF4_TYPE, &image_type)) {
                if (aff4::fnmatch(
                        export_pattern.c_str(),
                        image.SerializeToString().c_str()) == 0) {
                    resolver.logger->info("Found image {}", image);
                    urns.push_back(image);
                }
            }
        }
    }

    for (const URN& export_urn: urns) {
        std::vector<std::string> components{export_dir, export_urn.Domain()};
        for (auto &c: break_path_into_components(export_urn.Path())) {
            components.push_back(escape_component(c));
        }

        // Prepend the domain (AFF4 volume) to the export directory to
        // make sure the exported stream is unique.
        std::string output_filename = join(components, PATH_SEP);
        AFF4Status res = ExtractStream(
            resolver, volume_objs,
            export_urn, output_filename, /* truncate = */ true);

        if (res != STATUS_OK) {
            resolver.logger->error("Error: {}", AFF4StatusToString(res));
        }
    }

    actions_run.insert("export");
    return CONTINUE;
}


AFF4Status BasicImager::GetNextPart() {
    output_volume_part ++;

    resolver.logger->info("Switching volume for part {}", output_volume_part);

    // Free the old volume.
    current_volume.reset(nullptr);

    return STATUS_OK;
}

AFF4Status BasicImager::GetCurrentVolume(AFF4Volume **volume) {
    if (!Get("output")->isSet()) {
        return INVALID_INPUT;
    }

    if (current_volume.get() != nullptr) {
        *volume = current_volume.get();

        return STATUS_OK;
    }

    bool truncate = GetArg<TCLAP::SwitchArg>("truncate")->getValue();

    std::string output_path = GetArg<TCLAP::ValueArg<std::string>>(
        "output")->getValue();

    AFF4Flusher<AFF4Stream> output_volume_backing_stream;
    if (output_path == "" ) {
        resolver.logger->error("Must specify an output path.");
        return INVALID_INPUT;

    } else if (output_path == "-") {
        if (max_output_volume_file_size > 0) {
            resolver.logger->error(
                "Can not specify splitting volumes when redirecting to stdout!");
            return INVALID_INPUT;
        }

        // Write to stdout.
        RETURN_IF_ERROR(AFF4Stdout::NewAFF4Stdout(
                            &resolver, output_volume_backing_stream));

    } else if (output_path.at(output_path.size()-1) == '/' ||
               output_path.at(output_path.size()-1) == '\\') {

        RETURN_IF_ERROR(AFF4Directory::NewAFF4Directory(
                            &resolver, output_path,
                            truncate, current_volume));

        *volume = current_volume.get();

        return STATUS_OK;

    } else {
        std::string volume_path = output_path;

        if (output_volume_part > 0) {
            volume_path = aff4_sprintf("%s.A%02d", output_path.c_str(),
                                       output_volume_part);
        }

        resolver.logger->warn("Output file {} will be truncated.", volume_path);

        RETURN_IF_ERROR(NewFileBackedObject(
                            &resolver, volume_path, truncate ? "truncate" : "append",
                            output_volume_backing_stream));
    }

    RETURN_IF_ERROR(ZipFile::NewZipFile(
                        &resolver,
                        std::move(output_volume_backing_stream),
                        current_volume));

    *volume = current_volume.get();

    return STATUS_OK;
}

#if 0
AFF4Status BasicImager::GetOutputVolumeURN(URN* volume_urn) {
    if (output_volume_urn.value.size() > 0) {
        *volume_urn = output_volume_urn;
        return STATUS_OK;
    }

    if (!Get("output")->isSet()) {
        return INVALID_INPUT;
    }

    std::string output_path = GetArg<TCLAP::ValueArg<std::string>>(
        "output")->getValue();

    if (output_path == "-") {
        output_volume_backing_urn = URN("builtin://stdout");
    } else {
        if (output_volume_part > 0) {
            output_path = aff4_sprintf("%s.A%02d", output_path.c_str(),
                                       output_volume_part);
        }

        output_volume_backing_urn = URN::NewURNFromFilename(output_path);
    }

    // We are allowed to write on the output file.
    if (Get("truncate")->isSet()) {
        resolver.logger->warn("Output file {} will be truncated.",
                              output_volume_backing_urn);
        resolver.Set(output_volume_backing_urn,
                     AFF4_STREAM_WRITE_MODE, new XSDString("truncate"));
    } else {
        resolver.Set(output_volume_backing_urn,
                     AFF4_STREAM_WRITE_MODE, new XSDString("append"));
    }

    // The output is a directory volume.
    if (AFF4Directory::IsDirectory(output_path, /* must_exist= */ true)) {
        AFF4ScopedPtr<AFF4Directory> volume = AFF4Directory::NewAFF4Directory(
                &resolver, output_volume_backing_urn);

        if (!volume) {
            return IO_ERROR;
        }

        *volume_urn = volume->urn;
        output_volume_urn = volume->urn;

        resolver.logger->info("Creating output AFF4 Directory structure.");
        return STATUS_OK;
    }

    // The output is a ZipFile volume.
    AFF4ScopedPtr<AFF4Stream> output_stream = resolver.AFF4FactoryOpen
            <AFF4Stream>(output_volume_backing_urn);

    if (!output_stream) {
        resolver.logger->error("Failed to create output file: {}: {}",
                               output_volume_backing_urn,
                               GetLastErrorMessage());

        return IO_ERROR;
    }

    AFF4ScopedPtr<ZipFile> zip = ZipFile::NewZipFile(
        &resolver, output_stream->urn);

    if (!zip) {
        return IO_ERROR;
    }

    *volume_urn = zip->urn;
    output_volume_urn = zip->urn;

    resolver.logger->info("Creating output AFF4 ZipFile.");

    return STATUS_OK;
}

#endif

AFF4Status BasicImager::handle_compression() {
    std::string compression_setting = GetArg<TCLAP::ValueArg<std::string>>(
                                          "compression")->getValue();

    if (compression_setting == "deflate") {
        compression = AFF4_IMAGE_COMPRESSION_ENUM_DEFLATE;
    } else if (compression_setting == "zlib") {
        compression = AFF4_IMAGE_COMPRESSION_ENUM_ZLIB;
    } else if (compression_setting == "snappy") {
        compression = AFF4_IMAGE_COMPRESSION_ENUM_SNAPPY;
    } else if (compression_setting == "lz4") {
        compression = AFF4_IMAGE_COMPRESSION_ENUM_LZ4;
    } else if (compression_setting == "none") {
        compression = AFF4_IMAGE_COMPRESSION_ENUM_STORED;
    } else {
        resolver.logger->error("Unknown compression scheme {}", compression_setting);
        return INVALID_INPUT;
    }

    resolver.logger->info("Setting compression {}", compression_setting);

    return CONTINUE;
}

#ifdef _WIN32
// We only allow a wild card in the last component.
std::vector<std::string> BasicImager::GlobFilename(std::string glob) const {
    std::vector<std::string> result;

    // Devices do not glob.
    if (glob.substr(0,2) == "\\\\") {
        result.push_back(glob);
        return result;
    }

    WIN32_FIND_DATA ffd;
    const auto found = glob.find_last_of("/\\");
    std::string path = "";

    // The path before the last PATH_SEP
    if (found != std::string::npos) {
        path = glob.substr(0, found);
    }

    HANDLE hFind = FindFirstFile(glob.c_str(), &ffd);
    if (INVALID_HANDLE_VALUE != hFind) {
        do {
            // If it is not a directory, add a result.
            if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                if (path.size() > 0) {
                    result.push_back(path + PATH_SEP_STR + ffd.cFileName);
                } else {
                    result.push_back(ffd.cFileName);
                }
            }
        } while (FindNextFile(hFind, &ffd) != 0);
    }
    FindClose(hFind);

    return result;
}
#else
#include <glob.h>

std::vector<std::string> BasicImager::GlobFilename(std::string glob_expression) const {
    std::vector<std::string> result;
    glob_t glob_data;

    int res = glob(glob_expression.c_str(),
                   GLOB_MARK|GLOB_BRACE|GLOB_TILDE,
                   nullptr,  // errfunc
                   &glob_data);

    if (res == GLOB_NOSPACE) {
        return result;
    }

    for (unsigned int i = 0; i < glob_data.gl_pathc; i++) {
        result.push_back(glob_data.gl_pathv[i]);
    }

    globfree(&glob_data);

    return result;
}
#endif

void BasicImager::Abort() {
    // Tell everything to wind down.
    should_abort = true;
}

#ifdef _WIN32
BOOL sigint_handler(DWORD dwCtrlType) {
    UNUSED(dwCtrlType);
    aff4_abort_signaled = true;

    return TRUE;
}
#else
#include <signal.h>
void sigint_handler(int s) {
    UNUSED(s);
    aff4_abort_signaled = true;
}
#endif

AFF4Status BasicImager::Initialize() {
#ifdef _WIN32
    if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)sigint_handler, true)) {
        resolver.logger->error("Unable to set interrupt handler: {}",
                               GetLastErrorMessage());
    }
#else
    signal(SIGINT, sigint_handler);
#endif

    return STATUS_OK;
}

} // namespace aff4
