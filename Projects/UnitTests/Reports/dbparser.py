#!/usr/bin/python

import pymongo
import bson

import sys
arguments = sys.argv[1:]

if 0 == len(arguments) or 1 != len(arguments):
	print 'Usage: ./dbparser.py [PlatformName]'
	exit(1)

platformName = arguments[0]
# platformName = 'MacOS'
# platformName = 'iPhone'
# platformName = 'Win32'
# platformName = 'Android'


def LogError(message):
	message = message.replace("|", "||")

	message = message.replace("'", "|'")
	message = message.replace("\n", "|n")
	message = message.replace("\r", "|r")

	message = message.replace("\u0085", "|x")
	message = message.replace("\u2028", "|l")
	message = message.replace("\u2029", "|p")

	message = message.replace("[", "|[")
	message = message.replace("]", "|]")

	print "##teamcity[message text='" + message + "' errorDetails='' status='ERROR']"

# connection = pymongo.Connection("localhost", 27017)
connection = pymongo.Connection("10.128.128.131", 27017)

if None != connection:
	#set database
	db = connection['UnitTests']

	#set collection
	collection = db['UnitTestsResult']

	#get cursor for collection
	platform = collection.find_one({'_id': platformName})
	if None != platform:
		testNames = platform.keys()
		testNames.sort()
		
		count = len(testNames)
		index = 0

		errorWasFound = 0
		usedTestName = ""
		for testName in testNames:
			if count - 2 == index:
				usedTestName = testName
				testData = platform[testName]
				errorNames = testData.keys()
				errorNames.sort()
				for errorName in errorNames:
					if 'TestResult' == errorName and 'All test passed.' == testData[errorName]:
						break
					elif '_id' != errorName:
						LogError(errorName + ": " + testData[errorName])
						errorWasFound = 1

				break
				
			index = index + 1
			
		if 1 == errorWasFound:
			LogErrror('Errors at test: ' + usedTestName)
			sys.stdout.flush()
			exit(1)
		else:
			print "##teamcity[message text='All tests passed.' errorDetails='' status='NORMAL']"
			
			sys.stdout.flush()
			exit(0)
		

	else:
		LogError("There are no tests for platform " + platformName + " at collection " + collectionName)
else:
	LogError("Can't connect to Database")
