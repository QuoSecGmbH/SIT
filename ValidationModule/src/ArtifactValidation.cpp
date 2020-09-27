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

#include "ArtifactValidation.h"
#include <windows.h>
#include <iostream>
#include "Directory.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "MetadataValidation.h"
#include "MetadataSerialization.h"


std::string ORCOutputDirectoryPath;

/*Oversees all tool output validations*/
std::vector<std::vector<std::string>> MainValidation(std::vector<std::string>ToolsToValidate, std::vector<std::string>&Samples,std::string Path) {
	
	std::cout << "                 Artifact validation : Start\n";
	std::cerr << "                 Artifact validation : Start\n";
	
	// One std::vector<std::string> for each Tool to check -> If empty except for Tool name then no failure 
	std::vector<std::vector<std::string>>FailedValidations; // Includes all tools with failed output validation

	// Holds Tool name and function output 
	std::vector<std::string> buffer;
	std::vector<std::string> functionBuffer;

	std::cerr << "                 Artifact validation : "<<Path<<" used as main path"<<"\n";

	if (!(Path.empty())) ORCOutputDirectoryPath = Path;

	if (!(CheckIfDirectoryExists(ORCOutputDirectoryPath))) {
		std::cerr << "                 Artifact validation : ERROR Path does not exist!\n";
		return FailedValidations;
	}


	unsigned int counter = 0;
	for (std::string Tool : ToolsToValidate) {
		if (Tool == "ArtifactModule") {
			// Holds the resulting <std::vector<std::string> from {Tool} output validation
			functionBuffer = ArtifactModuleValidation(Samples);
			
			//{Tool} string signals beginning of {Tool} section
			buffer.push_back(Tool);
			
			// Insert the <std::vector<std::string> result with all the failed validations for this Tool
			buffer.insert(buffer.end(), functionBuffer.begin(), functionBuffer.end());

			// Insert EOS string to signal the end of this {Tool}'s section
			buffer.push_back("<EOS>");
			
			// Insert the buffer into FailedValidations for output
			FailedValidations.push_back(buffer);
		}
		counter++;
	}

	int counterFailedValidations = 0;

	for (std::vector<std::string> out : FailedValidations) {
		for (std::string out2 : out) {
			if (!(out2.compare("<EOS>")) && !(out2.compare("GetThis"))
				&& !(out2.compare("ArtifactModule.csv"))
				&& !(out2.compare("ArtifactModule"))) {
				std::cout << "          NOT successfully validated : " << out2 << "\n";
				std::cerr << "          NOT successfully validated : " << out2 << "\n";
				counterFailedValidations++;
			}
		}
	}

	std::cerr << "                 Artifact validation : "<<counterFailedValidations<<" failed tool validations."<<"\n";
	std::cout << "                 Artifact validation : Successfully finished\n";
	std::cerr << "                 Artifact validation : Successfully finished\n";

	/* Returns the list of failed Validations */
	return FailedValidations;
}

