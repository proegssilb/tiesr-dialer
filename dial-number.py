#!/usr/bin/env python

"""A script to dial a phone number via Google Voice.

This script will attempt to match its first command-line argument against a
regular expression, and if this works, dial the number.
"""

#re for regular expressions
#sys for accessing command-line arguments
import re, sys, getpass

#And Google Voice API.
import googlevoice as gv
from googlevoice.util import input

#Our magic regular expression
#    (A 7, 10, or 11 digit number)
regex = "(\A\d{10,11}$|\A\d{7}$)"

#Remove all non-numeric characters
#    -"\D" matches a single non-numeric character.
#    -re.sub replaces all matches of the first string (in the third string)
#        with the second string.
subs = re.sub("\D", '', sys.argv[1])

#Find all phone numbers, and keep the first one.
res = re.findall(regex, subs)[0]

#Create a Google Voice API object, login, and dial the number.
v = gv.Voice()
v.login()
v.call(res, v.phones[0])

