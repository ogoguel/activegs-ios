/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#pragma once

//#ifndef MAC
#ifdef _WIN32
	#include <atlstr.h>
#endif

#include "../common/ActiveGSList.h"
#include "../common/activedownload.h"
#include "../common/ki.h"

#include "../kegs/src/sound.h"

extern int outputInfoInternal(const char*);


/*
#if defined(_DEBUG)|| defined(_GLIBCXX_DEBUG)
#define ADD2CONSOLE(X)
#else
extern void add2Console(const char*);
#define ADD2CONSOLE(X) addConsole(X)
#endif
*/

#define ACTIVEGSXMLPATH "_XML"

#if defined(WIN32) && !defined(UNDER_CE)
#define STRING2CONSTCHAR(X) ((const char*)X)
#else
#define STRING2CONSTCHAR(X) (X.c_str())
#endif


enum eParamType
{
	EMULATOR,
	BRAM
};

struct param
{
	MyString data;
	MyString value;
	int		hex;
};

class CSetStart
{
public:
	int slot;
//	int addr;
	MyString file;

	CSetStart() { slot=0; };
};

typedef enum 
{
	OPTION_ERROR = -1,
	OPTION_CONFIGVERSION = 0,
	OPTION_GUID,
	OPTION_HALTONBADREAD,
	OPTION_HALTONBADACC,
	OPTION_HALTONHALTS,
	OPTION_ENABLEREWIND,
	OPTION_COLORMODE,
	OPTION_VIDEOFX,
	OPTION_JOYSTICKMODE,
	OPTION_BROWSERNAME,
	OPTION_BROWSERVERSION,
	OPTION_LATESTRUNBUILD,
	OPTION_LATESTUPDATECHECK,
	OPTION_DONOTSHOWUPDATEALERT,

	OPTION_DONOTSHOWDISKSTATUS,
	OPTION_PNGBORDER,
	OPTION_SIMULATESPACE,
	OPTION_LATESTVERSION,
	
	OPTION_SPEED,
	OPTION_LATESTVERSIONDATE,
	OPTION_AUDIORATE,
	OPTION_MEMORYSIZE,
	OPTION_JOYXSCALE,
	OPTION_JOYYSCALE,
    OPTION_DISKSOUND,
    OPTION_FIXEDVBLANK,
	OPTION_ADBDELAY,
	OPTION_ADBRATE,

#ifdef DRIVER_IOS
	OPTION_FRAMERATE,
	OPTION_LOCKZOOM,
	OPTION_LASTTAB,
	OPTION_DEBUGMODE,
	OPTION_EXTERNALKBD,
#endif
#ifdef ACTIVEGSKARATEKA
    OPTION_SHOWICONS,
    OPTION_ENABLECHEAT,
    OPTION_SOUNDMODE,
    OPTION_KARATEKAREWIND,
#endif
#ifdef VIRTUALAPPLE
    OPTION_DEBUG,
    OPTION_SCORES,
    OPTION_GAMECENTER,
#endif
	NB_OPTIONS
} option_id;


typedef struct  
{
	int inConfig;
	int	inDefault;
	const char* name ;
	int	defaultint;
	const char* defaultstr;
	const char**	convertTableDesc;
	const char**	convertTableAlias;
	const int*		convertTableInt;
} namevaluedef;

class namevalue
{
public:
	const namevaluedef*	def;
	MyString		strvalue;
	int				intvalue;
	int		confOverrided;
	
	namevalue() { intvalue=0;confOverrided=0; def=NULL; }
	
	int convertToInt(const char* _value);
	int	belongsToValue(int v);	
	void getAlias( MyString& _str);
	const char* getDescription(int _value);
	int	getOrder(int _value);

};



class COption
{
public:
    int     initiliazed;
	namevalue	options[NB_OPTIONS];
	namevalue&	find(option_id _id) { return options[_id]; };
	option_id	findByName(const char* _name);
	
	void	getParameterFilename(MyString& filename,int nb);
	void	getStrValue(MyString& str,option_id _id);
	int		getIntValue(option_id _id);
	void	setValue(option_id _id,const char* _value, int overideConf=0);
	void	setIntValue(option_id _id,int _value,int overideConf=0);
	//void	setConfValue(option_id _id,const char* _value);
	//void	setConfIntValue(option_id _id,int _value);
    void    initOptions();
	void	setDefaultOptions();
	void	generateGUID(MyString& guid);
	void	loadOptions();
	int		loadParameters(int _nb);
	void	saveOptions(int _nb);
	bool	addEmulatorParam(const char* _param);
	const char*	getDescription(option_id _id, int _value);
     COption() { initiliazed = 0; };
///	void initOptions();
	void	enableOptions();
};



enum eslotstatus
{
	UNDEFINED,
	MOUNTED,
	READY2MOUNT,
	EJECTED,
	FAILED,
	NOTHING,
	DELAYEDMOUNT
};

enum eSetSlotPos
{
	ADD,
	REPLACECURRENT
};

class CActiveImage
{
public:
	int	iActive;
	int	iNb;
	CActiveImage(){iActive=0; iNb=0;}
};

class CSlotInfo
{
public:

#if defined(WIN32) && !defined(UNDER_CE)
	CString	url;
#else
	MyString	url;		// original url
#endif

	MyString filename;	// physical name
	MyString	prefix;
	MyString shortname;	// physical filename
	int		patch_addr;
	MyString	patch_value;
	int		size;
	int		delay;
	eslotstatus status;
	CSlotInfo() { delay=0; size=0; status=UNDEFINED; patch_addr=0; }
};

#define ACTIVEGSMAXIMAGE 16


