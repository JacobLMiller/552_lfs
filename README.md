# 552_lfs

To compile, run 
```
make mklfs 
make lfs
```

Then, initiate an empty disk with 
```
./mklfs FILE_NAME
```

And start up the file system with 

```
./lfs FILE_NAME MNT_PNT
```


**Files**
*mklfs.c*
mklfs.c creates a flash memory device by taking input arguments from the command line, creating a file with the specified name and size, and writing a header to the file containing disk parameters. The fill_device function initializes the device header data and writes it to the file using Flash_Write function from the "flash.h" library.  

*log.h*
log.h is used to represent information about a segment. 

*log.c*
log.c writes the buffer of an i_node structure to a specific sector of a flash file and returns 0 if the write was successful.

*lfs.h*
lfs.h declares and defines various functions for FUSE callbacks and  other helper functions for loading a Flash device, initializing the root directory, and setting file metadata.

*lfs.c*
lfs.c is the implementation of the a FUSE-based filesystem in C, with a specific set of operations defined in the "struct fuse_operations". 

*dump_flash.c*
dump_flash.c reads data from a flash memory file mentioned in the form of command line argument, by opening the file using Flash_Open function from "flash.h" library and checking for any errors. 

*dir.c*
dir.c  appends a file inode to a directory inode by traversing the linked list of inodes until the end is reached and adding the new inode as the last element in the list.

*global.h*
global.h contains includes for libraries common to all (most) .c files 

*types.h*
types.h contains the various structs, typedefs, and datastructure defs needed for LFS

*inode-tab.c*
inode-tab.c is responsible for managing the inode table. 