/* Main Validation for ArtifactModule output */
std::vector<std::string> ArtifactModuleValidation(std::vector<std::string>& samples) {
	
	/* Holds the metdatafile name with identifiers (<{metadatafile}.csv>) if not found, without identifiers {metadatafile}.csv if found, as well as the name of each sample 
		Example output: "GetThis.csv""sample1""<sample2>" 
		Metadata file was found and valid, sample1 directory was found, sample 2 directory was not found
	*/
	std::vector<std::string> output;
	// The name of the metadata output file from Artifact Module
	std::string metadataName = "ArtifactModule.csv";
	// Path to the current sample from InputForVerification
	std::string samplePath;
	//Used for constructing samplePath
	std::string tmpPath;
	// Contains the Output from CheckIfFilesExist
	std::vector<std::string> tmpFile;
	// Contains all artifact entries found in metadata csv file
	std::vector<std::string> artifactMetadataEntries;
	// Contains all artifact entries found in output directory
	std::vector<std::string> artifactEntries;

	// Check if ArtifactModule.csv file exists and if it contains valid metadata
	tmpFile.push_back(metadataName);
	// CheckIfFilesExist method returns all the files that do exist from the input
	tmpFile = CheckIfFilesExist(ORCOutputDirectoryPath, tmpFile);
	
	// Validation if metadata csv file was found
	if (std::find(tmpFile.begin(),tmpFile.end(),metadataName)!= tmpFile.end()) {
		// ArtifactModule.csv was NOT found
		output.push_back("<" + metadataName + ">");
		std::cout << "             Metadata file NOT found : "<< metadataName<<"\n";
		std::cerr << "             Metadata file NOT found : " << metadataName << "\n";
		//std::cout << "metadataName file " << metadataName << " was NOT found in GetThisVerification with Path: " << ORCOutputDirectoryPath << "\n";
	}
	else { // Validation if metadata csv file contains valid metadata
		
		// Used only for IsValidMetadataFile input
		std::string tmp = ORCOutputDirectoryPath;
		tmp.append("\\" + metadataName);
		
		if (!(IsValidMetadataFile(tmp,"GetThis", artifactMetadataEntries,artifactEntries))) {
			// ArtifactModule.csv was found but NOT valid
			output.push_back("<<" + metadataName + ">>");
			std::cout << "   Metadata file found but NOT valid : "<< metadataName <<"\n";
			std::cerr << "   Metadata file found but NOT valid : "<< metadataName <<"\n";
		}
		else {
			// metadataName added if file was found and valid
			output.push_back(metadataName);
		}
	}

	// Sample directory
	std::string filteredSample;

	// Add samples to vector
	for (std::vector <std::string> file : metadataStrings[0]) {
		if (file[5] == "SampleName") continue;
			
		if (file[5].size() != 0) {
			filteredSample = file[5].substr(1,file[5].find("\\")-1);
			// Check if samples already included in samples vector
			if (std::find(samples.begin(), samples.end(), filteredSample) == samples.end()) {
				samples.push_back(filteredSample);
			}
		}
		else {
			std::cout << "                 Sample name NOT found : "<< file[5]<<"\n";
			std::cerr << "                 Sample name NOT found : " << file[5] << "\n";
		}
	}	

	// Check if all the samples from InputForVerification are available as folders
	for (std::string sample : samples) {

		tmpPath="\\"; // Represents "\"
		tmpPath.append(sample);
		samplePath = ORCOutputDirectoryPath;
		samplePath.append(tmpPath);
		
		// Check if folder with name {sample} available
		if (CheckIfDirectoryExists(samplePath)) { 
			// Insert {sample} to identify the sample from which the files might be missing
			output.push_back(sample);
		}
		// Sample directory does not exist
		else {
			// Failed samples will have <{sample}> identifiers
			std::string failedSample;
			failedSample.append("<" + sample + ">");
			
			// Insert {sample} to identify the sample 
			output.push_back(failedSample);

			std::cout << " Sample directory NOT found for sample : " << sample << "\n";
			std::cerr << " Sample directory NOT found for sample : " << sample << "\n";
		}		
	}

	// Returns vector with sections for each sample, with either its missing files or identifiers to signal missing sample directory
	return output;
}

/* Tools that may output anything */
std::vector<std::string> UnknownToolVerification(std::vector<std::string> InputForVerification) {
	//TODO for future additions to ValidationModule
	std::vector<std::string> output;
	return output;
}

bool CheckIfDirectoryExists(std::string path) { // Source: https://stackoverflow.com/questions/18100097/portable-way-to-check-if-directory-exists-windows-linux-c
	const char* pathC = path.c_str();
	struct stat info;

	if (stat(pathC, &info) != 0)
		return false;
	else if (info.st_mode & S_IFDIR)  
		return true;
	else 
		return true;
}

std::vector<std::string> CheckIfFilesExist(std::string path, std::vector<std::string> files) {

	// Used for conversion to LPCWSTR
	std::string pathFile;
	
	// Individual file extracted from files
	std::string file;

	std::vector<std::string> output;

	for (std::string file : files) {
		pathFile = path;

		//Change to loop for every file in files
		pathFile.append("\\");
		pathFile.append(file);

		// Convert string pathFile to LPCWSTR pathC
		std::wstring pathW = s2ws(pathFile);
		LPCWSTR pathC = pathW.c_str();

		GetFileAttributes(pathC); // from winbase.h
		if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(pathC) && GetLastError() == ERROR_FILE_NOT_FOUND)
		{
			//File not found 
			output.push_back(file);
		}
	}	

	// Returns vector with all the file paths that were not found
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

	GetModuleFileNameA(NULL, currentPath, MAX_PATH);
	
	Path = currentPath;
	Path.erase(Path.find(ValidationModuleExecutableName), ValidationModuleExecutableName.length());
	Path.append(ORCOutputFolder); 

	return Path;
}