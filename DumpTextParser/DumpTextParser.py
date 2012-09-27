#!/usr/bin/env python

#TODO:
# 1. Module name is missing - i should probably pipe it
# 2. add #System Uptime
# 3. add #Process Uptime

class DumpTextParser:
    
    init = False
    
    process_name = "not_found"
    faulting_ip_call = "not_found"
    faulting_ip_assembly_instruction = "not_found"
    faulting_thread_ID = -1
    stack_text = []
    exception_code = -1
    exception_code_description = "not_found"
    Thread_By_Index_Map = {}
    Thread_By_ID_Map = {}
    all_threads_map = {} #(thread_ID,list of function calls)
    lucid_threads = []
    system_uptime = 0.0
    process_uptime = 0.0 
    
    # we want to collect more than one "sequence" (ends with empty line)
    ReOccuringData = [\
        "Teb:",
        ]
    
    OneLine = [\
        "System Uptime:",\
        "Process Uptime:",\
        ]
    
    # we want to collect at least one "sequence" (ends with empty line)
    CollectData = [\
        "FAULTING_IP",\
        "ExceptionCode",\
        "PROCESS_NAME",\
        "FAULTING_IP",\
        "FAULTING_THREAD",\
        "STACK_TEXT",\
        "EXCEPTION_CODE:",\
        "runaway",\
        "Teb:",\
        "System Uptime:",\
        "Process Uptime:",\
        ]
    
    def __init__(self):
        ################# processed  data
        pass

    def _GenerateLineIdentifier(self,line):
        return (lambda l: line in l)
      
    def _IsEmptyLine(self,line):
        if not line.strip():
           return True
        return False

    def _SingleLine(self,val):
        for el in self.OneLine:
            if el == val:
                return True
        return False

    def _ShouldStartCollectData(self,line):
        for el in self.CollectData:
            if el in line:
                return el
        return None
    def _TimeStrToFloat(self,str):
        time_split = str.split(":")
        time = float(time_split[2])
        time += float(time_split[1])*60.0
        time += float(time_split[0])*60.0*60.0
        return time
    #find threads that lucid is found in the call stack
    def _IsLucidInCallStack(self,callstack):
        for l in callstack:
            if "lucid" in l:
                return True
            if "sandbox_" in l:
                return True
        return False
    
    def CreateRepresentingString(self):
        if (self.init == False):
            print "You need to use 'ReadFile' first !"
            return ""
        
        representing_string = ""                 
        
        #representing_string+= "Process Name = %s\n" % self.process_name
        representing_string+="faulting_ip_call = %s\n" % self.faulting_ip_call
        representing_string+="faulting_ip_assembly_instruction = %s\n" % self.faulting_ip_assembly_instruction
        #representing_string+="faulting_thread = 0x%X\n" % (self.faulting_thread_ID)
        representing_string+="process uptime %.2f\n" % self.process_uptime
        representing_string+="system uptime %.2f\n" % self.system_uptime
        
        #print "exception stack:"
        #for el in self.stack_text:
        #    print el[:-1]
                       
        representing_string+="exception code = 0x%X\n" % (self.exception_code)
        representing_string+="exception code description = %s\n" % self.exception_code_str
        #print "Displaying time data for %d found Threads:"
        #for key,val in self.Thread_By_Index_Map.items():
        #    print "Thread %d) (ID = %X) time = %f seconds" % (key,val[0],val[1])
            
        #for t_ID in self.lucid_threads:
        #    print "Thread 0x%X" %t_ID
        #    val = self.all_threads_map[t_ID]
        #    for call in val:
        #        print call
        
        representing_string+="faulting thread stack:"
        for el in self.all_threads_map[self.faulting_thread_ID]:
            representing_string+=el
            representing_string+="\n"


        return representing_string
        
    
    # main function
    def ReadFile(self,input_log_file):
        #init all vars
        self.process_name = "not_found"
        self.faulting_ip_call = "not_found"
        self.faulting_ip_assembly_instruction = "not_found"
        self.faulting_thread_ID = -1
        self.stack_text = []
        self.exception_code = -1
        self.exception_code_description = "not_found"
        self.Thread_By_Index_Map = {}
        self.Thread_By_ID_Map = {}
        self.all_threads_map = {} #(thread_ID,list of function calls)
        self.lucid_threads = []
        self.system_uptime = 0.0
        self.process_uptime = 0.0
        
        #load and parse the file
        file = open(input_log_file)
            
        collect = False
        dict = {}
        accum_lines = []
        block_name = None
        
        for line in file.xreadlines():
            #print block_name
            if (self._IsEmptyLine(line) or self._SingleLine(block_name)):
                if (collect == True):
                    #if (block_name == None):
                    #    print "Error! block_name is None"
                    dict[block_name] = accum_lines
                    
                    accum_lines = []
                    #print "Collect Stopped"
                    collect = False
                    
            curr_block_name = self._ShouldStartCollectData(line)        
            
            if (curr_block_name != None):
                block_name = curr_block_name
                #print "Collect Started for [%s]" % block_name
                collect = True
                
            #elif (GenerateLineIdentifier("{m}/--\{m} Dumping All Stacks")(line) == True):
            #    print line
            #elif (GenerateLineIdentifier("{m}/--\{m} Displaying thread times")(line) == True):
            #    print line
            
            if (collect == True):
                #print line
                accum_lines.append(line)                
        
        
        # Harvesting data
        
        
        ##################################################################
        ## process name
        
        if ("PROCESS_NAME" in dict.keys()):
            self.process_name  = dict["PROCESS_NAME"][0].split()[1]
                            
        ##################################################################
        ## Faulting IP
        
        if ("FAULTING_IP" in dict.keys()):
            self.faulting_ip_call = dict["FAULTING_IP"][1][:-1] #also removing the \n    
            self.faulting_ip_assembly_instruction = dict["FAULTING_IP"][2][:-1] #also removing the \n    
   
        ##################################################################    
        ## FAULTING_THREAD
        

        if ("FAULTING_THREAD" in dict.keys()):
            faulting_thread_str = dict["FAULTING_THREAD"][0].split()[1]
            #print faulting_thread_str
            self.faulting_thread_ID = int(faulting_thread_str,16)     
       
        
        ##################################################################    
        ## STACK_TEXT
        
        if ("STACK_TEXT" in dict.keys()):
            self.stack_text = dict["STACK_TEXT"][1:]
            #print faulting_thread_str    
                
        ##################################################################    
        ## ExceptionCode
        
        
        if ("ExceptionCode" in dict.keys()):
            exception_code_str = dict["ExceptionCode"][0].split()[1]
            self.exception_code = int(exception_code_str,16)
        
        ##################################################################    
        ## EXCEPTION_CODE:
        
        if ("EXCEPTION_CODE:" in dict.keys()):
            self.exception_code_str = dict["EXCEPTION_CODE:"][0][len("EXCEPTION_CODE:"):-1]


        ###################################################################
        ## process and system uptime
        if ("System Uptime:" in dict.keys()):
            self.system_uptime = self._TimeStrToFloat(dict["System Uptime:"][0].split()[4])
        if ("Process Uptime:" in dict.keys()):
            self.process_uptime = self._TimeStrToFloat(dict["Process Uptime:"][0].split()[4])

        ##################################################################    
        ## Thread Times (+plus getting mapping of dbgend thread index and ID)
        
        
        
        Threads_Times = []
        if ("runaway" in dict.keys()):
            Threads_Times = dict["runaway"][3:]
                    
        #   0:c98       0 days 0:00:02.823
        for el in Threads_Times:
            thread_index_str = el.split()[0].split(":")[0]
            thread_ID_str = el.split()[0].split(":")[1]
            thread_index = int(thread_index_str)
            thread_ID = int(thread_ID_str,16)
                
            time = self._TimeStrToFloat(el.split()[3])
            
            self.Thread_By_Index_Map[thread_index] = (thread_ID,time)
            self.Thread_By_ID_Map[thread_ID] = (thread_index,time)
                    
        # now, get the stack information for all stacks
        
        file.close()
        #repoen
        file = open(input_log_file)
        
        collect = False
        thread_stack      = [] #thread stack
        thread_ID = -1
        
        for line in file.xreadlines():
            if (collect == True) and self._IsEmptyLine(line):
                self.all_threads_map[thread_ID] = thread_stack
                thread_stack = []
                continue
            if "Teb:" in line:
                collect = True
                loc = line.find(".")
                if (loc<0):
                    print "Error! can't parse thread stack"
                thread_ID = int(line[loc+1:].split()[0],16)
                continue
            if "ChildEBP" in line:
                continue        
            if (collect == True) and ("Done" in line) :
                collect = False
            if collect == True:
                loc = line.find(" ")
                loc = line.find(" ",loc+1)
                thread_stack.append(line[loc+1:-1])
                      
            
        for key,val in self.all_threads_map.items():
            if self._IsLucidInCallStack(val):
                self.lucid_threads.append(key)
        
        file.close()   
        print "Done."
        self.init = True
    
    
