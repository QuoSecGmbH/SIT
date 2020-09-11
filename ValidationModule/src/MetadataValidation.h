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

/*Checks if the Metadata in CSV form is valid*/

#include <string>
#include <vector>

// Checks if the file at path is a valid csv metadata file as used by {tool} from DFIR ORC
bool IsValidMetadataFile(std::string path, std::string tool, std::vector<std::string>& artifactMetadataEntries, std::vector<std::string>& artifactEntries);

// Checks if serialized file includes valid metadata from ArtifactModule tool 
bool IsValidMetadataFileArtifactModule(std::vector<std::vector<std::string>> file, std::vector<std::string>& artifactMetadataEntries, std::vector<std::string>& artifactEntries);

// Checks if serialized file includes valid metadata from UnknownTool tool 
bool IsValidMetadataFileUnknownTool(std::vector<std::vector<std::string>> file);

// Returns all the entries in the directory at {path}
std::vector<std::string>findDirectoryEntries(std::string path);