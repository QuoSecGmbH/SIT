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
