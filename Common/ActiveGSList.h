/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#pragma once
#include <vector>

#define MAX_SCREENSHOTS 10
#include "../Common/3rdpartylib/simplexml.h"
#include "../kegs/Src/StdString.h"
#include "../kegs/Src/defc.h"
#include "../Common/ki.h"



struct ActiveGSElement
{
	
	MyString theid;
	MyString thumb;

#define MAX_SCREENSHOTS 10
	MyString screenShots[MAX_SCREENSHOTS];
	
	simplexml* pXML;
	int		is2GS;

	
	MyString	uid;
	MyString	name;
	MyString	nameurl;
	MyString	desc;
	MyString	year;
	MyString	publisher;
	MyString	publisherurl;

	MyString	visibleName;

	
	int fillFromXML(simplexml* ptrconfig);
};

class ActiveGSList
{
public:
	MyString	pathName;
    MyString    trackerName;
	MyString	sourceName;
	MyString	sourceRevision;


	simplexml*	rootXML;
	std::vector<ActiveGSElement> elements;

	ActiveGSList();
	~ActiveGSList();
	int loadFromFile(const char* _file);
	int processString(const char* _str);
    void reset();
};
