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

/* Converts serialized csv metadata into rdf AFF4 turtle file */
#include <string>
#include <vector>


/* Oversees the insertion of metadataSerialized into turtle at path. If no turtle exists, it is created. */
bool ConversionRDF(std::string path, std::vector<std::vector<std::vector<std::string>>> metadataSerialized);

/* Creates a new AFF4 turtle */
bool createTurtle(std::string path);

/* Inserts one or more serialized metadata files into an AFF4 turtle */
bool insertIntoTurtle(std::vector<std::vector<std::vector<std::string>>> metadataSerialized, std::string path);

/* The name for the turtleFile */
extern std::string turtleName;

/* The headers for the turtleFile */
extern std::string turtleHeaders;
