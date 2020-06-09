/*Verifies if the Metadata in CSV form is valid*/

#include <string>
#include <vector>

// Checks if the file at path is a valid csv metadata file as used by {tool} from DFIR ORC
bool IsValidMetadataFile(std::string path, std::string tool);

// Checks if serialized file includes valid metadata from GetThis tool 
bool IsValidMetadataFileGetThis(std::vector<std::vector<std::string>> file);

// Checks if serialized file includes valid metadata from GetSamples tool 
bool IsValidMetadataFileGetSamples(std::vector<std::vector<std::string>> file);

// Checks if serialized file includes valid metadata from GetSectors tool 
bool IsValidMetadataFileGetGetSectors(std::vector<std::vector<std::string>> file);

// Checks if serialized file includes valid metadata from RegInfo tool 
bool IsValidMetadataFileRegInfo(std::vector<std::vector<std::string>> file);

// Checks if serialized file includes valid metadata from FastFind tool 
bool IsValidMetadataFileFastFind(std::vector<std::vector<std::string>> file);

// Checks if serialized file includes valid metadata from FatInfo tool 
bool IsValidMetadataFileFatInfo(std::vector<std::vector<std::string>> file);

// Checks if serialized file includes valid metadata from NTFSInfo tool 
bool IsValidMetadataFileNTFSInfo(std::vector<std::vector<std::string>> file);

// Checks if serialized file includes valid metadata from USNInfo tool 
bool IsValidMetadataFileUSNInfo(std::vector<std::vector<std::string>> file);

// Checks if serialized file includes valid metadata from ObjInfo tool 
bool IsValidMetadataFileObjInfo(std::vector<std::vector<std::string>> file);

// Checks if serialized file includes valid metadata from UnknownTool tool 
bool IsValidMetadataFileUnknownTool(std::vector<std::vector<std::string>> file);