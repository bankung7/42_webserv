#!/use/bin/python

import sys # for argv
import os # for path

def main():
    if len(sys.argv) == 2:
        # define filepath
        filepath = "../" + sys.argv[1]
        print(filepath)
    print("testing output")

main()