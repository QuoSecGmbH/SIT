/*
Copyright 2015 Google Inc. All rights reserved.

Licensed under the Apache License, Version 2.0 (the "License"); you may not use
this file except in compliance with the License.  You may obtain a copy of the
License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied.  See the License for the
specific language governing permissions and limitations under the License.
*/

/*
  Modifications and additions as part of the SIT AFF4Module https://github.com/QuoSecGmbH/SIT, by Fabian Faust
*/

/*
  Utilities for AFF4 imaging. These are mostly high level utilities used by the
  command line imager.
*/
#ifndef SRC_AFF4_IMAGER_UTILS_H
#define SRC_AFF4_IMAGER_UTILS_H

#include "aff4/config.h"

#include "aff4/aff4_errors.h"
#include "aff4/data_store.h"
#include "aff4/libaff4.h"
#include "aff4/rdf.h"

#include "aff4/tclap_parsers.h"

// Supports all integer inputs given as hex.
#define TCLAP_SETBASE_ZERO 1
#include <tclap/CmdLine.h>
#include <set>
#include <list>

namespace aff4 {

class VolumeManager;


class BasicImager {
    friend VolumeManager;

 public:
    // Must be at the top to make sure it is destroyed last.
    MemoryDataStore resolver;

 protected:
    // The current volume we are writing on.
    AFF4Flusher<AFF4Volume> current_volume;

    // Volumes we currently have open,
    VolumeGroup volume_objs;

    // For multipart volume.
    int output_volume_part = 0;

    // Maximum size of output volume.
    size_t max_output_volume_file_size = 0;

    // Type of compression we should use.
    AFF4_IMAGE_COMPRESSION_ENUM compression = AFF4_IMAGE_COMPRESSION_ENUM_ZLIB;

    /**
     * When this is set the imager will try to abort as soon as possible.
     *
     * Any streams it is currently writing will be closed and flushed. Any volumes
     * will be finalized. The imager will then return from the Run() method with
     * an AFF4Status of ABORT.
     */
    bool should_abort = false;

    // As actions are executed they are added to this.
    std::set<std::string> actions_run;

    // The list of metadata files we copy (from the --metadata flag).
    std::vector<std::string> metadataFiles;

    // The list of input streams we copy (from the --inputs flag).
    std::vector<std::string> inputs;

    // The generic filename that is ignored by --input as metadatafile;
    std::string genericMetafilename;

    // The path to the temp folder (from the --temp flag).
    std::string temp;

    // The path to the temp folder (from the --temp flag), for recursive removal of non-empty folders in process_temp().
    std::string tempRecursive;

    // The name for the folder to create for the exported files.
    std::string export_dir_name;

    virtual std::string GetName() const {
        return "AFF4 Imager";
    }

    virtual std::string GetVersion() const {
        return AFF4_VERSION;
    }

    // Returns a reference to the current volume. Not owned.
    AFF4Status GetCurrentVolume(AFF4Volume **volume);
    // Switch to the next volume.
    AFF4Status GetNextPart();

    std::unique_ptr<TCLAP::CmdLine> cmd;

    virtual AFF4Status handle_logging();
    virtual AFF4Status handle_aff4_volumes();
    virtual AFF4Status handle_view();
    virtual AFF4Status handle_list();
    virtual AFF4Status parse_input();
    virtual AFF4Status process_input();
    virtual AFF4Status parse_metadataFiles();
    virtual AFF4Status process_metadataFiles();
    virtual AFF4Status handle_hash();
    virtual AFF4Status handle_exportAll();
    virtual AFF4Status handle_export();
    virtual AFF4Status parse_temp();
    virtual AFF4Status process_temp();
    virtual AFF4Status handle_compression();

