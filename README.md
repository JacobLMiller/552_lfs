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

*global.h*
global.h contains includes for libraries common to all (most) .c files 

*types.h*
types.h contains the various structs, typedefs, and datastructure defs needed for LFS

*inode-tab.c*
inode-tab.c is responsible for managing the inode table. 