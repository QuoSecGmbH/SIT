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

