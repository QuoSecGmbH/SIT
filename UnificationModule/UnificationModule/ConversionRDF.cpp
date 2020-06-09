#include "ConversionRDF.h"
#include "ArtifactVerification.h"
#include <iostream>
#include <fstream>
#include <sstream>


std::string turtleName = "genericMETA.turtle";

std::string turtleHeaders = std::string("@id <aff4:metadata> .").append("\n").append(std::string("@prefix rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> .").append("\n").append(std::string("@prefix aff4: <http://aff4.org/Schema#> .").append("\n").append(std::string("@prefix xsd: <http://www.w3.org/2001/XMLSchema#> .").append("\n"))));



bool ConversionRDF(std::string path, std::vector<std::vector<std::vector<std::string>>> metadataSerialized) {
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
	

	if (!(insertIntoTurtle(metadataSerialized, turtlePath))) return false; // TODO ERROR HANDLING

	
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

bool insertIntoTurtle(std::vector<std::vector<std::vector<std::string>>> metadataSerialized, std::string path) {

	// Open turtle stream
	std::ofstream turtleStream;
	turtleStream.open(path, std::ios_base::app);

	std::cout << " insertIntoTurtle() at path: "<<path << "\n";
	std::string pathIdentifier;

	// TODO DIFFERENCES BETWEEN TOOLS

	

	for (std::vector<std::vector<std::string>> toolMetadata : metadataSerialized) {
		// toolMetadata is metadata from one tool
		if (toolMetadata.size() == 0) continue;

		// Prepare path identifier "</file:/path/to/file.txt>"
		std::string pathIdentifierGeneric = "</file:";
		// Path to main output directory
		
		// Path without turtleName
		pathIdentifierGeneric.append(path.substr(0, path.find(turtleName)));
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

				// Contains the sample path to the file itself, without the " " symbols
				if (pathIdentifier.size() >= 6) pathIdentifier.append(file[5].substr(1,file[5].length()-2));

				pathIdentifier.append(">");

				std::cout << " insertIntoTurtle() pathIdentifier:  " << pathIdentifier << "\n";

				// Add pathIdentifier for file
				turtleStream << pathIdentifier << "\n\n";

				// TODO remove
				
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
			
			// TODO REMOVE
			if (counter == 5) break;

		}
		std::cout << "Test output after break " << "\n";
	}

	// Close turtle stream
	turtleStream.close();
	return true;
}