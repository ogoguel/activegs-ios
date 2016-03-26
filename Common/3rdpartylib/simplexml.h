/*****************************************************************************

    This file is part of SimpleXML, a simplified C++ tree based parser
    of XML 1.0 documents.
    Copyright (C) 2003 by Brian Ecker.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*****************************************************************************/

#ifndef SIMPLEXML_H
#define SIMPLEXML_H

#include <stdio.h>

#if defined(WIN32) && !defined(UNDER_CE)
 #define strncasecmp _strnicmp
 #define strcasecmp _stricmp
#endif 

extern const char* lastxmlerror;
#define SETXMLERROR(STR) lastxmlerror=STR;
#define RESETXMLERROR() lastxmlerror=NULL;
#define GETXMLERROR() lastxmlerror
typedef struct keyvalue {

  char *key;
  void *value;

  struct keyvalue *next;

} keyvalue_rec;


class simplexml 
{
public:
	simplexml( const char *encoded, const simplexml *parent=NULL);
	~simplexml(void);

	const simplexml *parent(void);

	const char *key(void);
	const char *value(void);

	int number_of_properties(void);
	const char *property(int property_number);
	const char *property(const char *key, int iter=0);
	keyvalue_rec* properties() { return _properties; };

	int number_of_children(void);
	simplexml *child(int child_number);
	simplexml *child(const char *key, int iter=0);
	
//	simplexml::print(MyString& _output);
	
private:
	const simplexml *_parent;

	char *_key;
	char *_value;

	keyvalue_rec *_properties;
	keyvalue_rec *_children;
};

long ntotok(const char *str, const char *tokens);
long nskiptok(const char *str, const char *tokens);
const char *stristr(const char *haystack, const char *needle);
int removeendingspace(const char *str,int len);

#endif /* SIMPLEXML_H */
