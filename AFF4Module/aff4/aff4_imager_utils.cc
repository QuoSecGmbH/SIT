/*
  Modifications and additions as part of the SIT AFF4Module https://github.com/QuoSecGmbH/SIT, by Fabian Faust
*/

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
#include <vector>
#include "verification.h"
#include <windows.h>
#include <cstdio>
#include <stdio.h>
#include <io.h>

// Source: https://stackoverflow.com/questions/23943728/case-insensitive-standard-string-comparison-in-c
bool icompare_pred(unsigned char a, unsigned char b)
{
    return std::tolower(a) == std::tolower(b);
}

// Source: https://stackoverflow.com/questions/23943728/case-insensitive-standard-string-comparison-in-c
bool icompare(std::string const& a, std::string const& b)
{
    if (a.length() == b.length()) {
        return std::equal(b.begin(), b.end(),
            a.begin(), icompare_pred);
    }
    else {
        return false;
    }
}

void convertUpperCase(std::string& str) {
    //const char* array = str.c_str();
    int counter = 0;
    for (char c : str) {
        str[counter] = std::toupper(c);
            counter++;
    }
}

namespace aff4 {


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

// TODO Extract all artifacts from image and recreate sample folders
AFF4Status ExtractAll(
    DataStore& resolver, VolumeGroup& volumes,
    URN input_urn,
    std::string filename,
    bool truncate, HANDLE hFile) {

    AFF4Flusher<AFF4Stream> in_stream;
    AFF4Flusher<FileBackedObject> file_stream;

    // Use the VolumeGroup to figure out which stream to get.
    RETURN_IF_ERROR(volumes.GetStream(input_urn, in_stream));

    resolver.logger->info("Getting stream {} to {} ({})",
        input_urn, filename, in_stream->Size());

    RETURN_IF_ERROR(NewFileBackedObject(
        &resolver, filename, truncate ? "truncate" : "append",
        file_stream));

    DefaultProgress progress(&resolver);
    progress.length = in_stream->Size();

    AFF4Status res = file_stream->WriteStream(in_stream.get(), &progress);

    return res;
}

// Get file stream from image
AFF4Status ExtractTemp(
    DataStore& resolver, VolumeGroup& volumes,
    URN input_urn,
    std::string filename,
    bool truncate, 
    std::string export_dir) {

    AFF4Flusher<AFF4Stream> in_stream;
    AFF4Flusher<FileBackedObject> file_stream;

    // Use the VolumeGroup to figure out which stream to get.
    RETURN_IF_ERROR(volumes.GetStream(input_urn, in_stream));

    resolver.logger->info("Getting stream {} to {} ({})",
        input_urn, filename, in_stream->Size());

    RETURN_IF_ERROR(NewFileBackedObject(
        &resolver,filename ,truncate ? "truncate" : "append",
        file_stream));

    DefaultProgress progress(&resolver);
    progress.length = in_stream->Size();

    AFF4Status res = file_stream->WriteStream(in_stream.get(), &progress);

    return res;
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

    if (result == CONTINUE) {
        result = parse_temp();
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

    if (result == CONTINUE && Get("hash")->isSet()) {
        result = handle_hash();
    }

    if (result == CONTINUE && temp.size() > 0) {
        result = process_temp();
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
            //std::cout << "parse_input() input file to consider for filter:  " << input << "\n";
            
            inputs.push_back(input);
        }
    }

    return CONTINUE;
}

AFF4Status BasicImager::process_input() {
    
    std::cout << "                Artifact integration : Start\n";
    std::cerr << "                Artifact integration : Start\n";
    
    genericMetafilename = "genericMETA.turtle";
    std::string inputForwardSlash; // temporary string to compare paths independently of slash changes by GlobFilename()
  
    int counter = 0;
    int counterNonArtifactFiles = 0;

    for (std::string glob : inputs) {               
        for (std::string input : GlobFilename(glob)) {
                  
            inputForwardSlash = input;
            // Replace backslashes with forward slashes
            std::replace(inputForwardSlash.begin(), inputForwardSlash.end(), '\\', '/');

            // Appends genericMetafilename to metadataFiles if found in input and not already in metadataFiles -> process_metadataFiles() will run next
            if ((inputForwardSlash.find(genericMetafilename) != std::string::npos) ) { // TODO && (std::find(metadataFiles.begin(), metadataFiles.end(), genericMetafilename) != metadataFiles.end()) == false
                metadataFiles.push_back(inputForwardSlash);
                counter--;
                // What if genericMetafilename specified by -m ?? TODO FIX !!
            }

            if (((std::find(metadataFiles.begin(), metadataFiles.end(), inputForwardSlash) != metadataFiles.end())==false) && (inputForwardSlash.find(genericMetafilename)!= std::string::npos)==false )
            {  // TODO if genericMETA.rdf without -m -> call metaHandler for this input, -> see above!
                                
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
                    std::cerr << "                Artifact integration : Added " << input << "\n";

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

                    std::cerr << "                Artifact integration : Added " << input << "\n";
                }
            }
            // Count log and config files
            if (input.find("GetThis.log") != std::string::npos
                || input.find("GetThis.csv") != std::string::npos
                || input.find("Config.xml") != std::string::npos
                || input.find("JobStatistics.csv") != std::string::npos
                || input.find("ProcessStatistics.csv") != std::string::npos
                || input.find("ArtifactModule.log") != std::string::npos) {
                counterNonArtifactFiles++;
            }
            // Count artifacts
            else counter++;
        }
    }

