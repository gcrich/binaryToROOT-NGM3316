#
# parseAllBinFilesInDirectory-NGM3316.py
#
# created by g.c. rich and inflicted upon others 10-28-14
#
# this script is intended to identify all binary files in a directory
# which appear to be 3316-generated
# upon identification, the script will invoke the parser for each binary

import subprocess

filesToParse = []

directoryListing = subprocess.Popen( 'ls -1', shell=True, stdout=subprocess.PIPE )
for directoryEntry in directoryListing.stdout.readlines():
    # first, remove trailing new-line character
    directoryEntry = directoryEntry.rstrip('\n')

    extension = directoryEntry[directoryEntry.rfind('.') + 1:]
    
    if extension == 'bin':
        filesToParse.append( directoryEntry )

print 'Identified the following binary files to parse..'
for file in filesToParse:
    print file

for file in filesToParse:
    print 'Parsing file ' + file
    subprocess.call( './binaryToROOT-NGM3316 ' + file, shell=True )