/* Provides verification for all the artifacts stored in the container, using Hash values */
#include <windows.h>
#include <string>

// Calculate the MD5 value of given file
DWORD calculateMD5(LPCSTR filename, std::string& hashCode);

// Calculate the SHA1 value of given file
DWORD calculateSHA1(LPCSTR filename, std::string& hashCode);