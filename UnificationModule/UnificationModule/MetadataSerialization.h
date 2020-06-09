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