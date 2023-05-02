import os 
import random
run = os.system

if __name__ == "__main__":

    for fname in ["LFS_DISK-100_files"]:
        run(f"./lfs {fname}.img tmp")
        os.chdir("tmp")
        run(f"./../examine_directory > {fname}.our_out")
        os.chdir("..")
        run(f"diff {fname}.out {fname}.our_out")