#!/usr/bin/env python
from DumpTextParser import *
#import Levenshtein
import os
import os,sys,string,pysvn,time,zipfile,CrashHTMLGenerator

svnClient = pysvn.Client()

def getBranchFromSvnNum(svnNum):
    svnLog = svnClient.log('svn://redlake/srv/svn/sw/',
                            revision_start=pysvn.Revision(pysvn.opt_revision_kind.number, svnNum),
                            revision_end=pysvn.Revision(pysvn.opt_revision_kind.number, svnNum),
                            discover_changed_paths=True)    
    cng_paths = svnLog[0]["changed_paths"]
    changed=cng_paths[0]["path"]    
    return changed    

# Sample usage:
# input: /branches/features/...
# output: X:/CM_AUTO/BRANCHES/branches/features/AuxiliaryLogic
def getKitPathFromSvnPath(svn_path):    
    kit_base = "X:\\CM_AUTO\\BRANCHES\\"
    
    splitted = svn_path.split("/")
    # 3 options:
    # * trunk
    # * branches/features
    # * branches/releases
    kit_path = ""
    if splitted[1] == "trunk":
        kit_path = kit_base+"trunk"
    elif splitted[1] == "branches":
        if splitted[2] == "features":
            kit_path = kit_base+"branches_features_"+splitted[3]
        elif splitted[2] == "releases":
            kit_path = kit_base+"branches_releases_"+splitted[3]
        else:
            print "Error! Can't understand [%s]" % svn_path
    else:
        print "Error! Can't understand [%s]" % svn_path
    return kit_path

def sortedDictValues(adict):
    keys = adict.keys()
    keys.sort()
    
    ret = []
    
    for i in keys:
        ret.append((i,adict.get(i)))    
    
    #return map(adict.get, keys)
    return ret

def GetFormattedStack(stack_text):
    ret = ''
    lines = []
    curr_line = ''
    for char in stack_text:
        if char=='\n':
            lines.append(curr_line)
            curr_line = ''
        else:
            curr_line += char
    
    for line in lines:
        if "lucid" in line:
            ret += '<FONT COLOR="00FF00">'
        ret += line
        ret += '<br>'
        if "lucid" in line:
            ret += '</FONT>'

    return ret

def CreateSpecificCrashHTML(analysis_file_name):
    
    outf_name = analysis_file_name[:-4]+".html"
    outf_text_name = analysis_file_name[:-4]+".txt"
    
    if os.path.exists(outf_name): # it already exists
        return outf_name
    
    dtp = DumpTextParser()
    dtp.ReadFile(analysis_file_name)
    print "DBG - [%s]" % analysis_file_name
    analyzed_crash = dtp.Analyze()

    htmlcode = ''
    htmlcode+= '<body bgcolor="#000000"> \n<TABLE cellpadding="4" style="border: 1px solid #FFFFFF; border-collapse: collapse;" border="1">\n'
    
    for e in analyzed_crash:
        htmlcode+=' <TR>\n'                
        for s in e:
            htmlcode+='<TD><FONT COLOR = "FFFFFF">'        
            if 'faulting thread stack:' in e[0] and s == e[1]:
                htmlcode+=GetFormattedStack(s)
            else:
                htmlcode+=str(s)
            htmlcode+='</FONT></TD>'
        htmlcode+=' </TR>\n'


    htmlcode+=' <TR>\n'
    htmlcode+='<TD><FONT COLOR = "FFFFFF">'
    htmlcode+= 'Raw Text'
    htmlcode+='</FONT></TD>'

    htmlcode+='<TD><FONT COLOR = "FFFFFF">'
    htmlcode+= "<a title='%s' class=body_con href=\"file:///%s\">""%s""</a>" % ('tooltip',outf_text_name,outf_text_name)   
    htmlcode+='</FONT></TD>'    




    htmlcode+=' </TR>\n'    

        
    htmlcode+= ' </FONT>\n'
    htmlcode+= ' </TABLE>\n'
    
   
    
    outf = open(outf_name,'wt')
    outf.write(htmlcode)
    outf.close()
    
    return outf_name



def Process():    
    dumps_path = "\\\\crash\\data\\"
    
    files = []
    dirList=os.listdir(dumps_path)
    for fname in dirList:
        files.append(fname)      
    
    
    htmlcode_highlevel = ''
    htmlcode_highlevel+= '<body bgcolor="#FFFFFF"> \n<TABLE cellpadding="4" style="border: 1px solid #000000; border-collapse: collapse;" border="1">\n'
    
    crash_by_revision_map = {}
    
    for d in files:
        if d.endswith(".txt"):
            curr_dmp = d[:-4]
            splt = curr_dmp.split('@')
            link = CreateSpecificCrashHTML(dumps_path+d)
            if splt[0] not in crash_by_revision_map:
                crash_by_revision_map[splt[0]] = []
            crash_by_revision_map[splt[0]].append(link)
                
            
    sorted_crash_map = sortedDictValues(crash_by_revision_map)
    sorted_crash_map.reverse()
    
    
    #curr:
    #\\crash\data\16596@INTEL-RON@wic.exe@06.04.2011@04.55.50.PM.dmp.html
    #should be
    #file://///lucidfs/public
    
    
    for c in sorted_crash_map:  
        htmlcode_highlevel+= '<TR>\n'
        htmlcode_highlevel+= '<TD> <FONT COLOR = "000000">\n'
        htmlcode_highlevel+= c[0]
        htmlcode_highlevel+= " - %s" % getKitPathFromSvnPath(getBranchFromSvnNum(int(c[0])))
        htmlcode_highlevel+= '</COLOR>'
        htmlcode_highlevel+= '</TD>\n'
        htmlcode_highlevel+= '<TD>\n'
        for e in c[1]:
            htmlcode_highlevel+= '<FONT COLOR = "000000"><br>'        
            #htmlcode_highlevel+= e
            
            htmlcode_highlevel+= "<a title='%s' class=body_con href=\"file:///%s\">""%s""</a>" % ('tooltip',e,e)
            
            htmlcode_highlevel+= '</COLOR>'
            htmlcode_highlevel+= '<br>'
        htmlcode_highlevel+= '</TD>\n'
        htmlcode_highlevel+= '</TR>\n'
        
    
    
    htmlcode_highlevel+= ' </FONT>\n'
    htmlcode_highlevel+= ' </TABLE>\n'
    
    outf = open(dumps_path+'dumps.html','wt')
    outf.write(htmlcode_highlevel)
    outf.close()
    
    
    #CreateSpecificCrashHTML('16580@LUCID145@ArmA2Demo.exe@06.04.2011@04.08.38.PM.dmp.txt')

#debug
#Process()
    