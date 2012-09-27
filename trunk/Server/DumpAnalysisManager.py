#!/usr/bin/env python
import os,sys,string,pysvn,time,zipfile,CrashHTMLGenerator

def getBranchFromSvnNum(svnNum,svnClient):   
    svnLog = svnClient.log('svn://redlake/srv/svn/sw/',
                            revision_start=pysvn.Revision(pysvn.opt_revision_kind.number, svnNum),
                            revision_end=pysvn.Revision(pysvn.opt_revision_kind.number, svnNum),
                            discover_changed_paths=True)    
    cng_paths = svnLog[0]["changed_paths"]
    changed=cng_paths[0]["path"]    
    return changed        



# Takes svn number and scans through a general path in CM_AUTO for the specific kit directory.
# sample usage:
# input: X:/CM_AUTO/BRANCHES/branches/features/AuxiliaryLogic , 16540
# output: 1.0.0.16540_20110403_125245_1187_16540_OK
def getSpecifcKitDirectory(kit_branch_path, svn_num):
    files = []
    dirList=os.listdir(kit_branch_path)
    for fname in dirList:
        files.append(fname)      
    
    for d in files:
        if d.endswith(".bat"):
            continue
        if d.endswith("fail"):
            continue        
        splitted = d.split(".")
        if splitted[3].split("_")[0] == str(svn_num):
            return d
    return "not_found"

def copySymbols(kit_path,to_path):
    #Obscured Version
    
    #lucidd3d9uENC.dll    -> lucidd3d9u.dll
    #lucidd3d9uENC.pdb        
    #lucidd3d10uENC.dll    -> lucidd3d10u.dll
    #lucidd3d10uENC.pdb        
    #sandbox_logicENC9.dll    -> sandbox_logic9.dll
    #sandbox_logicENC10.dll    -> sandbox_logic10.dll
    #sandbox_logicENC9.pdb    -> sandbox_logicENC.pdb    
    #LucidInterop.dll
    #LucidInterop.pdb    
    #lucidlogger.dll
    #lucidlogger.pdb    
    #gpuEnumerator.dll
    #gpuEnumerator.pdb
    
    bin_dir = kit_path+"\\Build\\release\\bin\\"
    print "Copying lucidd3d9uENC.dll"
    os.system("copy %slucidd3d9uENC.dll %s" % (bin_dir, to_path+"lucidd3d9.dll"))
    print "Copying slucidd3d9uENC.pdb"
    os.system("copy %slucidd3d9uENC.pdb %s" % (bin_dir, to_path))
    print "Copying lucidd3d10uENC.dll"
    os.system("copy %slucidd3d10uENC.dll %s" % (bin_dir, to_path+"lucidd3d10.dll"))
    print "Copying lucidd3d10uENC.pdb"
    os.system("copy %slucidd3d10uENC.pdb %s" % (bin_dir, to_path))
    print "Copying sandbox_logicENC9.dll"
    os.system("copy %ssandbox_logicENC9.dll %s" % (bin_dir, to_path+"sandbox_logic9.dll"))
    print "Copying sandbox_logicENC10.dll"
    os.system("copy %ssandbox_logicENC10.dll %s" % (bin_dir, to_path+"sandbox_logic10.dll"))
    print "Copying sandbox_logicENC9.pdb"
    os.system("copy %ssandbox_logicENC9.pdb %s" % (bin_dir, to_path+"sandbox_logicENC.pdb"))
    print "Copying lucidInterop*.*"
    os.system("copy %slucidInterop*.* %s" % (bin_dir, to_path))
    print "Copying lucidlogger*.*"
    os.system("copy %slucidlogger*.* %s" % (bin_dir, to_path))
    print "Copying gpuEnumerator*.*"
    os.system("copy %sgpuEnumerator*.* %s" % (bin_dir, to_path))
    #print "Copying everything from %s to %s" % (bin_dir, to_path)
    #os.system("copy %s*.* %s" % (bin_dir, to_path))
    
    


def AnalyzeDumpsIteration():
    print "------============------------"
    print " New Iteration "
    print "------============------------"
    print ""
    dumps_dir = 'E:\\'

    svnClient = pysvn.Client()
        

    files = []
    dirList=os.listdir(dumps_dir)
#    print "--=== Current Data Directory Status ===--"
    for fname in dirList:        
#        print fname
        files.append(fname)
        
        
    #debuggers_path = "C:\\Program Files\\Debugging Tools for Windows (x86)"
    debuggers_path = "C:\\DumpServer\\Debuggers\\"
        
    for file_name in files:
        if file_name.endswith(".zip"):
            dmp_file_name = file_name[:-4]  
            if os.path.exists(dumps_dir+dmp_file_name+".done"): #check if upload from gamer is done                
                print "Checking on [%s]" % file_name
                if os.path.exists(dumps_dir+dmp_file_name+".txt"):
                    print "%s already exists, skipping analysis." % dmp_file_name
                else:
                    #copy zip file
                    print "Copying [%s]"%dumps_dir+file_name
                    os.system('copy  "'+dumps_dir+file_name+'"')                                      
                    #extract the dmp inside
                    print "Decompressing [%s]" % file_name
                    os.system('7z -y e "'+file_name+'"')

                              
                    print "Analyzing [%s] ..." % dmp_file_name
                    splitted = dmp_file_name.split("@")
                    svn_num = int(splitted[0])
                    print "Svn Number: %d" % svn_num
                    svn_path = getBranchFromSvnNum(svn_num,svnClient)
                    kit_general_path = CrashHTMLGenerator.getKitPathFromSvnPath(svn_path)
                    specific_kit_path = getSpecifcKitDirectory(kit_general_path,svn_num)
                    if (specific_kit_path == "not_found"):
                        print "Error! Can't find kit for: [%s] svn num [%d]" % (kit_general_path, svn_num)
                        print "Skipping Analysis."
                        continue
                    kit_path =  kit_general_path+"\\"+specific_kit_path
                    print "Kit Path: %s" % kit_path
                                
                    
                    copySymbols(kit_path,dumps_dir+"Symbols\\")
                    #os.system(debuggers_path+"MiniDumpReader.exe "+dumps_dir+dmp_file_name+" "+dumps_dir+"Symbols\\ >> "+dumps_dir+dmp_file_name+".txt")
                    os.system(debuggers_path+'MiniDumpReader.exe "'+dmp_file_name+'" "'+dumps_dir+'Symbols" >> "'+dumps_dir+dmp_file_name+'.txt"')
                    print "deleting %s" % file_name
                    os.system('del "' + file_name+'"')
                    print "deleting %s" % dmp_file_name
                    os.system('del "' + dmp_file_name+'"')  
                    print "Finished Analyzing [%s] !" % dmp_file_name
                    print "Result log in [%s]." % dumps_dir+dmp_file_name+".txt"
            else:
                print "[%s] upload not completed yet" % dmp_file_name
            
        
        #if fname.endswith(".dmp"):        
        #    print fname


while(True):
    AnalyzeDumpsIteration()
    CrashHTMLGenerator.Process()
    print "Entering Sleep for 60 seconds"
    for i in xrange(6):
        if i!=0:
            print "%d seconds left" % (60-(10*i))
        time.sleep(10)
    