/*MIT License

Copyright (c) [2020] [Fabian Faust]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. */

#include "MetadataValidation.h"
#include "MetadataSerialization.h"
#include "ArtifactValidation.h"
#include <iostream>
#include <windows.h>
#include <locale>
#include <codecvt>

bool IsValidMetadataFile(std::string path, std::string tool, std::vector<std::string>& artifactMetadataEntries, std::vector<std::string>& artifactEntries) {
	
	std::cout << "                 Metadata validation : Start\n";
	std::cerr << "                 Metadata validation : Start\n";
	
	std::string filename = path;
	std::vector<std::vector<std::string>> file;
	
	// ".csv" file ending was not found
	if (path.find(".csv") == std::string::npos) {
		std::cout << "                 Metadata validation : ERROR metadata file not CSV format at" <<path<<"\n";
		std::cerr << "                 Metadata validation : ERROR metadata file not CSV format at" << path << "\n";
		return false;
	}

	// Call MetadataSerialization to Open file and check if content fitting for {tool} 
	file = serializeCSV(path,tool);

	if (path.find("ArtifactModule.csv") != std::string::npos) return IsValidMetadataFileArtifactModule(file, artifactMetadataEntries,artifactEntries);

	std::cout << "                 Metadata validation : Successfully finished\n";
	std::cerr << "                 Metadata validation : Successfully finished\n";

	return true;
}

//Checks if serialized file includes valid metadata from ArtifactModule tool
bool IsValidMetadataFileArtifactModule(std::vector<std::vector<std::string>> file, std::vector<std::string>& artifactMetadataEntries, std::vector<std::string>& artifactEntries) {
	
	std::cout << "  ArtifactModule metadata validation : Start\n";
	std::cerr << "  ArtifactModule metadata validation : Start\n";

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

	for (size_t i = 2; i < file.size(); i++) {
		if (file[i][5].length() == 0) {
			// Missing SampleName
			std::cout << "  ArtifactModule metadata validation : ERROR SampleName empty\n";
			std::cerr << "  ArtifactModule metadata validation : ERROR SampleName empty\n";
			return false;
		}
	}

	// Contains the full name path for each artifact entry
	std::string artifactMetadataEntry;

	// Append all artifact entries in metadata file
	for (size_t i = 1; i < file.size(); i++) {
		artifactMetadataEntry = file[i][5];

		if ((file[i][4].find('~') != std::string::npos)
			&& (i + 1) < file.size()
			&& file[i][5] == file[i+1][5])		
		{// Skip redundant ~ entry
		}	
		else {
			// Remove quotation marks present in csv file
			artifactMetadataEntry.erase(0, 1);

			size_t index = artifactMetadataEntry.length() - 1;
			if (index >= 0)
				artifactMetadataEntry.erase(index, 1);

			artifactMetadataEntries.push_back(artifactMetadataEntry);
		}
	}

	std::string path = ORCOutputDirectoryPath;
	std::string tmpPath;

	// Find all Sample directory entries in output directory
	artifactEntries = findDirectoryEntries(path);
	std::vector<std::string> artifactEntriesRec = artifactEntries;


	// Find all sample entries and compare them with metadata entries
	for (std::string entry : artifactEntriesRec) {
		
		if (entry.find('.') == std::string::npos) {
			// If entry is directory, recursively find additional entries. Only one level required for samples	

			std::cout << "  ArtifactModule metadata validation : Validating artifact-metadata sets for sample " << entry << "\n";
			std::cerr << "  ArtifactModule metadata validation : Validating artifact-metadata sets for sample " << entry << "\n";

			tmpPath = path;
			tmpPath.append("\\");
			tmpPath.append(entry);		
			
			std::vector <std::string>tmp = findDirectoryEntries(tmpPath);
			
			artifactEntries.insert(artifactEntries.end(), tmp.begin(), tmp.end());
			
			int counterMetadataNotFound = 0;
			int counterArtifactNotFound = 0;
			
			// Check if all artifact entries from sample {entry} have metadata entries
			for (std::string sampleEntry : tmp) {
				std::string tmpEntry = entry;
				tmpEntry.append("\\");
				tmpEntry.append(sampleEntry);
				
				// Checking if artifact entry in artifactMetadataEntries
				if (std::find(artifactMetadataEntries.begin(), artifactMetadataEntries.end(), tmpEntry) == artifactMetadataEntries.end()) {
					counterMetadataNotFound++;

					std::cerr << "\n  ArtifactModule metadata validation : In Sample " << entry << " metadata-entry for artifact " << sampleEntry << " NOT found!" << "\n";					
				}
			}

			// Check if all metadata entries from sample {entry} have artifact entries
			std::string metadataEntryFilename;
			
			for (std::string metadataEntry : artifactMetadataEntries) {

				// Check if {entry} is the correct sample for metadataEntry 
				if (metadataEntry.find(entry) != std::string::npos) {
					
					// Isolate filename from sample name
					metadataEntryFilename = metadataEntry.substr(entry.length() + 1);

					if (std::find(artifactEntries.begin(), artifactEntries.end(), metadataEntryFilename) == artifactEntries.end()) {
						counterArtifactNotFound++;

						std::cerr << "  ArtifactModule metadata validation : In Sample " << entry << " artifact for metadata-entry " << metadataEntryFilename << " NOT found!" << "\n\n";	
					}
				}
			}

			if (counterArtifactNotFound > 0 || counterMetadataNotFound > 0) {

				std::cout << "  ArtifactModule metadata validation : For sample " << entry << " " << counterArtifactNotFound << " missing artifacts for existing metadata!" << "\n";
				std::cerr << "  ArtifactModule metadata validation : For sample " << entry << " " << counterArtifactNotFound << " missing artifacts for existing metadata!" << "\n";

				std::cout << "  ArtifactModule metadata validation : For sample " << entry << " " << counterMetadataNotFound << " missing metadata for existing artifacts!" << "\n";
				std::cerr << "  ArtifactModule metadata validation : For sample " << entry << " " << counterMetadataNotFound << " missing metadata for existing artifacts!" << "\n";
			}
			else {
				std::cout << "  ArtifactModule metadata validation : For sample "<< entry << " "<<tmp.size() << " artifact-metadata sets successfully validated!" << "\n";
				std::cerr << "  ArtifactModule metadata validation : For sample " << entry << " " << tmp.size() << " artifact-metadata sets successfully validated!" << "\n";

			}
		}
	}

	std::cout << "  ArtifactModule metadata validation : Successfully finished\n";
	std::cerr << "  ArtifactModule metadata validation : Successfully finished\n";
	
	return true;
}

