#include "MetadataVerification.h"
#include "MetadataSerialization.h"
#include <iostream>

bool IsValidMetadataFile(std::string path, std::string tool) {
	std::string filename = path;
	std::vector<std::vector<std::string>> file;
	
	/*// Isolate filename from path
	while (filename.find("\\")) {
		filename.erase(0, filename.find("\\"));
		std::cout << "IsValidMetadataFile(): filename: "<< filename << "\n";
	}*/

	// ".csv" file ending was not found
	if (path.find(".csv") == std::string::npos) return false;

	// Call MetadataSerialization to Open file and check if content fitting for {tool} 
	file = serializeCSV(path,tool);

	// DEBUG TODO REMOVE, prints out the first 10 segments of file
	unsigned int counter = 0;
	for (std::vector<std::string> tmp : file) {
		for (std::string tmp2 : tmp) {
			std::cout << "IsValidmetadataFile: Check serialized CSV file : "<< tmp2 << "\n";
			counter++;
			if (counter == 10) break;
		}
		if (counter == 10) break;
	}

	if (path.find("GetThis.csv") != std::string::npos) return IsValidMetadataFileGetThis(file);


	return true;
}

//Checks if serialized file includes valid metadata from GetThis tool
bool IsValidMetadataFileGetThis(std::vector<std::vector<std::string>> file) {

	std::cout << "Enter IsValidMetadataFileGetThis" << "\n";
	//std::cout << "Enter IsValidMetadataFileGetThis: file[1][0]:  "<<file[1][0]<< "\n";
	
	// Check necessary categories exist TODO return appropriate error 
	
	if (file[0][0] != "ComputerName") return false;
	if (file[0][1] != "VolumeID") return false;
	if (file[0][2] != "ParentFRN") return false;
	if (file[0][3] != "FRN") return false;
	if (file[0][4] != "FullName") return false;
	if (file[0][5] != "SampleName") return false;
	if (file[0][6] != "SizeInBytes") return false;
	if (file[0][7] != "MD5") return false;
	if (file[0][8] != "SHA1") return false;
	if (file[0][9] != "FindMatch") return false;
	if (file[0][10] != "ContentType") return false;
	if (file[0][11] != "SampleCollectionDate") return false;
	if (file[0][12] != "CreationDate") return false;
	if (file[0][13] != "LastModificationDate") return false;
	if (file[0][14] != "LastAccessDate") return false;
	if (file[0][15] != "LastAttrChangeDate") return false;
	if (file[0][16] != "FileNameCreationDate") return false;
	if (file[0][17] != "FileNameLastModificationDate") return false;
	if (file[0][18] != "FileNameLastAccessDate") return false;
	if (file[0][19] != "FileNameLastAttrModificationDate") return false;
	if (file[0][20] != "AttrType") return false;
	if (file[0][21] != "AttrName") return false;
	if (file[0][22] != "AttrID") return false;
	if (file[0][23] != "SnapshotID") return false;
	if (file[0][24] != "SHA256") return false;
	if (file[0][25] != "SSDeep") return false;
	if (file[0][26] != "TLSH") return false;
	if (file[0][27] != "YaraRules") return false;

	// TODO appropriate error treatment
	for (int i = 2; i < file.size(); i++) {
		if (file[i][4].length()==0) return false; // Missing FileName
	}
	


	std::cout << "IsValidMetadataFileGetThis returns true" << "\n";
	
	return true;

	//AttrType,AttrName,AttrID,SnapshotID,SHA256,SSDeep,TLSH,YaraRules
}

// Checks if serialized file includes valid metadata from GetSamples tool 
bool IsValidMetadataFileGetSamples(std::vector<std::vector<std::string>> file) {
	//TODO
	return true;
}

// Checks if serialized file includes valid metadata from GetSectors tool 
bool IsValidMetadataFileGetGetSectors(std::vector<std::vector<std::string>> file) {
	// TODO
	return true;
}

// Checks if serialized file includes valid metadata from RegInfo tool 
bool IsValidMetadataFileRegInfo(std::vector<std::vector<std::string>> file) {
	// TODO
	return true;
}

// Checks if serialized file includes valid metadata from FastFind tool 
bool IsValidMetadataFileFastFind(std::vector<std::vector<std::string>> file) {
	// TODO
	return true;
}

// Checks if serialized file includes valid metadata from FatInfo tool 
bool IsValidMetadataFileFatInfo(std::vector<std::vector<std::string>> file) {
	// TODO
	return true;
}

// Checks if serialized file includes valid metadata from NTFSInfo tool 
bool IsValidMetadataFileNTFSInfo(std::vector<std::vector<std::string>> file) {
	// TODO
	return true;
}

// Checks if serialized file includes valid metadata from USNInfo tool 
bool IsValidMetadataFileUSNInfo(std::vector<std::vector<std::string>> file) {
	// TODO
	return true;
}

// Checks if serialized file includes valid metadata from ObjInfo tool 
bool IsValidMetadataFileObjInfo(std::vector<std::vector<std::string>> file) {
	// TODO
	return true;
}

// Checks if serialized file includes valid metadata from UnknownTool tool 
bool IsValidMetadataFileUnknownTool(std::vector<std::vector<std::string>> file) {
	// TODO 
	return true;
}