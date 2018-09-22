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

#include <ctype.h>
#include <string.h>

#include "simplexml.h"
#include "../CEMulatorCtrl.h"


#undef debug_printf
#define debug_printf(X,...) 

 const char* lastxmlerror=NULL;

simplexml::simplexml(const char *encoded, const simplexml *parent)
{
	const char *sptr, *tptr /*, *uptr*/;
	char *buf, *end, lastc;
	long len;
	keyvalue_rec *ptr;

	_parent = parent;

	_key = NULL;
	_value = NULL;
	_properties = NULL;
	_children = NULL;

	ptr = NULL;

	// decode the xml string

	sptr = strchr(encoded,'<');
	if (sptr==NULL)
		return;

	if (!parent && strncmp(sptr,"<?xml",5))
	{
		printf("not an xml file!");
		return ;
	}
	
	
	while(1)
	{
	// OG ignore encoding
	if (sptr[1]=='?')
	{
		sptr++;
		while(*sptr &&  *sptr!='<')
			sptr++;

		if (!*sptr)
		{
			printf("error!");
			return ;
		}
	}
	else 
		break;
	}


	sptr++;
	len = ntotok(sptr,"> \t\r\n"); // og
	if (sptr[len-1]=='/')
		len--;
	
	
	_key = new char[len+1];
	strncpy(_key,sptr,len);
	
//	_key = (char*)sptr;
	_key[len] = 0;
	
	
	

	debug_printf("found tag:%s\n",_key);
	
	sptr += len;
	lastc = sptr[0];
	//OG Manage autoclosing tag before entering the parsing loop
	if (lastc=='/')
		return ;

	/** Parse parameters ************************************************/
	while (sptr && sptr[0] && lastc != '>' /*&& lastc != '/'*/) {
		len = nskiptok(sptr,"> \t\r\n");
		lastc = sptr[len-1];
		sptr += len;


		if (lastc=='>' || sptr==NULL)
			break;
/*
		if (lastc=='/' && sptr[0]=='>') {
			sptr += 2;
			break;
		}
*/
		if (sptr[0]=='/' && sptr[1]=='>')
		{
			return;
			/*
			sptr+=2;
			break;
			 */
		}

		len = ntotok(sptr,"=");
		if (sptr[len]=='=' && sptr[len+1]) {
			if (_properties==NULL) {
				_properties = new keyvalue_rec;
				ptr = _properties;
			} else {
				ptr->next = new keyvalue_rec;
				ptr = ptr->next;
			}
			ptr->next = NULL;
			long  l = ntotok(sptr,"\r\n\t =");
			ptr->key = new char[l+1];
			strncpy(ptr->key, sptr, l);
			ptr->key[l] = 0;

			debug_printf("found property:[%s]\n",ptr->key);

			sptr += len+1;
			len = nskiptok(sptr," \t\r\n");
			sptr+=len;
			if (sptr[0]=='"') {
				sptr++;
				len = ntotok(sptr,"\"");
				ptr->value = new char[len+1];
				strncpy((char *)ptr->value,sptr,len);
				((char *)ptr->value)[len] = 0;
				sptr += len+1;
			} else {
				delete ptr->key;
				ptr->key=NULL;
				ptr->value=NULL;
				SETXMLERROR("misformed xml 3\n");
				return ;
				/*
				len = ntotok(sptr,"> \t\r\n");
				ptr->value = new char[len+1];
				strncpy((char *)ptr->value,sptr,len);
				((char *)ptr->value)[len] = 0;
				sptr += len;
				*/
			}
	
			debug_printf("found property value:%s\n",ptr->value);

			// OG avance
			len = nskiptok(sptr," \t\r\n");
			sptr += len;

			lastc = sptr[0];
			/*
			if (sptr)
			{
				lastc = sptr[0];
			}
			*/
		}
	}

	// Empty tag?  <foo/>
	if (lastc=='/') {
		return;
	}
/*
	if (lastc=='>') {
#ifdef _DEBUG	
		printf("ignore\n");
#endif
	//	sptr++;
	}
*/
	len = nskiptok(sptr,"> \t\r\n");

	if (!sptr[len])
		return;

	debug_printf("%c%c%c%c%c\n",sptr[len],sptr[len+1],sptr[len+2],sptr[len+3],sptr[len+4]);
	// skip comments before determining tag type
	while (strncasecmp(&sptr[len],"<!--",4)==0) {
		sptr = stristr(&sptr[len],"-->") + 3;
		ASSERT(sptr);
		len = 0;
	}

	if (sptr[len]=='<') {

		debug_printf("key:%s",_key);
		sptr += len;
		buf = new char[strlen(_key) + 4]; // 4 => "</>\0"
		sprintf(buf,"</%s>",_key);

		while (sptr && strncasecmp(sptr,buf,strlen(buf))) {
			tptr = strchr(sptr,'<');
			if (tptr==NULL) {
				delete[] buf;
				return;
			}
			tptr++;
			len = ntotok(tptr,"> \t\r\n");
			
			// OG Autoclosing Tagfix
			if (tptr[len-1]=='/')
				len--;
			
			if (tptr[0]=='/')
			{
				SETXMLERROR("misformed xml 1");
				return ;
			}
			char* keyn =  (char*)new char[len+1];
			strncpy(keyn,tptr,len);
			keyn[len] = 0;
			
			if (!strcmp(keyn,"![CDATA["))
			{
				delete keyn,
				keyn=NULL;
				// trim
				tptr += 8; //strlen("![CDATA[");
				
				const char* ed = stristr(tptr,"]]>");
				if (!ed) 
					printf("CDATA does not end");
				
				int trimleft = nskiptok(tptr," \t\r\n");
				tptr+=trimleft;
				int len2 = ed-tptr;
				_value = new char[len2+1];
				strncpy(_value,tptr,len2);
				_value[len2] = 0;
				debug_printf("found CDATA value: %s",_value);

				// closing tag
				delete buf;
				buf=NULL;

				tptr = ed+3;
				sptr = tptr;
				// skip
				return ;
			}
			else 
			if (!strcmp(keyn,"!--"))
			{
				delete keyn;
				keyn=NULL;
				debug_printf("found comment");
				const char* ed = stristr(tptr,"-->");
				if (!ed) 
					printf("comment does not end");
				
				tptr = ed+3;
				sptr = tptr;
				// skip
				//return ;
				
			}

			else
			{
			
				if (_children==NULL) {
					_children = new keyvalue_rec;
					ptr = _children;
				} else {
					ptr->next = new keyvalue_rec;
					ptr = ptr->next;
				}
				ptr->next = NULL;
			
				ptr->key = keyn;
			
				
				debug_printf("adding key:%s\n",ptr->key);
				ptr->value = new simplexml(sptr,this);
			
			
			// OG Autoclosing Tag Fix
			int l = ntotok(sptr+1,"><");
			if ( (sptr[l]=='/') && (sptr[l+1]=='>') )
			{
				// autoclosing tag)
				sptr += l+2;
			}
			else
			{
				end = new char[len + 4]; // 4 => "</>\0"
				sprintf(end,"</%s>",keyn);

				sptr = stristr(sptr,end);
				if (sptr)
				{
					sptr += strlen(end);
					delete[] end;
				}
				else
				{
					delete[] end;
					SETXMLERROR("misformed XML\n");
					return ;
				}
			}
			}

			sptr = strchr(sptr,'<');
		}

		delete[] buf;

	} else {

		// OG skip trailing space
		sptr+=len;

		buf = new char[strlen(_key) + 4]; // 4 => "</>\0"
		sprintf(buf,"</%s>",_key);
		tptr = stristr(sptr,buf);
//		ASSERT(tptr);
		delete[] buf;

		if (tptr==NULL) 
			len = strlen(sptr);
		else 
			len = tptr - sptr;

		len = removeendingspace(sptr,len);
		_value = new char[len+1];
		strncpy(_value,sptr,len);
		_value[len] = 0;

		debug_printf("found value:%s\n",_value);
	}
}

