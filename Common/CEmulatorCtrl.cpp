/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#include "cemulatorctrl.h"
#include "svnversion.h"

#include "../kegs/src/adb.h"
#include "../kegs/src/clock.h"
#include "../kegs/src/moremem.h"
#include "../kegs/src/paddles.h"
#include "../kegs/src/sim65816.h"
#include "../kegs/src/iwm.h"
#include "../kegs/src/SaveState.h"
#include "../kegs/src/raster.h"
#include "../kegs/src/graphcounter.h"
#include "../kegs/src/driver.h"
#include "../kegs/src/video.h"
#include "../kegs/src/async_event.h"
extern void joystick_shut();

#define CONFIG_VERSION 3

const int	joyint[] = { (int)JOYSTICK_TYPE_NATIVE_1, (int)JOYSTICK_TYPE_KEYPAD, (int)JOYSTICK_TYPE_MOUSE, /* JOYSTICK_TYPE_NATIVE_2,*/ JOYSTICK_TYPE_NONE, JOYSTICK_TYPE_ICADE, -1 } ;
const char*	joyalias[] = { "joystick1","keypad", "mouse", /*, "Joystick2",*/ "none", "iCade", NULL  };
const char*	joydesc[] = { "Joystick1","Keypad", "Mouse", /*, "Joystick2",*/ "None", "iCade", NULL  };
const int	colorint[] = { (int)COLORMODE_AUTO, (int)COLORMODE_BW, (int)COLORMODE_GREEN, (int)COLORMODE_AMBER,(int)COLORMODE_MONO, -1 };
const char* coloralias[] = { "auto", "bw", "green", "amber", "mono",NULL  };
const char* colordesc[] = { "Colors", "B&W", "Green", "Amber", "Mono", NULL  };
const char* videoalias[] = {  "crt" ,  "lcd" ,"default", NULL  };
const char* videodesc[] = {   "CRT" , "LCD",NULL,  NULL  };
const int videoint[] = {  (int)VIDEOFX_CRT,  (int)VIDEOFX_LCD, (int)VIDEOFX_LCD, -1 };
const char* speeddesc[] = { "Normal", "Slow", "Zip", "Unlimited",  NULL  };
const char* speedalias[] = { "normal", "slow", "zip", "unlimited",  NULL  };
const int speedint[] = { (int)SPEED_GS, (int)SPEED_1MHZ, (int)SPEED_ZIP, (int)SPEED_UNLIMITED, -1 };


#define DEFAULT_AUDIO_RATE 44100
#define DEFAULT_LOCKZOOM 0
#define DEFAULT_DEBUGMODE 0
#define DEFAULT_DISKSOUND 1
#define DEFAULT_VIDEO VIDEOFX_CRT

const namevaluedef defoptions[] = 
{
	{ 1,0, "configVersion"  , CONFIG_VERSION,NULL, NULL, NULL },
	{ 1,0, "guid"  , 0, NULL, NULL, NULL },
	{ 1,1, "haltOnBadRead" , 0, NULL, NULL, NULL },
	{ 1,1, "haltOnBadAcc"  , 0, NULL, NULL, NULL },
	{ 1,1, "haltOnHalts"  , 0, NULL, NULL, NULL },
	{ 1,1, "enableRewind"  ,0, NULL, NULL, NULL },
	{ 1,1, "colorMode"  , 0, NULL, colordesc,coloralias , colorint },
	{ 1,1, "videoFX", (int)DEFAULT_VIDEO, NULL, videodesc, videoalias ,  videoint },
	{ 1,1, "joystickMode", 0, NULL, joydesc, joyalias, joyint },
	{ 0,0, "browserName"  , 0, NULL, NULL, NULL },
	{ 0,0, "browserVer"  , 0, NULL, NULL, NULL },
	{ 2,0, "latestRunBuild"  , 0, NULL, NULL, NULL },
	{ 2,0, "latestUpdateCheck"  , 0, NULL, NULL, NULL },
	{ 1,0, "doNotShowUpdateAlert"  , 0, NULL, NULL, NULL },
	{ 0,0, "doNotShowDiskStatus"  , 0, NULL, NULL, NULL },	
	{ 0,0, "PNGBorder"  , 16, NULL, NULL, NULL },	
	{ 0,0, "simulateSpace"  , 0X31, NULL, NULL, NULL }, // 0X31=SPACE KEY
	{ 0,0, "latestVersion"  , 0, NULL, NULL, NULL },
	{ 0,0, "speed"  , 0, NULL, speeddesc, speedalias, speedint },
	{ 0,0, "latestVersionDate"  , 0, NULL, NULL, NULL },
	{ 1,1, "audioRate"  , DEFAULT_AUDIO_RATE, NULL, NULL, NULL },
	{ 1,1, "memorySize"  , 2, NULL, NULL, NULL },
	{ 0,0, "joyXScale"  , 1000, NULL, NULL, NULL },
	{ 0,0, "joyYScale"  , 1000, NULL, NULL, NULL },
    { 1,1, "diskSound"  , DEFAULT_DISKSOUND, NULL, NULL, NULL },
    { 0,1, "fixedVBlank"  , 0, NULL, NULL, NULL },
	 { 0,1, "adbDelay"  , 45, NULL, NULL, NULL },
     { 0,1, "adbRate"  , 3, NULL, NULL, NULL }
#ifdef DRIVER_IOS
    ,
	{ 1,1, "frameRate"  , 0, NULL, NULL, NULL },
	{ 1,1, "lockZoom"  , DEFAULT_LOCKZOOM, NULL, NULL, NULL },
	{ 1,1, "lastTab"  , 0, NULL, NULL, NULL },
	{ 1,1, "debugMode"  , DEFAULT_DEBUGMODE, NULL, NULL, NULL },
	{ 1,1, "externalKBD"  , 0, NULL, NULL, NULL }
#endif	
#ifdef ACTIVEGSKARATEKA
	,
    { 1,1, "showIcons"  , 1, NULL, NULL, NULL },
    { 0,1, "enableCheats"  , 0, NULL, NULL, NULL },
    { 1,1, "soundMode"  , 0, NULL, NULL, NULL },
    { 1,1, "karatekaRewind"  , 1, NULL, NULL, NULL },
    
    
#endif
#ifdef VIRTUALAPPLE
    { 1,1, "debug"  , 0, NULL, NULL, NULL },
    { 1,1, "scores"  , 0, NULL, NULL, NULL },
    { 1,1, "gamecenter"  , 0, NULL, NULL, NULL },
#endif
};

COption option;

const char*	COption::getDescription(option_id _id, int _value)
{
	namevalue& nv = find(_id);;
	return nv.getDescription(_value);
}

int namevalue::belongsToValue(int _v)
{
	if (!def->convertTableInt) return 1;
	int i=0;
	while(def->convertTableInt[i]!=-1)
	{
		if (def->convertTableInt[i]==_v)
			return 1;
		i++;
	}

	return 0;
}

int namevalue::getOrder(int _value)
{
	int p = 0;
	while(def->convertTableInt[p] != -1)
	{
		if (def->convertTableInt[p]==_value)
			return p;
		p++;
	}
	return -1;
}

const char* namevalue::getDescription(int _value)
{
	const char* ret = "n/a";
	if (!def->convertTableInt) return ret;
	int o = getOrder(_value);
	if (o<0) return ret;
	return def->convertTableDesc[o];
}


CEmulator* CEmulator::theEmulator = NULL;

