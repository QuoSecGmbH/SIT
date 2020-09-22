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

/* This is the Validation Module that checks the completeness and validity of the forensic artifacts collected by 
    the ArtifactModule and converts the CSV metadata into RDF turtle format for the AFF4Module */

#define _CRT_SECURE_NO_WARNINGS

#include "ArtifactValidation.h"
#include "ConversionRDF.h"
#include "MetadataSerialization.h"
#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#include <windows.h>
#include <direct.h>
#include <stdlib.h>
#include <chrono>
#include <ctime>

int main(int argc, char* argv[]) {
    
    // Redirect stderr into logfile
    FILE* logStream; 
    freopen_s(&logStream, "ValidationModuleLog.txt", "w", stderr);

    SYSTEMTIME start, end;
    GetSystemTime(&start);

    std::string version = "v0.5";

    std::cerr << "SIT ValidationModule "<<version<<"\n\n";
    std::cout << "SIT ValidationModule " << version << "\n\n";

    std::cerr << "                          Start time : "
        <<start.wDay<<"."
        <<start.wMonth<<"."
        <<start.wYear<<" "
        <<start.wHour<<":"
        <<start.wMinute<<":"
        <<start.wSecond<<"."
        <<start.wMilliseconds<<" (UTC)"
        <<"\n\n";    
    std::cerr << "                    ValidationModule : Start\n";

    std::vector<std::string> ToolsToVerify;
    std::vector<std::string> Samples;
    std::vector<std::vector<std::string>> FailedValidations;
    std::string Path;

    char cwd[_MAX_PATH];
    if (_getcwd(cwd, _MAX_PATH) == NULL) {
        std::cout << "                    ValidationModule : ERROR path is NULL!\n";
        std::cerr << "                    ValidationModule : ERROR path is NULL!\n";
    }

    Path = cwd;
    Path.append(argv[1]);
    
    // Tools to verify, by default "GetThis" / "ArtifactModule"
    ToolsToVerify.push_back("ArtifactModule");
 
    // Validity check for artifacts at {Path}
    FailedValidations= MainValidation(ToolsToVerify, Samples,Path);

    // Artifacts selected for imaging
    std::vector <std::string> artifacts;

    // Convert metadata csv into rdf turtle   
    if (!(ConversionRDF(ORCOutputDirectoryPath, metadataStrings,artifacts))) {
        std::cout << "                    ValidationModule : RDF conversion FAILED!\n";
        std::cerr << "                    ValidationModule : RDF conversion FAILED!\n";

        std::cout << "                    ValidationModule : NOT successfully finished!\n";
        std::cerr << "                    ValidationModule : NOT successfully finished!\n";
        return -1;
    }

    // Remove redundant ArtifactModule.7z
    std::string ArtifactModuleArchivePath = Path;
    ArtifactModuleArchivePath.push_back('/');
    ArtifactModuleArchivePath.append("ArtifactModule.7z");

    if (remove(ArtifactModuleArchivePath.c_str()) != 0) {
        std::cerr << "                    ValidationModule : Archive in temp directory at "<<ArtifactModuleArchivePath<< " NOT removed!\n";
    }
    
    std::string artifactNewPath;

    // Select files for Partial Image
    std::cerr << "                    ValidationModule : Start reordering files in temp directory\n";
    for (std::string sample : Samples) {
        // Move all files up in main output directory for imaging
        int indexArtifact = 0;
        for (std::string artifact : artifacts) {
            if (artifact.find(sample) != std::string::npos) {
                // Skip this iteration if next artifact is duplicate and skip last element
                if (((indexArtifact + 1) < artifacts.size() && artifact == artifacts[indexArtifact + 1])) {
                    // Skip this iteration
                }
                else {
                    
                    artifactNewPath = artifact;
                    artifactNewPath = artifactNewPath.substr(0, artifactNewPath.find(sample)) + artifactNewPath.substr(artifactNewPath.find(sample) + sample.size());

                    if (rename(artifact.c_str(), artifactNewPath.c_str()) != 0) {
                        std::cerr << "                    ValidationModule : File " << artifact << " in temp directory NOT successfully moved!\n";
                    }
                }
            }
            indexArtifact++;
        }
    }
    std::cerr << "                    ValidationModule : Successfully finished reordering files in temp directory\n";

    std::cerr << "                    ValidationModule : Successfully finished\n\n";

    GetSystemTime(&end);

    std::cerr << "                            End time : "
        << end.wDay << "."
        << end.wMonth << "."
        << end.wYear << " "
        << end.wHour << ":"
        << end.wMinute << ":"
        << end.wSecond << "."
        << end.wMilliseconds << " (UTC)"
        << "\n\n";

    // Calculate elapsed time 
    union timeunion {
        FILETIME ft;
        ULARGE_INTEGER ui;
    };

    timeunion startT, endT;

    SystemTimeToFileTime(&start, &startT.ft);
    SystemTimeToFileTime(&end, &endT.ft);

    long long elapsed = (endT.ui.QuadPart - startT.ui.QuadPart);
    long long elapsedSec = elapsed / 10000000;
    long long elapsedMilsec = (elapsed / 10000)-(elapsedSec*1000);

    std::cerr << "                        Elapsed time : "<<elapsedSec<<" sec, "<<elapsedMilsec<<" msec"<<"\n\n";

    // Close stream 
    std::fclose(stderr);
}

