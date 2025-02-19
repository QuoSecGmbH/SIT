// This file defines the AFF4 Lexicon.

/*
  Modifications and additions as part of the SIT AFF4Module https://github.com/QuoSecGmbH/SIT, by Fabian Faust
*/

/* NOTE: The LEXICON_DEFINE macro expands to extern const std::string
in headers */

LEXICON_DEFINE(AFF4_VERSION, "1.0");
LEXICON_DEFINE(AFF4_VERSION_MAJOR, "1");
LEXICON_DEFINE(AFF4_VERSION_MINOR, "0");
LEXICON_DEFINE(AFF4_TOOL, "libaff4");

LEXICON_DEFINE(AFF4_NAMESPACE, "http://aff4.org/Schema#");
LEXICON_DEFINE(XSD_NAMESPACE, "http://www.w3.org/2001/XMLSchema#");
LEXICON_DEFINE(RDF_NAMESPACE, "http://www.w3.org/1999/02/22-rdf-syntax-ns#");
LEXICON_DEFINE(AFF4_DISK_NAMESPACE, "http://aff4.org/Schema#disk/");
LEXICON_DEFINE(AFF4_MEMORY_NAMESPACE, "http://aff4.org/Schema#memory/");
LEXICON_DEFINE(AFF4_LEGACY_NAMESPACE, "http://afflib.org/2009/aff4#");

/**
 * The default AFF4 prefix for AFF4 objects.
 */
LEXICON_DEFINE(AFF4_PREFIX, "aff4://");
LEXICON_DEFINE(FILE_PREFIX, "file://");
LEXICON_DEFINE(BUILTIN_PREFIX, "builtin://");

// Attributes in this namespace will never be written to persistant
// storage. They are simply used as a way for storing metadata about an AFF4
// object internally.
LEXICON_DEFINE(AFF4_VOLATILE_NAMESPACE, "http://aff4.org/VolatileSchema#");

// Commonly used RDF types.
LEXICON_DEFINE(URNType, "URN");
LEXICON_DEFINE(XSDStringType, "http://www.w3.org/2001/XMLSchema#string");
LEXICON_DEFINE(RDFBytesType, "http://www.w3.org/2001/XMLSchema#hexBinary");
LEXICON_DEFINE(XSDIntegerType, "http://www.w3.org/2001/XMLSchema#integer");
LEXICON_DEFINE(XSDIntegerTypeInt, "http://www.w3.org/2001/XMLSchema#int");
LEXICON_DEFINE(XSDIntegerTypeLong, "http://www.w3.org/2001/XMLSchema#long");
LEXICON_DEFINE(XSDBooleanType, "http://www.w3.org/2001/XMLSchema#boolean");

/// Attribute names for different AFF4 objects.

/// Base AFF4Object
LEXICON_DEFINE(AFF4_TYPE, "http://www.w3.org/1999/02/22-rdf-syntax-ns#type");
/**
 * Defines the URN where this object is stored.
 */
LEXICON_DEFINE(AFF4_STORED, "http://aff4.org/Schema#stored");
LEXICON_DEFINE(AFF4_LEGACY_STORED, "http://afflib.org/2009/aff4#stored");
/**
 * Defines the URNs of any child objects.
 */
LEXICON_DEFINE(AFF4_CONTAINS, "http://aff4.org/Schema#contains");
/**
 * Defines the URN that this object is a child of.
 */
LEXICON_DEFINE(AFF4_TARGET, "http://aff4.org/Schema#target");

/*
 * Each container should have this file which contains the URN of the container.
 */
LEXICON_DEFINE(AFF4_CONTAINER_DESCRIPTION, "container.description");
/**
 * Defines the default name for the information turtle file.
 */
LEXICON_DEFINE(AFF4_CONTAINER_INFO_TURTLE, "information.turtle");
LEXICON_DEFINE(AFF4_CONTAINER_INFO_YAML, "information.yaml");
/**
 * Each AFF4 container should have this file to denote the AFF4 standard which this container is using.
 */
LEXICON_DEFINE(AFF4_CONTAINER_VERSION_TXT, "version.txt");

/// AFF4 ZipFile containers.
LEXICON_DEFINE(AFF4_ZIP_TYPE, "http://aff4.org/Schema#ZipVolume");