// Checks if serialized file includes valid metadata from UnknownTool tool 
bool IsValidMetadataFileUnknownTool(std::vector<std::vector<std::string>> file) {
	// TODO for future additions to SIT 
	return true;
}

std::vector<std::string> findDirectoryEntries(std::string path) {
	std::vector<std::string> entries;
	std::string pattern(path);
	pattern.append("\\*");
	WIN32_FIND_DATA data;
	HANDLE hFind;
	if ((hFind = FindFirstFile(s2ws(pattern).c_str(), &data)) != INVALID_HANDLE_VALUE) {
		do {

			std::wstring ws(data.cFileName);

			using convert_type = std::codecvt_utf8<wchar_t>;
			std::wstring_convert<convert_type, wchar_t> converter;

			std::string tmp = converter.to_bytes(ws);

			//Filter out non-artifact entries
			if (tmp != "ArtifactModule.7z"
				&& tmp != "ArtifactModule.log"
				&& tmp != "Config.xml"
				&& tmp != "JobStatistics.csv"
				&& tmp != "ProcessStatistics.csv"
				&& tmp != "ArtifactModule.csv"
				&& tmp != "ArtifactModule.log"
				&& tmp != "."
				&& tmp != "..") {
				entries.push_back(tmp);

			}
		} while (FindNextFile(hFind, &data) != 0);
		FindClose(hFind);
	}
	return entries;
}