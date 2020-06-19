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

/* This is the Unification Module that verifies the Completeness and Validity of the forensic Artifacts 
    and converts the CSV metadata into a RDF turtle file for the AFF4Module */

#include <iostream>
#include <string>
#include <vector>
#include "ArtifactVerification.h"
#include "ConversionRDF.h"
#include "MetadataSerialization.h"
#include <cstdio>


int main(int argc, char* argv[]) {
    std::vector<std::string> ToolsToVerify;
    std::vector<std::string> Samples;
    std::vector<std::vector<std::string>> MainVerificationFeedback;
    std::string Path;

    if (argc>=2)  Path = argv[1];
 
    std::cout <<" Path at main: "<< Path<< "\n";
    
    // Tools to verify, by default "GetThis" / "ArtifactModule"
    ToolsToVerify.push_back("GetThis");
 
    // Call ArtifactVerification to check if all necessary directories and metadatafiles available
    MainVerificationFeedback= MainVerification(ToolsToVerify, Samples,Path);
    
    // TODO call MetadataVerification to check if metadata is valid
    
    // TODO convert CSV metadata to RDF turtle
    for (std::vector<std::string> out : MainVerificationFeedback) {
        for (std::string out2 : out) {
            std::cout << "Main VerificationFeedback in UnificationModule: "<< out2 << "\n";
        }       
    }

    // Artifacts selected for imaging
    std::vector <std::string> artifacts;

    // Convert metadata csv into rdf turtle
    if (!(ConversionRDF(ORCOutputDirectoryPath, metadataStrings,artifacts))) {
        std::cout <<" main() : ConversionRDF returned false "<< "\n";
    }
    
    std::string artifactNewPath;

    // Select files for Partial Image
    for (std::string sample : Samples) {
        // Move all files up in main output directory for imaging

        for (std::string artifact : artifacts) {
            std::cout << " main artifact: " << artifact << "\n";
           
            artifactNewPath = Path;
            
            if (artifactNewPath[artifactNewPath.length()-1]!='/') artifactNewPath.append("/");
           
            // Temporary path
            std::string tmp = artifact;
            
            // Extract artifact name from path

            // Replace backslashes with forwardslashes 
            if (tmp.find("\\") != std::string::npos) tmp[tmp.find("\\")] = '/';

            while (tmp.find('/') != std::string::npos) {
                tmp = tmp.substr(tmp.find('/')+1);
            }
          
            // Create new Path for artifact
            artifactNewPath.append(tmp);

            if (rename(artifact.c_str(), artifactNewPath.c_str())) std::cout << "File moved \n";
            else  std::cout << "File not moved \n";

        }
    }

}