// Can be "read", "truncate", "append"
LEXICON_DEFINE(AFF4_STREAM_WRITE_MODE, "http://aff4.org/VolatileSchema#writable");

// FileBackedObjects are either marked explicitly or using the file:// scheme.
LEXICON_DEFINE(AFF4_FILE_TYPE, "http://aff4.org/Schema#file");

// file:// based URNs do not always have a direct mapping to filesystem
// paths. This volatile attribute is used to control the filename mapping.
LEXICON_DEFINE(AFF4_FILE_NAME, "http://aff4.org/VolatileSchema#filename");

// The original filename the stream had.
LEXICON_DEFINE(AFF4_STREAM_ORIGINAL_FILENAME, "http://aff4.org/Schema#original_filename");

/// ZipFileSegment
LEXICON_DEFINE(AFF4_ZIP_SEGMENT_TYPE, "http://aff4.org/Schema#zip_segment");

/** Custom AFF4 metadata types.
* Note: These types are not part of the official standard. They are necessary to allow the inclusion of custom metadata types.
*/

// A custom timestamp format. 
LEXICON_DEFINE(AFF4_STREAM_TIMESTAMP, "http://aff4.org/Schema#timestamp");

// A custom original path of acquisition format.
LEXICON_DEFINE(AFF4_STREAM_PATH_OF_ACQUISITION, "http://aff4.org/Schema#path_of_acquisition");

// A custom format to specify the name of the computer.
LEXICON_DEFINE(AFF4_STREAM_COMPUTERNAME, "http://aff4.org/Schema#ComputerName");

// A custom format to specify the ID of the volume.
LEXICON_DEFINE(AFF4_STREAM_VOLUMEID, "http://aff4.org/Schema#VolumeID");

// A custom format to specify the FRN of the parent.
LEXICON_DEFINE(AFF4_STREAM_PARENTFRN, "http://aff4.org/Schema#ParentFRN");

// A custom format to specify the FRN.
LEXICON_DEFINE(AFF4_STREAM_FRN, "http://aff4.org/Schema#FRN");

// A custom format to specify the full name.
LEXICON_DEFINE(AFF4_STREAM_FULLNAME, "http://aff4.org/Schema#FullName");

// A custom format to specify the sample name.
LEXICON_DEFINE(AFF4_STREAM_SAMPLENAME, "http://aff4.org/Schema#SampleName");

// A custom format to specify the size in bytes.
LEXICON_DEFINE(AFF4_STREAM_SIZEINBYTES, "http://aff4.org/Schema#SizeInBytes");

// A custom format to specify the MD5 hash code.
LEXICON_DEFINE(AFF4_STREAM_MD5, "http://aff4.org/Schema#MD5");

// A custom format to specify the SHA1 hash code.
LEXICON_DEFINE(AFF4_STREAM_SHA1, "http://aff4.org/Schema#SHA1");

// A custom format to specify the FindMatch parameter.
LEXICON_DEFINE(AFF4_STREAM_FINDMATCH, "http://aff4.org/Schema#FindMatch");

// A custom format to specify the content type.
LEXICON_DEFINE(AFF4_STREAM_CONTENTTYPE, "http://aff4.org/Schema#ContentType");

// A custom format to specify the sample collection date.
LEXICON_DEFINE(AFF4_STREAM_SAMPLECOLLECTIONDATE, "http://aff4.org/Schema#SampleCollectionDate");

// A custom format to specify the creation date.
LEXICON_DEFINE(AFF4_STREAM_CREATIONDATE, "http://aff4.org/Schema#CreationDate");

// A custom format to specify the last modification date.
LEXICON_DEFINE(AFF4_STREAM_LASTMODIFICATIONDATE, "http://aff4.org/Schema#LastModificationDate");

// A custom format to specify the last access date.
LEXICON_DEFINE(AFF4_STREAM_LASTACCESSDATE, "http://aff4.org/Schema#LastAccessDate");

// A custom format to specify the last attribute change date.
LEXICON_DEFINE(AFF4_STREAM_LASTATTRCHANGEDATE, "http://aff4.org/Schema#LastAttrChangeDate");

