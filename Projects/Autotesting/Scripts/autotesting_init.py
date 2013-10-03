#!/usr/bin/python
#
#  autotesting_init.py
#  DAVA SDK
#
#  Created by Dmitry Shpakov on 6/13/12.
#  Copyright (c) 2008 DAVA Consulting, LLC. All rights reserved.
#

import os;
import sys;
import os.path;
import string;
import platform;
import shutil;
import subprocess;
import filecmp;

arguments = sys.argv[1:]

if len(arguments) != 3:
    print 'Usage: ./autotesting_init.py PlatformName ProjectName ProjectFolder'
    exit(1)

def copy_file(srcFolder, destFolder, fileName):
    fileSrcPath = os.path.realpath(srcFolder + "/" + fileName)
    fileDestPath = os.path.realpath(destFolder + "/" + fileName)
    if os.path.exists(fileDestPath):
        print "delete " + fileDestPath
        os.remove(fileDestPath)
    print "copy " + fileSrcPath + " to " + fileDestPath
    shutil.copy(fileSrcPath, fileDestPath)
    
print "*** DAVA Initializing autotesting"
platformName = arguments[0]
projectName = arguments[1]
projectFolder = arguments[2]

print "platform.system: " + platform.system()

currentDir = os.getcwd(); 
frameworkDir =  os.path.realpath(currentDir + "/../../../")
projectDir = os.path.realpath(currentDir + "/../../../../" + projectFolder)
print "Framework directory:" + frameworkDir
print "Project directory:" + projectDir


# Change Config.h ot turn on AUTOTESTING mode
if 2 <= len(arguments):
    autotestingConfigSrcPath = os.path.realpath(currentDir + "/../Data/Config.h")
    autotestingConfigDestPath = os.path.realpath(frameworkDir + "/Sources/Internal/Autotesting/Config.h")
    if os.path.exists(autotestingConfigDestPath):
        if filecmp.cmp(autotestingConfigSrcPath, autotestingConfigDestPath):
            print "skip copy Config.h - dest file exists and equal to src file"
        else:
            print "delete " + autotestingConfigDestPath
            os.remove(autotestingConfigDestPath)
            print "copy " + autotestingConfigSrcPath + " to " + autotestingConfigDestPath
            shutil.copy(autotestingConfigSrcPath, autotestingConfigDestPath)
    else:
        print "copy " + autotestingConfigSrcPath + " to " + autotestingConfigDestPath
        shutil.copy(autotestingConfigSrcPath, autotestingConfigDestPath)

autotestingSrcFolder = os.path.realpath(projectDir + "/Autotesting")
autotestingDestFolder = os.path.realpath(projectDir + "/Data/Autotesting")
    
scripts = ["/generate_id.py", "/copy_tests.py"]

if (platform.system() == "Darwin"):
    if (platformName == "iOS"):
        scripts.append("/runOnDevice.sh")
        scripts.append("/floatsign.sh")
        scripts.append("/packipa.sh")
        scripts.append("/transporter_chief.rb")
        scripts.append("/testRun.js")

autotestingReportsFolder = os.path.realpath(autotestingSrcFolder + "/Reports")      
# Remove as depricated
if os.path.exists(autotestingReportsFolder):   
    print "remove previous report folder: " + autotestingReportsFolder       
    shutil.rmtree(autotestingReportsFolder)

    
print "copy scripts from " + currentDir + " to " + autotestingSrcFolder
for scriptName in scripts:
    scriptSrcPath = os.path.realpath(currentDir + scriptName)
    scriptDestPath = os.path.realpath(autotestingSrcFolder + scriptName)
    if os.path.exists(scriptDestPath):    
        print "delete " + scriptDestPath
        os.remove(scriptDestPath)
    print "copy " + scriptSrcPath + " to " + scriptDestPath
    shutil.copy(scriptSrcPath, scriptDestPath)

if os.path.exists(autotestingDestFolder):    
    print "Autotesting already exists - delete " + autotestingDestFolder
    shutil.rmtree(autotestingDestFolder)

os.mkdir(autotestingDestFolder)

luaScriptDestFolder = os.path.realpath(autotestingDestFolder + "/Scripts")
os.mkdir(luaScriptDestFolder)

copy_file(currentDir, luaScriptDestFolder, "autotesting_api.lua")
copy_file(currentDir, luaScriptDestFolder, "coxpcall.lua")

os.chdir(projectDir)

params = ["python", "./Autotesting/copy_tests.py"]
print "subprocess.call " + "[%s]" % ", ".join(map(str, params))
subprocess.call(params)

params = ["python", "./Autotesting/generate_id.py", projectName]
print "subprocess.call " + "[%s]" % ", ".join(map(str, params))
subprocess.call(params)

os.chdir(currentDir)
   
print "*** DAVA Initialized autotesting"