// trim ending space
int removeendingspace(const char *str,int len)
{
	if (!str) return 0;
	while(len)
	{
		if (strchr(" \t\r\n",str[len-1])==NULL)
			break;
		len--;
	}
	return len;
}

long ntotok(const char *str, const char *tokens)
{
	long i;

	if (str==NULL)
		return -1;

	if (tokens==NULL)
		return 0;

	for (i=0; str[i] && strchr(tokens,str[i])==NULL; i++);

	return i;
}

long nskiptok(const char *str, const char *tokens)
{
	long i;

	if (str==NULL)
		return -1;

	if (tokens==NULL)
		return 0;

	const char* p;
	for (i=0; str[i] && (p=strchr(tokens,str[i]))!=NULL; i++);
/*
	for (i=0; str[i] ; i++)
	{
		const char* p = strchr(tokens,str[i]);
		if (p!=NULL) break;
	}
*/
	return i;
}

const char *stristr(const char *haystack, const char *needle)
{
        long i, n;

        for (i=0; haystack[i]; i++) {
                if (tolower(haystack[i])==tolower(needle[0])) {
                        for (n=0;
                                haystack[i+n] && needle[n]
                                && tolower(haystack[i+n])==tolower(needle[n]);
                                        n++);
                        if (n==strlen(needle))
                                return(&haystack[i]);
                }
        }
        return NULL;
}

