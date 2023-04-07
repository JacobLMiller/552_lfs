import os 
import random

words = "42 cat dog orange tomato blue frog fromg froMg".split(" ")

#Add sha256, echo, cat, diff, etc. to test script
run = os.system


def init_test():
    
    os.chdir("tmp")
    for i in range(5):
        run(f"touch f{i}")
    run("ls")

    f = open("f1", "w")
    f.write("Hello world!\n")
    f.close()

    run("cat f1")

    f = open("f2", 'w')
    f.write(" ".join( words[random.randint(0,7)] for _ in range(100)))
    f.close()

    for i in range(50):
        f = open("f3", 'a')
        f.write(f" {i} \n")
        f.close()


    f = open("og",'w')
    f.write("I am copied.\n")
    f.close()

    run("cp og cpy")
    run("cat cpy")

    run("touch bigfile")
    f = open("bigfile", 'w')
    f.write(" ".join( words[random.randint(0,len(words)-1)] for _ in range(1000) )  )
    f.close()    

if __name__ == "__main__":

    os.chdir("tmp")
    f = open("ah", "w")
    f.write("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n")
    f.close()
    for i in range(16):
        run(f"touch foo{i}")
    for i in range(16):
        run(f"touch bar{i}")