    /**
     * This method should be called by imager programs to parse the command line
     * and set internal state. The method should not actually do anything other
     * than interpret and copy the command line. This method could be called
     * multiple times! If you extend this class, you must also call this method of
     * the base class - especially if you also need to check flags parsed by the
     * base class.
     *
     * @param argc
     * @param argv
     *
     * @return STATUS_OK if it is ok to continue processing. Otherwise the program
     * should not call the Run() method.
     */
    virtual AFF4Status ParseArgs();

    /**
     * This method is called after the args are parsed. This is where we actually
     * process the args and do things.
     *
     *
     * @return STATUS_OK if the method actually ran something, CONTINUE if there
     * was nothing to do.
     */
    virtual AFF4Status ProcessArgs();

    // We use a list here to preserve insertion order.
    std::list<std::unique_ptr<TCLAP::Arg>> args;

    void AddArg(TCLAP::Arg* arg) {
        args.push_back(std::unique_ptr<TCLAP::Arg>(arg));
    }

    TCLAP::Arg* Get(std::string name) {
        for (auto it = args.begin(); it != args.end(); it++) {
            if ((*it)->getName() == name) {
                return it->get();
            }
        }

        resolver.logger->critical("Parameter {} not known", name);
        abort();
    }

    template<typename T>
    T* GetArg(std::string name) {
        return dynamic_cast<T*>(Get(name));
    }

    /**
     * Expands the glob into a list of filenames that would match. This
     * functionality is especially required on windows where users have no shell
     * expansion.
     *
     * @param glob
     *
     * @return expanded filenames.
     */
    std::vector<std::string> GlobFilename(std::string glob) const;

  public:
    /**
     * This should be overloaded for imagers that need to do something before they
     * start. The method is called during the imager's initialization routine.
     *
     * By default we initialize signal handler.
     *
     * @return If this returns anything other that STATUS_OK we abort.
     */
    virtual AFF4Status Initialize();

