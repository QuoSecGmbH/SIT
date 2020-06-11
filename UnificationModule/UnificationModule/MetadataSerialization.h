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


// Serializes .csv files into strings

#include <string>
#include <vector>

/* Serializes the csv file at {path} into a string, adds it to metadataStrings
for RDF conversion and returns it
*/
std::vector<std::vector<std::string>> serializeCSV(std::string path,std::string tool);

/* Contains the serialized csv metadata
First line contains only "<{tool}>"  containing the tool from which the metadata originated.
Each file will be its own vector of vectors, each line its own vector 
and each element will be its own string
*/
extern std::vector<std::vector<std::vector<std::string>>> metadataStrings;

std::vector<std::vector<std::string>> getCSVFile(std::string path, std::string tool);

void split(const std::string& str, std::string& delim, std::vector<std::string>& parts);