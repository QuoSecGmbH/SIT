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

#include "ArtifactVerification.h"
#include <windows.h>
#include <iostream>
#include "Directory.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "MetadataVerification.h"


std::string ORCOutputDirectoryPath;

/*Oversees all tool output verifications*/
std::vector<std::vector<std::string>> MainVerification(std::vector<std::string>ToolsToVerify, std::vector<std::vector<std::string>>InputForVerification) {
	
	// One std::vector<std::string> for each Tool to check -> If empty except for Tool name then no failure 
	std::vector<std::vector<std::string>>FailedVerifications; // Includes all tools with failed output verification

	// Holds Tool name and function output 
	std::vector<std::string> buffer;
	std::vector<std::string> functionBuffer;

	ORCOutputDirectoryPath= GetORCOutputDirectory();
	std::cout << "ORCOutputDirectoryPath: " << ORCOutputDirectoryPath << "\n";

	if (!(CheckIfDirectoryExists(ORCOutputDirectoryPath))) return FailedVerifications; // TODO return error

	unsigned int counter = 0;
	for (std::string Tool : ToolsToVerify) {
		if (Tool == "GetThis") {
			// Holds the resulting <std::vector<std::string> from {Tool} output verification
			functionBuffer = GetThisVerification(InputForVerification[counter]);
			
			//{Tool} string signals beginning of {Tool} section
			buffer.push_back(Tool);
			//buffer.append(GetThisVerification(InputForVerification[counter]));
			
			// Insert the <std::vector<std::string> result with all the failed verifications for this Tool
			buffer.insert(buffer.end(), functionBuffer.begin(), functionBuffer.end());

			// Insert EOF string to signal the end of this {Tool}'s section
			buffer.push_back("<EOS>");
			
			// Insert the buffer into FailedVerifications for output
			FailedVerifications.push_back(buffer);
		}
		/*if (Tool == "GetSamples") {
			if (!(GetSamplesVerification(InputForVerification[counter]))) FailedVerifications.push_back(Tool);
		}
		if (Tool == "GetSectors") {
			if (!(GetSectorsVerification(InputForVerification[counter]))) FailedVerifications.push_back(Tool);
		}
		if (Tool == "RegInfo") {
			if (!(RegInfoVerification(InputForVerification[counter]))) FailedVerifications.push_back(Tool);
		}
		if (Tool == "FastFind") {
			if (!(FastFindVerification(InputForVerification[counter]))) FailedVerifications.push_back(Tool);
		}
		if (Tool == "FatInfo") {
			if (!(FatInfoVerification(InputForVerification[counter]))) FailedVerifications.push_back(Tool);
		}
		if (Tool == "NTFSInfo") {
			if (!(NTFSInfoVerification(InputForVerification[counter]))) FailedVerifications.push_back(Tool);
		}
		if (Tool == "USNInfo") {
			if (!(USNInfoVerification(InputForVerification[counter]))) FailedVerifications.push_back(Tool);
		}
		if (Tool == "ObjInfo") {
			if (!(ObjInfoVerification(InputForVerification[counter]))) FailedVerifications.push_back(Tool);
		}
		if (Tool == "UnknownTool") {
			if (!(UnknownToolVerification(InputForVerification[counter]))) FailedVerifications.push_back(Tool);
		}*/
		counter++;
	}
	




	/* Returns the list of failed Verifications */
	return FailedVerifications;
}

