# SIT
**Selective Imaging Tool**

**Goal:** 
Create a Framework for Selective Imaging on Live Windows systems.

**How it works** 
1. Creation of one single configured portable binary without external dependencies, based on DFIR ORC framework. 
2. File artifacts collected by a modified version of the GetThis DFIR ORC tool **[ArtifactModule]**. Modifications focused on extended documentation and user feedback for forensic soundness.
3. Collected artifacts checked for conformity to selection parameters to detect unwanted behavior and verify metadata for each artifact. Conversion of metadata from CSV to RDF format for next step **[UnificationModule]**.
4. Storage in forensic AFF4 format with central metadata registry for each artifact **[AFF4Module]**.
5. Verification of all artifacts using SHA1 and MD5 hash codes **[VerificationModule]**.

**Advantages over stock DFIR ORC**
- Improved documentation and examiner feedback to allow undelayed reaction to unexpected behavior or unsuccessful / incomplete results. 
- Appropriate storage with metadata as central concern, not afterthough.
- Extensive verification

**SIT development goals**
- Maintain forensic soundness
  - Provide extensive documentation
  - Ensure data integrity
  - Provide verification
  - Minimize footprint
- Modularity
  - Minimize modifications to DFIR ORC to ensure its modularity is maintained
  - Three separate standalone modules, to allow easy replacement, modification and removal
- Slim, easy to understand code and well commented code to support modifications

DFIR ORC repository: https://github.com/DFIR-ORC/dfir-orc. A general understanding of the DFIR ORC structure is beneficial for the configuration process. 

DFIR ORC itself is a modular framework for forensic artifact acquisition on live systems. It allows embedding of already included acquisition tools and external custom tools into a single portable binary. The external tools need to be windows executable binaries. 

The SIT modules can also be run independently if so desired, to ensure that the operation on the live system can be as minimal as possible, at the cost of dropping some features of the tool.  

![](https://github.com/QuoSecGmbH/SIT/blob/master/Concept.png)

The general structure will be as follows:

**ArtifactModule**
- Based on GetThis ORC tool
- Expanded tool documentation
- Expanded tool feedback to examiner on execution 
- Reviewed forensic soundness aspects 

**UnificationModule:** 
- Checks if the output of the DFIR ORC tools is conforming to the configuration.
- Checks if the metadata output is valid and if the associated artifacts exist.
- Converts the metadata from multiple csv files (one from each tool) into one rdf turtle file with AFF4 tags.

**AFF4Module:**
- Based on c-aff4imager
- Creates an AFF4, zip based, container.
- Inserts all the artifacts into this container
- Inserts the metadata rdf turtle into the central AFF4 rdf turtle registry.

**VerificationModule:**
- Verifies each artifact in the AFF4 images with the hash codes collected by the DFIR ORC tools upon acquisition.
- Checks the feedback from the previous modules and provides final feedback to the user if the partial image creation was successful
  and if the image is valid. If this is not the case, the user has the opportunity to the repeat the entire process with a different
  configuration, without having to do a manual pre analysis to determine if this necessary.
  
**Current progress:** 

ArtifactModule **[WIP]** 

Unification module **[WIP]** // missing optimization, logging and error handling

AFF4Module **[WIP]** // missing optimization, logging and error handling
Note: The AFF4Module is based on the "c-aff4" project: https://github.com/Velocidex/c-aff4 .
      It adds several features, mainly the ability to add custom metadata in the form of rdf turtle files. The code does NOT yet have 
      the required notices for the modified code segments.

VerificationModule **[WIP]**