simplexml::~simplexml(void)
{
	keyvalue_rec *ptr,*last;

	if (_key!=NULL) {
		delete[] _key;
	}

	if (_value!=NULL) {
		delete[] _value;
	}

	if (_properties!=NULL) {
		for (ptr=_properties->next, last=_properties;
			ptr!=NULL;
				last=ptr, ptr=ptr->next) {
			delete[] last->key;
			delete[] (char *) last->value;
			delete last;
		}
		delete[] last->key;
		delete[] (char *) last->value;
		delete last;
	}

	if (_children!=NULL) {
		for (ptr=_children->next, last=_children;
			ptr!=NULL;
				last=ptr, ptr=ptr->next) {
			delete[] last->key;
			delete (simplexml *) last->value;
			delete last;
		}
		delete[] last->key;
		delete (simplexml *) last->value;
		delete last;
	}
}

const simplexml *simplexml::parent(void)
{
	return((const simplexml *) _parent);
}

const char *simplexml::key(void)
{
	return((const char *) _key);
}

const char *simplexml::value(void)
{
	return((const char*) _value);
}

int simplexml::number_of_properties(void)
{
	int i;
	keyvalue_rec *ptr;

	for (i=0, ptr=_properties; ptr!=NULL; i++, ptr=ptr->next);

	return i;
}

const char *simplexml::property(int property_number)
{
	int i;
	keyvalue_rec *ptr;

	for (i=0, ptr=_properties; 
		i!=property_number && ptr!=NULL; 
			i++, ptr=ptr->next);

	if (ptr)
		return (const char *) ptr->value;
	else
		return NULL;
}

const char *simplexml::property(const char *key, int iter)
{
	int i;
	keyvalue_rec *ptr;

	for (ptr=_properties; ptr!=NULL; ptr=ptr->next) {
		if (strcmp(ptr->key,key)==0) {
			for (i=0; i<iter && ptr!=NULL; i++, ptr=ptr->next);
			if (ptr)
				return (const char *) ptr->value;
			else
				return NULL;
		}
	}
	return NULL;
}

int simplexml::number_of_children(void)
{
	int i;
	keyvalue_rec *ptr;

	for (i=0, ptr=_children; ptr!=NULL; i++, ptr=ptr->next);

	return i;
}

simplexml *simplexml::child(int child_number)
{
	int i;
	keyvalue_rec *ptr;

	for (i=0, ptr=_children; 
		i!=child_number && ptr!=NULL; 
			i++, ptr=ptr->next);

	if (ptr)
		return (simplexml *) ptr->value;
	else
		return NULL;
}

simplexml *simplexml::child(const char *key, int iter)
{
	int i;
	keyvalue_rec *ptr;

	for (ptr=_children; ptr!=NULL; ptr=ptr->next) {
		if (strcmp(ptr->key,key)==0) {
			for (i=0; i<iter && ptr!=NULL; i++, ptr=ptr->next);
			if (ptr)
				return (simplexml *) ptr->value;
			else
				return NULL;
		}
	}
	return NULL;
}
