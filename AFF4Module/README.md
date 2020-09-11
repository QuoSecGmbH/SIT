# IMPORTANT NOTE
 AFF4Module is based on the c-aff4imager, in accordance with the Apache 2.0 License,
 with no association to its original creators. The changed name reflects that, in order
 to avoid confusion with the original. The c-aff4imager project can be accessed at
 https://github.com/Velocidex/c-aff4
 ALL changes are documented in SIT-changelog.md
 The SIT VerificationModule is integrated into the AFF4Module for efficiency reasons.


# AFF4 -The Advanced Forensics File Format

The Advanced Forensics File Format 4 (AFF4) is an open source format
used for the storage of digital evidence and data.

The standard is currently maintained here:
https://github.com/aff4/Standard

Reference Images are found:
https://github.com/aff4/ReferenceImages

This project implementats a C/C++ library for creating, reading and
manipulating AFF4 images. The project also includes the canonical
aff4imager binary which provides a general purpose standalone imaging
tool.

The library and binary are known to work on Windows (All versions).


## What is currently supported.

Currently this library supports most of the features described in the
standard https://github.com/aff4/Standard.

1. Reading and Writing ZipFile style volumes

   a. Supports splitting of output volumes into volume groups
      (e.g. splitting at 1GB volumes).

2. Reading and Writing Directory style volumes.

3. Reading and Writing AFF4 Image streams using the deflate or snappy
   compressor.

4. Reading RDF metadata using Turtle.

5. Multi-threaded imaging for efficient utilization on multi core
   systems.


## What is not yet supported.

This implementation currently does not implement Section 6. Hashing of
the standard. This includes verifying or generating linear or block
hashes.

## Copyright

Copyright 2015-2017 Google Inc.
Copyright 2018-present Velocidex Innovations.

## References

[1] "Extending the advanced forensic format to accommodate multiple data sources,
logical evidence, arbitrary information and forensic workflow" M.I. Cohen,
Simson Garfinkel and Bradley Schatz, digital investigation 6 (2009) S57–S68.