#define MAX_CHECK 16
struct ConfigBP
{
    MyString value;
	char check[MAX_CHECK];
    int addr;
    enum mode_breakpoint mode;
};

class CEmulatorConfig
{
public:
	
	CActiveImage	activeImages[3][MAXSLOT7DRIVES];

	CSlotInfo	localImages[3][MAXSLOT7DRIVES][ACTIVEGSMAXIMAGE];
	MyString	baseURL;	
	CEmulatorConfig();
    int     nb_bps;
    ConfigBP BPs[MAX_BREAK_POINTS];
	void setBaseURL(const char* _url);
	MyString	uid;
	MyString	name;
	MyString	nameurl;
	MyString	desc;
	MyString	year;
	MyString	publisher;
	MyString	publisherurl;
//	MyString	persistentPath;
	MyString	visibleName;

	bool		need2remount;
//	bool		fastConfig;		// pour ignore les images dans le xml si le parametre a ÈtÈ passÈ en ligne de commande
#if defined(WIN32) && defined(_USRDLL)
	LONG		speed;
	LONG		bootslot;
	CString		xmlconfig;	
	CString		emulatorParam;
	CString		systemParam;
	CString		bramParam;
	CString		setStartParam;
#else
	int			speed;	
	int			bootslot;
	MyString	xmlconfig;	
	MyString	emulatorParam;
	MyString	bramParam;
	MyString	systemParam;
	MyString	setStartParam;
#endif
	MyString	xmlEmulatorParam;
	MyString	xmlSystemParam;
	MyString	xmlBramParam;
	MyString	stateFilename;

	simplexml*	pXML;
	CSlotInfo*		setSlot(int _slot, int _drive, const char* _image,  eSetSlotPos _pos);
	CActiveImage& getActiveImage(int _slot, int _drive);
	CSlotInfo& getLocalImage(int _slot,int _drive, int _pos);
	const char*	getSlot(int _slot, int _drive);
	int		xmlAlreadyLoaded;
	int		processXML(const char*_path,simplexml* ptrconfig);
	void	loadXMLFile();
	void	saveXMLFile(const char* _desc);
	void	processCommandLine(int argc, char** argv);
	void	resetConfig();
};

#define EMULATOR_MAX_PARAMS 256


extern COption		option;

class CEmulator
{

public:
	CEmulator**	parent;
	static CEmulator* theEmulator;

#ifdef ACTIVEGSPLUGIN
	void*	pInstance;
	void	setPluginInstance(void*);
#endif

	int			lights[4];
	int			tracks[4];
	void	setLight(int _motorOn,int _slot, int _drive, int _curtrack);
	CEmulatorConfig*	config;
	int		nbparams;
	struct param params[EMULATOR_MAX_PARAMS];
	volatile bool	bInThread;
	void*	theWindow;		// HWND or WINDOWREF
	int     x;
	int		y;
	int	width;
	int	height;

public:
	void	onSpeedChanged(int _speed, bool _ignoreAcceptEvents=false);
	void	onBootSlotChanged(int _bootslot);
	void	onXMLConfigChanged(const char *_xmlconfig,simplexml* _pXML);
	void	onEmulatorParamChanged(const char* _param);
	void	addUniqueParam(struct param _p);
	void	sendStats();
	void	checkAlerts();
	int		acceptEvents();

public:
	void		setConfig(CEmulatorConfig* _config);
	CSetStart	setStart;

	void		resetLocalIMG();	
	int		loadDiskImage(int _slot,int _drive);
	void		reloadDisk(int _slot,int _drive);
	void		relaunchEmulator();
	void		launchEmulator();
	void		terminateEmulator();
	// To be defined in son
	virtual void	x_LaunchThread() = 0;
	virtual void	x_WaitForEndThread() = 0;


public:
//	void		setURL(const char* _url);

	bool		setEmulatorParam(const char* _p ) {return parseParam(_p,EMULATOR);  }
	MyString		bramParam;
	void		setBramParam(const char* _p);
	bool		parseParam(const char* _p,eParamType _param);
	bool		setParam(eParamType _type, const char* _data, const char* _value);


	bool	setStartParam();
	bool	extractSetStartParam(const char* run);

public:
	void	mountDisks();
	void	forceFocus();
	

//protected:
public:

	CEmulator(CEmulator** _parent);
	virtual	~CEmulator() ;
	void	destroy();

public:
	void	updateBramParameters();	
	CSlotInfo& getLocalIMGInfo(int _slot,int _drive);
	int getLocalMultipleIMGInfo(int _slot,int _drive,CSlotInfo& info, int pos);
	int	getSmartSwap();
	int	smartSwap();
	int swapDisk(int _slot,int _drive, int pos=-1);
	int	theThread();
	void	mountDisk(int _slot,int _drive, const char* _url);
	int		mountImages();
	int		mountDelayedDisk();
	bool		remainingDelayedDisk();
	void		ejectDisk(int _slot,int _drive);
	void		readImageInfo(int _slot,int _drive, int _imagelen, int _imagestart, int _isprodos);
	
	MyString			statusMsg;

	void initBramParam(const char* _param);
	void initEmulatorParam(const char* _param);
	
void  processCommand(const char* _val, MyString& ret);
	
	virtual	void refreshStatus(const char* _msg) {} ;
		virtual	void showLoading( int _loading,int _disk) {};
		
	
		virtual void showStatus(const char* _msg) {} ;
		virtual void disableConsole() {} ;
		virtual void loadingInProgress(int _motorOn,int _slot, int _drive) {};
		virtual	int	activeRead(char *buf) { buf[0]=0; return 0; }
	
	virtual void getEngineString(MyString& _engine) {};
};

void x_display_alert(int _abort, const char* _title, const char* _message);




