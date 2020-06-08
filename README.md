# SIT
**Selective Imaging Tool**

This tool will bridge the gap between the collection of artifacts and metadata on live systems and the creation of a valid partial image. This includes but is not limited to:
- Checking if artifacts were gathered as configured and necessary metadata is available
- Storage in modern forensic AFF4 format 
- Verification using hash codes

It is built to be used embedded into a DFIR ORC binary: https://github.com/DFIR-ORC/dfir-orc. 
A general understanding of the DFIR ORC structure is beneficial. 
The tool can also be run independently if so desired. 

DFIR ORC is a framework for forensic artifact acquisition on live systems. It allows of embedding already included acquisition tools and 
external custom tools into a single portable binary. The external tools need to be windows executable binaries. 

By default the DFIR ORC tools acquire artifacts, depending on their configuration, and drop them into 7z archives. 
This tool will start after these tools have finished their acquisition and complete the process to a partial image. 
This includes but is not limited to, validity checks for the metadata, creation of an AFF4 image, insertion of all the artifacts and 
metadata into this image and hash verifications.

![](https://raw.githubusercontent.com/QuoSecGmbH/SIT/master/Concept.png?token=APCTILFH7ZUFWNYFISX26HK63Y32O)

The general structure will be as follows:

There will be three modules, each of them being standalone, portable windows executables. 

**Unification module:** 
- Checks if the output of the DFIR ORC tools is conforming to the configuration.
- Checks if the metadata output is valid and if the associated artifacts exist.
- Converts the metadata from multiple csv files (one from each tool) into one rdf turtle file with AFF4 tags.

**AFF4Module:**
- Creates an AFF4, zip based, container.
- Inserts all the artifacts into this container
- Inserts the metadata rdf turtle into the central AFF4 rdf turtle registry.

**VerificationModule:**
- Verifies each artifact in the AFF4 images with the hash codes collected by the DFIR ORC tools upon acquisition.
- Checks the feedback from the previous modules and provides final feedback to the user if the partial image creation was successful
  and if the image is valid. If this is not the case, the user has the opportunity to the repeat the entire process with a different
  configuration, without having to do a manual pre analysis to determine if this necessary.
  
**Current progress:** 

Unification module [in work]

AFF4Module [basic functionality done] // missing optimization, logging and error handling
Note: The AFF4Module is based on the "c-aff4" project: https://github.com/Velocidex/c-aff4 .
      It adds several features, mainly the ability to add custom metadata in the form of rdf turtle files. The code does NOT yet have 
      the required notices for the modified code segments.

VerificationModule [TODO]

