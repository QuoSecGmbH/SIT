#include "MetadataSerialization.h"
#include <fstream>
#include<iostream>
#include <sstream>
#include<iterator>
#include<algorithm>

std::vector<std::vector<std::vector<std::string>>> metadataStrings;

std::vector<std::vector<std::string>> serializeCSV(std::string path,std::string tool) {
	std::vector<std::vector<std::string>> serializedFile;


	// Reads in the content of the CSV file at {path}
	serializedFile = getCSVFile(path,tool);
	
	// TODO CHECK WHY THIS HAPPENS! REMOVE first 3 chars from first element
	if (serializedFile[0][0].length() >= 4) {
		std::cout << "serializeCSV 3 char remover activated for: BEFORE " << serializedFile[0][0] << "\n";
		serializedFile[0][0] = serializedFile[0][0].substr(3, serializedFile[0][0].length() - 1);
		std::cout << "serializeCSV 3 char remover activated for: AFTER "<< serializedFile[0][0] << "\n";
	}
	
	// Create tool "header"
	std::vector<std::string> toolHeader = serializedFile[0];
	toolHeader[0]=tool;
	// Add toolHeader as the last line of serializedFile
	serializedFile.push_back(toolHeader);


	//Append serializedFile to central metadataStrings for RDF conversion
	metadataStrings.push_back(serializedFile);
		
	return serializedFile;
}


// Inspired by: https://thispointer.com/how-to-read-data-from-a-csv-file-in-c/
std::vector<std::vector<std::string>> getCSVFile(std::string path,std::string tool) {
		
	std::cout << "getCSVFile start for path: "<< path << "\n";

	// Contains the entire file, with each line being its own vector of strings
	std::vector<std::vector<std::string>> file;
	
	// Opens stream 
	std::ifstream CSVfile(path);
	
	// The delimiter separating each CSV element
	std::string delimiter = ",";


	std::string line = "";

	//Used for constructing file
	std::vector<std::string> tmp;

	// Iterate through each line and split the content using delimeter
	
	
	while (getline(CSVfile, line))
	{
		tmp.clear();
		split(line, delimiter, tmp);
		std::cout <<" getCSVFile() tmp length in loop: "<<tmp.size()<<"\n";
		file.push_back(tmp);
	}

	/*std::string line;
	while (std::getline(stream, line)) {

	}*/
	
	// Close the stream
	CSVfile.close();

	// REMOVE ONLY DEBUG
	int counterOuterLoop = 0;
	int counterInnerLoop = 0;
	
	
	//std::vector<std::string>::iterator test2;
	/*for (std::vector<std::string>::iterator test2 = file.begin(); test2 != file.end(); test2++) {

	}*/


	for (auto test : file) {
		counterInnerLoop = 0;
		std::cout << "getCSVFile() check line size from test in outer: " << counterOuterLoop << " test size: " << test.size() << "\n";
		for (std::string test2 : test) {
			counterInnerLoop++;
			std::cout <<"getCSVFile() check file content in outer: "<<counterOuterLoop<<" and inner: "<<counterInnerLoop<< " content: "<<test2<< "\n";
		}
		counterOuterLoop++;
		//test.clear();
		std::cout << "getCSVFile() test clear test size after: "<< test.size()<< "\n";
		break;
	}

	std::cout << "getCSVFile end for path: " << path << "\n";
	std::cout << "getCSVFile end for path: size of file : " << file.size() << "\n";
	
	return file;


}

// Source: https://stackoverflow.com/questions/289347/using-strtok-with-a-stdstring // TODO CHANGE
void split(const std::string& str, std::string& delim, std::vector<std::string>& parts) {
	/*size_t start, end = 0;
	while (end < str.size()) {
		start = end;
		while (start < str.size() && (delim.find(str[start]) != std::string::npos)) {
			start++;  // skip initial whitespace
		}
		end = start;
		while (end < str.size() && (delim.find(str[end]) == std::string::npos)) {
			end++; // skip to end of word
		}
		if (end - start != 0) {  // just ignore zero-length strings.
			parts.push_back(std::string(str, start, end - start));
		}
	}*/

	std::string output;
	std::istringstream iss(str);
	while (std::getline(iss, output, ','))
	{
		parts.push_back(output);
		//std::cout << "split output: "<<output << "\n";
	}

}