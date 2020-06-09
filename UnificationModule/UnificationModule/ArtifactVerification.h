#include <string>
#include <vector>



/* Verifies if all specifically requested artifacts are available */

/*Oversees all tool output verifications*/
std::vector<std::vector<std::string>> MainVerification(std::vector<std::string>ToolsToVerify, std::vector<std::vector<std::string>>InputForVerification);

/* Tools that output files and metadata */
std::vector<std::string> GetThisVerification(std::vector<std::string> InputForVerification);
std::vector<std::string> GetSamplesVerification(std::vector<std::string> InputForVerification);
std::vector<std::string> GetSectorsVerification(std::vector<std::string> InputForVerification);
std::vector<std::string> RegInfoVerification(std::vector<std::string> InputForVerification);

/* Tools that output files only */
std::vector<std::string> FastFindVerification(std::vector<std::string> InputForVerification);

/* Tools that ouput metadata only */
std::vector<std::string> FatInfoVerification(std::vector<std::string> InputForVerification);
std::vector<std::string> NTFSInfoVerification(std::vector<std::string> InputForVerification);
std::vector<std::string> USNInfoVerification(std::vector<std::string> InputForVerification);
std::vector<std::string> ObjInfoVerification(std::vector<std::string> InputForVerification);

/* Tools that may output anything */
std::vector<std::string> UnknownToolVerification(std::vector<std::string> InputForVerification);

bool CheckIfDirectoryExists(std::string path);

/*Checks if files in list files exist in path. Path uses ',' as denominator between files. Returns file that
do NOT exist */
std::vector<std::string> CheckIfFilesExist(std::string path, std::vector<std::string> files);

std::wstring s2ws(const std::string& s);

std::string GetORCOutputDirectory();

// The output directory for the DFIR ORC tools
extern std::string ORCOutputDirectoryPath;