/* Tools that output files and metadata */
std::vector<std::string> GetThisVerification(std::vector<std::string> InputForVerification) {
	
	/* Holds the metdatafile name with identifiers (<{metadatafile}.csv>) if not found, without identifiers {metadatafile}.csv if found, as well as the name of each sample 
		Example output: "GetThis.csv""sample1""<sample2>" 
		Metadata file was found and valid, sample1 directory was found, sample 2 directory was not found
	*/
	std::string metadataName = "GetThis.csv";
	std::vector<std::string> output;  
	
	// Path to the current sample from InputForVerification
	std::string samplePath;
	
	//Used for constructing samplePath
	std::string tmpPath;
	
	// Contains the Output from CheckIfFilesExist
	std::vector<std::string> tmpFile;
	
	// Files that could not be found
	//std::vector<std::string> unlocatedFiles; 

	// TODO REMOVE ONLY DEBUG
	//std::vector<std::string> tmp;

	//Used to construct output
	//std::vector<std::string> CheckOutput;

	// Check if metadataName file exists and if it contains valid metadata
	tmpFile.push_back(metadataName);
	// CheckIfFilesExist method returns all the files that do exist from the input
	tmpFile = CheckIfFilesExist(ORCOutputDirectoryPath, tmpFile);
	

	if (std::find(tmpFile.begin(),tmpFile.end(),metadataName)!= tmpFile.end()) {
		// Identifier if metadataName file was not found
		output.push_back("<" + metadataName + ">");
		std::cout << "metadataName file " << metadataName << " was NOT found in GetThisVerification with Path: " << ORCOutputDirectoryPath << "\n";
	}
	else {
		// Used only for IsValidMetadataFile input
		std::string tmp = ORCOutputDirectoryPath;
		tmp.append("\\" + metadataName);
		
		if (!(IsValidMetadataFile(tmp,"GetThis"))) {
			// Identifier if metadataName file was found but not valid
					// TODO CHECK IF VALID METADATA
			output.push_back("<<" + metadataName + ">>");
			std::cout << "metadataName file " << metadataName << " was found BUT NOT VALID in GetThisVerification with Path : " << ORCOutputDirectoryPath << "\n";
		}

		else {
			// metadataName added if file was found and valid
			output.push_back(metadataName);
			std::cout << "metadataName file " << metadataName << " was found in GetThisVerification with Path : " << ORCOutputDirectoryPath << "\n";

		}
	}

	// TODO check if files from metadata exist MAYBE

	// Check if all the samples from InputForVerification are available as folders
	for (std::string sample : InputForVerification) {

		tmpPath="\\"; // Represents "\"
		tmpPath.append(sample);
		samplePath = ORCOutputDirectoryPath;
		samplePath.append(tmpPath);
		std::cout << "sample in GetThisVerification loop: " << sample << " samplePath: " << samplePath << "\n";
		
		// Check if folder with name {sample} available
		if (CheckIfDirectoryExists(samplePath)) { 
			//samplePath.append("\\");
			//samplePath.append("test.txt,test2.txt,test3.txt")

			/*// Only for testing TODO REMOVE
			tmp.push_back("test.txt");
			tmp.push_back("test2.txt");
			tmp.push_back("test3.txt");*/
			
			//CheckOutput = CheckIfFilesExist(samplePath, tmp);
			
			// Signals beginning of {sample} section
			//output.push_back("<BOS>");

			// Insert {sample} to identify the sample from which the files might be missing
			output.push_back(sample);
			
			std::cout << "sample directory in GetThisVerification loop FOUND: " << sample << "\n";

			// Insert the missing files into output
			//output.insert(output.end(), CheckOutput.begin(), CheckOutput.end());

			// Signals end of this {sample}'s section
			//output.push_back("<EOS>");
				
			/*if (true) { // Check if files in folder {ORCOutputFolder} match {ntfs_find path match} pattern
				//CheckIfDirectoryExists(ORCOutputDirectoryPath);
				 // TODO remove custom debug string
				std::cout << "First file  that could be found :" << unlocatedFiles[0] << "\n"; // TODO remove
			}
			else {
				// Files in folder {ORCOutputFolder} don't match {ntfs_find path match} pattern
				return output;
			}*/
		}
		
		// Sample directory doesnt exist
		else {
			
			
			// Signals beginning of {sample} section
			//output.push_back("<BOS>");

			// Failed samples will have <{sample}> identifiers
			std::string failedSample;
			failedSample.append("<" + sample + ">");
			
			// Insert {sample} to identify the sample 
			output.push_back(failedSample);

			std::cout << "sample directory in GetThisVerification loop NOT FOUND: " << sample << "\n";
			

			// Signals end of this {sample}'s section
			//output.push_back("<EOS>");


		}
		
	}
	// TODO REMOVE ONLY DEBUG
	for (std::string outS : output) std::cout << " output from GetThisVerification: " << outS << "\n";

	// Returns vector with sections for each sample, with either its missing files or identifiers to signal missing sample directory
	return output;
}
std::vector<std::string> GetSamplesVerification(std::vector<std::string> InputForVerification) {
	//TODO
	std::vector<std::string> output;
	return output;

}
std::vector<std::string> GetSectorsVerification(std::vector<std::string> InputForVerification) {
	//TODO
	std::vector<std::string> output;
	return output;
}
std::vector<std::string> RegInfoVerification(std::vector<std::string> InputForVerification) {
	//TODO
	std::vector<std::string> output;
	return output;
}

