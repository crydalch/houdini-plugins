#!/usr/bin/python3

import os
import argparse
import time
import subprocess
import pprint

"""
Utility to simplify working with multiple versions of Houdini installed.

Until we can source into the Bash environment directly, this script is
wrapped by a small Bash script, which can properly source the environment into the current shell.

Requires that you add the following bash code to either .bashrc or .aliases file, adjusting the
path to htool.py, based on where it's installed of course:

    htool() {
        if [ "$#" -ne 1 ]; then
            eval $(python3 $HOME/bin/htool.py $1 $2)
        else
            python3 $HOME/bin/htool.py $1
        fi
    }

"""

# TODO: Make it cross-platform
# TODO: Argument for looking in places other than /opt
# TODO: Support for choosing latest daily build, latest production build
# TODO: Support definind a major version of Houdini to restrict the search to
# TODO: Migrating user settings between versions
# TODO: Use the shell (Bash, Tcsh, Zsh, etc...) chosen by the user
# TODO: Set the Houdini OpenCL device
# TODO: Adding other Houdini environment variables (and would want to list all Houdini env variables)
# TODO: Project manager? Probably separate tool
# TODO: 'Fix warnings' tool, which uses HOM to fix the warnings, and copies the original hip file for safety

parser = argparse.ArgumentParser()

parser.add_argument("-l", "--list", action="store_true", help="list Houdini installations in /opt with creation date/time")
parser.add_argument("-p", "--pick", action="store", help="choose Houdini version to use")

args = parser.parse_args()
 
def convTime(epoch_time):
    return time.strftime("%Y/%m/%d - %H:%M:%S", time.localtime(epoch_time))

def printInstalledHFS():
    """
    Prints the versions of Houdini installed at /opt
    """
    hfsDirs = [d for d in os.listdir("/opt") if not "hfs.current" in d and d.startswith("hfs")]
    hfsDirs.sort()
    for d in hfsDirs:
        dir_ctime = convTime(os.path.getctime("/opt/%s"%(d)))
        print("%s \t%s" %(d.strip("hfs"),dir_ctime))

def setPickedHFS():
    """
    Run the Houdini environment for the version picked by the user; we run it via bash, which is 
    why the command is printed instead of run
    """
    #os.system("bash -c 'cd /opt/hfs%s;source houdini_setup;cd ~-'"%(args.pick))
    #subprocess.call("cd /opt/hfs%s;source houdini_setup;cd ~-"%(args.pick),shell=True)
    print("cd /opt/hfs%s;source houdini_setup;cd ~-"%(args.pick))

if args.list:
    printInstalledHFS()

elif args.pick:
    setPickedHFS()