    virtual AFF4Status RegisterArgs() {
        AddArg(new TCLAP::SwitchArg("V", "view", "View AFF4 metadata", false));
        AddArg(new TCLAP::SwitchArg("l", "list", "List all image streams in the volume.", false));
        AddArg(new TCLAP::MultiSwitchArg(
                   "d", "debug", "Display debugging logging (repeat for more info)",
                   false));

        AddArg(new TCLAP::SwitchArg(
                   "v", "verbose", "Display more verbose information", false));

        AddArg(new TCLAP::SwitchArg(
                   "t", "truncate", "Truncate the output file. Normally volumes and "
                   "images are appended to existing files, but this flag forces the "
                   "output file to be truncated first.",
                   false));

        AddArg(new TCLAP::MultiArgToNextFlag(
                   "i", "input", "File to image. If specified we copy these file to the "
                   "output volume located at --output. If there is no AFF4 volume on "
                   "--output yet, we create a new volume on it. A filename of @ means "
                   "to read filenames from stdin (In this case no glob expansion will "
                   "occur). \n"
                   "This can be specified multiple times with shell expansion. e.g.:\n"
                   "-i /bin/*",
                   false, "/path/to/file/or/device"));
        
        AddArg(new TCLAP::MultiArgToNextFlag(   // CUSTOM metadata file argument
                    "m", "metadata", "Metadata file to process. If specified this file will be "
                    "analyzed for RDF metadata sets associated with existing images. The filename of "
                    "these images needs to be included with the metadata to ensure correct "
                    "identification. Multiple independent metadata sets may be specified in one file."
                    "If a set of metadata cannot be connected to a corresponding file, this set will " 
                    "be added as independent metadata to information.turtle. The RDF metadata file "
                    " will be processed last if paired with -i.",
                    false, "/path/to/file/or/device"));

        AddArg(new TCLAP::SwitchArg(   // CUSTOM hash verification
                    "", "hash", "Verification using hash codes. If specified all stored files "
                    "with hash codes in their metadata will have MD5 and SHA1 codes created and compared "
                    "to the existing ones. The codes will be created after the files have been stored in the "
                    "container, to ensure meaningfulness in regards to data integrity. "
                    "Missing hash codes in the metadata or negative results for the comparison will be reported. ",
                    false));

        AddArg(new TCLAP::MultiArgToNextFlag(   // CUSTOM temp folder argument
            "", "temp", "The folder at specified path will be removed after all reading "
            "and writing operations to this directory are done. Useful for removing custom temp "
            "folders from external tools. If used in conjunction with hash, the folder at the "
            "specified path will be used as temporary folder or created there if necessary. ",
            false, "/path/to/folder"));

        AddArg(new TCLAP::SwitchArg(
                   "", "relative", "If specified all files will be written relative to "
                   "the current directory. By default we write all files' absolute paths",
                   false));

        AddArg(new TCLAP::ValueArg<std::string>(
                   "e", "export", "Name of the streams to export. If specified we try "
                   "to open this stream and write it to the export directory. Note that "
                   "you will also need to specify an AFF4 volume path to load so we know "
                   "where to find the stream. Specifying a relative URN "
                   "implies a stream residing in a loaded volume. E.g.\n"

                   " -e /dev/sda -D /tmp/ my_volume.aff4",
                   false, "", "string"));

        AddArg(new TCLAP::ValueArg<std::string>( // CUSTOM exportAll argument
            "", "exportAll", "Exports all files in the image to the same directory "
            "and into the folder specified. If export_dir is specified, the files "
            " will be extracted into this directory. If original file paths are stored "
            " as metadata the original directory structure will be replicated. "
            " --exportAll /PartialImageExport -D /tmp/ my_volume.aff4",
            false,"", "string"));
        
        AddArg(new TCLAP::ValueArg<std::string>(
                   "", "logfile", "Specify a file to store log messages to",
                   false, "", "string"));

        AddArg(new TCLAP::ValueArg<std::string>(
                   "D", "export_dir", "Directory to export to (Defaults to current "
                   "directory)",
                   false, ".", "path to directory"));

        AddArg(new TCLAP::ValueArg<std::string>(
                   "o", "output", "Output file to write to. If the file does not "
                   "exist we create it. NOTE: If output is a directory we write an "
                   "AFF4 Directory volume when imaging. If the output is '-' we write "
                   "to stdout.", false, "",
                   "/path/to/file"));

        AddArg(new TCLAP::SizeArg(
                   "s", "split", "Split output volumes at this size.", false, 0,
                   "Size (E.g. 100Mb)"));

        AddArg(new TCLAP::ValueArg<std::string>(
                   "c", "compression", "Type of compression to use (default deflate).",
                   false, "", "deflate, zlib, snappy, lz4, none"));

        AddArg(new TCLAP::ValueArg<int>(
                   "", "threads", "Total number of threads to use.",
                   false, 2, "(default 2)"));

        AddArg(new TCLAP::UnlabeledMultiArg<std::string>(
                   "aff4_volumes",
                   "These AFF4 Volumes will be loaded and their metadata will "
                   "be parsed before the program runs.\n"
                   "Note that this is necessary before you can extract streams with the "
                   "--export or --exportAll flag.",
                   false, "/path/to/aff4/volume"));

        return STATUS_OK;
    }

    virtual AFF4Status Run(int argc, char** argv);

    virtual void Abort();

    BasicImager() : volume_objs(&resolver) {}
    virtual ~BasicImager() {}
};

class VolumeManager : public DefaultProgress {

public:
    VolumeManager(DataStore *resolver, BasicImager *imager) :
    DefaultProgress(resolver), imager(imager) {}

    bool Report(aff4_off_t readptr) override;
    void ManageStream(AFF4Stream *stream);
    bool MaybeSwitchVolumes();

protected:
    // Not owned.
    BasicImager *imager = nullptr;
    std::vector<AFF4Stream *> streams;
};


} // namespace aff4

#endif  // SRC_AFF4_IMAGER_UTILS_H_
