# SIT ValidationModule 

The ValidationModule validates the artifacts and metadata collected by the ArtifactModule, as well as
converting the metadata into a rdf turtle file for the AFF4Module

It can be run both standalone and integrated into a DFIR ORC binary. 

For compilation check INSTALL

The module is built with minimal external dependencies in mind to support portability without 
excessive static library usage.

# Execution

Start the executable as DFIR ORC module or from the command line. For the parameters check  
dfir-orc-SIT-config/config/DFIR-ORC_config.xml

By default a log file called ValidationModuleLog.txt is created in the same directory. 

A temp directory, by default called tempSIT is created in the same directory to avoid usage of the Windows 
temp directory. This directory is by default removed by the VerificationModule