/* Tools that output files only */
std::vector<std::string> FastFindVerification(std::vector<std::string> InputForVerification) {
	//TODO
	std::vector<std::string> output;
	return output;
}

/* Tools that ouput metadata only */
std::vector<std::string> FatInfoVerification(std::vector<std::string> InputForVerification) {
	//TODO
	std::vector<std::string> output;
	return output;
}
std::vector<std::string> NTFSInfoVerification(std::vector<std::string> InputForVerification) {
	//TODO
	std::vector<std::string> output;
	return output;;
}
std::vector<std::string> USNInfoVerification(std::vector<std::string> InputForVerification) {
	//TODO
	std::vector<std::string> output;
	return output;
}
std::vector<std::string> ObjInfoVerification(std::vector<std::string> InputForVerification) {
	//TODO
	std::vector<std::string> output;
	return output;
}

/* Tools that may output anything */
std::vector<std::string> UnknownToolVerification(std::vector<std::string> InputForVerification) {
	//TODO
	std::vector<std::string> output;
	return output;
}

bool CheckIfDirectoryExists(std::string path) { // Source: https://stackoverflow.com/questions/18100097/portable-way-to-check-if-directory-exists-windows-linux-c
	const char* pathC = path.c_str();
	struct stat info;

	if (stat(pathC, &info) != 0)
		return false;
	//printf("cannot access %s\n", pathC);
	else if (info.st_mode & S_IFDIR)  // S_ISDIR() doesn't exist on my windows 
		return true;
	//printf("%s is a directory\n", pathC);
	else
		return true;
	//printf("%s is no directory\n", pathC);


}

std::vector<std::string> CheckIfFilesExist(std::string path, std::vector<std::string> files) {

	// Used for conversion to LPCWSTR
	std::string pathFile;
	
	// Individual file extracted from files
	std::string file;

	std::vector<std::string> output;
	
	
	/*while (!(files.empty())) {
		file = files.substr(0, files.find(',')); // file is first substr in files up to first ',' 
		pathFile = path;

		//Change to loop for every file in files
		pathFile.append("\\");

		pathFile.append(file);

		std::cout << "file : " << file << "\n";
		std::cout << "files : " << files << "\n";
		std::cout << "CheckIfFilesExist pathFile: " << pathFile << "\n";

		// Conver string pathFile to LPCWSTR pathC
		std::wstring pathW = s2ws(pathFile);
		LPCWSTR pathC = pathW.c_str();

		GetFileAttributes(pathC); // from winbase.h
		if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(pathC) && GetLastError() == ERROR_FILE_NOT_FOUND)
		{
			//File not found 
			std::cout << "File not found : " << pathFile << "\n";

			//TODO CHECK FOR POSSIBLE ERRORS
		}

		files.erase(0, files.find(',')+1); // Erase first string from files
		std::cout << "files after erasing first file : " << files << "\n";
	}*/

	for (std::string file : files) {
		pathFile = path;

		//Change to loop for every file in files
		pathFile.append("\\");

		pathFile.append(file);

		std::cout << "CheckIfFilesExist() :  file: " << file << " pathFile: "<< pathFile <<"\n";


		// Conver string pathFile to LPCWSTR pathC
		std::wstring pathW = s2ws(pathFile);
		LPCWSTR pathC = pathW.c_str();

		GetFileAttributes(pathC); // from winbase.h
		if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(pathC) && GetLastError() == ERROR_FILE_NOT_FOUND)
		{
			//File not found 
			std::cout << "CheckIfFilesExist() : File: "<< file<<" not found : " << pathFile << "\n";
			output.push_back(file);
			//TODO CHECK FOR POSSIBLE ERRORS
		}
	}

	

	// Returns string with all the files that were not found
	return output;
}

std::wstring s2ws(const std::string& s) // Source https://stackoverflow.com/questions/27220/how-to-convert-stdstring-to-lpcwstr-in-c-unicode
{
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}

std::string GetORCOutputDirectory() {
	char currentPath[MAX_PATH]; // TODO Replace with better solution since MAX_PATH might not be enough
	std::string Path;

	//ORCOutputFolder
	GetModuleFileNameA(NULL, currentPath, MAX_PATH);
	// TODO Use GetModuleFileNameEx for DFIR ORC integration?
	
	Path = currentPath;
	Path.erase(Path.find(UnificationModuleExecutableName), UnificationModuleExecutableName.length());
	Path.append(ORCOutputFolder); // TODO error handling if path name wrong

	return Path;
}