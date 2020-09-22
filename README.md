# SIT
**Selective Imaging Tool**

**Features:** 
1. Framework for filesystem level Selective Imaging on live Windows systems.
2. Maintain forensic soundness by targeting challenges specific for live system forensics
	1. Standalone validation check to identify unexpected or manipulated output
	2. Usage of AFF4 forensic format to facilitate direct artifact-metadata set association	
	3. Standalone verification check to verify artifact integrity
	4. Extensive live user feedback and logging to allow appropriate reaction
	5. Extensive error handling and logging to ensure reliable output 
	6. Artifact backup archive to create redundancy	
	7. Avoid writing files outside of binary directory if possible -> external flash drive usage adviced
	8. Minimize footprint, maximize efficiency
	9. Minimize usage of external libraries due to portable nature and to avoid dependency on static libraries
3. Modularity 
	1. Any module can be disabled, replaced or run separately
	2. Custom executables can be added 
	3. Minimal changes to DFIR ORC framework to maintain compatibility

**How it works** 
1. Creation of one single configured portable binary without external dependencies, based on DFIR ORC framework (https://github.com/DFIR-ORC/dfir-orc). 
2. File artifacts collected by a modified version of the GetThis DFIR ORC tool **[ArtifactModule]**. Modifications focused on extended documentation and user feedback for forensic soundness.
3. Extensive validation to detect unexpected or manipulated output. Conversion of metadata from CSV to RDF format for next step **[ValidationModule]**.
4. Storage in forensic AFF4 format with central metadata registry for each artifact **[AFF4Module]**.
5. Verification of all artifacts using SHA1 and MD5 hash codes **[VerificationModule]**. 

**How to use**
1. Follow instructions in INSTALL to compile and configure
2. Move SIT.exe into target directory! For forensical soundness use external flash drive to execute SIT from.
3. Use the command line to execute SIT (Admin rights required!)
4. A temporary directory will be created to avoid using the Windows default temp directory outside the flash drive.
5. The AFF4 image, the backup and statistic archives, as well as log files should be created in the same directory.
6. (Optional) Disable different modules by using /key= to set a specifi module, /key-= to disable a single module or archive from being executed or /key+= to add a non default module/archive to execution.
7. (Optional) Extract all artifacts to the current directory of the SIT.exe. A custom folder will be created and for every artifact the original directory and name is restored, if available in the metadata. The functionality is available in a separate optional archive. Activate with /key=ImageExtract

**DFIR ORC Documentation**
For more information on how to use DFIR ORC specific features such as the /key command: https://dfir-orc.github.io/index.html

**Metadata categories**
![](https://github.com/QuoSecGmbH/SIT/blob/master/metadataCategories.png)
Source: https://dfir-orc.github.io/GetThis.html

**Modules**

**ArtifactModule**
- Based on GetThis ORC tool
- Expanded tool documentation
- Expanded tool feedback to examiner on execution 
- Reviewed forensic soundness aspects 

**ValidationModule:** 
- Extensive validation to detect unexpected or manipulated output.
	- Checks if every artifact entry has complete and valid metadata
	- Checks if every metadata entry has corresponding and valid artifact
	- Checks if evident manipulations or missing files can be identified
- Converts the metadata from multiple csv files (one from each tool) into one rdf turtle file with AFF4 tags.

**AFF4Module:**
- Based on the c-aff4imager project (https://github.com/Velocidex/c-aff4).
- Creates an AFF4, zip based, container.
- Integrates all the artifacts into this container
- Integrates the metadata rdf turtle into the central AFF4 rdf turtle registry.

**VerificationModule:**
- Integrated into AFF4Module code for efficiency reasons
- Verifies each artifact in the AFF4 images with the hash codes collected by the ArtifactModule upon acquisition.
  



