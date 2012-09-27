#!/usr/bin/env python
from DumpTextParser import *

input_1 = "Dump.txt"
input_2 = "Dump2.txt"

dtp_1 = DumpTextParser()
dtp_1.ReadFile(input_1)
rep_str_dtp_1 = dtp_1.CreateRepresentingString()

dtp_2 = DumpTextParser()
dtp_2.ReadFile(input_2)
rep_str_dtp_2 = dtp_2.CreateRepresentingString()

import Levenshtein

strDistance = Levenshtein.ratio(rep_str_dtp_1,rep_str_dtp_2)

print "111111111111111111111111111111111"
print rep_str_dtp_1

print "222222222222222222222222222222222"
print rep_str_dtp_2

print "String Distance = %f - between [%s] and [%s]" % (strDistance,input_1,input_2)

t = 1
t += 1

    