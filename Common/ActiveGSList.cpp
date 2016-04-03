/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#include "ActiveGSList.h"
#include "activedownload.h"

ActiveGSList::~ActiveGSList()
{
	if (rootXML)
	{
		delete rootXML ;
		rootXML = NULL;
	}
	
}

void ActiveGSList::reset()
{
    elements.clear();
    sourceName.Empty();
    sourceRevision.Empty();
    pathName.Empty();
    
    if (rootXML)
    {
        delete rootXML;
        rootXML=NULL;
    }
}

ActiveGSList::ActiveGSList()
{
    rootXML = NULL;
    reset();
}

int ActiveGSList::loadFromFile(const char* _url)
{
	MyString path;
	MyString shortname;
	
    CDownload dl("*absolute*");
    dl.bNotifyDownloadFailure = false; // sera traité avec l'existence du fichier
    dl.setRequiredExt("activegsxml");
    
	if (!dl.retrieveFile(_url,path,shortname))
	{
		printf("could not load config  %s\n",_url);
		return 0;
	}

	printf("processing config  %s\n",_url);
	pathName = _url;

	FILE* f = fopen(path.c_str(),"rb");
	if (!f)
	{
		printf("cannot open config%s\n",path.c_str());
		return 0 ;
	}
	fseek(f,0,SEEK_END);
	int si = ftell(f);
	if (!si)
	{
		fclose(f);
		printf("cannot open config%s (file empty)\n",path.c_str());
		return 0;
	}
	fseek(f,0,SEEK_SET);
	char* s = new char[si+1];
	memset(s,0,si+1);
	fread(s,1,si,f);
	fclose(f);

	int b = processString(s);

	delete s;
	return b;
}


int ActiveGSList::processString(const char* _str)
{

	RESETXMLERROR();
	simplexml* root= new simplexml(_str);	
	if (GETXMLERROR())
	{
		printf("bad xml %s\n",GETXMLERROR());
		return 0;
	}

	if (!root->key())
	{
		printf("xml empty\n");
		return 0 ;
	}
	// regarde si c'est un type list
	if (!strcasecmp(root->key(),"list"))
	{
		
		int child = root->number_of_children();
		for(int i=0;i<child;i++)
		{
			simplexml* ptr = root->child(i);
			if (!strcasecmp(ptr->key(),"source"))
			{
				const char* name = ptr->value();
				if (name)
					sourceName = name;
				const char* revision = ptr->property("revision");
				if (revision)
					sourceRevision = revision;
			}
			else
			if (!strcasecmp(ptr->key(),"config"))
			{
				ActiveGSElement el ;
				if ( el.fillFromXML(ptr) )
					elements.push_back (el);
			}
		}
	}
	else
	{
		// 1 seul
		ActiveGSElement el ;
		if ( el.fillFromXML(root) )
			elements.push_back (el);
	}
	
	rootXML= root;
	return 1;
	
}



int ActiveGSElement::fillFromXML(simplexml* ptrconfig)
{
static int baseid=0;

	const char* ver = ptrconfig->property("version");
	if (!ver || atoi(ver)!=2)
	{
		printf("invalid xml version - xml config skipped\n");
		return  0;
	}

    
    // default Apple2
    is2GS = 0;
    
	// attribue un id

	const char* _id = ptrconfig->property("id");
	if (_id)
		theid = _id;
	else
		theid.Format("Image%d",baseid++);

			
	int child = ptrconfig->number_of_children();
	for(int i=0;i<child;i++)
	{
		simplexml* ptr = ptrconfig->child(i);
		
		if (!strcasecmp(ptr->key(),"name"))
		{
			const char* s = ptr->value();
			name = s;
		}
		else
		if (!strcasecmp(ptr->key(),"desc"))
		{
			const char* s = ptr->value();
			desc = s;
		}
		else
		if (!strcasecmp(ptr->key(),"publisher"))
		{
			const char* s = ptr->value();
			publisher = s;
		}
		else
		if (!strcasecmp(ptr->key(),"year"))
		{
			const char* s = ptr->value();
			year = s;
		}
		else
		if (!strcasecmp(ptr->key(),"pic"))
		{
			const char* type = ptr->property("type");
			if (type && !strcasecmp(type,"thumbnail"))
			{
				const char* fn = ptr->value();
				if (fn && strcasecmp(fn,"http://www.virtualapple.org/images/vascreen1.jpg"))
					thumb = ptr->value();
			}
			else
			if (type && !strcasecmp(type,"screenshot"))
			{
				const char* fn = ptr->value();
			
				for(int i=0;i<MAX_SCREENSHOTS;i++)
				{
                    if (screenShots[i].GetLength()==0)
					{
						screenShots[i] = fn;
						break;
					}
				}
			}
		}
		else 
		if (!strcasecmp(ptr->key(),"format"))
		{
			const char* type = ptr->value();
			if (type && ( !strcasecmp(type,"2GS") || !strcasecmp(type,"GS") ) )
				is2GS = 1;
		}
	}
	pXML = ptrconfig;
	return 1;
}
