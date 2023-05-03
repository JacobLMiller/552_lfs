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

We opted for the read-only LFS project, which is fully functional. Our code can load a file system from a checkpoint and read files, directories, symlinks, and hard links. Fuse callbacks provide an interface with the operating system to allow one to use commands like ls, cat, stat, etc. from the terminal. 

------------------------------------------

The following fuse callbacks are operational: 

* readdir: First looks up the inode associated with the path formal parameter and ensures that it is a DIR_TYPE. Directories are files, so the file is read into a buffer then cast to an array of dir_entry structs. A for loop then goes over each entry and calls the FUSE filler function on non-empty entries. 

* getattr: Sets the appropriate info in the stat struct passed by fuse. Currently gives uid, gid, atime, mtime to all inode types. A simple switch statement is used to set the mode of the file uisng IF_DIR, IF_REG, or IF_SYM. 

* read: Looks up the inode associated with path and reads the file into memory and memcpys it into the fuse buf. 

* readlink: Identical to read, though the link file contains a file string. It is copied into buf and fuse passes it to read. 

-------------------------

**inode table**

We implemented inodes perhaps a bit strangely. We don't know ahead of time how many inodes we might need, so an array can be an inefficient data structure to store them despite it's ease of use; i.e. we might have to resize the array many times. We elected to use a hash table instead. The implementation we settled on was a table of fixed sized (NUM_BUCKETS = 256), and each inode is assigned a bucket based on a hash of it's name. Each bucket contains a linked list of inodes, so lookup can be O(n) in the worst case but shouldn't happen if the hash function is pseudo uniform. This was motivated by the fact that fuse only gives us a file via it's path, so lookup via path seemed the easiest to interface with.
The inode hashtable is a static global ITAB available in inode-tab.c; An image abstraction is shown below.

![ITAB data structure](figs/ITAB.png)

**inodes**

The inodes themselves are implemented as simple structs with 4 fields: the inode number, a metadata struct, an array of address structs, and a pointer to the next inode in the list (possibly null). 

*metadata*: The metadata struct (simply named meta) contains various fields that might be good to have on hand; We have given it a file type (enum with FILE_TYPE, DIR_TYPE, and LINK_TYPE fields), a size in bytes, the number of blocks the file uses, and it's name as a string. 

*address*: The address struct (named block_addr) contains the segment number and block number the file data can be found on disk. This is also where we store the buffer for the data in memory, in the field buf which is allocated BLOCK_SIZE bytes when the block_addr struct is initialized. As mentioned, an inode contains an array (hard coded to size 8) of block_addresses, so the max file size we could support is 8 blocks. We do not implement indirect addressing. 

![inode data structures](figs/structs.png)

-------------------------

**Logging**

Currently, the `log' portion of our log based file system is not quite correct. While we do greedily assign new blocks to the end of the current segment and always write out new blocks (no modifying old ones). The only thing really missing is a fully functional checkpoint system, which we plan to implement by making note of all of the segment tables inode structs and writing them to disk at segment 0. 

------------------------------------

# Challenges
We found it quite difficult to decide on implementation details. While we understood LFS at a very high, conceptual level it felt impossible to know where to even start. The fuse callbacks were concrete, already implemented functions so it felt easiest to start by designing around those. This may have been a little backwards (e.g. designing the inode table as a hashmap, since that feels natural when given a string). 

C is not the language of choice for either of us, so many many bugs and segfaults were battled against while writing this code. There is a known memory leak in inode-tab.c, in flush_to_log(), currently line 80. Sometimes this points to unallocated memory. While we have not observed this yet calling a crash, it likely would if the fs is run long enough. This unfamiliarity made development time take longer than either of us would like for a class project. 

Currently however, there should be no other seg faults or errors as long as files aren't too big. Many fuse operations that aren't detailed above are unsupported. There are likely more memory leaks however. We aimed to make the code concise but readable.

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