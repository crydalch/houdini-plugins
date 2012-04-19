#!/usr/bin/python

import subprocess, sys, os, shutil, time

def watcher(watchDirectory):
    print "Scan a directory every 5 seconds for new job files."

    foundJob = False

    while not foundJob:
        if os.listdir(watchDirectory):

            # Only checks out a Houdini license if we find a job to run
            import hou
            rawFiles = [(watchDirectory + '/' + b) for b in os.listdir(watchDirectory)]
            print rawFiles
            jobsToDo = [parseJobs(rf) for rf in rawFiles]
            print jobsToDo
            for j in jobsToDo:
                print j[0]
                hou.hipFile.load(j[0])
                
                for ropOp in j[1:]:
                    print ropOp
                    rop = hou.node(ropOp)
                    startJob(ropOp)
            # Release the license when finished
            if hasattr(hou, "releaseLicense"):
                hou.releaseLicense()
            for f in rawFiles:
                shutil.move(f, './done')
        else:
            print "idle..."
            time.sleep(5)

def parseJobs(jobFiles):
    """
    Reads job file, and returns a list, including the hipfile and any rop(s) to be rendered.
    """
    f = open(jobFiles)
    fl = f.readlines()
    #j = [i.replace('\n','').split(' ') for i in f.readlines()]
    j = fl[0].replace('\n','').split(' ')
    f.close()
    return j

def _alfReport(_ropNode):
    """
    We need to see the alfred progress report; this node returns the appropriate type to be set
    """
    ropNodeTypes = {"ifd":"vm_alfprogress", "geometry":"alfprogress"}
    return ropNodeTypes[_ropNode.type().name()]

def startJob(ropNode):
    """
    Renders a job with a given hip file.
    """
    rop = hou.node(ropNode)
    rop.name()
    rop.parm(_alfReport(ropNode)).set(1)
    rop.render()

def main():
    print "FlooWatch started..."
    sys.path.append(
    os.environ['HFS'] + "/houdini/python%d.%dlibs" % sys.version_info[:2])
    watchDir = sys.argv[1]
    watcher(watchDir)

if __name__ == "__main__":
    main()