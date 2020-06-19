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

#include "ConversionRDF.h"
#include "ArtifactVerification.h"
#include <iostream>
#include <fstream>
#include <sstream>


std::string turtleName = "genericMETA.turtle";
std::string turtleHeaders = std::string("@id <aff4:metadata> .").append("\n").append(std::string("@prefix rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> .").append("\n").append(std::string("@prefix aff4: <http://aff4.org/Schema#> .").append("\n").append(std::string("@prefix xsd: <http://www.w3.org/2001/XMLSchema#> .").append("\n"))));

bool ConversionRDF(std::string path, std::vector<std::vector<std::vector<std::string>>> metadataSerialized, std::vector <std::string>& artifacts) {
	// std::vector<std::string> CheckIfFilesExist(std::string path, std::vector<std::string> files);
	std::vector<std::string> turtle;
	turtle.push_back(turtleName);
	
	// Returns files from turtle that exist at path 
	turtle = CheckIfFilesExist(path,turtle);

	std::string turtlePath = path;
	turtlePath.append("\\");
	turtlePath.append(turtleName);

	// turtle does not exist at path
	if (turtle.size() >= 1 && turtle[0] == turtleName) {
		// Create new turtle		
		std::cout << "ConversionRDF() create turtleName: "<< turtleName<<" at path : "<<turtlePath<< "\n";
		if (!(createTurtle(turtlePath))) return false; // TODO ERROR HANDLING
	}	

	if (!(insertIntoTurtle(metadataSerialized, turtlePath,artifacts))) return false; // TODO ERROR HANDLING
	
	return true;
}

bool createTurtle(std::string path) {

	std::ofstream turtle(path);
	std::cout <<" createTurtle() " << "\n";
	// Adds the necessary turtle rdf headers
	turtle << turtleHeaders;
	turtle << "\n";
	
	turtle.flush();
	turtle.close();
	return true;
}

bool insertIntoTurtle(std::vector<std::vector<std::vector<std::string>>> metadataSerialized, std::string path, std::vector <std::string>& artifacts) {

	// Open turtle stream
	std::ofstream turtleStream;
	turtleStream.open(path, std::ios_base::app);

	std::cout << " insertIntoTurtle() at path: "<<path << "\n";
	std::string pathIdentifier;

	// TODO DIFFERENCES BETWEEN TOOLS

	// TODO REPLACE BACKSLASH WITH FORWARDSLASH
	
	// Isolate outputDirectory from path
	std::string outputDirectory = path;
	while (outputDirectory.find('/') != std::string::npos) {
		outputDirectory = outputDirectory.substr(outputDirectory.find('/') + 1);
	}
	
	// TODO CHANGE FOR GENERIC METADATA FILE NAME
	outputDirectory = outputDirectory.substr(0, outputDirectory.find("\\"));
	std::cout <<" insertIntoTurtle(): outputDirectory filtered: "<< outputDirectory << "\n";

	for (std::vector<std::vector<std::string>> toolMetadata : metadataSerialized) {
		// toolMetadata is metadata from one tool
		if (toolMetadata.size() == 0) continue;

		// Prepare path identifier "</file:/path/to/file.txt>"
		std::string pathIdentifierGeneric = "</file:";
		// Add main output directory relative path
		pathIdentifierGeneric.append(outputDirectory);
		pathIdentifierGeneric.append("/");
		//pathIdentifierGeneric.append(path.substr();
		// Contains absolute path for artifact WITH sample folder
		std::string artifactAbsolutePath;
		
		// Path without turtleName -> necessary if using absolute paths
		//pathIdentifierGeneric.append(path.substr(0, path.find(turtleName)));
		
		// Tool identifier
		std::string tool = toolMetadata[toolMetadata.size()-1][0];
		std::cout <<" toolMetadata tool: "<<tool<<"\n";
		std::cout << " MetadataSerialized size: " << metadataSerialized.size() << "\n";
		std::cout << " toolMetadata size: " << toolMetadata.size() << "\n";

		//pathIdentifierGeneric.append(tool.substr(0, tool.find(".csv") - 1));
		//pathIdentifierGeneric.append("\\");
		// pathIdentifier at this point: "</file:/path/to/";
	
		int counter = 0; // To treat first line differently

		for (auto file : toolMetadata) {
			// file is metadata from one artifact/file, file[0] contains Attribute types
			std::cout << " insertIntoTurtle(): file iteration : "<<counter<< " toolMetadata length: " <<toolMetadata.size()<< "\n";
			// TODO Should not happen
			if (file.size() == 0) continue;


			if (counter >= 1) {
				// Adds one empty line between each file 
				turtleStream << "\n";

				pathIdentifier = pathIdentifierGeneric;

				// Contains the path to the file itself without sample folder, without the " " symbols
				if (pathIdentifier.size() >= 6) pathIdentifier.append(file[5].substr(file[5].find("\\") + 1, (file[5].length() - 2) - (file[5].find("\\"))));
				
				artifactAbsolutePath = path.substr(0, path.find(turtleName));
				artifactAbsolutePath.append(file[5].substr(1, file[5].length() - 2));
				
				// Replace both backslashes with forwardslashes 
				if (artifactAbsolutePath.find("\\")) artifactAbsolutePath[artifactAbsolutePath.find("\\")] = '/';
				//if (artifactAbsolutePath.find("\\")) artifactAbsolutePath[artifactAbsolutePath.find("\\")] = '/';

				// Adds current absolute path for artifact to vector for image selection
				artifacts.push_back(artifactAbsolutePath);

				// Replace backslash with forwardslash 
				if (pathIdentifier.find("\\") != std::string::npos) pathIdentifier[pathIdentifier.find("\\")] = '/';

				std::cout << " insertIntoTurtle() artifactAbsolutePath: BEFORE  " << file[5].substr(file[5].find("\\") + 1, (file[5].length() - 2)- (file[5].find("\\"))) << "\n";

				pathIdentifier.append(">");

				std::cout << " insertIntoTurtle() pathIdentifier:  " << pathIdentifier << "\n";

				// Add pathIdentifier for file
				turtleStream << pathIdentifier << "\n\n";
				
			}

			std::cout << " insertIntoTurtle(): file size(): "<<file.size() << "\n";
			
			if (counter >= 1) {

				// Index for each individual element of file
				int elementCounter = 0;

				for (std::string element : file) {
					// element is one metadata element
					//if (tool == "") do;
					//std::cout << " insertIntoTurtle(): element: " << element << "\n";
					if (element.empty()) continue;

					// Add new attribute for each element
					turtleStream << "\t" <<"aff4:" << toolMetadata[0][elementCounter] << "\t" << element;

					if (true) turtleStream << "^^xsd:string" << ';' << "\n";
					
					std::cout << "insertIntoTurtle(): elementCounter: "<<elementCounter<< "\n";

					elementCounter++;
				}

			}
			counter++;		

		}
	}

	// Close turtle stream
	turtleStream.close();
	return true;
}