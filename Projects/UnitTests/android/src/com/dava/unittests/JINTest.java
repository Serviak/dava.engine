package com.dava.unittests;

public class JINTest {

	public static boolean PassString(String string)
	{
		return true;
	}
	
	public static int PassStringArray(String[] strings)
	{
		int counter = 0;
		for(int i = 0; i < strings.length; i++)
		{
			counter = i+1;
		}
		return counter;
	}
	
}