    actions_run.insert("input");

    std::cout << "                Artifacts integrated : "<<counter<<"\n";
    std::cerr << "                Artifacts integrated : " << counter << "\n";

    std::cout << "     Log and config files integrated : " << counterNonArtifactFiles << "\n";
    std::cerr << "     Log and config files integrated : " << counterNonArtifactFiles << "\n";

    std::cout << "                Artifact integration : Successfully finished\n";
    std::cerr << "                Artifact integration : Successfully finished\n";

    return CONTINUE;
}

AFF4Status BasicImager::parse_metadataFiles() {

    for (const auto& input : GetArg<TCLAP::MultiArgToNextFlag>("metadata")->getValue()) {
  
        std::string tmp = input;

            metadataFiles.push_back(tmp);
    }

    return CONTINUE;
}

AFF4Status BasicImager::process_metadataFiles() {

    std::cout << "                Metadata integration : Start\n";
    std::cerr << "                Metadata integration : Start\n";
    
    // Set if unsuccessful processing of metadataFile. Will cause the metadataFile to be included in image as file.
    bool error = false;

    for (std::string glob : metadataFiles) {
        
        for (std::string metadataFile : GlobFilename(glob)) {
            
            std::string metadataFileContent; 

            // Reads in metadataFile as String
            std::ifstream t(metadataFile);
            std::string metadataFileString((std::istreambuf_iterator<char>(t)),
            std::istreambuf_iterator<char>());
                                             
            // TODO REVIEW INPUT SECURITY E.G STRINGS                   

           // Serialize metadataFileString and set in information.turtle                     
           unsigned int prefixCounter = 0;
           std::string header;
           size_t position;
           
           //Check for "@id <aff4:metadata> ."
           header = "@id <aff4:metadata> .";
           position = metadataFileString.find(header);
           if (position != std::string::npos) {
               prefixCounter++;
               metadataFileString.erase(position, header.length());
           }
           //Check for "@prefix rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> ."
           header = "@prefix rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> .";
           position = metadataFileString.find(header);
           if (position != std::string::npos) {
               prefixCounter++;
               metadataFileString.erase(position, header.length());
           }
           //Check for "@prefix aff4: <http://aff4.org/Schema#> ."
           header = "@prefix aff4: <http://aff4.org/Schema#> .";
           position = metadataFileString.find(header);
           if (position != std::string::npos) {
               prefixCounter++;
               metadataFileString.erase(position, header.length());
           }
           //Check for "@prefix xsd: <http://www.w3.org/2001/XMLSchema#> ."
           header = "@prefix xsd: <http://www.w3.org/2001/XMLSchema#> .";
           position = metadataFileString.find(header);
           if (position != std::string::npos) {
               prefixCounter++;
               metadataFileString.erase(position, header.length());
           }
           
           if (prefixCounter == 4) {
               std::cout << "                      Metadata input : Valid\n";
               std::cerr << "                      Metadata input : Valid\n";
           }
           else {
               std::cout << "                      Metadata input : NOT valid!\n";
               std::cerr << "                      Metadata input : NOT valid!\n";
               // TODO reaction
           }

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
               
               // Check for <..> statement              
               startDelimiterPos = metadataFileString.find("<"); 
               
               if (startDelimiterPos != std::string::npos) {
                   endDelimiterPos = metadataFileString.find(">");
                   
                   if (endDelimiterPos != std::string::npos) {
                       
                       // Selects filePath
                       filePath = metadataFileString.substr(startDelimiterPos+1, endDelimiterPos - startDelimiterPos -1);
                       
                       // Checks if filePath empty 
                       if (filePath.length() > 0) {             

                           // Remove filePath and its Delimiters
                           metadataFileString.erase(startDelimiterPos, filePath.length()+2);

                           //Appending metadata from filePath
                           
                           // Check for next filePath delimiter as end limit for this metadata section
                           // if no startDelimiter left in file, append artifical one TODO remove
                           startDelimiterPos = metadataFileString.find(startDelimiter);
                           if (startDelimiterPos == std::string::npos) {
                               metadataFileString.append(startDelimiter);
                               warningArtificalDelimiter = true;
                           }

                           metadataSection = metadataFileString.substr(0, startDelimiterPos - 1);                                                    
                           metadataSectionRemover = metadataSection; // Copy to allow removal of substring metadataSection 

                           AFF4Volume* volume;
                           RETURN_IF_ERROR(GetCurrentVolume(&volume));

                           imageURN = volume->urn;
                           attributeURN = imageURN;
                           attributeURN.Append(filePath);
                           testURN.Set(volume->urn.Append(filePath));
                           
                           // Appends metadata if information.turtle includes the required file
                           if (resolver.HasURN(attributeURN)) {
                               
                               // TODO CHECK BEHAVIOR IF ATTRIBUTES IN WRONG ORDER IN METADATA TURTLE
                               std::vector <std::string> aff4Types = {"ComputerName",
                                   "VolumeID",
                                   "ParentFRN",
                                   "FRN",
                                   "FullName",
                                   "SampleName",
                                   "SizeInBytes",
                                   "MD5",
                                   "SHA1",
                                   "FindMatch",
                                   "ContentType",
                                   "SampleCollectionDate",
                                   "CreationDate",
                                   "LastModificationDate",
                                   "LastAccessDate",
                                   "LastAttrChangeDate",
                                   "FileNameCreationDate",
                                   "FileNameLastModificationDate",
                                   "FileNameLastAccessDate",
                                   "FileNameLastAttrModificationDate",
                                   "AttrType",
                                   "AttrName",
                                   "AttrID",
                                   "SnapshotID",
                                   "SHA256",
                                   "SSDeep",
                                   "TLSH",
                                   "YaraRules" };
                               
                                   for (std::string aff4Type : aff4Types) {
                                       size_t startPosition; // Used to store startPositions for substr() usage
                                       size_t RDFdelimiter; // Used to store either ";" or "."
                                       std::string AFF4Type; // Used to store the AFF4Type that is currently being checked 

                                       // Contains the metadataSection substring beginning with AFF4Type
                                       std::string metadataSubstr;

                                       // Contains the metadata
                                       std::string metadata;
                                       std::string metadataFiltered;

                                       AFF4Type = "aff4:";
                                       AFF4Type.append(aff4Type);
                                      
                                       if (metadataSection.find(AFF4Type) != std::string::npos) {

                                           startPosition = metadataSection.find(AFF4Type);

                                           // Contains metadataSection substring beginning with AFF4Type 

                                           metadataSubstr = metadataSection.substr(startPosition);

                                           // Contains the metadata
                                           startPosition = metadataSubstr.find(AFF4Type) + AFF4Type.length();
                                           if (metadataSubstr.find(";") != std::string::npos) {
                                               RDFdelimiter = metadataSubstr.find(";");
                                               metadata = metadataSubstr.substr(startPosition, RDFdelimiter - startPosition + 1);
                                           }

                                           else {
                                               RDFdelimiter = metadataSubstr.find(".");
                                               metadata = metadataSubstr.substr(startPosition, RDFdelimiter - startPosition + 1);
                                           }
                                           // Remove aff4:Type + metadata
                                           metadataSection.erase(metadataSection.find(AFF4Type), AFF4Type.length() + metadata.length());

                                           // Filter metadata 
                                           metadataFiltered = metadata.substr(metadata.find('"') + 1); // Start at first "
                                           metadataFiltered = metadataFiltered.substr(0, metadataFiltered.find('"'));
                                       
                                           // Removes redundant ^^xsd:string and spaces
                                           if ((metadataFiltered.find("^^xsd:string") != std::string::npos)) {
                                               std::string tmp = "\"";
                                                   // Removes redundant ^^xsd:string
                                                   metadataFiltered = metadataFiltered.substr(0, metadataFiltered.find("^^xsd:string"));
                                                   // Remove spaces
                                                   metadataFiltered.erase(remove_if(metadataFiltered.begin(), metadataFiltered.end(), isspace), metadataFiltered.end());
                                                   
                                           }
                                         
                                           // Check for input type
                                             
                                           if (aff4Type == "ComputerName") resolver.Set(testURN, AFF4_STREAM_COMPUTERNAME, new XSDString(metadataFiltered), true); 
                                           if (aff4Type == "VolumeID") resolver.Set(testURN, AFF4_STREAM_VOLUMEID, new XSDString(metadataFiltered), true); 
                                           if (aff4Type == "ParentFRN") resolver.Set(testURN, AFF4_STREAM_PARENTFRN, new XSDString(metadataFiltered), true); 
                                           if (aff4Type == "FRN") resolver.Set(testURN, AFF4_STREAM_FRN, new XSDString(metadataFiltered), true); 
                                           if (aff4Type == "FullName") resolver.Set(testURN, AFF4_STREAM_FULLNAME, new XSDString(metadataFiltered), true); 
                                           if (aff4Type == "SampleName") resolver.Set(testURN, AFF4_STREAM_SAMPLENAME, new XSDString(metadataFiltered), true); 
                                           if (aff4Type == "SizeInBytes") resolver.Set(testURN, AFF4_STREAM_SIZEINBYTES, new XSDString(metadataFiltered), true); 
                                           if (aff4Type == "MD5") resolver.Set(testURN, AFF4_STREAM_MD5, new XSDString(metadataFiltered), true); 
                                           if (aff4Type == "SHA1") resolver.Set(testURN, AFF4_STREAM_SHA1, new XSDString(metadataFiltered), true); 
                                           if (aff4Type == "FindMatch") resolver.Set(testURN, AFF4_STREAM_FINDMATCH, new XSDString(metadataFiltered), true); 
                                           if (aff4Type == "ContentType") resolver.Set(testURN, AFF4_STREAM_CONTENTTYPE, new XSDString(metadataFiltered), true); 
                                           if (aff4Type == "SampleCollectionDate") resolver.Set(testURN, AFF4_STREAM_SAMPLECOLLECTIONDATE, new XSDString(metadataFiltered), true); 
                                           if (aff4Type == "CreationDate") resolver.Set(testURN, AFF4_STREAM_CREATIONDATE, new XSDString(metadataFiltered), true); 
                                           if (aff4Type == "LastModificationDate") resolver.Set(testURN, AFF4_STREAM_LASTMODIFICATIONDATE, new XSDString(metadataFiltered), true); 
                                           if (aff4Type == "LastAccessDate") resolver.Set(testURN, AFF4_STREAM_LASTACCESSDATE, new XSDString(metadataFiltered), true); 
                                           if (aff4Type == "LastAttrChangeDate") resolver.Set(testURN, AFF4_STREAM_LASTATTRCHANGEDATE, new XSDString(metadataFiltered), true);                                            
                                           if (aff4Type == "FileNameCreationDate") resolver.Set(testURN, AFF4_STREAM_FILENAMECREATIONDATE, new XSDString(metadataFiltered), true); 
                                           if (aff4Type == "FileNameLastModificationDate") resolver.Set(testURN, AFF4_STREAM_FILENAMELASTMODIFICATIONDATE, new XSDString(metadataFiltered), true);
                                           if (aff4Type == "FileNameLastAccessDate") resolver.Set(testURN, AFF4_STREAM_FILENAMELASTACCESSDATE, new XSDString(metadataFiltered), true);
                                           if (aff4Type == "FileNameLastAttrModificationDate") resolver.Set(testURN, AFF4_STREAM_FILENAMELASTATTRMODIFICATIONDATE, new XSDString(metadataFiltered), true); 
                                           if (aff4Type == "AttrType") resolver.Set(testURN, AFF4_STREAM_ATTRTYPE, new XSDString(metadataFiltered), true); 
                                           if (aff4Type == "AttrName") resolver.Set(testURN, AFF4_STREAM_ATTRNAME, new XSDString(metadataFiltered), true); 
                                           if (aff4Type == "AttrID") resolver.Set(testURN, AFF4_STREAM_ATTRID, new XSDString(metadataFiltered), true); 
                                           if (aff4Type == "SnapshotID") resolver.Set(testURN, AFF4_STREAM_SNAPSHOTID, new XSDString(metadataFiltered), true); 
                                           if (aff4Type == "SHA256") resolver.Set(testURN, AFF4_STREAM_SHA256, new XSDString(metadataFiltered), true); 
                                           if (aff4Type == "SSDeep") resolver.Set(testURN, AFF4_STREAM_SSDEEP, new XSDString(metadataFiltered), true); 
                                           if (aff4Type == "TLSH") resolver.Set(testURN, AFF4_STREAM_TLSH, new XSDString(metadataFiltered), true); 
                                           if (aff4Type == "YaraRules") resolver.Set(testURN, AFF4_STREAM_YARARULES, new XSDString(metadataFiltered), true); 
                                           //else resolver.Set(testURN, AFF4_STREAM_UNKNOWN, new XSDString(metadataFiltered), true); 
                                       }
                               }                                                                        
                           }                                                                                                                           
                           
                           else { 
                               // File from metadataFile was not found in information.turtle
                               std::cout << "                Metadata integration : Metadata entry "<< attributeURN.SerializeToString().c_str() <<" NOT found in image!"<<"\n";
                               std::cerr << "                Metadata integration : Metadata entry " << attributeURN.SerializeToString().c_str() << " NOT found in image!" << "\n";
              
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
                   // All metadata successfully? appended to information.turtle
                   metadataFileString.append(eof_signal); // Append to filePath? More elegant solution?
               }
           }
        }
    }

    if (error) {
        // TODO insert metadataString as aff4 image if invalid rdf file
    }

    std::cout << "                Metadata integration : Successfully finished\n";
    std::cerr << "                Metadata integration : Successfully finished\n";

    return CONTINUE;
}

AFF4Status BasicImager::handle_hash() {
    
    std::cout << "                   Hash verification : Start\n";
    std::cerr << "                   Hash verification : Start\n";

    HANDLE hFile = NULL;

    std::vector<URN> urns;

    char cwd[PATH_MAX];
    if (getcwd(cwd, PATH_MAX) == NULL) {
        return IO_ERROR;
    }

    std::string export_dir = cwd;

    // Replace relative path with temp directory if specified
    if (temp.size() > 0) export_dir = temp;

    // These are all acceptable stream types.
    for (const URN image_type : std::vector<URN>{
            URN(AFF4_IMAGE_TYPE),
                URN(AFF4_MAP_TYPE)
        }) {
        for (const URN& image : resolver.Query(AFF4_TYPE, &image_type)) {
            resolver.logger->info("Found image {}", image);
            
            std::string urn_stringImage = image.SerializeToString().c_str();
            // Skip log and config files
            if (urn_stringImage.find("GetThis.log") != std::string::npos
                || urn_stringImage.find("GetThis.csv") != std::string::npos
                || urn_stringImage.find("Config.xml") != std::string::npos
                || urn_stringImage.find("JobStatistics.csv") != std::string::npos
                || urn_stringImage.find("ProcessStatistics.csv") != std::string::npos
                || urn_stringImage.find("ArtifactModule.log") != std::string::npos) {
                continue;
            }
            
            urns.push_back(image);
        }
    }

    // Counter for how many artifacts were successfully verified
    int counter = 0;
    // Successful MD5 verifications
    int counterMD5 = 0;
    // Successful SHA1 verifications
    int counterSHA1 = 0;
    
    // String for calculated MD5 code
    std::string MD5 = "";
    // String for calculated SHA1 code
    std::string SHA1 = "";
    // Set if verification not successful
    bool failedVerificationNotifier = false;
    // Counts number of failed verifications
    int failedVerifications = 0;

    std::cout << "                     Artifacts found : "<<urns.size()<<"\n";
    std::cerr << "                     Artifacts found : " << urns.size() << "\n";

    // Iterates over each valid urn
    for (const URN& export_urn : urns) {
        std::vector<std::string> components{ export_dir, export_urn.Domain() };
        for (auto& c : break_path_into_components(export_urn.Path())) {
            components.push_back(escape_component(c));
        }

        // Prepend the domain (AFF4 volume) to the export directory to
        // make sure the exported stream is unique.
        std::string output_filename = join(components, PATH_SEP);
        std::string urn_string = export_urn.SerializeToString().c_str();
       
        // Extracts files to temp directory to calculate hash codes
        AFF4Status res = ExtractTemp(
            resolver, volume_objs,
            export_urn, output_filename, /* truncate = */ true, export_dir);

        // TODO into log
        if (res != STATUS_OK) {
            resolver.logger->error("Error: {}", AFF4StatusToString(res));
        }
        
        // Calculate MD5 code for file
        if (calculateMD5(output_filename.c_str(),MD5) == 0) counterMD5++;
        else { 
            std::cerr << "               MD5 hash verification : For file "<<urn_string<<" NOT successfully calculated!"<<"\n";
            failedVerificationNotifier = true;
        }
        
        // String for MD5 code from metadata 
        std::string MD5MetaString = "";
        
        // Acquire MD5 code from metadata registry
        MD5MetaString = resolver.GetAttributeValue(export_urn, AFF4_STREAM_MD5);
        
        // Convert 
        convertUpperCase(MD5);
        
        if (MD5MetaString == "NOT_FOUND") {
            std::cerr << "               MD5 hash verification : For file " << urn_string << " hash code NOT found in metadata!" << "\n";
            failedVerificationNotifier = true;
        }       
        else if (MD5MetaString.find(MD5) != std::string::npos) {
            failedVerificationNotifier = failedVerificationNotifier; // TODO REPLACE
        }
        else {
            std::cerr << "               MD5 hash verification : For file " << urn_string << " hash codes NOT equal!" << "\n";
            failedVerificationNotifier = true;
        }

        // Calculate SHA1 code for file
        if (calculateSHA1(output_filename.c_str(),SHA1) == 0) counterSHA1++;
        else {
            std::cerr << "              SHA1 hash verification : For file " << urn_string << " NOT successfully calculated!" << "\n";
            failedVerificationNotifier = true;
        }

        // TODO error treatment

        // String for MD5 code from metadata 
        std::string SHA1MetaString = "";

        // Acquire MD5 code from metadata registry
        SHA1MetaString = resolver.GetAttributeValue(export_urn, AFF4_STREAM_SHA1);

        // Convert 
        convertUpperCase(SHA1);

        if (SHA1MetaString == "NOT_FOUND") {
            std::cerr << "              SHA1 hash verification : For file " << urn_string << " hash code NOT found in metadata!" << "\n";
            failedVerificationNotifier = true;
        }
        else if (SHA1MetaString.find(SHA1) != std::string::npos) {
            failedVerificationNotifier = failedVerificationNotifier; // TODO REPLACE
        }
        else {
            std::cerr << "              SHA1 hash verification : For file " << urn_string << " hash codes NOT equal!" << "\n";
            failedVerificationNotifier = true;
        }

        // TODO error treatment
        if (std::remove(output_filename.c_str()) != 0) failedVerificationNotifier = failedVerificationNotifier; // TODO REPLACE

        if (failedVerificationNotifier) failedVerifications++;

        failedVerificationNotifier = false;
        counter++;

        MD5 = "";
        SHA1 = "";
    }

    // TODO exclude log and csv files
    std::cout << "                   Artifacts checked : "<<counter<<"\n";
    std::cerr << "                   Artifacts checked : " << counter << "\n";

    std::cout << "     Artifacts successfully verified : "<<(counter-failedVerifications)<<"\n";
    std::cerr << "     Artifacts successfully verified : " << (counter - failedVerifications) << "\n";

    std::cout << " Artifacts NOT successfully verified : "<<failedVerifications<<"\n";
    std::cerr << " Artifacts NOT successfully verified : " << failedVerifications << "\n";

    std::cout << "                   Hash verification : Successfully finished\n";
    std::cerr << "                   Hash verification : Successfully finished\n";

    return CONTINUE;
}

// Export all artifacts in container TODO
AFF4Status BasicImager::handle_exportAll() {

    HANDLE hFile = NULL;

    std::vector<URN> urns;

    char cwd[_MAX_PATH];
    if (_getcwd(cwd, _MAX_PATH) == NULL) {
        // TODO ERROR HANDLING
    }
    std::string export_dir = cwd;

    // These are all acceptable stream types.
    for (const URN image_type : std::vector<URN>{
            URN(AFF4_IMAGE_TYPE),
                URN(AFF4_MAP_TYPE)
        }) {
        for (const URN& image : resolver.Query(AFF4_TYPE, &image_type)) {
            resolver.logger->info("Found image {}", image);
            urns.push_back(image);
        }
    }

    for (const URN& export_urn : urns) {
        std::vector<std::string> components{ export_dir, export_urn.Domain() };
        for (auto& c : break_path_into_components(export_urn.Path())) {
            components.push_back(escape_component(c));
        }

        // Prepend the domain (AFF4 volume) to the export directory to
        // make sure the exported stream is unique.
        std::string output_filename = join(components, PATH_SEP);
        std::string urn_string = export_urn.SerializeToString().c_str();
        
        AFF4Status res = ExtractAll(
            resolver, volume_objs,
            export_urn, output_filename, /* truncate = */ true, hFile);

        if (res != STATUS_OK) {
            resolver.logger->error("Error: {}", AFF4StatusToString(res));
        }
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

AFF4Status BasicImager::parse_temp() {
    for (const auto& input : GetArg<TCLAP::MultiArgToNextFlag>("temp")->getValue()) {
            temp.append(input);       
    }

    // For recursive removal of non-empty folders in process_temp()
    tempRecursive = temp;
    return CONTINUE;
}

AFF4Status BasicImager::process_temp() {

    WIN32_FIND_DATA FindFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    DWORD dwError;

    std::string FilePath = tempRecursive;
    std::string tmp = "\\*.*";
    FilePath.append(tmp);   
    
    // Remove all files and subfolders from temp directory
    hFind = FindFirstFile(FilePath.c_str(), &FindFileData);

    //printf("First file name is %s\n", FindFileData.cFileName);
    while (FindNextFile(hFind, &FindFileData) != 0)
    {
        std::string FileNameString = tempRecursive;
        FileNameString.append("\\");
        FileNameString.append(FindFileData.cFileName);

        // TODO FILTER OUT ".." and "." at last position in FileNameString
        if (FileNameString.find('.') != FileNameString.size() - 1 && FileNameString.find('.') != FileNameString.size() - 2 && FileNameString.find("..") != FileNameString.size() - 2) {
            if (remove(FileNameString.c_str()) != 0) {

                if (RemoveDirectory(FileNameString.c_str()) == 0) {
                    
                    // Replace current tempRecursive with next Directory if not already equal and if not file
                    if (tempRecursive.compare(FileNameString) != 0 && tempRecursive.find('.') == std::string::npos ) {
                        tmp = tempRecursive;
                        tempRecursive = FileNameString;
                        
                        process_temp();
                      
                        // Reset tempRecursive
                        tempRecursive = tmp;
                    }
                    if (RemoveDirectory(FileNameString.c_str()) == 0) {}//std::cout << "process_temp(): Could NOT remove file at " << FileNameString << "\n";

                }
                else {
                    // Deleted file at  {FileNameString} 
                }
            }
            else {
                // Deleted file at  {FileNameString} 
            }
        }
    }

    dwError = GetLastError();
    FindClose(hFind);
    
    if (dwError != ERROR_NO_MORE_FILES)
    {
        //printf("FindNextFile error. Error is %u\n", dwError);
        //return (-1);
        std::cerr << "                      Temp directory : NOT successfully emptied " << dwError << "\n";
        std::cout << "                      Temp directory : NOT successfully emptied " << dwError << "\n";
    }

    if ((tempRecursive == tmp) && RemoveDirectory(temp.c_str()) == 0) {
        std::cerr << "                      Temp directory : NOT successfully removed "<<GetLastError()<<"\n";
        std::cout << "                      Temp directory : NOT successfully removed " << GetLastError() << "\n";

    }
    
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

        // TODO fix
        //resolver.logger->warn("Output file {} will be truncated.", volume_path);

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
