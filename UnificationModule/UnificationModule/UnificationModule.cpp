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
    and converts the CSV metadata from different tools into one RDF turtle file for the AFF4Module */

#include <iostream>
#include <string>
#include <vector>
#include "ArtifactVerification.h"
#include "ConversionRDF.h"
#include "MetadataSerialization.h"
//int argc, char* argv[]

int main() {
    std::vector<std::string> ToolsToVerifyTest;
    std::vector<std::vector<std::string>> InputForVerificationTest;
    std::vector<std::vector<std::string>> MainVerificationFeedback;

    // DEBUG TODO REMOVE
    std::vector<std::string> TestInput;
    TestInput.push_back("Sample1");
    TestInput.push_back("Sample4");

    //DEBUG TODO REMOVE
    ToolsToVerifyTest.push_back("GetThis");
    InputForVerificationTest.push_back(TestInput);
 
    // TODO call ArtifactVerification to check if all necessary directories and metadatafiles available
    MainVerificationFeedback= MainVerification(ToolsToVerifyTest, InputForVerificationTest);
    
    // TODO call MetadataVerification to check if metadata is valid
    // TODO convert CSV metadata to RDF turtle
    for (std::vector<std::string> out : MainVerificationFeedback) {
        for (std::string out2 : out) {
            std::cout << "Main VerificationFeedback in UnificationModule: "<< out2 << "\n";
        }
        
    }

    // 
    if (!(ConversionRDF(ORCOutputDirectoryPath, metadataStrings))) {
        std::cout <<" main() : ConversionRDF returned false "<< "\n";
    }
    
}

