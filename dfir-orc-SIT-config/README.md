# IMPORTANT NOTE
 dfir-orc-SIT-config is a modified version of dfir-orc-config, in accordance with OPEN LICENCE 2.0/LICENCE OUVERTE 2.0, 
 with no association to its original creators. The changed name reflects that, in order to avoid confusion with the 
 original. Not all instances of the original name have been changed to retain compatibility outside the changed areas. 
 The original can be accessed at: https://github.com/DFIR-ORC/dfir-orc-config
 ALL changes are documented in SIT-changelog.md 

# DFIR ORC Configuration

To configure DFIR ORC, you need:
* configuration files in XML format, located in the "config" directory
* items to embed (especially DFIR-Orc binaries in 32 and 64 bits), stored in the "tools" directory

The configurations given as example here use Sysinternals "Autoruns" tools. You have to download and put it in the "tools" directory.

The "tools" directory must therefore contain the following files:
* DFIR-Orc_x64.exe
* DFIR-Orc_x86.exe
* autorunsc.exe

Finally, to generate a configured DFIR-Orc executable, you have to run the ".\Configure.cmd" script (on a Windows system, **from an elevated command prompt**).
The generated binary is created in the "output" directory.


## Authors and contributors

Authors and contributors are the same as listed in the [AUTHORS file of GitHub repository of the source code](https://github.com/dfir-orc/dfir-orc/blob/master/AUTHORS.txt).