char bram_default[]={
(char)0x00, (char)0x00, (char)0x00, (char)0x01, (char)0x00, (char)0x00, (char)0x0D, (char)0x06, (char)0x02, (char)0x01, (char)0x01, (char)0x00, (char)0x01, (char)0x00, (char)0x00, (char)0x00,
(char)0x00, (char)0x00, (char)0x07, (char)0x06, (char)0x02, (char)0x01, (char)0x01, (char)0x00, (char)0x00, (char)0x00, (char)0x0F, (char)0x06, (char)0x06, (char)0x00, (char)0x05, (char)0x06,
(char)0x01, (char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0x01, (char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0x03, (char)0x02, (char)0x02, (char)0x02,
(char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0x00, (char)0x01, (char)0x02, (char)0x03, (char)0x04, (char)0x05, (char)0x06,
(char)0x07, (char)0x00, (char)0x00, (char)0x01, (char)0x02, (char)0x03, (char)0x04, (char)0x05, (char)0x06, (char)0x07, (char)0x08, (char)0x09, (char)0x0A, (char)0x0B, (char)0x0C, (char)0x0D,
(char)0x0E, (char)0x0F, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF,
(char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF,
(char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF,
(char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF,
(char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF,
(char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF,
(char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF,
(char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF,
(char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF,
(char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF,
(char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFD, (char)0x96, (char)0x57, (char)0x3C
};

void updateBramCheckSum(unsigned char* buf)	// original code in FF/B61D
{
	register short int crc=0;			// LDA #0
	int i;									// CLC
	for(i=255-4-1;i>=0;i--)						// LDX #FA
	{
		crc = (crc << 1) | ((crc>>15) & 1);	// ROL (1-bits)
#ifdef UNDER_CE
		crc += buf[i];
		crc += buf[i+1]*256;
#else
		crc += *(unsigned short*)(buf+i);	// ADC bram,X
#endif
	}									    // DEX
											// CPX #FF
											// BNE Loop
	*(unsigned short*)(buf+252) = crc;		// TAX
	*(unsigned short*)(buf+254) = crc ^0xAAAA;	// EOR #AAAA
}


void initializeBram(unsigned char* _bram_ptr)
{
	memcpy(_bram_ptr,bram_default,256);
	updateBramParameters(_bram_ptr);
	updateBramCheckSum(_bram_ptr);
	
}


void COption::generateGUID(MyString& guid)
{

	int crc = calcCRC(CDownload::getPersistentPath());

	time_t now;
   time( &now );
   int lowtime = (int)now;
	guid.Format("%08X%08X",crc,lowtime);
	
}

void CEmulatorConfig::resetConfig()
{
	speed = SPEED_AUTO;	
	bootslot = -1;	//auto	
//	fastConfig = false;
//	pXML=NULL;

	// vire tous les slots
	for(int i=0;i<3;i++)
	{
		for(int m=0;m<MAXSLOT7DRIVES;m++)
		{
			activeImages[i][m]=CActiveImage();
			for(int a=0;a<ACTIVEGSMAXIMAGE;a++)
				localImages[i][m][a]=CSlotInfo();
		}
	}
	xmlSystemParam.Empty();
	xmlEmulatorParam.Empty();
	xmlAlreadyLoaded = 0;
	
}

int namevalue::convertToInt(const char* _value)
{
	if (!def->convertTableInt) return 0;
	int i=0;
	const char* str;
	while( (str=def->convertTableAlias[i]) != NULL )
	{
		if(!strcasecmp(_value,str))		
			return def->convertTableInt[i];
		i++;
	}
	printf("err: could not convert %s : returning %s (%d)\n",_value,def->convertTableAlias[0],def->convertTableInt[0]);
	return def->convertTableInt[0];
}

option_id	COption::findByName(const char* _name)
{
	if (!_name) return (option_id)-1;

	for (int i=0;i<NB_OPTIONS;i++)
	{
		if (!strcasecmp(_name,options[i].def->name))
			return (option_id)i;
	}
#ifdef _DEBUG
	printf("cannot find option : %s\n",_name);
#endif
	return (option_id)-1;

}

void COption::setValue(option_id _id,const char* _value,int _confOverrided)
{
   
	
	namevalue& nv = find(_id);
	int isnum=1;
	const char*p=_value;
	while(*p)
	{
		if (*p<'0' || *p>'9')
		{
			isnum=0;
			break;
		}
		p++;
	}
	int v;
	if (isnum)
		v=atoi(_value);
	
	if (!nv.def->convertTableInt)
	{
		
		if (!isnum)
		{
			nv.strvalue = _value;
			printf("> option %s: %s\n",nv.def->name,_value);
		}
		else 
		{
			nv.intvalue = v;
			printf("> option %s: (%d)\n",nv.def->name,nv.intvalue);
		}
		
		nv.confOverrided = _confOverrided;

	}
	else
	{
	
		if (isnum && nv.belongsToValue(v))
		{	
			nv.intvalue = v;
			nv.confOverrided = _confOverrided;
			printf("> option %s: %s (%d)\n",nv.def->name,nv.getDescription(v),nv.intvalue);
		}		
		else
		{
			// regarde si c'est une valeur en texte?
			int pos=0;
			const char* s;
			while( (s = nv.def->convertTableAlias[pos]) != NULL )
			{
				if (!strcasecmp(s,_value))
				{
					nv.intvalue = nv.def->convertTableInt[pos];
					nv.confOverrided = _confOverrided;
					printf("* option %s: %s (%d)\n",nv.def->name,_value,nv.intvalue);
					return ;
				}
				pos++;
			}	
			printf("non supported value:%s for option:%s\n",_value,nv.def->name);
		}
	}
}


void namevalue::getAlias(MyString& _str)
{
	if (this->strvalue.IsEmpty())
	{
		// convert to int first
		int v = this->intvalue;
		if (!this->def->convertTableInt)
			_str.Format("%d",v);
		else
		{
			int i = getOrder(v);
			_str = this->def->convertTableAlias[i];
		}
		return ;
	}
	else
		_str = this->strvalue;
}



void COption::initOptions()
{
	
     for(int i=0;i<NB_OPTIONS;i++)
     {
     
     options[i].intvalue = defoptions[i].defaultint;
     options[i].strvalue = defoptions[i].defaultstr;
     }
    initiliazed = 1;
    setDefaultOptions();
}


void COption::setDefaultOptions()
{
	
    if (!initiliazed)
        x_fatal_exit("option not initialized");
	for(int i =0;i<NB_OPTIONS;i++)
	{
		options[i].def = &defoptions[i];
		if (options[i].def->inDefault)
		{
			if (options[i].def->convertTableInt)
				options[i].intvalue =options[i].def->convertTableInt[0];
			else
			{
				options[i].intvalue = options[i].def->defaultint;
				options[i].strvalue = options[i].def->defaultstr;
			}

		}
	}
    
    if (g_driver.x_apply_default_options!=NULL)
		    g_driver.x_apply_default_options((void*)this);

	printf("*restored default options\n");
}


void COption::setIntValue(option_id _id, int _v, int _confOverrided)
{
	MyString str;

	namevalue& nv  =find(_id);
	
	if (!nv.belongsToValue(_v))
	{
		printf("err: unrecognized value %d for %s\n",_v,nv.def->name);
		return ;
	}

	nv.intvalue = _v;
	if (nv.def->convertTableInt)
		printf("option %s set to %s (%d)\n",nv.def->name,nv.getDescription(_v),_v);
	else
		printf("option %s set to (%d)\n",nv.def->name,_v);
	
	nv.confOverrided = _confOverrided;
	
}

void COption::getStrValue(MyString& _str,option_id _id)
{
	namevalue& nv =find(_id);
	
	if (nv.strvalue.IsEmpty())
	{
		if (nv.def->convertTableInt)
			_str = nv.getDescription(nv.intvalue);
		else
			_str.Format("%d",nv.intvalue);
	}
	else
		_str = nv.strvalue.c_str();
}

int COption::getIntValue(option_id _id)
{
	namevalue& nv =find(_id);

	if (!nv.def->convertTableInt || nv.strvalue.IsEmpty())
		return nv.intvalue;
	else
	{
		printf("getintvalue: mismatched type for %s\n",nv.def->name);
		return 0;
	}
}

void COption::saveOptions(int _nb)
{
	MyString filename;
	getParameterFilename(filename,_nb);
	FILE* f = fopen(filename.c_str(),"wt");
	if (!f)
	{
		outputInfo("cannot create %s for saving options\n",filename.c_str());
		return ;
	}
	for(int i=0;i<NB_OPTIONS;i++)
	{
		if (options[i].def->inConfig == _nb)
		{
			MyString line;
			MyString desc;
			options[i].getAlias(desc);
			line.Format("%s:%s\n",options[i].def->name,desc.c_str());
			fputs(line.c_str(),f);
		}
	}
	fclose(f);
	//outputInfo("options saved (%s)\n",filename.c_str());
	enableOptions();
}




void COption::loadOptions()
{
	
	int  b = loadParameters(1);
	if (!b)
	{
		// si il n'y avait pas de fichier conf - rajoute le guid au option par défaut et sauve le
		MyString guid;
		generateGUID(guid);
		setValue(OPTION_GUID,guid.c_str());
		saveOptions(1);
	}
	
	// vérifie le numéro de version
	int v = getIntValue(OPTION_CONFIGVERSION);
	if (v < CONFIG_VERSION)
	{
		// réinitialise
		initOptions();
		setIntValue(OPTION_CONFIGVERSION,CONFIG_VERSION);
		printf("upgrading config file to v" TOSTRING(CONFIG_VERSION));
		saveOptions(1);
	}
	
	loadParameters(2);
}

void COption::getParameterFilename(MyString& filename,int _nb)
{
	filename = CDownload::getPersistentPath();
	filename += ACTIVEGS_DIRECTORY_SEPARATOR ;
	filename += "activegs";
	if (_nb==1)
		filename += ".conf";
	else
		filename += ".ext";	
}

int COption::loadParameters(int _nb)
{

	MyString filename;
	getParameterFilename(filename,_nb);
	
	FILE* f = fopen(filename,"rt");
	if (f)
	{
		outputInfo("loadOptions from %s\n",filename.c_str());
		char line[1025];
		line[1024]=0;
		while(fgets(line,1024,f))
		{
			int i=0;
			while(line[i]!=':' && line[i])i++;
			if (line[i]==':')
			{
				line[i++]=0;
				option_id id = findByName(line);
				if (id == OPTION_ERROR)
					outputInfo("option %s ignored\n",line);
				else
				{
                  
					int vpos = i;
					while( line[i] 
							&&
							(line[i]!='\t') 
							&&
							(line[i]!='\r') 
							&&
							(line[i]!='\n')
                          /*
							&&
							(line[i]!=' ') */) i++;
				
					if (line[vpos])
					{
                        // trim
                        while(line[i-1]==' ')i--;
                        	line[i]=0;
						setValue(id,line+vpos);
					}
				}
			}
		}
		fclose(f);
		return 1;
	}
	else
		return 0;
}


CEmulatorConfig::CEmulatorConfig()
{
	xmlAlreadyLoaded = 0;
	pXML=NULL;
	bramParam.Empty();
	emulatorParam.Empty();
    nb_bps=0;
	resetConfig();
}

void CEmulatorConfig::setBaseURL(const char* _url)
{
	MyString p = _url;
	
	if (!strncasecmp(p.c_str(),"file://localhost/",17))
		baseURL = p.substr(16,p.length()-16); // absolute path OG 250810 - leave the trailing slash
	else
	if (!strncasecmp(p.c_str(),"file:///",8))
#ifdef WIN32
		baseURL = p.substr(8,p.length()-8); // absolute path
#else
		baseURL = p.substr(7,p.length()-7); // keep the / on mac
#endif
	else
	if (!strncasecmp(p.c_str(),"file://",7))
		baseURL = p.substr(7,p.length()-7); // absolute path
	else
		baseURL = p;

	baseURL.Replace("%20"," ");
	printf("baseURL set to %s\n",baseURL.c_str());
	/*
	CDownload dl;
	dl.setBaseURL(baseURL.c_str());
	*/
}

CSlotInfo& CEmulatorConfig::getLocalImage(int _slot, int _drive, int _pos)
{
	return localImages[_slot-5][_drive-1][_pos];
}

CActiveImage& CEmulatorConfig::getActiveImage(int _slot, int _drive)
{
	return activeImages[_slot-5][_drive-1];
}

CSlotInfo* CEmulatorConfig::setSlot(int _slot, int _disk, const char * _url_normalized, enum eSetSlotPos _pos)
{
	if  (_slot<5 || _slot > 7 || _disk < 1 || (_slot==7 && _disk > MAXSLOT7DRIVES) || (_slot!=7 && _disk > 2) )
    {
        printf("invalid slot : %d:%d\n",_slot,_disk);
			return NULL;
    }
	// normalise pour garder la compatibilité avec les version < 3.5
	MyString url = _url_normalized;
	int pos = url.ReverseFind('?');
	if (pos != -1 )
	{
		int i = url.at(pos+1);
		size_t l = url.length();
		if (l==(pos+2) && i>='0' && i<='9')
		{
			url = url.Left(pos);
			url += "#pos=";
			url += i;
		}
	}
	
	const char* _url = url.c_str();
	if (_url && !_url[0]) _url=0;
	CActiveImage& ai = activeImages[_slot-5][_disk-1 ];

	int active=0;
	if (_pos==REPLACECURRENT)	// posactive
	{
		active = ai.iActive;
		if (!ai.iNb)
		{
			// ignore SetSlot = NULL si aucun disk n'avait été préalablement chargé
			if (!_url) return NULL;
			ai.iNb=1;
		}
	}
	else
	// if (_pos==ADD)	
	{
		// regarde si le disk était déjà présent dans la liste
		for(int i =0;i<ai.iNb;i++)
		{
			CSlotInfo& ref = localImages[_slot-5][_disk-1 ][i];
			if (!ref.url.CompareNoCase(_url))
			{
				printf("image %s already in the list... ignoring!\n",_url);
				return NULL ;
			}
		}
		active = ai.iNb++;
	}
	
	
	CSlotInfo& ref = localImages[_slot-5][_disk-1 ][active];
	
	if (ref.status == MOUNTED || _url )
	{
		if (!_url)	
		{
			ref.status = EJECTED ;
			outputInfo("status set to EJECTED for S%dD%d\n",_slot,_disk);
		}
		else
		{
			size_t len = strlen(_url);
			if (len>2048)
			{
				// overflow!
				outputInfo("status set to FAILED(invalid length) for S%dD%d\n",_slot,_disk);
				ref.status = FAILED;
			}
			else
			{
				ref.url	= _url;
				ref.url.Trim();	// OG Fix ending space
				ref.shortname = getfile(_url);
				ref.status = READY2MOUNT;
				outputInfo("status set to READY2MOUNT for S%dD%d, image: %s\n",_slot,_disk,ref.shortname.c_str());
			}
		}
		need2remount=true;
		x_refresh_panel(PANEL_DISK);
	}
	return &ref;
}

const char* CEmulatorConfig::getSlot(int _slot, int _disk)
{
	if (_slot<5 || _slot > 7 || _disk < 1 || _disk > MAXSLOT7DRIVES)
			return NULL;
	CActiveImage active = activeImages[_slot-5][_disk-1 ];
	CSlotInfo& ref = localImages[_slot-5][_disk-1 ][active.iActive];
	return STRING2CONSTCHAR(ref.url);
}


void CEmulatorConfig::processCommandLine(int argc, char** argv)
{
	
	for(int i=1;i<argc;i++)
	{
		bool bOption = argv[i][0]=='-';
		if (bOption)
		{
			const char* option = argv[i]+1;
			if (!strcasecmp(option,"help"))
			{
#if defined(WIN32) && !defined(UNDER_CE)
				::MessageBox(NULL,"ActiveGS.exe [<filename>.activegsxml|<filename.2mg>|<filename.dsk>]", ACTIVEGSVERSIONSTRFULL,MB_OK);
				::PostQuitMessage(0);
#endif
			}
			else
			if ((i+1)<argc)
			{
				const char* value = argv[++i];
				if (!strcasecmp(option,"slot51"))
					this->setSlot(5,1,value,ADD);
				else
				if (!strcasecmp(option,"slot52"))
					this->setSlot(5,2,value,ADD);
				else
				if (!strcasecmp(option,"slot61"))
					this->setSlot(6,1,value,ADD);
				else
				if (!strcasecmp(option,"slot62"))
					this->setSlot(6,2,value,ADD);
				else
				if (!strcasecmp(option,"slot71"))
					this->setSlot(7,1,value,ADD);
				else
				if (!strcasecmp(option,"slot72"))
					this->setSlot(7,2,value,ADD);
				else
				if (!strcasecmp(option,"BootSlot"))
				{
					int bs = atoi(value);
					this->bootslot = bs;
				}
				else
				if (!strcasecmp(option,"Speed"))
				{
					int s = atoi(value);
					this->speed=s;
				}
				else
				if (!strcasecmp(option,"EmulatorParam"))
					this->emulatorParam=value;
				else
				if (!strcasecmp(option,"SystemParam"))
					this->systemParam=value;
				else
				if (!strcasecmp(option,"BramParam"))
					this->bramParam=value;
				else
				if (!strcasecmp(option,"XMLConfig"))
					this->xmlconfig=value;
				else
				if (!strcasecmp(option,"state"))
				{
					CDownload dl(this->baseURL);
				//	dl.setBaseURL(_path);
					MyString path;
					MyString shortn;
					dl.retrieveFile(value,path,shortn);
					this->stateFilename=path.c_str();
				}
			}
		}
		else
		{
			// vérifie l'extension (mais strip le #)
			MyString extv(argv[i]);
			int l = extv.ReverseFind("#");
			if (l>0)
				extv = extv.Left(l);
			const char* ext = getext(extv.c_str());
			if ( !strcasecmp(ext,"activegsxml") || !strcasecmp(ext,"xml"))
			{
				this->xmlconfig = argv[i] ; 
			}
			else
			{
				MyString path;
				MyString file;
			//	CDownload dl;
				int slot = 0;
				
			//	dl.deleteCachedFile(argv[i]);
				CDownload dl(this->baseURL);
				size_t size = dl.retrieveFile(argv[i],path,file);
				ext = getext(path.c_str());
                if (!strcasecmp(ext,"hdv"))
				{
					slot = 7;
                }
				else
				if (!strcasecmp(ext,"2mg") || !strcasecmp(ext,"po"))
				{
					if (size> 819712) // OG Fix 2MG size recognitioN (CF. RASTAN 819264 & POMS 7 819712)
						slot = 7;
					else
						slot = 5;
				}
				else
				if ( !strcasecmp(ext,"bin") || !strcasecmp(ext,"dsk") || !strcasecmp(ext,"nib") || !strcasecmp(ext,"po") || !strcasecmp(ext,"do") )
					slot = 6;
				if (slot)
				{
					setSlot(slot,1,path,REPLACECURRENT);
				//	bootslot = slot; OG Laisse la valeur par défaut!
					xmlconfig.Empty();
				}
				else
					outputInfo("Unknown extension .%s\n",ext);
			}
		}
	}
}

int CEmulatorConfig::processXML(const char*_path,simplexml* ptrconfig)
{
	
	const char* ver = ptrconfig->property("version");
	if (!ver || atoi(ver)!=2)
	{
		outputInfo("invalid xml version - xml config skipped\n");
		return  0;
	}
	
	int child = ptrconfig->number_of_children();
	for(int i=0;i<child;i++)
	{
		simplexml* ptr = ptrconfig->child(i);
		if (!strcasecmp(ptr->key(),"uid"))
		{
			const char* s = ptr->value();
			uid = s;
		}
		else
			if (/*!fastConfig && */!strcasecmp(ptr->key(),"image"))
			{
				int slot = atoi(ptr->property("slot"));
				int disk = atoi(ptr->property("disk"));
				if (
					( ( slot==5 || slot==6 ) && ( disk==1 || disk==2) )
					||
					( (slot==7) && ( disk>=1 || disk<=MAXSLOT7DRIVES) )
					)
				{

					MyString absp ;
					const char* s = ptr->value();
					int patch_addr = 0;
					const char* patch_value = NULL;
									
					for(int idisk=0;idisk<ptr->number_of_children();idisk++)
					{
						simplexml* ptrd = ptr->child(idisk);
						if (!strcmp(ptrd->key(),"name"))
								s = ptrd->value();
						else
						if (!strcmp(ptrd->key(),"patch"))
						{
							const char* addr_str = ptrd->property("addr");
							if (addr_str)
							{
								sscanf(addr_str,"%X",&patch_addr);
								patch_value = ptrd->value();
                                if (strlen(patch_value)&1)
                                    patch_value=NULL; // doit être pair
							}
						}
					}

					if (s)
					{
					//	CDownload dl(this->baseURL);
						CDownload::makeAbsolutePath(s,_path,absp);
						
						CSlotInfo* pslot = setSlot(slot,disk,absp.c_str(),ADD);
						if (pslot && patch_value && patch_addr)
						{
							pslot->patch_addr = patch_addr;
							pslot->patch_value = patch_value;
                            pslot->patch_value.MakeUpper();
						}
					}
				}
			}
			else		
			if (/*!fastConfig && */!strcasecmp(ptr->key(),"bootslot"))
			{
				int slot = atoi(ptr->value());
				if (slot>=5 && slot<=7)
				{
					bootslot = slot;
				}
			}
			else
            if (!strcasecmp(ptr->key(),"runtime"))
            {
                const char* mode_str = ptr->property("mode");
                if (mode_str)
                {
					enum mode_breakpoint mode = BRK_DISABLED;
                if (strchr(mode_str,'X'))
                    mode = (enum mode_breakpoint)(mode|BRK_X);
                if (strchr(mode_str,'R'))
                    mode = (enum mode_breakpoint)(mode|BRK_READ);
                if (strchr(mode_str,'W'))
                    mode = (enum mode_breakpoint)(mode|BRK_WRITE);
                if (strchr(mode_str,'P'))
                    mode = (enum mode_breakpoint)(mode|BRK_PATCH);
				 if (strchr(mode_str,'B'))
                    mode = (enum mode_breakpoint)(mode|BRK_STOP);
                 if (strchr(mode_str,'S'))
                    mode = (enum mode_breakpoint)(mode|BRK_SLOWSPEED);
				if (strchr(mode_str,'D'))
                    mode = (enum mode_breakpoint)(mode|BRK_DEFAULTSPEED);
				if (strchr(mode_str,'M'))
                    mode = (enum mode_breakpoint)(mode|BRK_MESSAGE);
				


                const char* addr_str = ptr->property("addr");
                if (addr_str && !(strlen(addr_str)&1))
                {
                    word32 patch_addr;
                    sscanf(addr_str,"%X",&patch_addr);
                    
                    const char* value_str = ptr->value();
                    if (value_str == NULL || !(strlen(addr_str)&1))
                    {
                    
                    int bp = nb_bps;
                    if (bp != MAX_BREAK_POINTS)
                    {


						const char* check = ptr->property("check");
						memset(BPs[bp].check,0,MAX_CHECK);
						if (check!=NULL)
						{
						//	printf("found check:%s\n",check);
							int i=0;
							while(i<MAX_CHECK-3)	// 3x 00 A LA FIN
							{
								char c1 = *check++;
								if (!c1) break; // done
								char c2 = *check++;
								if (!c2) break; // misformed
								if (c1>='a')
									c1 -= ('a'-10);
								else
								if (c1>='A')
									c1 -= ('A'-10);
								else
									c1 -= '0';

								if (c2>='a')
									c2 -= ('a'-10);
								else
								if (c2>='A')
									c2 -= ('A'-10);
								else
									c2 -= '0';

								BPs[bp].check[i++]= c1*16+c2;
							}

						}

                        BPs[bp].addr= patch_addr;
                        BPs[bp].value = value_str;
                        BPs[bp].mode = mode;
                        nb_bps++;
                    }
                    }
                }
                }
            }
            else
			if (!strcasecmp(ptr->key(),"speed"))
			{
				int sp = atoi(ptr->value());
				if (sp>=0 && sp<SPEED_ENUMSIZE)
				{
					speed = sp;
				}
			}
			else
			if (!strcasecmp(ptr->key(),"bramparam"))
			{
				const char* s = ptr->value();
				xmlBramParam = s; 			
			}
			else
			if (!strcasecmp(ptr->key(),"emulatorparam"))
			{
				const char* s = ptr->value();
				xmlEmulatorParam = s;
			}
			else
			if (!strcasecmp(ptr->key(),"systemparam"))
			{
				const char* s = ptr->value();
				xmlSystemParam = s;
			}
			else
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
			if (!strcasecmp(ptr->key(),"state"))
			{
				const char* s = ptr->value();
				CDownload dl(_path);	// ???
				MyString path;
				MyString shortn;
				dl.retrieveFile(s,path,shortn);
				stateFilename=path.c_str();
			}
		
	}

	return 1;
}


void CEmulatorConfig::loadXMLFile()
{
	
	
	if (xmlconfig.IsEmpty() && !pXML)
	{
		xmlEmulatorParam.Empty();
		xmlSystemParam.Empty();
		xmlBramParam.Empty();
		return ;
	}

	if (xmlAlreadyLoaded)
	{
		printf("discard xml...\n");
		return ;
	}
	resetConfig();
	xmlAlreadyLoaded = 1;

	
	// If XML already present, use preparsed structrue
	if (pXML)
	{
		processXML(baseURL.c_str(),pXML);
		return ;
	}
	
	MyString fullpath;
	MyString fulldir;
	//CDownload dl;
	CDownload::makeAbsolutePath(STRING2CONSTCHAR(xmlconfig),baseURL.c_str(),fullpath);
	getdir(fullpath.c_str(),fulldir);
	xmlconfig = fullpath.c_str() ;
	
	ActiveGSList list;
	if (!list.loadFromFile(STRING2CONSTCHAR(xmlconfig)))
		return ;

	simplexml* ptrconfig = NULL;
	if (list.elements.size()==1)
		ptrconfig = list.elements.at(0).pXML;
	else
	{
		CStdStringA xmlconfig2 = xmlconfig;
		int pos = xmlconfig2.ReverseFind("#id=");
		if (pos==-1)
		{
			printf("No key requested with .activegsxml list format\n");
			return ;
		}

		MyString key = xmlconfig2.Mid(pos+4);
		for(unsigned int i=0;i<list.elements.size();i++)
		{
			if (list.elements.at(i).theid == key)
			{
				ptrconfig = list.elements.at(i).pXML;
				break;
			}
		}
		if (!ptrconfig)
		{
			printf("id: %s not found in %s\n",key.c_str(),STRING2CONSTCHAR(xmlconfig));
			return ;
		}
	}

	ASSERT(ptrconfig);
	processXML(fulldir.c_str(),ptrconfig);
	/*
	delete root;
	delete s;
	*/
}


//void geturldomain(const char* p,MyString& _domain);

void CEmulator::sendStats()
{
#pragma message("TODO refaire les stats!")
	/*
#ifndef DRIVER_IOS
	MyString str="http://analytics.freetoolsassociation.com/analytics.php?";
	const char* app =ACTIVEGSAPP ;
	str +="&p=";
	str += app;
	str +="&b=";
	const char* ver = TOSTRING(ACTIVEGSMAJOR) "." TOSTRING(ACTIVEGSMINOR) "." TOSTRING(ACTIVEGSBUILD);
	str += ver;
	str +="&n=";
	MyString browser; option.getStrValue(browser,OPTION_BROWSERNAME);
	str += browser.substr(0,4);
	str +="&v=";
	MyString browseroption; option.getStrValue(browseroption,OPTION_BROWSERVERSION);
	str += browseroption;
	str +="&g=";
	MyString guid; option.getStrValue(guid,OPTION_GUID);
	str += guid;
	MyString domain;
	geturldomain(config->baseURL.c_str(),domain);
	str += "&d=";
	str += domain.c_str();
	
#ifdef ACTIVEGSPLUGIN
	CDownload dl;
	dl.GetFile(str.c_str(),NULL);
#endif
#endif
	*/
}

void CEmulator::setConfig(CEmulatorConfig* _config)
{
	config = _config;

	sendStats();
	config->loadXMLFile();
	/*
	CDownload dl;
	dl.setBaseURL(config->baseURL.c_str());
	*/
}


char read2MG(const char* ref,int block, int byte)
{
	return ref[0x40+block*512+byte];
}

int find2MG(const char* ref,int block, char header, const char* str)
{
	int byte = 4;
	
	while (1)
	{
		char r = read2MG(ref,block,byte);
		if (!r) return 0;

		if (r==header)
		{
			int c=0;
			while(str[c])
			{
				if (read2MG(ref,block,byte+c+1)!=str[c]) break;
				c++;
			}
			if (!str[c])
			{
				// found!
				return block*512+byte;
			}
		}
		byte+=0x27;
		if (byte==0x1FF)
		{
			byte =4;
			block = read2MG(ref,block,2) + read2MG(ref,block,3)*256;
			if (!block) return 0;
		}

	}
	return 0;
}



int findSetStart(const char* buffer)
{
	// start dir
	
	int sys = find2MG(buffer,2,(char)0xD6,"SYSTEM");
	if (!sys)
	{
		outputInfo("cannot find System dir!");
		return false;
	}
	
	int sysbloc = buffer[sys+0x40+0x11]+buffer[sys+0x40+0x12]*256;

	int setstart = find2MG(buffer,sysbloc,0x1D,"SETSTART.DATA");
	if (!setstart)
	{
		outputInfo("cannot find setstart file");
		return false;
	}
	
	return setstart;

}

bool CEmulator::setStartParam()
{
	// prend le nom du fichier en 71

	int slot = setStart.slot/10;
	int drive = setStart.slot%10;
	CSlotInfo& ref = getLocalIMGInfo(slot,drive);
	const char* filename= ref.filename.c_str(); //getLocalIMG(setStart.slot/10,setStart.slot%10);
	if (!filename) return false;
	FILE* f=fopen(filename,"rb");
	if (!f) return false;

	// get file size
	fseek(f,0,SEEK_END);
	int filesize=ftell(f);

	char* buffer=(char*)malloc(filesize);

	fseek(f,0,SEEK_SET);
	fread(buffer,1,filesize,f);
	fclose(f);

	// 
	int offset = findSetStart(buffer);
	if (!offset)
	{
		free(buffer);
		return false;
	}

	char *entry = buffer+offset+0x40;
		
	/*
	char *entry = buffer+setStart.addr;
	*/
	if ((entry[0]!=0x1D) || (strcmp(entry+1,"SETSTART.DATA")) )
	{
		free(buffer);
		return false;
	}
	

	
	// buffer + 17 = block
	int blockpos = (entry[17]+entry[18]*256)*512+64;
	
	// buffer + 19 = size
	int size  = setStart.file.length();
	entry[0x15]=(size+3)&0xFF;
	entry[0x16]=((size+3)>>8)&0xFF;
	entry[0x17]=0;


	char* block = buffer+blockpos;
	memset(block,0,256);
	block[0]=size&0xFF;
	block[1]=(size>>8)&0xFF;
	strcpy(block+2,setStart.file.c_str());
	block[size+2]=0x31; // ????

	f=fopen(filename,"wb");
	if (!f)
	{
		free(buffer);
		return false;
	}
	fseek(f,0,SEEK_SET);
	fwrite(buffer,1,filesize,f);
	fclose(f);
	free(buffer);

	struct param p;
		p.data = "SetStart";
		p.value = setStart.file;
		p.hex = offset+0x40;	//setStart.addr;
		addUniqueParam(p);
	//	params.push_back(p);

	x_refresh_panel(PANEL_PARAMS);
	
	return true;
}


int CEmulator::acceptEvents()
{
	return r_sim65816.get_state()==RUNNING;
}


void CEmulator::onEmulatorParamChanged(const char* _param)
{
	if (!acceptEvents())
	{
		outputInfo("### onEmulatorParamChanged Ignored");
		return;
	}
	option.addEmulatorParam(_param);
	setEmulatorParam(_param);
	option.enableOptions();
}

void CEmulator::onBootSlotChanged(int _bootslot)
{
	if (!acceptEvents())
	{
		outputInfo("### OnBootSlotChanged Ignored");
		return;
	}
	config->bootslot = _bootslot;
	relaunchEmulator();
}

void CEmulator::relaunchEmulator()
{
	outputInfo("Trying to relaunch Emulator()\n");
	
	if (this !=theEmulator)
	{
		outputInfo("Another instance was running in another process : killing it()\n");
		delete theEmulator;
	}

	forceFocus();

	if (bInThread)
	{
		outputInfo("Asking for current instance to restart\n");
		r_sim65816.restart_emulator();
	}
	else
	{
		outputInfo("reactivate emulator!\n");
		launchEmulator();
	}
}


void	CEmulator::onSpeedChanged(int _speed, bool _ignoreAcceptEvents)
{

	if (!_ignoreAcceptEvents && !acceptEvents())
	{
		outputInfo("### onSpeedChanged Ignored");
		return;
	}
		
	/*
	int sp = g_sim65816.set_limit_speed((speedenum)_speed);
	config->speed = sp;
	*/
	option.setIntValue(OPTION_SPEED,_speed);
	option.enableOptions();
	forceFocus();
}

void	CEmulator::addUniqueParam(struct param _p)
{
	int i;
	for(i=0;i<nbparams;i++)
		if (params[i].data == _p.data)
		{
			params[i].value = _p.value ;
			params[i].hex = _p.hex ;
			return ;
		}
	if (i<EMULATOR_MAX_PARAMS)
	{
		params[i].value = _p.value ;
		params[i].hex = _p.hex ;
		params[i].data = _p.data ;
		nbparams++;
	}
		
	/*
	vector <struct param> params;

	vector<struct param>::iterator v;
	v = params.begin();
	while(v!=params.end())
	{
		if (v->data == _p.data)
		{
			v->value = _p.value;
			v->hex = _p.hex;
			return ;
		}
		v++;
	}
	params.push_back(_p);
*/
}


const char* currentBuild=TOSTRING(ACTIVEGSMAJOR)"." TOSTRING(ACTIVEGSMINOR) "." TOSTRING(ACTIVEGSBUILD);



void CEmulator::checkAlerts()
{
	
	static int bRunOnlyOnce=0;
	if (bRunOnlyOnce) return ;
	bRunOnlyOnce=1;
	
#ifdef _DEBUG
	option.setValue(OPTION_LATESTVERSION,"5.1.4");
#endif
	
	if (!option.getIntValue(OPTION_DONOTSHOWUPDATEALERT))
	{	
		MyString newestBuild;  option.getStrValue(newestBuild,OPTION_LATESTVERSION);
		MyString lastestUpdatedBuild; option.getStrValue(lastestUpdatedBuild,OPTION_LATESTUPDATECHECK);
		if (	!newestBuild.IsEmpty()  
			&&	lastestUpdatedBuild.compare(newestBuild)
			&&	newestBuild.compare(currentBuild) )
			{
				option.setValue(OPTION_LATESTUPDATECHECK,newestBuild);
				option.saveOptions(2);

				x_alert("A newer version (%s) of ActiveGS is available!\nCheck http://activegs.freetoolsassociation.com for more info.",newestBuild.c_str(),currentBuild);
				
				return ; 
			}
		
		
	}
	 
}

bool CEmulator::setParam(eParamType _type, const char* _data, const char* _value)
{

	static const struct { const char* name; int* address; } variables[] =
	{ 
		
	// boot
		{ "boot", (int*)0x28 },
		
	// drive emulation
		{ "driveemulation", &g_iwm.g_fast_disk_emul },
		{ "slow", (int*)0 },
		{ "fast", (int*)1 },

	// slot
		{ "slot6", (int*)0x26 },
		{ "drive", (int*)0 },
		{ "disk", (int*)0 },
		{ "card",(int*)1},

	// columns
		{ "columns", (int*)0x19 },
		{ "40", (int*)0 },
		{ "80", (int*)1 },

	// speed
	
		{ "forcespeed", (int*)0x20 },
		{ "normal",(int*)0},
		{ "fast",(int*)1},
		
		{ "zipspeed",&g_moremem.g_zipgs_reg_c05a},
	//	{ "simulatespace",&g_adb.g_simulate_space},
		{ "emulate6502",&g_sim65816.g_6502_emulation },
		{ "apple2erom",&g_sim65816.g_use_apple2e_rom },

	// bram
		{ "font", (int*)0x1A },
		{ "background", (int*)0x1B },
		{ "border", (int*)0x1C },

		{ "black",	(int*)0x00 },
		{ "dark-blue",	(int*)0x02 },
		{ "dark-green",	(int*)0x04 },
		{ "medium-blue",	(int*)0x06 },
		{ "brown",	(int*)0x08 },
		{ "light-gray",	(int*)0x0A },
		{ "green",	(int*)0x0C },
		{ "aquamarine",	(int*)0x0E },
		{ "deep-red",	(int*)0x01 },
		{ "purple",	(int*)0x03 },
		{ "dark-gray",	(int*)0x05 },
		{ "light-blue",	(int*)0x07 },
		{ "orange",	(int*)0x09 },
		{ "pink",	(int*)0x0b },
		{ "yellow",	(int*)0x0d },
		{ "white",	(int*)0x0f },

		{ "language",	(int*)0x29 },
		{ "keyboard",	(int*)0x2a },
		{ "french",	(int*)0x02 },
		/*
* $0 =       USA                 Dvorak 
*              $1 =       UK                   USA
*              $2 =     French                 USA
*              $3 =     Danish                 USA
*              $4 =     Spanish                USA
*              $5 =     Italian                USA
*              $6 =     German                 USA 
*              $7 =     Swedish  
		*/
		{ NULL,	NULL }
	};

	int value = -1000;
	int* data = 0;
		
	if (!strcasecmp(_data,"driveemulation"))
		data=0;
	
	int i=0;
	while(variables[i].name)
	{
		if (!strcasecmp(_data,variables[i].name))
		{
			data = (int*)variables[i++].address;
			while(variables[i].name)
			{
				if (!strcasecmp(_value,variables[i].name))
				{
					value = (intptr_t)(variables[i].address);
					break;
				}
				i++;
			}
		}
		else
			i++;
	}



	if (value==-1000)
		value=atoi(_value);
	if (!data)
		data=(int*)atoi(_data);

	if ( data && (value!=-1000))
	{
		outputInfo("%s = %s (%d)\n",_data,_value,value);
		
		struct param p;
		p.data = _data;
		p.value = _value;
		p.hex = value;

		if (data==&g_moremem.g_zipgs_reg_c05a)
		{
			g_moremem.g_zipgs_reg_c05a = ((g_moremem.g_zipgs_reg_c05a&0x0F)|(value&0x0F)<<4);
			p.value.Format("%3d%%",(int)((16-((g_moremem.g_zipgs_reg_c05a>>4)&0x0F))*100/16));
		}
		else
		if ((intptr_t)data > 256)
//		if (_type==EMULATOR)
			*data = value;
		else
		{
			// bram
			int idx = (intptr_t)data;
			if (idx>0 && idx<256)
				g_clock.g_bram_ptr[idx]=value;
			else 
				printf("invalid bram index : %s (%d)\n",_data,idx);

		}
		
		addUniqueParam(p);
	}
	else
	{
#ifdef _DEBUG
		outputInfo("unknown parameters (%s)(%s)\n",_data,_value);
#endif
	}

	return true;

}

bool CEmulator::parseParam(const char* _param,eParamType _type)
{

	// Format
	// data=value;data2=value;
	
	const char* run = _param;
	if (!run) return false;
	
	MyString data;
	MyString value;
	
	while(1)
	{
		// data
		data.Empty();

		// skip ;
		while (*run == ';') run++;
		if (!*run) return false; // done

		while(*run!='=' && *run!=':')
		{
			if (!*run) return false;
			data += *run;
			run++;
		}
		run++;	// passe le =
		value.Empty();

		while(*run && *run!=';')
			value += *run++;		
		
		if (!strcasecmp(data.c_str(),"setstart"))
			extractSetStartParam(value.c_str());
		else
		{
			if (data[0]!='_')
				setParam(_type,data.c_str(),value.c_str());
		}
		if (!*run) break;
		run++;
	}

	return true;
}


bool COption::addEmulatorParam(const char* _param)
{

	const char* run = _param;
	if (!_param) return true;
//	printf("loading system param\n");

	MyString data;
	MyString value;
	
	while(1)
	{
		// data
		data.Empty();

		// ignore the initial _ (for backward compatibility)
		if (*run=='_') run++; 
		while(*run!='=' && *run!=':')
		{
			if (!*run) return false;
			data += *run;
			run++;
		}
		run++;	// passe le =
		value.Empty();

		while(*run && *run!=';')
			value += *run++;		

		// regarde si c'est une option
		option_id id = findByName(data.c_str());
		if (id!=OPTION_ERROR)
			setValue(id,value.c_str(),1); 
			/*
		if (data[0]=='_')
		{
			MyString data2;
			data2 = data.substr(1);
			option_id id = findByName(data2.c_str());
			if (id==OPTION_ERROR)
				printf("could not process : %s %s\n",data2.c_str(),value.c_str());
			else
				setValue(id,value.c_str(),1); 
		}
		*/
		
		if (!*run) break;
		run++;
	}

	return true;
}



bool CEmulator::extractSetStartParam(const char* run)
{
	// Format slot|file

	setStart.slot = 0;
	int slot;
	
	MyString data;
	while(*run!=':' && *run)
		data += *run++;
			
	slot = atoi(data.c_str());
	if ( (slot!=51) && (slot!=52) && (slot!=71) && (slot!=72) ) return false;

	data.Empty();
	while(*run)
		data += *run++;

	setStart.slot = slot;
	setStart.file = data;

	return true;
}

// CChildView


void CEmulator::updateBramParameters()
{
	if (!this) return ;

	parseParam(bramParam.c_str(),BRAM);
	x_refresh_panel(PANEL_PARAMS);
}

void CEmulator::setBramParam(const char* _p)
{
	

	if (bramParam.IsEmpty())
		bramParam = _p;
	else
	{
		bramParam += ';';
		bramParam += _p;
	}
}



void CEmulator::ejectDisk(int _slot,int _drive)
{
	if (!this) return ;
	CSlotInfo& ref = getLocalIMGInfo(_slot,_drive);
	if (ref.status == UNDEFINED) return ;
	ref.status = EJECTED ;
	outputInfo("status set to EJECTED for S%dD%d\n",_slot,_drive);
	config->need2remount=true;
	x_refresh_panel(PANEL_DISK);

     if(g_sim65816.g_enable_disk_sound)
         play_sound(SOUND_EJECTION);
}


void CEmulator::reloadDisk(int _slot,int _drive)
{
	if (!this) return ;
	CSlotInfo& ref = getLocalIMGInfo(_slot,_drive);
	if (
		(ref.status == EJECTED )||
		(ref.status == FAILED ) )
	{
			ref.status = READY2MOUNT;
			outputInfo("status set to READY2MOUNT for S%dD%d\n",_slot,_drive);
			config->need2remount=true;
			x_refresh_panel(PANEL_DISK);
	}
}



CEmulator::CEmulator(CEmulator** _parent)
{
	if (theEmulator)
		x_fatal_exit("MULTIPLE INSTANCES");
	bInThread = false;
	theEmulator= this;
	parent = _parent ;
#ifdef ACTIVEGSPLUGIN
	pInstance = NULL;
#endif
	theWindow = NULL;
	g_sim65816.halt_sim = 0;
	x=0;
	y=0;
	memset(lights,0,sizeof(lights));
	memset(tracks,0,sizeof(tracks));
	config = NULL;
}

#ifdef ACTIVEGSPLUGIN
void CEmulator::setPluginInstance(void* _instance)
{
	pInstance = _instance;
}
#endif

void CEmulator::mountDisks()
{
	config->need2remount=true;
	forceFocus();
}


int CEmulator::theThread()
{
	
#ifdef TEST_RASTER
	
	x_test_raster();
	
#else

	if (!config)
	{
		x_fatal_exit("!!!MISSING CONFIG!!!\n");
		return 0;
	}

	if (bInThread)
	{
		x_fatal_exit("THREAD ALREADY RUNNING\n");
		return 0;
	}

	
	bInThread=true;
	outputInfo("emulator thread launched!\n");
	
	while(1)
	{
		config->need2remount=true;
		mountDelayedDisk();
		
		nbparams=0;
		bramParam.Empty();
		
	
		// 1- OPTION PAR DEFAULT DE L'EMULATEUR
		
		// initialise les options par défault
		setStart.slot=0;
		g_sim65816.g_6502_emulation = 0;
		g_sim65816.g_use_apple2e_rom = 0;


		// paramètre auto!
		if (config->bootslot==-1)
		{
			// default
			config->bootslot=5;	
			for(int b = 5;b<=7;b++)
			{
				const char* s = config->getSlot(b,1);
				if (s && s[0])
				{
					printf("bootslot set to %d\n",b);
					config->bootslot=b;
					break;
				}
			}
		}

		switch(config->bootslot)
		{
			case 6:
				setEmulatorParam("driveemulation:slow;emulate6502:1;");
				setBramParam("slot6:card;boot:6;border:black;background:black;font:white");

				break;
			case 5:
				setBramParam("boot=5");
				break;
			default:
				setEmulatorParam("driveemulation:fast;");
				setBramParam("slot6:disk");
				break;
		}
	
		// 1 la vitesse

		g_moremem.g_zipgs_reg_c05a = 0x0F; // MAX SPEED
		
		int sp = config->speed;
		if (sp == SPEED_AUTO)
		{
			if (config->bootslot==6)
				sp = SPEED_1MHZ;
			else
				sp = SPEED_GS;
			printf("speed auto set to %d\n",sp);
		}

		onSpeedChanged(sp,true);

		// 2 - LES OPTIONS DU FICHIERS DE CONF
	
		option.loadOptions();
				
		// 3 - LES OPTIONS DU FICHIER XML CONFIG
	
		printf("*loading option from xml\n");
		printf("EmulatorParam: %s\n",config->xmlEmulatorParam.c_str());
		printf("BramParam: %s\n",config->xmlBramParam.c_str());
		printf("SystemParam: %s\n",config->xmlSystemParam.c_str());

		option.addEmulatorParam(config->xmlSystemParam.c_str());
		setBramParam(config->xmlSystemParam.c_str());
		setBramParam(config->xmlBramParam.c_str());
		option.addEmulatorParam(config->xmlEmulatorParam.c_str());
		setEmulatorParam(config->xmlEmulatorParam.c_str());

		// 4 -  LES OPTIONS JAVASCRIPT
		printf("*loading option from javascript\n");
		option.addEmulatorParam(STRING2CONSTCHAR(config->systemParam));
		setBramParam(STRING2CONSTCHAR(config->systemParam));
		setBramParam(STRING2CONSTCHAR(config->bramParam));
		setEmulatorParam(STRING2CONSTCHAR(config->emulatorParam));
		option.addEmulatorParam(STRING2CONSTCHAR(config->emulatorParam));
		
		option.enableOptions();

        mountDisks();

		g_adb.g_adb_repeat_delay = option.getIntValue(OPTION_ADBDELAY);
		g_adb.g_adb_repeat_rate = option.getIntValue(OPTION_ADBRATE);

		int ar = option.getIntValue(OPTION_AUDIORATE);
		if (ar)
		{
			if (ar<11025  ||  ar>44100)
				ar  = 22100;
			g_sound.g_audio_enable = 1;
			g_sound.g_audio_rate = ar;
		}
		else 
			g_sound.g_audio_enable = 0;

		int mem = option.getIntValue(OPTION_MEMORYSIZE);

		if (mem <=8 && mem>=0)
			mem *= 1024*1024;
		else
		if (mem>=128 && mem<=8*1024)
		{
			mem *= 1024;
		}
		else
		if (mem>=128*1024 && mem<=8*1024*1024)
		{
		}
		else
			mem  = 2*1024*1024;

		g_sim65816.g_mem_size_exp = mem;

		if (config->bootslot!=6 && g_sim65816.g_use_apple2e_rom)
		{
			printf("disabling apple 2e rom (only works when booting from slot6\n");
			g_sim65816.g_use_apple2e_rom = 0;
		}

		
		
		g_savestate.init();
		if (config->stateFilename.IsEmpty())
		{
		}
		else
		{
			
            printf("about to restore : %s\n",config->stateFilename.c_str());
			g_savestate.restoreState(config->stateFilename.c_str());
		}

		printf("*** notifying boot option ***\n");
		x_refresh_panel(PANEL_BOOT);
		printf("*** launching Emulator Mainloop\n");
			
#ifndef MAC
		kegsmain(0,NULL);
#else
		extern int macmain(int,char**);
		macmain(0,NULL);		
#endif
		g_savestate.shut();

		joystick_shut();	
		
	
		if (!r_sim65816.restart_requested)
			break;
		else
		{
			outputInfo("*** Restarting emulator\n");
			r_sim65816.restart_requested = 0;
		
			outputInfo("*** bRestarting\n");
			setConfig(config);
		}
	}
	
#endif // TEST_RASTER

	outputInfo("end of emulator thread\n");
	
	bInThread=false;
	return 0;

	
}

void CEmulator::forceFocus()
{
	// TODO OG
}


void CEmulator::readImageInfo(int _slot,int _drive, int _imagelen, int _imagestart, int _isprodos)
{
	if (!this) return ;
	CSlotInfo& ref = getLocalIMGInfo(_slot,_drive);
	ref.size  = _imagelen;
	MyString prefix("?");
	if (_isprodos) //_info.track0_len!=-1)
	{
		char ptr[32];
		memset(ptr,0,sizeof(ptr));
		const char* filename = ref.filename.c_str(); 
		if (filename) 
		{
			FILE* f = fopen(filename,"rb");
			if (f)
			{
				fseek(f,_imagestart+0x404,SEEK_SET);
				if (16==fread(ptr,1,16,f))
				if ( ((ptr[0]&0xF0)==0xF0) )
				{	
					int len= ptr[0]&0x0F;
					ptr[len+1] = 0;
					prefix = "/";
					prefix += (ptr+1);
				}
				
				fclose(f);
			}
		}
	}
	else
		prefix = "NON-PRODOS";
	ref.prefix = prefix;
}

int CEmulator::getLocalMultipleIMGInfo(int _slot,int _drive,CSlotInfo& info, int _pos)
{
	CActiveImage& active = config->getActiveImage(_slot,_drive);
	if (_pos>=active.iNb) return 0;
	info = config->getLocalImage(_slot,_drive,_pos); 
	if (active.iActive==_pos) return 1;
	else
		return -1;
}

int CEmulator::mountDelayedDisk()
{
	for(int slot=5;slot<=7;slot++)
		for(int disk=1;disk<=2;disk++)
		{
			CSlotInfo& ref = getLocalIMGInfo(slot,disk);
			if (ref.status==DELAYEDMOUNT)
			{
				outputInfo("status set to READY2MOUNT for S%dD%d\n",slot,disk);
				ref.status=READY2MOUNT;
			}

		}
	return 0;
}

bool CEmulator::remainingDelayedDisk()
{
	for(int slot=5;slot<=7;slot++)
		for(int disk=1;disk<=2;disk++)
		{
			CSlotInfo& ref = getLocalIMGInfo(slot,disk);
			if (ref.status==DELAYEDMOUNT)
				return true;
		}
	return false;
}

int CEmulator::getSmartSwap()
{
	int found = 0;
	for(int slot=5;slot<=6;slot++)
	{
		for(int disk=1;disk<=2;disk++)
		{
			CActiveImage& active = config->getActiveImage(slot,disk);
			if (active.iNb > 1)
			{
				if (found)
				{
					// multiple multiple : does not know how to handle
					return 0;
				}
				else
					found = slot*10+disk;
			}
		}
	}
	return found;
}

int CEmulator::smartSwap()
{
	int smart = getSmartSwap();
	if (!smart)
	{
		outputInfo("smart swap not available\n");
		return 0;
	}
	return swapDisk(smart/10,smart%10);
}



int CEmulator::swapDisk(int _slot,int _drive, int pos )
{
	CActiveImage& active = config->getActiveImage(_slot,_drive);
	if (active.iNb <=1)
	{
		outputInfo("nothing to swap\n");
		return 0;
	}

	CSlotInfo& ref = getLocalIMGInfo(_slot,_drive);
	if (ref.status != UNDEFINED) 
	{
		ref.status = EJECTED ;
		outputInfo("status set to EJECTED for S%dD%d\n",_slot,_drive);
	}
	//ejectDisk(_slot,_drive);

 
        // passe à la suivante
        if (pos<0)
            pos = active.iActive+1;
        if (pos>=active.iNb)
            pos=0;
    
	active.iActive = pos;
	outputInfo("swapping to entry %d\n",pos);

	CSlotInfo& info = getLocalIMGInfo(_slot,_drive);
	if (!info.url.IsEmpty())
	{
		outputInfo("status set to DELAYEDMOUNT for S%dD%d\n",_slot,_drive);
		info.status = DELAYEDMOUNT;
		info.delay = (int)(60*1.5);	// 5s
	}

	config->need2remount=true;
	x_refresh_panel(PANEL_DISK);
	return 1;
}

CSlotInfo& CEmulator::getLocalIMGInfo(int _slot,int _drive)
{
	CActiveImage& active = config->getActiveImage(_slot,_drive);
	return config->getLocalImage(_slot,_drive,active.iActive);
}


int CEmulator::mountImages()
{
	if (!this) return 0;

#if defined(ACTIVEGSPLUGIN) && !defined(DRIVER_IOS)
	checkAlerts();
#endif

	if (config->need2remount)
	{
		//outputInfo("mounting images...\n");
		
		int ri = 0;
		for(int slot=5;slot<=7;slot++)
		{
			int maxdrive=2;
			if (slot==7) maxdrive=MAXSLOT7DRIVES;
			for(int drive=1;drive<=maxdrive;drive++)
			{
				ri |= loadDiskImage(/*name,*/slot,drive);
				if (r_sim65816.should_emulator_terminate())
					return 0 ;
				// regarde si il faut patcher le disque systËme
				if (setStart.slot == (slot*10)+drive)
					setStartParam();
			}
		}
			
		config->need2remount = (int)remainingDelayedDisk();
		if (ri)
			x_refresh_panel(PANEL_DISK);

		return 1;	
	}
	else
		return 0;

}

int CEmulator::loadDiskImage(int _slot,int _drive)
{
	ASSERT(theEmulator==this);

	CSlotInfo& ref = getLocalIMGInfo(_slot,_drive);
	if (ref.status != READY2MOUNT) 
		return 0;

	if (ref.url.IsEmpty())
	{
		ref.status = NOTHING;
		outputInfo("status set to NOTHING for S%dD%d\n",_slot,_drive);
		ref.filename.Empty();
		ref.shortname.Empty();
		return 1;
	}

	MyString path;
	MyString shortname;

	CDownload dl(this->config->baseURL);
#ifdef DRIVER_ANDROID
	dl.fromMainThread = false;
#endif


	if (dl.retrieveFile(STRING2CONSTCHAR(ref.url),path,shortname))
	{
		ref.filename = path ;
		ref.shortname = shortname ;
		ref.status = MOUNTED ;
		outputInfo("status set to MOUNTED for S%dD%d\n",_slot,_drive);
		//ref.delay = 1;
		::showStatus("Image %s mounted [S%dD%d]",ref.shortname.c_str(),_slot,_drive);
		return 1 ;
	}
	else
	{
		ref.status = FAILED ;
		outputInfo("status set to FAILED for S%dD%d\n",_slot,_drive);
		ref.shortname = getfile(STRING2CONSTCHAR(ref.url));
		return 1;
	}

}

void CEmulator::resetLocalIMG(void)
{
}

// Interface

const char* getLocalIMG(int _slot,int _drive)
{
	CEmulator* emu = CEmulator::theEmulator;
	CSlotInfo& ref = emu->getLocalIMGInfo(_slot,_drive);
	
	if (ref.status == DELAYEDMOUNT)
	{
		ref.delay--;
		if (ref.delay<=0)
		{
			ref.status = READY2MOUNT;
			outputInfo("status set to READY2MOUNT for S%dD%d : image %s\n",_slot,_drive,ref.shortname.c_str());
		}
		emu->config->need2remount=true;
		return NULL;
	}

	if (ref.status == MOUNTED)
		return ref.filename.c_str() ;
	else
		return NULL;
}


int mountImages()
{
	return CEmulator::theEmulator->mountImages();
}

void updateBramParameters(unsigned char* bramPtr)
{
	return CEmulator::theEmulator->updateBramParameters();
}

void readImageInfo(int _slot,int _drive, int _imagelen, int _imagestart, int _isprodos)
{
	return CEmulator::theEmulator->readImageInfo(_slot,_drive,_imagelen,_imagestart,_isprodos);
}

/*
void refreshInfo()
{
	CEmulator* emu = CEmulator::theEmulator;
	if (!emu) return ;
	emu->infoRefresh();
}
*/

void disableConsole()
{
	CEmulator* emu = CEmulator::theEmulator;
	if (!emu) return ;
	emu->disableConsole();

}

int activeRead(char* buf)
{
	CEmulator* emu = CEmulator::theEmulator;
	if (!emu)
	{
		buf[0]=0;
		return 0;
	}
	else
		return (emu->activeRead(buf));

}



void	COption::enableOptions()
{

	
	r_sim65816.g_ignore_bad_acc = !getIntValue(OPTION_HALTONBADACC);
	r_sim65816.g_ignore_halts = !getIntValue(OPTION_HALTONHALTS);
	r_sim65816.g_halt_on_bad_read = getIntValue(OPTION_HALTONBADREAD);
	r_sim65816.set_rewind_enable(getIntValue(OPTION_ENABLEREWIND));
	
	r_sim65816.set_color_mode( (colormodeenum)getIntValue(OPTION_COLORMODE));
	r_sim65816.set_video_fx((videofxenum)getIntValue(OPTION_VIDEOFX));
	
	g_joystick_type = getIntValue(OPTION_JOYSTICKMODE);
#ifndef VIRTUALAPPLE
	g_sim65816.set_fixed_vblank( getIntValue(OPTION_FIXEDVBLANK),(speedenum)getIntValue(OPTION_SPEED) );
#endif
    
	g_sim65816.g_enable_disk_sound = getIntValue(OPTION_DISKSOUND);
    
	g_paddles.g_joystick_scale_factor_x = (getIntValue(OPTION_JOYXSCALE)*256)/1000; // 0x100 = 1
	g_paddles.g_joystick_scale_factor_y = (getIntValue(OPTION_JOYYSCALE)*256)/1000; // 0x100 = 1
	
    
    g_adb.g_simulate_space = getIntValue(OPTION_SIMULATESPACE);
    

	if (r_sim65816.get_state()==RUNNING)
		paddle_fixup_joystick_type();	
#ifdef DRIVER_IOS
	extern int x_frame_rate;
	extern int x_lock_zoom;	
	x_frame_rate = getIntValue(OPTION_FRAMERATE);
#warning "TODO CHANGE FPS IN REALTIME"
	x_lock_zoom = getIntValue(OPTION_LOCKZOOM);
#endif
	
	x_refresh_panel(PANEL_RUNTIME);


}


void	CEmulator::setLight(int _motorOn,int _slot, int _drive, int _track)
{
	int id=0;
	for(int slot=5;slot<=6;slot++)
		for(int drive=1; drive<=2; drive++)
		{
			if ( (_slot==slot) && (_drive==drive) )
			{
				lights[id]=_motorOn;
				tracks[id]=_track;
			}
			else
				lights[id]=0;
			id++;	
		}
	
}

//extern char targetSpeed[256];
extern char estimatedSpeed[256];


void  CEmulator::processCommand( const char* _val, MyString& ret)
{
//	outputInfo("processingCommand:%s\n",_val);
	if (!strcmp(_val,"help"))
		ret = "({ commands : [ \"help\" , \"version\", \"system\", \"status\", \"runtime\", \"reboot\",\"savestate\", \"restorestate\",  \"pause\", \"resume\" ,\"pauseresume\", \"swap\", \"screenshot\",\"saveoptions\",\"defaultoptions\",\"sendkey:ctrl;alt;esc\" ] })";
	else
	if (!strcasecmp(_val,"saveoptions"))
	{
		option.saveOptions(1);
		ret = "({ \"info\" : \"options saved\"})";
	}
	else
	if (!strcasecmp(_val,"defaultOptions"))
	{
		option.setDefaultOptions();
		option.enableOptions();
		ret = "({ \"info\" : \"default options restored\"})";
	}
	else
	if (!strncasecmp(_val,"sendkey:",8 /* sizeof ("sendkey:")*/))
	{
#if defined (DRIVER_OSX) && !defined(ACTIVEGSPLUGIN)
		ret = "({ \"info\" : \"not implemented\"})";
#else
		const char* keystr[] = { "^ctrl" , "^alt", "^esc", "^reset", "^option", NULL };
		const int keycode[] = { 0x36 , 0x37 , 0x35, 0x7F, 0x3A, 0 };
#define MAX_CODE 100
		int	code[MAX_CODE];
		int nbcode=0;
		const char* ptr = _val+8;
		MyString keysent;
		while(*ptr)
		{
			const char* p = ptr;
			while(*p && *p!=';') p++;
			int l = p-ptr;
			int k = 0;
			while(keystr[k])
			{
				if (!strncasecmp(keystr[k],ptr,l))
				{
					code[nbcode++]=k;
				//	printf("found %s (%X)\n",keystr[k],keycode[k]);
					keysent+=keystr[k];
					keysent+=";";
					break;
				}
				k++;
			}
			if (!keystr[k])
				printf("key not valid %s\n",ptr);
			ptr = p;
			if (*ptr==';') ptr++;
			if (nbcode==MAX_CODE) break;
		}

		for(int i=0;i<nbcode;i++)
		{
			add_event_key(keycode[code[i]],0);
			for(int j=0;j<30;j++)
				add_event_delay();
		}
		
		for(int i=0;i<60;i++)
				add_event_delay();

		for(int i=nbcode-1;i>=0;i--)
		{
			add_event_key(keycode[code[i]],1);
		}
	
		ret.Format("({ \"info\" : \"key sent %s\"})",keysent.c_str());
#endif
	}
	else
	if (!strcasecmp(_val,"version"))
	{
		ret = "({ \"info\" : \"" ACTIVEGSVERSIONSTRFULL "\"})";
	}
	else
		/*
	if (!strcmp(_val,"rewind"))
	{
		stateActionRequired = STATE_REWIND;
		ret = "({ \"info\" : \"rewind activated\"})";
	}
	else
	*/
	if (!strcasecmp(_val,"system"))
	{
		ret = "( { ";
		
		ret += " \"currentBuild\" : \"" TOSTRING(ACTIVEGSMAJOR) "." TOSTRING(ACTIVEGSMINOR) "." TOSTRING(ACTIVEGSBUILD) "\",";
		MyString str;
		int g_joystick_plugged = (g_joystick_native_type1 != -1) || (g_joystick_native_type2 != -1) ? 1 : 0 ;
		str.Format("\"joystickPlugged\" : %d,",g_joystick_plugged);
		ret += str.c_str();
		MyString engine;
		getEngineString(engine);
		
		str.Format("\"renderingEngine\" : \"%s\",",engine.c_str());
		ret += str.c_str();
		
		str.Format("\"memorySize\" : %d,",option.getIntValue(OPTION_MEMORYSIZE));
		ret += str.c_str();

		str.Format("\"audioRate\" : %d",option.getIntValue(OPTION_AUDIORATE));
		ret += str.c_str();

		
		ret +="})";
	}	
	else	
	if (!strcasecmp(_val,"reboot"))
	{
		// Force la relecture du fichier XML!
		config->xmlAlreadyLoaded = 0;
		onBootSlotChanged(-1);	// Bootslot Auto
		ret = "({ \"info\" : \"Rebooted!\" })";
	}	
	else
	if (!strcasecmp(_val,"status"))
	{
		MyString str;
		ret = "( {";	
		str.Format("\"vbl\" : %d,",g_sim65816.g_vbl_count);
		ret += str;
		str.Format("\"speed\" : { estimated:\"%s\", target:\"",estimatedSpeed);
		if (!g_sim65816.g_speed_fast)
			str +="1";
		else
			str += option.getDescription(OPTION_SPEED,g_sim65816.get_limit_speed());
				str +="\"},";
		ret += str;
		ret += " \"lights\" : {";
		for(int i=0;i<4;i++)
		{
			str.Format(" \"slot%d%d\" : { \"on\" : %d, \"track\" : %d }",(i/2)+5,(i&1)+1,lights[i],tracks[i]);
			ret += str;	
			if (i!=3)
				ret+=",";
	
		}
		ret += "}";
		ret += "} )";
	}	
	else
	if (!strcasecmp(_val,"runtime"))
	{
		MyString str;
		ret = "( {";
		str.Format("\"pause\" : %d,",r_sim65816.get_state()==IN_PAUSE);
		ret += str;
		str.Format("\"swap\" : %d,",getSmartSwap());
		ret += str;
		str.Format("\"rewindAvailable\" : %d,",r_sim65816.get_rewind_enable());
		ret += str.c_str();
		
		str.Format("\"savedState\" : %d,", g_savestate.getSavedStateVBL());
		ret += str.c_str();
		
		ret += "\"slots\" : [";
		int nb = 0;
		for(int slot=5;slot<=7;slot++)
		{
			int maxdrive=2;
			if (slot==7) maxdrive=MAXSLOT7DRIVES;
			for(int disk=1;disk<=maxdrive;disk++)
				{
					for(int active=0;active<ACTIVEGSMAXIMAGE;active++)
					{
					 CSlotInfo info;
					int g = getLocalMultipleIMGInfo(slot,disk,info,active);
					if (!g) continue;
					if (info.status==UNDEFINED || info.status ==NOTHING) continue;
					if (g!=1) g=0;
					MyString prefix;
					switch(info.status)
					{
						case MOUNTED:
							prefix = "MOUNTED";;
							break;
						case EJECTED:
							prefix = "EJECTED";
							break;
						case READY2MOUNT:
							prefix = "READY2MOUNT";
							break;
						case DELAYEDMOUNT:
							prefix = "DELAYEDMOUNT";
							break;
						case FAILED:
							prefix	="FAILED";
							break;
                        default:
                            prefix = "N/A";
                            break;
					}
				
					if (nb)
						ret +=",";
					MyString str;
					str.Format(" { \"slot\" : %d%d, \"active\" : %d, \"name\" : \"%s\" , \"status\" : \"%s\" }",slot,disk,g,info.shortname.c_str(),prefix.c_str());
					
					ret+=str.c_str();
					nb++;
				}
			}
		}
		ret+="]";
		str.Format(",\"joystickMode\" : \"%s\"",option.getDescription(OPTION_JOYSTICKMODE,g_joystick_type));
		ret += str.c_str();
		ret += "} )";
	}	
	else
	if (!strcasecmp(_val,"pause"))
	{
	//	extern int g_config_control_panel;
	//	if (!isInitialized())
		if (r_sim65816.get_state()==RUNNING)
		{
			r_sim65816.pause();
			ret = "({ \"info\" : \"Paused!\" , \"pause\" : 1 })";
		}
		else
		if (r_sim65816.get_state()==IN_PAUSE)
			ret = "({ \"error\" : \"already in pause\" })";
		else
			ret = "({ \"error\" : \"emulator not accepting event\" })";
		
	}	
	else
	if (!strcasecmp(_val,"pauseresume"))
	{
	
		if (r_sim65816.get_state()==RUNNING)
		{
			r_sim65816.pause();
			ret = "({ \"info\" : \"Paused!\" , \"pause\" : 1 })";
		}
		else
		if (r_sim65816.get_state()==IN_PAUSE)
		{
			r_sim65816.resume();
			ret = "({ \"info\" : \"Resumed!\" , \"pause\" : 0 })";
		}
		else
			ret = "({ \"error\" : \"emulator not accepting event\" })";
		
	}	
	else
	if (!strcasecmp(_val,"savestate"))
	{
		if (!r_sim65816.get_rewind_enable())
			ret= "({\"error\" : \"Feature not available\"})";
			g_savestate.saveState(NULL);
		//	int vbl = g_savestate.getSavedStateVBL();
			MyString str;
			str.Format("({\"info\" : \"State Saved\",})",g_sim65816.g_vbl_count);
			ret= str;
	}
	else
	if (!strcasecmp(_val,"restorestate"))
	{
		if (!r_sim65816.get_rewind_enable())
			ret= "({\"error\" : \"Feature not available\"})";
		int vbl = g_savestate.getSavedStateVBL();
		if (!vbl)
			ret= "({\"error\" : \"Nothing to restore\" })";
		else
		{
			g_savestate.restoreState(NULL);
			MyString str;
			str.Format("({\"info\" : \"State Restored\", \"vbl\" : %d })",vbl);
			ret= str;
		}
	}
	else
	if (!strcasecmp(_val,"resume"))
	{
	//	extern int g_config_control_panel;
		if (r_sim65816.get_state()==IN_PAUSE)
		{
				r_sim65816.resume();
				ret = "({ \"info\" : \"Resumed!\" , \"pause\" : 0 })";
		}
		else
		if (r_sim65816.get_state()==RUNNING)
			ret = "({ \"error\" : \"nothing to resume\" })";
		else
			ret = "({ \"error\" : \"Emulator not accepting event\" })";
	
	}	
	else
	if (!strcasecmp(_val,"screenshot"))
	{
		extern int savePNG(MyString&);
		savePNG(ret);
	}	
	else
	if (!strcasecmp(_val,"swap"))
	{
		int sw = getSmartSwap();
		if (!sw)
			ret= "({\"error\" : \"Nothing to swap\"})";
		else
		{
			smartSwap();
			CSlotInfo& info = this->getLocalIMGInfo(sw/10,sw%10);
			ret = info.shortname;
			ret.Format("({ \"info\" : \"disk %s inserted\", \"slot\" : %d, name : \"%s\" })",info.shortname.c_str(),sw,info.shortname.c_str());
		}
	}
	else
	{
		ret="({ \"error\" : \"unsupported command:";
		ret += _val ;
		ret += "\"})";
	}
}


// initialize le son depuis le thread principal
void	CEmulator::launchEmulator()
{
	x_async_sound_init();
	//CDownload::initPersistentPath();
	x_LaunchThread();
	// attend la fin de l'émulateur
//	x_WaitForEndThread();
	
}



void	CEmulator::terminateEmulator()
{
	outputInfo("terminateCEmulator (%X)\n",this);
	
	if (bInThread)
	{
		
		// envoie le signal de quit
		r_sim65816.quit_emulator();
		
		// attend la fin de l'émulateur
		x_WaitForEndThread();
		
		x_async_snd_shutdown();
		
	}
	
	// l'émulateur a quitté correctement : il est pret à redémarrer
	r_sim65816.reset_quit();
	// OG 080111 Force un redraw pour rafraichir le contenu du browser si besoin 
	refresh_video(1);
	
	
}

CEmulator::~CEmulator()
{
	outputInfo("~CEmulator (%X)",this);

	// termine le singleton
	theEmulator=NULL;

	if (parent)
		*parent = NULL;

	
}

void CEmulator::onXMLConfigChanged(const char *_xmlconfig,simplexml* _pXML)
{
	if (!acceptEvents())
	{
		outputInfo("### onXMLConfigChanged Ignored");
		return;
	}

	MyString newxmlconfig = _xmlconfig; // le copie si la valeur était déjà stockée dans xmlconfig
//	config->resetConfig(); // remets les valeur par défault
	config->xmlconfig = newxmlconfig.c_str();
	config->pXML = _pXML;
	config->xmlAlreadyLoaded=0; // pour forcer la relecture au prochain chargement
	relaunchEmulator();

}

extern void addNotificationWindow();

void addConsoleWindow(Kimage* _dst)
{
#if defined(_DEBUG) && defined(WIN32) && 0
	extern void addConsole(const char*);

//	extern char estimatedSpeed[256];
	extern	void addConsole(const char* _str);
//	extern	int getFrameRate(int _id);
	extern	void drawConsole(Kimage* _dst);
	char str[256];
	int fr0 = getFrameRate(0);
	int fr1 = getFrameRate(1);
	int fr2 = getFrameRate(2);
	int fr3 = getFrameRate(3);
	sprintf(str,"fps:%02d.%0d 60hz:%02d.%0d req:%02d.%0d c054:%02d.%0d ",fr0/10,fr0%10,fr3/10,fr3%10,fr1/10,fr1%10,fr2/10,fr2%10);
//	sprintf(str,"p1:%d p2:%d fps:%d ",getFrameRate(3),getFrameRate(4),getFrameRate(1));
	addConsole(str);
	drawConsole(_dst);
#endif
//	addNotificationWindow();
}




#ifdef TEST_RASTER
int x_test_raster()
{
	x_video_init();
	r_sim65816.set_state(RUNNING);
	
	while(!r_sim65816.should_emulator_terminate())
	{
		
		static int pos=0;
	
		// faire scroller 
		
		//	int	bitmapBytesPerRow   = (pixelsWide * 4);// 1
		int bitmapBytesPerRow = g_kimage_offscreen.width_act * (g_kimage_offscreen.mdepth >> 3);

		int destx=0;
		int desty = 0;
		int col = pos;
		
		x_wait_for_last_refresh(1.0/60.0);

		CHANGE_BORDER(0,0x00FF00);

		byte* ptrdest = (byte*)g_kimage_offscreen.data_ptr ;
		for(int i=0;i<g_kimage_offscreen.height;i+=1)
		{
			col = pos;
			for(int j=0;j<g_kimage_offscreen.width_act/2;j++)
			{
				((word32*)ptrdest)[j] = col;
				col+=10;
			}
		//	memset(ptrdest,col,bitmapBytesPerRow);
			ptrdest += bitmapBytesPerRow;
		//	col += 10;
		}
		pos+=10;

		CHANGE_BORDER(0,-1);
		x_wait_for_vbl();
		CHANGE_BORDER(0,0);
		DRAW_BORDER();
		refresh_video(1);
		
		g_sim65816.g_vbl_count++;
	}

	r_sim65816.set_state(SHUTTING_DOWN);
	
	
	x_video_shut();

	return 1;
	
}

#endif

float g_notification_y=1.0;
#ifdef _DEBUG
int g_notification_show=1;
#else
int g_notification_show=0;
#endif
int g_notification_motion = 1;
float g_notification_speed = 1.0;


#define NOTIFICATION_HEIGHT 50
#define NOTIFICATION_WIDTH 300
#define NOTIFICATION_MARGIN 20


void addNotificationWindow()
{

	if (g_kimage_offscreen.mdepth!=32) return ;
	if (!g_notification_show) return ;
	
	int bitmapBytesPerRow = g_kimage_offscreen.width_act * (g_kimage_offscreen.mdepth >> 3);

	int destx= g_kimage_offscreen.width_act - NOTIFICATION_WIDTH - NOTIFICATION_MARGIN ;
		int col = 0xFFFFFF;
		
	for(int i=0;i<NOTIFICATION_HEIGHT;i+=1)
	{
		int y = (int)g_notification_y + i;
		if (y<0) continue;
		if (y>=g_kimage_offscreen.height) continue;

		byte* ptrdest = (byte*)g_kimage_offscreen.data_ptr + y*bitmapBytesPerRow;

		for(int j=0;j<NOTIFICATION_WIDTH;j++)
		{
			int x = j + destx;
			if ( (x<0) || (x>g_kimage_offscreen.width_act ) ) continue;
			((word32*)ptrdest)[x] = col;
		}
	}
	if (g_notification_motion)
	{
		g_notification_y += g_notification_speed;
		g_notification_speed += 0.5f;
		if (g_notification_y>= g_kimage_offscreen.height-(NOTIFICATION_HEIGHT+NOTIFICATION_MARGIN))
		{
			g_notification_y = (float)(g_kimage_offscreen.height-(NOTIFICATION_HEIGHT+NOTIFICATION_MARGIN));
			if (g_notification_speed<2.0f)
			{
				g_notification_motion = 0;
				g_notification_show = 3*60;
			}
			else
				g_notification_speed *= -0.5f;		
			
		}
	}
	else
	{
		g_notification_show--;
	}

//	g_notification_y++;
	if (g_notification_y>g_kimage_offscreen.height+NOTIFICATION_HEIGHT)
		g_notification_y = -NOTIFICATION_HEIGHT;

		g_video.g_needfullrefreshfornextframe = 1;

}