// A custom format to specify the filename creation date.
LEXICON_DEFINE(AFF4_STREAM_FILENAMECREATIONDATE, "http://aff4.org/Schema#FileNameCreationDate");

// A custom format to specify the filename last modification date.
LEXICON_DEFINE(AFF4_STREAM_FILENAMELASTMODIFICATIONDATE, "http://aff4.org/Schema#FileNameLastModificationDate");

// A custom format to specify the filename last access date.
LEXICON_DEFINE(AFF4_STREAM_FILENAMELASTACCESSDATE, "http://aff4.org/Schema#FileNameLastAccessDate");

// A custom format to specify the filename last attribute modification date.
LEXICON_DEFINE(AFF4_STREAM_FILENAMELASTATTRMODIFICATIONDATE, "http://aff4.org/Schema#FileNameLastAttrModificationDate");

// A custom format to specify the attribute type.
LEXICON_DEFINE(AFF4_STREAM_ATTRTYPE, "http://aff4.org/Schema#AttrType");

// A custom format to specify the attribute name.
LEXICON_DEFINE(AFF4_STREAM_ATTRNAME, "http://aff4.org/Schema#AttrName");

// A custom format to specify the attribute ID.
LEXICON_DEFINE(AFF4_STREAM_ATTRID, "http://aff4.org/Schema#AttrID");

// A custom format to specify the snapshot ID.
LEXICON_DEFINE(AFF4_STREAM_SNAPSHOTID, "http://aff4.org/Schema#SnapshotID");

// A custom format to specify the SHA256 hash code.
LEXICON_DEFINE(AFF4_STREAM_SHA256, "http://aff4.org/Schema#SHA256");

// A custom format to specify the SSDeep.
LEXICON_DEFINE(AFF4_STREAM_SSDEEP, "http://aff4.org/Schema#SSDeep");

// A custom format to specify the TLSH.
LEXICON_DEFINE(AFF4_STREAM_TLSH, "http://aff4.org/Schema#TLSH");

// A custom format to specify the yara rules.
LEXICON_DEFINE(AFF4_STREAM_YARARULES, "http://aff4.org/Schema#YaraRules");

// A custom format to specify the examiner responsible for the metadata.
LEXICON_DEFINE(AFF4_STREAM_EXAMINER, "http://aff4.org/Schema#examiner");

// A custom format to store notes.
LEXICON_DEFINE(AFF4_STREAM_NOTES, "http://aff4.org/Schema#notes");

// A custom unknown format. Used if unable to identify metadata type.
LEXICON_DEFINE(AFF4_STREAM_UNKNOWN, "http://aff4.org/Schema#unknown");

// A custom format to include the original unrecognized metadata type, which should be saved as unknown. Used if unable to identify metadata type.
LEXICON_DEFINE(AFF4_STREAM_UNKNOWN_ORIGINAL_TYPE, "http://aff4.org/Schema#unknown_original_type");

/**
 * AFF4 Primary Object Types.
 *
 * Note: These types are abstract entities which refer to a logical
 * image. They do not say anything about the actual storage methods
 * used. Instead they refer to the AFF4 stream which stores the data
 * in their dataStream attribute.
 *
 * Since a single AFF4 URN may represent several different types,
 * these types may also apply to a concrete object, in which case no
 * aff4:dataStream is needed.
 */
LEXICON_DEFINE(AFF4_IMAGE_TYPE, "http://aff4.org/Schema#Image");
LEXICON_DEFINE(AFF4_DISK_IMAGE_TYPE, "http://aff4.org/Schema#DiskImage");
LEXICON_DEFINE(AFF4_VOLUME_IMAGE_TYPE, "http://aff4.org/Schema#VolumeImage");
LEXICON_DEFINE(AFF4_MEMORY_IMAGE_TYPE, "http://aff4.org/Schema#MemoryImage");
LEXICON_DEFINE(AFF4_CONTIGUOUS_IMAGE_TYPE, "http://aff4.org/Schema#ContiguousImage");
LEXICON_DEFINE(AFF4_DISCONTIGUOUS_IMAGE_TYPE, "http://aff4.org/Schema#DiscontiguousImage");

