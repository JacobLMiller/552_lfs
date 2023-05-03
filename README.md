# 552_lfs

To compile, run 
```
make
```
The files for flash.c and flash.h need to be present in the directory, and you will need to have the fuse library installed and available in your PATH. 

We can boot up a file system with the command

```
./lfs FILE_NAME MNT_PNT
```
where the FILE_NAME is a valid flash img file and MNT_PNT is an empty directory. 

# Summary

*What works, what doesn't work?*

We opted for the read-only LFS project, which is fully functional. Our code can load a file system from a checkpoint and read files, directories, symlinks, and hard links. Fuse callbacks provide an interface with the operating system to allow one to use commands like ls, cat, stat, etc. from the terminal. No writing, cleaning, or checkpoint taking is implemented.

Note that third and fourth level indexing is not implemented, but should be safe. The remainder of the large file not covered by the first 12+K+K^2 blocks should just be zeros. 

------------------------------------------

The following fuse callbacks are operational: 

* readdir: First looks up the inode associated with the path formal parameter and ensures that it is a DIR_TYPE. Directories are files, so the file is read into a buffer then cast to an array of dir_entry structs. A for loop then goes over each entry and calls the FUSE filler function on non-empty entries. 

* getattr: Sets the appropriate info in the stat struct passed by fuse. Currently gives uid, gid, atime, mtime to all inode types. A simple switch statement is used to set the mode of the file uisng IF_DIR, IF_REG, or IF_SYM. 

* read: Looks up the inode associated with path and reads the file into memory and memcpys it into the fuse buf. 

* readlink: Identical to read, though the link file contains a file string. It is copied into buf and fuse passes it to read. 

-------------------------

**inode table**

Once we have searched for a particular file once, we store the inode location in a hash table hashed by it's full filename (path and all). This makes things much faster once an inital ls (or similar) command has been called. 

------------------------------------

# Challenges
We found it quite difficult to decide on implementation details. C is neither of our preferred programming languages, but we were able to settle in after the first part of the project. 

The most difficult thing we found to implement was the actual reading of files from disk. We opted for a safe approach that may not be the most efficient. We first determine how many blocks we will need to read and then load their addresses into an array, which is the hard part thanks to indirect addressing. Then we just make a pass through every address and concatenate them into a file buffer of appropriate size. There is likely a clean, recursive way of reading indirect blocks but we had a lot of trouble with functions destroying pointers on return. 

The read-only LFS option was much more straightforward and the providing of test img files was much appreciated as a way to gauge progress. A project of this scope was also more in-line with the time we had to dedicate to the class in addition to paper readings. 

--------------------------------------

# **Files**

*eval.py*

A python script that tests that each .img file provided on the course webpage is loaded correctly. 

*global.h*

Includes needed throughout the project, sets the debug mode, and defines unsigned int. 

*init.c* 

Includes functions to load a flash device from a .img file, booting up a file system from a checkpoint, and reading a file from an inode. 

*inode-tab.c* 

Manages an inode hash table that makes reading much faster after initial boot.

*lfs.c*

Location of the main function and the fuse interface. Implements fuse callbacks.

*lfs.h*

Defines fuse callback functions.

*makefile*

A simple make file that links all .h and .c files and compiles them into lfs 

*README.md* 

The raw markdown of our writeup.

*types.h*

Definitions for structs and other data structures mostly as defined in the read-only LFS spec. 