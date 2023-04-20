#define _XOPEN_SOURCE 700

#define FUSE_USE_VERSION 35 

#define u_int unsigned int

#define DEBUG 1

#define DEF_CP_INTERVAL 4
#define DEF_CLEAN_START 4
#define DEF_CLEAN_STOP  8


#include "flash.h"
#include "types.h"
#include "log.h"
#include "lfs.h"

int main(int argc, char **argv){

    //Quit if no file handle
    if (argc < 3){
        printf("Please provide name of flash device to be mounted\n");
        return 1;
    }

    char *devicename = argv[argc-2];
    char *mntpnt     = argv[argc-1];

    for(int i = 0; i < argc-1; i++){
        if(strcmp(argv[i],"-i") == 0){
            //cpnt_interval = atoi(argv[i+1]);
        }
        else if (strcmp(argv[i],"-c") == 0){
            //cln_thrshld = atoi(argv[i+1]);
        }
        else if (strcmp(argv[i],"-C") == 0){
            //stp_thrshld = (argv[i+1]);
        }
    }    

    FD = load_device(devicename);
    Flash_Close(FD);
    // init_root();


    #define N_ARGS 4
    char *fuseargs[N_ARGS] = {
        "fuse_sys",
        mntpnt,
        "-f",
        "-s",
        // "-o auto_cache"
        // "-d"
    };

    // fuse_main(N_ARGS,fuseargs, &ops, NULL);

    return 0;    

}