/**
 * AFF4 Evimetry Legacy Object Types.
 * The aff4:Image may contain device information, and MUST contain the size and reference to the map, via
 * aff4:dataStream
 */
LEXICON_DEFINE(AFF4_LEGACY_IMAGE_TYPE, "http://afflib.org/2009/aff4#Image");
LEXICON_DEFINE(AFF4_LEGACY_DISK_IMAGE_TYPE, "http://afflib.org/2009/aff4#device");
LEXICON_DEFINE(AFF4_LEGACY_VOLUME_IMAGE_TYPE, "http://afflib.org/2009/aff4#volume");
LEXICON_DEFINE(AFF4_LEGACY_CONTIGUOUS_IMAGE_TYPE, "http://afflib.org/2009/aff4#contiguousMap");

/**
 * AFF4 Map Stream
 * The aff4:Map stream will contain the hash digests, and dataStream references to the lower streams.
 */
LEXICON_DEFINE(AFF4_MAP_TYPE, "http://aff4.org/Schema#Map");
LEXICON_DEFINE(AFF4_LEGACY_MAP_TYPE, "http://afflib.org/2009/aff4#map");
/**
 * The AFF4 Stream to use when filling in gaps for aff4:DiscontiguousImage
 */
LEXICON_DEFINE(AFF4_MAP_GAP_STREAM, "http://aff4.org/Schema#mapGapDefaultStream");

/**
 * AFF4 Data Stream
 */
LEXICON_DEFINE(AFF4_IMAGESTREAM_TYPE, "http://aff4.org/Schema#ImageStream");
LEXICON_DEFINE(AFF4_DATASTREAM, "http://aff4.org/Schema#dataStream");
LEXICON_DEFINE(AFF4_STREAM_SIZE, "http://aff4.org/Schema#size");
LEXICON_DEFINE(AFF4_STREAM_CHUNK_SIZE, "http://aff4.org/Schema#chunkSize");
LEXICON_DEFINE(AFF4_STREAM_VERSION, "http://aff4.org/Schema#version");
LEXICON_DEFINE(AFF4_STREAM_CHUNKS_PER_SEGMENT, "http://aff4.org/Schema#chunksInSegment");

/**
 * AFF4 Evimetry Legacy Data Stream
 */
LEXICON_DEFINE(AFF4_LEGACY_IMAGESTREAM_TYPE, "http://afflib.org/2009/aff4#stream");
LEXICON_DEFINE(AFF4_LEGACY_DATASTREAM, "http://afflib.org/2009/aff4#dataStream");
LEXICON_DEFINE(AFF4_LEGACY_STREAM_SIZE, "http://afflib.org/2009/aff4#size");
LEXICON_DEFINE(AFF4_LEGACY_STREAM_CHUNK_SIZE, "http://afflib.org/2009/aff4#chunk_size");
LEXICON_DEFINE(AFF4_LEGACY_STREAM_VERSION, "http://afflib.org/200version");
LEXICON_DEFINE(AFF4_LEGACY_STREAM_CHUNKS_PER_SEGMENT, "http://afflib.org/2009/aff4#chunks_in_segment");

/**
 * Compression Methods.
 */
LEXICON_DEFINE(AFF4_IMAGE_COMPRESSION, "http://aff4.org/Schema#compressionMethod");
LEXICON_DEFINE(AFF4_LEGACY_IMAGE_COMPRESSION, "http://afflib.org/2009/aff4#CompressionMethod");
LEXICON_DEFINE(AFF4_IMAGE_COMPRESSION_ZLIB, "https://www.ietf.org/rfc/rfc1950.txt");
LEXICON_DEFINE(AFF4_IMAGE_COMPRESSION_DEFLATE, "https://tools.ietf.org/html/rfc1951");
LEXICON_DEFINE(AFF4_IMAGE_COMPRESSION_SNAPPY, "http://code.google.com/p/snappy/");
LEXICON_DEFINE(AFF4_IMAGE_COMPRESSION_SNAPPY2, "https://github.com/google/snappy");
LEXICON_DEFINE(AFF4_IMAGE_COMPRESSION_LZ4, "https://code.google.com/p/lz4/");
LEXICON_DEFINE(AFF4_IMAGE_COMPRESSION_STORED, "http://aff4.org/Schema#NullCompressor");
LEXICON_DEFINE(AFF4_LEGACY_IMAGE_COMPRESSION_STORED, "http://afflib.org/2009/aff4#nullCompressor");

