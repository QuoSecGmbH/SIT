# SIT
**Selective Imaging Tool**

This tool will bridge the gap between the collection of artifacts and metadata on live systems by the DFIR ORC framework, and the creation of a valid partial images as a Selective Imaging Framework. 

**Main Goals for creation of a Selective Imaging Framework**
- Ability to gather forensic artifacts and metadata while maintaining forensic soundness [DFIR ORC]
- Validity check to verify if the artifacts have been collected as configured and if the required metadata is available [SIT]
- Storage in modern forensic container with central metadata registry [SIT]
- Verification of all artifacts [SIT]

**Main SIT Goals**
- Maintain forensic soundness
  - Provide extensive documentation
  - Ensure data integrity
  - Provide verification
  - Minimize footprint
- Modularity
  - Minimize modifications to DFIR ORC to ensure its modularity is maintained
  - Three separate standalone modules, to allow easy replacement, modification and removal
- Slim, easy to understand code and well commented code to support modifications

The resulting framework will be a single portable DFIR ORC binary without dependencies. DFIR ORC repository: https://github.com/DFIR-ORC/dfir-orc. A general understanding of the DFIR ORC structure is beneficial for the configuration process. 

SIT can also be run independently if so desired, to ensure that the operation on the live system can be as minimal as possible, at the cost of dropping some features of the tool.  

DFIR ORC itself is a modular framework for forensic artifact acquisition on live systems. It allows embedding of already included acquisition tools and external custom tools into a single portable binary. The external tools need to be windows executable binaries. 

By default the DFIR ORC tools acquire artifacts, depending on their configuration, and drop them into 7z archives. 
This tool will start after these tools have finished their acquisition and complete the process to a partial image. 
This includes but is not limited to, validity checks for the metadata, creation of an AFF4 image, insertion of all the artifacts and 
metadata into this image and hash verifications.

![](https://github.com/QuoSecGmbH/SIT/blob/master/Concept.png)

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


