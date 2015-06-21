# NDISDriverInst

A library to install/uninstall NDIS driver on Windows.

### Features

* This library is based on WDK sample bindview which is a dialogue based executable program.  
* Currently, it only supports NetService driver. Certainly, you can modify it to support Client and Protocol driver.  


### Build

I build it with WDK7600.16385.1 xp fre/chk environment.  
Provide static(default) and dynamic library build.  
If you want to build dynamic library(dll), rename file "sources_dynamic" to "sources", then build.  
They all have debug and release version. Debug version has a _d flag in the library name.  
I have changed TARGETPATH to "Lib", so you'll get your library in "Lib" folder.  


### Using

You just need include NDISDriverInst.h.  
Default, you will use the static library.  
If you want to use dynamic library(dll), you should define macro NDIS_INST_IMPORT_DYN before include the header.  


### Examples

see test.cpp

### PS

I only test it with WDK sample "passthru" driver.  
Please tell me when error or suggestion, thanks.  