/**
 * Default namespace for symbolic streams.
 */
LEXICON_DEFINE(AFF4_IMAGESTREAM_ZERO, "http://aff4.org/Schema#Zero");
LEXICON_DEFINE(AFF4_IMAGESTREAM_FF, "http://aff4.org/Schema#FFDevice");
LEXICON_DEFINE(AFF4_IMAGESTREAM_UNKNOWN, "http://aff4.org/Schema#UnknownData");
LEXICON_DEFINE(AFF4_IMAGESTREAM_UNREADABLE, "http://aff4.org/Schema#UnreadableData");
LEXICON_DEFINE(AFF4_IMAGESTREAM_SYMBOLIC_PREFIX, "http://aff4.org/Schema#SymbolicStream");

LEXICON_DEFINE(AFF4_LEGACY_IMAGESTREAM_ZERO, "http://afflib.org/2009/aff4#Zero");
LEXICON_DEFINE(AFF4_LEGACY_IMAGESTREAM_FF, "http://afflib.org/2009/aff4#FF");
LEXICON_DEFINE(AFF4_LEGACY_IMAGESTREAM_UNKNOWN, "http://afflib.org/2009/aff4#UnknownData");
LEXICON_DEFINE(AFF4_LEGACY_IMAGESTREAM_UNREADABLE, "http://afflib.org/2009/aff4#UnreadableData");
LEXICON_DEFINE(AFF4_LEGACY_IMAGESTREAM_SYMBOLIC_PREFIX, "http://afflib.org/2012/SymbolicStream#");

// AFF4Map - stores a mapping from one stream to another.


// Categories describe the general type of an image.
LEXICON_DEFINE(AFF4_CATEGORY, "http://aff4.org/Schema#category");

LEXICON_DEFINE(AFF4_MEMORY_PHYSICAL, "http://aff4.org/Schema#memory/physical");
LEXICON_DEFINE(AFF4_MEMORY_VIRTUAL, "http://aff4.org/Schema#memory/virtual");
LEXICON_DEFINE(AFF4_MEMORY_PAGEFILE, "http://aff4.org/Schema#memory/pagefile");
LEXICON_DEFINE(AFF4_MEMORY_PAGEFILE_NUM, "http://aff4.org/Schema#memory/pagefile_number");

LEXICON_DEFINE(AFF4_DISK_RAW, "http://aff4.org/Schema#disk/raw");
LEXICON_DEFINE(AFF4_DISK_PARTITION, "http://aff4.org/Schema#disk/partition");


// The constant stream is a psuedo stream which just returns a constant.
LEXICON_DEFINE(AFF4_CONSTANT_TYPE, "http://aff4.org/Schema#constant");

// The constant to repeat, default 0).
LEXICON_DEFINE(AFF4_CONSTANT_CHAR, "http://aff4.org/Schema#constant_char");

LEXICON_DEFINE(AFF4_DIRECTORY_TYPE, "http://aff4.org/Schema#directory");

// An AFF4 Directory stores all members as files on the filesystem. Some
// filesystems cannot represent the URNs properly, hence we need a mapping
// between the URN and the filename. This attribute stores the _relative_ path
// of the filename for the member URN relative to the container's path.
LEXICON_DEFINE(AFF4_DIRECTORY_CHILD_FILENAME, "http://aff4.org/Schema#directory/filename");

// Cryptographic Hashing

LEXICON_DEFINE(AFF4_HASH_TYPE, "http://aff4.org/Schema#hash");

LEXICON_DEFINE(AFF4_HASH_MD5, "http://aff4.org/Schema#MD5");
LEXICON_DEFINE(AFF4_HASH_SHA1, "http://aff4.org/Schema#SHA1");
LEXICON_DEFINE(AFF4_HASH_SHA256, "http://aff4.org/Schema#SHA256");
LEXICON_DEFINE(AFF4_HASH_SHA512, "http://aff4.org/Schema#SHA512");
LEXICON_DEFINE(AFF4_HASH_BLAKE2B, "http://aff4.org/Schema#blake2b");

