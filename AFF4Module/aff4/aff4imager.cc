/*
Copyright 2014 Google Inc. All rights reserved.

Licensed under the Apache License, Version 2.0 (the "License"); you may not use
this file except in compliance with the License.  You may obtain a copy of the
License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied.  See the License for the
specific language governing permissions and limitations under the License.
*/

/*
  Modifications and additions as part of the SIT AFF4Module https://github.com/QuoSecGmbH/SIT, by Fabian Faust
*/

/*
  This is the command line tool to manager aff4 image volumes and acquire
  images.
*/
#include "aff4/libaff4.h"
#include "aff4/aff4_imager_utils.h"
#include <iostream>
#include <vector>
#include <windows.h>

int main(int argc, char* argv[]) {

    SYSTEMTIME start, end;
    GetSystemTime(&start);

    // Set correct logfile
    FILE* logStream;
    bool verificationModule = false;
    bool extraction = false;
    std::vector<std::string> args(argv, argv + argc);
    std::string version = "v0.6";

    for (size_t i = 1; i < args.size(); ++i) {
        if (args[i] == "--hash") {
            // Start as VerificationModule with corresponding logfile
            verificationModule = true;
            // Redirect stderr into logfile
            freopen_s(&logStream, "VerificationModuleLog.txt", "w", stderr);
            std::cerr << "SIT VerificationModule "<<version<<"\n\n";
            std::cout << "SIT VerificationModule " << version << "\n\n";

            std::cerr << "                          Start time : "
                << start.wDay << "."
                << start.wMonth << "."
                << start.wYear << " "
                << start.wHour << ":"
                << start.wMinute << ":"
                << start.wSecond << "."
                << start.wMilliseconds << " (UTC)"
                << "\n\n";

            break;
        }
        if (args[i] == "--exportAll") {
            extraction = true;
            break;
        }
    }
    if (!verificationModule) {
        if (!extraction) freopen_s(&logStream, "AFF4ModuleLog.txt", "w", stderr);
        else freopen_s(&logStream, "AFF4ModuleExtractionLog.txt", "w", stderr);
  
        std::cerr << "SIT AFF4Module " << version << "\n\n";
        std::cout << "SIT AFF4Module " << version << "\n\n";

        std::cerr << "                          Start time : "
            << start.wDay << "."
            << start.wMonth << "."
            << start.wYear << " "
            << start.wHour << ":"
            << start.wMinute << ":"
            << start.wSecond << "."
            << start.wMilliseconds << " (UTC)"
            << "\n\n";

    }

    aff4::BasicImager imager;


    // Execute the imager process
    aff4::AFF4Status res = imager.Run(argc, argv);

    // Calculate elapsed time 
    union timeunion {
        FILETIME ft;
        ULARGE_INTEGER ui;
    };

    timeunion startT, endT;

    if (res == aff4::STATUS_OK || res == aff4::CONTINUE) {

        GetSystemTime(&end);

        std::cerr << "\n" << "                            End time : "
            << end.wDay << "."
            << end.wMonth << "."
            << end.wYear << " "
            << end.wHour << ":"
            << end.wMinute << ":"
            << end.wSecond << "."
            << end.wMilliseconds << " (UTC)"
            << "\n\n";

        SystemTimeToFileTime(&start, &startT.ft);
        SystemTimeToFileTime(&end, &endT.ft);

        long long elapsed = (endT.ui.QuadPart - startT.ui.QuadPart);
        long long elapsedSec = elapsed / 10000000;
        long long elapsedMilsec = (elapsed / 10000) - (elapsedSec * 1000);

        std::cerr << "                        Elapsed time : " << elapsedSec << " sec, " << elapsedMilsec << " msec" << "\n\n";

        // Close stream 
        std::fclose(stderr);


        return 0;
    }

    imager.resolver.logger->error("Imaging failed with error: {}", AFF4StatusToString(res));
    std::cout << "            Imaging failed with error: "<<AFF4StatusToString(res)<<"\n";
    std::cerr << "            Imaging failed with error: "<<AFF4StatusToString(res)<<"\n";

    GetSystemTime(&end);

    std::cerr <<"\n"<< "                            End time : "
        << end.wDay << "."
        << end.wMonth << "."
        << end.wYear << " "
        << end.wHour << ":"
        << end.wMinute << ":"
        << end.wSecond << "."
        << end.wMilliseconds << " (UTC)"
        << "\n\n";

    SystemTimeToFileTime(&start, &startT.ft);
    SystemTimeToFileTime(&end, &endT.ft);

    long long elapsed = (endT.ui.QuadPart - startT.ui.QuadPart);
    long long elapsedSec = elapsed / 10000000;
    long long elapsedMilsec = (elapsed / 10000) - (elapsedSec * 1000);

    std::cerr << "                        Elapsed time : " << elapsedSec << " sec, " << elapsedMilsec << " msec" << "\n\n";

    // Close stream 
    std::fclose(stderr);

    return res;
}

