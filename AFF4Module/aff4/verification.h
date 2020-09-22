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

/*Created by Fabian Faust*/

/* Provides verification for all the artifacts stored in the container, using Hash values */
#include <windows.h>
#include <string>

// Calculate the MD5 value of given file
DWORD calculateMD5(LPCSTR filename, std::string& hashCode);

// Calculate the SHA1 value of given file
DWORD calculateSHA1(LPCSTR filename, std::string& hashCode);

// Calculate the SHA256 value of given file
DWORD calculateSHA256(LPCSTR filename, std::string& hashCode);