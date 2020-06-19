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

#include <string>
#include <vector>



/* Verifies if all specifically requested artifacts are available */

/*Oversees all tool output verifications*/
std::vector<std::vector<std::string>> MainVerification(std::vector<std::string>ToolsToVerify, std::vector<std::string>& Samples, std::string Path);

/* Tool that collects files and metadata */
std::vector<std::string> GetThisVerification(std::vector<std::string>& samples);

/* Tools that may output anything */
std::vector<std::string> UnknownToolVerification();

bool CheckIfDirectoryExists(std::string path);

/*Checks if files in list files exist in path. Path uses ',' as denominator between files. Returns file that do NOT exist */
std::vector<std::string> CheckIfFilesExist(std::string path, std::vector<std::string> files);

std::wstring s2ws(const std::string& s);

std::string GetORCOutputDirectory();

// The output directory for the DFIR ORC tools
extern std::string ORCOutputDirectoryPath;

