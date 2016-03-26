/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#ifndef DRIVER_OSX
#error
#endif

#include "defc.h"
#include "protos_macdriver.h"
#include "adb.h"
#include "video.h"
#include "sim65816.h"
#include "sound.h"


pascal OSStatus quit_event_handler(EventHandlerCallRef call_ref, EventRef event, void *ignore);
pascal OSStatus my_cmd_handler(EventHandlerCallRef handlerRef, EventRef event, void *userdata);
pascal OSStatus my_win_handler(EventHandlerCallRef handlerRef, EventRef event, void *userdata);
pascal OSStatus dummy_event_handler(EventHandlerCallRef call_ref, EventRef in_event, void *ignore);


int		g_quit_seen = 0;

#define MAX_STATUS_LINES	7
#define X_LINE_LENGTH		88
#define MAX_MAC_ARGS		128

int	g_mac_argc = 0;
char	*g_mac_argv[MAX_MAC_ARGS];

extern void x_update_modifiers(word32 state);
extern char g_argv0_path[];
extern char *g_status_ptrs[MAX_STATUS_LINES];
extern const char g_kegs_version_str[];

extern WindowRef	g_main_window;
WindowRef	g_main_window_saved;
EventHandlerUPP g_quit_handler_UPP;
EventHandlerUPP g_dummy_event_handler_UPP;
RgnHandle	g_event_rgnhandle = 0;
FMFontFamily	g_status_font_family;


int		g_ignore_next_click = 0;

int		g_mainwin_active = 0;
int	g_mac_mouse_x = 0;
int	g_mac_mouse_y = 0;

extern int g_video_act_width;
extern int g_video_act_height;
extern int g_video_act_margin_left;
extern int g_video_act_margin_right;
extern int g_video_act_margin_top;
extern int g_video_act_margin_bottom;


Ptr		g_mac_fullscreen_state = 0;
Rect		g_main_window_saved_rect;

extern  char *g_fatal_log_strs[];
extern int g_fatal_log;

extern void x_add_modifiers(word32);

int
x_show_alert(int is_fatal, const char *str)
{
	DialogRef	alert;
	DialogItemIndex	out_item_hit;
	CFStringRef	cfstrref, cfstrref2;
	CFStringRef	okstrref;
	AlertStdCFStringAlertParamRec alert_param;
	OSStatus osstat;
	char	*bufptr, *buf2ptr;
	int	sum, len;
	int	i;
	
	/* The dialog eats all events--including key-up events */
	/* Call adb_all_keys_up() to prevent annoying key-repeat problems */
	/*  for instance, a key-down causes a dialog to appear--and the */
	/*  eats the key-up event...then as soon as the dialog goes, adb.c */
	/*  auto-repeat will repeat the key, and the dialog re-appears...*/
	adb_all_keys_up();
	
	sum = 20;
	for(i = 0; i < g_fatal_log; i++) {
		sum += strlen(g_fatal_log_strs[i]);
	}
	bufptr = (char*)malloc(sum);
	buf2ptr = bufptr;
	for(i = 0; i < g_fatal_log; i++) {
		len = strlen(g_fatal_log_strs[i]);
		len = MIN(len, sum);
		len = MAX(len, 0);
		memcpy(bufptr, g_fatal_log_strs[i], MIN(len, sum));
		bufptr += len;
		bufptr[0] = 0;
		sum = sum - len;
	}
	
	cfstrref = CFStringCreateWithCString(NULL, buf2ptr,
										 kCFStringEncodingMacRoman);
	
	printf("buf2ptr: :%s:\n", buf2ptr);
	
	osstat = GetStandardAlertDefaultParams(&alert_param,
										   kStdCFStringAlertVersionOne);
	
	if(str) {
		// Provide an extra option--create a file
		cfstrref2 = CFStringCreateWithFormat(kCFAllocatorDefault, NULL,
											 CFSTR("Create ~/Library/KEGS/%s"), str);
		alert_param.otherText = cfstrref2;
	}
	okstrref = CFSTR("Click OK to continue");
	if(is_fatal) {
		okstrref = CFSTR("Clock OK to exit KEGS");
	}
	CreateStandardAlert(kAlertStopAlert, cfstrref, okstrref,
						&alert_param, &alert);
	out_item_hit = -1;
	RunStandardAlert(alert, NULL, &out_item_hit);
	printf("out_item_hit: %d\n", out_item_hit);
	free(buf2ptr);
	
	clear_fatal_logs();		/* free the fatal_log string memory */
	return (out_item_hit >= 3);
	
}



pascal OSStatus
quit_event_handler(EventHandlerCallRef call_ref, EventRef event, void *ignore)
{
	OSStatus	err;
	
	err = CallNextEventHandler(call_ref, event);
	if(err == noErr) {
		g_quit_seen = 1;
	}
	return err;
}

void
show_simple_alert(const char *str1,const char *str2, const char *str3, int num)
{
	char		buf[256];
	
	g_fatal_log_strs[0] = kegs_malloc_str(str1);
	g_fatal_log_strs[1] = kegs_malloc_str(str2);
	g_fatal_log_strs[2] = kegs_malloc_str(str3);
	g_fatal_log = 3;
	if(num != 0) {
		snprintf(buf, 250, ": %d", num);
		g_fatal_log_strs[g_fatal_log++] = kegs_malloc_str(buf);
	}
	x_show_alert(0, 0);
}



void cfg_toggle_config_panel();

pascal OSStatus
my_cmd_handler( EventHandlerCallRef handlerRef, EventRef event, void *userdata)
{
	OSStatus	osresult;
	HICommand	command;
	word32		command_id;
	
	osresult = eventNotHandledErr;
	
	GetEventParameter(event, kEventParamDirectObject, typeHICommand, NULL,
					  sizeof(HICommand), NULL, &command);
	
	command_id = (word32)command.commandID;
	switch(command_id) {
		case 'Kbep':
			SysBeep(10);
			osresult = noErr;
			break;
		case 'abou':
			show_simple_alert("KEGSMAC v", (char *)g_kegs_version_str,
							  ", Copyright 2004 Kent Dickey\n"
							  "Latest version at http://kegs.sourceforge.net/\n", 0);
			osresult = noErr;
			break;
		case 'KCFG':
#ifdef ACTIVEGS
#else
			cfg_toggle_config_panel();
#endif
			osresult = noErr;
			break;
		case 'quit':
			break;
		case 'swin':
			/* not sure what this is, but Panther sends it */
			break;
		default:
			printf("commandID %08x unknown\n", command_id);
			SysBeep(90);
			break;
	}
	return osresult;
}



pascal OSStatus
my_win_handler(EventHandlerCallRef handlerRef, EventRef event, void *userdata)
{
	OSStatus os_result;
	UInt32	event_kind;
	
	os_result = eventNotHandledErr;
	
	// SysBeep(1);
	
	event_kind = GetEventKind(event);
	// show_alert("win handler", event_kind);
	if(event_kind == kEventWindowDrawContent)
	{
		update_window();
	} if(event_kind == kEventWindowClose) {
		
		r_sim65816.quit_emulator(); 
		
#ifndef ACTIVEGS
		g_quit_seen = 1;
		my_exit(0);
#else
		// OG notify emulator that window has closed
		g_main_window = 0;
#endif
	} else {
		//show_event(GetEventClass(event), event_kind, 0);
		update_window();
	}
	
	return os_result;
}


pascal OSStatus
dummy_event_handler(EventHandlerCallRef call_ref, EventRef in_event,
					void *ignore)
{
	OSStatus	err;
	EventHandlerRef	installed_handler;
	EventTypeSpec	event_spec = { kEventClassApplication, kEventAppQuit };
	
	// From http://developer.apple.com/qa/qa2001/qa1061.html
	// Trick to move main event queue to use ReceiveNextEvent in an event
	//  handler called by RunApplicationEventLoop
	
	err = InstallApplicationEventHandler(g_quit_handler_UPP, 1, &event_spec,
										 NULL, &installed_handler);
	
    	x_async_sound_init();
	kegsmain(g_mac_argc, g_mac_argv);
		x_async_snd_shutdown();
    
	return noErr;
}

/*
void add_event_key(int _k,int _u) 
{
}
 */

void
check_input_events()
{
	OSStatus	err;
	EventTargetRef	target;
	EventRef	event;
	UInt32		event_class, event_kind;
	byte		mac_keycode;
	UInt32		keycode;
	UInt32		modifiers;
	Point		mouse_point, mouse_delta_point;
	WindowRef	window_ref;
	int	button, button_state;
	EventMouseButton	mouse_button;
	int		handled;
	int		mouse_events;
	int		is_up;
	int		in_win;
	int		ignore;
	
	if(g_quit_seen) {
		exit(0);
	}
	
	if (!g_main_window)
		return ;
	SetPortWindowPort(g_main_window);
	
	mouse_events = 0;
	target = GetEventDispatcherTarget();
	while(1) {
		err = ReceiveNextEvent(0, NULL, kEventDurationNoWait,
							   true, &event);
		
		if(err == eventLoopTimedOutErr) {
			break;
		}
		if(err != noErr) {
			printf("ReceiveNextEvent err: %d\n", (int)err);
			break;
		}
		
		event_class = GetEventClass(event);
		event_kind = GetEventKind(event);
		handled = 0;
		switch(event_class) {
			case kEventClassKeyboard:
				handled = 1;
				keycode = 0;
				modifiers = 0;
				GetEventParameter(event, kEventParamKeyMacCharCodes,
								  typeChar, NULL, sizeof(byte), NULL,
								  &mac_keycode);
				GetEventParameter(event, kEventParamKeyCode,
								  typeUInt32, NULL, sizeof(UInt32), NULL,
								  &keycode);
				GetEventParameter(event, kEventParamKeyModifiers,
								  typeUInt32, NULL, sizeof(UInt32), NULL,
								  &modifiers);
				
				x_update_modifiers((word32)modifiers);
				
				// Key up/down event
				is_up = -1;
				switch(event_kind) {
					case kEventRawKeyDown:
						is_up = 0;
						//printf("key down: %02x, %08x\n",
						//	(int)mac_keycode, (int)keycode);
						break;
					case kEventRawKeyUp:
						is_up = 1;
						//printf("key up: %02x, %08x\n",
						//	(int)mac_keycode, (int)keycode);
						break;
					case kEventRawKeyModifiersChanged:
						is_up = -1;
						//printf("key xxx: %08x\n", (int)modifiers);
						break;
				}
				if(is_up >= 0) {
					adb_physical_key_update((int)keycode, is_up);
				}
				break;
			case kEventClassMouse:
				handled = 2;
				mouse_events++;
				GetEventParameter(event, kEventParamMouseLocation,
								  typeQDPoint, NULL, sizeof(Point), NULL,
								  &mouse_point);
				GetWindowRegion(g_main_window, kWindowContentRgn,
								g_event_rgnhandle);
				in_win =  PtInRgn(mouse_point, g_event_rgnhandle);
				// in_win = 1 if it was in the contect region of window
				err = GetEventParameter(event, kEventParamMouseDelta,
										typeQDPoint, NULL, sizeof(Point), NULL,
										&mouse_delta_point);
				button = 0;
				button_state = -1;
				switch(event_kind) {
					case kEventMouseDown:
						button_state = 7;
						handled = 3;
						break;
					case kEventMouseUp:
						button_state = 0;
						handled = 3;
						break;
				}
				if(button_state >= 0) {
					GetEventParameter(event, kEventParamMouseButton,
									  typeMouseButton, NULL,
									  sizeof(EventMouseButton), NULL,
									  &mouse_button);
					button = mouse_button;
					if(button > 1) {
						button = 4 - button;
						button = 1 << button;
					}
					ignore = (button_state != 0) &&
					(!in_win || g_ignore_next_click);
					ignore = ignore || !g_mainwin_active;
					if(ignore) {
						// Outside of A2 window, ignore clicks
						button = 0;
					}
					if(button_state == 0) {
						g_ignore_next_click = 0;
					}
				}
				
				GlobalToLocal(&mouse_point);
				
				if(g_adb.g_warp_pointer) {
					if(err == 0) {
						g_mac_mouse_x += mouse_delta_point.h;
						g_mac_mouse_y += mouse_delta_point.v;
					}
					mac_warp_mouse();
				} else {
					g_mac_mouse_x = mouse_point.h -
					g_video_act_margin_left;
					g_mac_mouse_y = mouse_point.v -
					g_video_act_margin_top;
				}
				
#if 0
				printf("Mouse %d at: %d,%d button:%d, button_st:%d\n",
					   mouse_events, g_mac_mouse_x, g_mac_mouse_y,
					   button, button_state);
				printf("Mouse deltas: err:%d, %d,%d\n", (int)err,
					   mouse_delta_point.h, mouse_delta_point.v);
#endif
				
				update_mouse(g_mac_mouse_x, g_mac_mouse_y,
							 button_state, button & 7);
				if(g_adb.g_warp_pointer) {
					g_mac_mouse_x = A2_WINDOW_WIDTH/2;
					g_mac_mouse_y = A2_WINDOW_HEIGHT/2;
					update_mouse(g_mac_mouse_x, g_mac_mouse_y,0,-1);
				}
				
			
#if 0
				// regarde si c'est un double click
				int nbclick;
				GetEventParameter(event, kEventParamClickCount,
								  typeUInt32, NULL,
								  sizeof(typeUInt32), NULL,
								  &nbclick);
				if (nbclick==2)
				{
					extern void add_simulate_space(); 
					add_simulate_space();
				}
#endif
				
				
				break;
			case kEventClassApplication:
				switch(event_kind) {
					case kEventAppActivated:
						handled = 1;
						g_mainwin_active = 1;
						window_ref = 0;
						GetEventParameter(event, kEventParamWindowRef,
										  typeWindowRef, NULL, sizeof(WindowRef),
										  NULL, &window_ref);
						if(window_ref == g_main_window) {
							g_ignore_next_click = 1;
						}
						break;
					case kEventAppDeactivated:
						handled = 1;
						g_mainwin_active = 0;
						g_ignore_next_click = 1;
						break;
				}
				break;
		}
	//	show_event(event_class, event_kind, handled);
		if(handled != 1) {
			(void)SendEventToEventTarget(event, target);
		}
		ReleaseEvent(event);
	}
	
	return;
}

void
temp_run_application_event_loop(void)
{
	OSStatus	err;
	EventRef	dummy_event;
	EventHandlerRef	install_handler;
	EventTypeSpec	event_spec = { 'KWIN', 'KWIN' };
	
	// Create UPP for dummy_event_handler and for quit_event_handler
	err = noErr;
	dummy_event = 0;
	
	g_dummy_event_handler_UPP = NewEventHandlerUPP(dummy_event_handler);
	g_quit_handler_UPP = NewEventHandlerUPP(quit_event_handler);
	if((g_dummy_event_handler_UPP == 0) || (g_quit_handler_UPP == 0)) {
		err = memFullErr;
	}
	
	if(err == noErr) {
		err = InstallApplicationEventHandler(g_dummy_event_handler_UPP,
											 1, &event_spec, 0, &install_handler);
		if(err == noErr) {
			err = MacCreateEvent(NULL, 'KWIN', 'KWIN',
								 GetCurrentEventTime(), kEventAttributeNone,
								 &dummy_event);
			if(err == noErr) {
				err = PostEventToQueue(GetMainEventQueue(),
									   dummy_event, kEventPriorityHigh);
			}
			if(err == noErr) {
				RunApplicationEventLoop();
			}
			
			(void)RemoveEventHandler(install_handler);
		}
	}
	
	if(dummy_event != NULL) {
		ReleaseEvent(dummy_event);
	}
}



int
#ifdef ACTIVEGS
macmain
#else
main
#endif
(int argc, char* argv[])
{
	ProcessSerialNumber my_psn;
	
	IBNibRef	nibRef;
	EventHandlerUPP	handlerUPP;
	EventTypeSpec	cmd_event[3];
	GDHandle g_gdhandle ;
	Rect		win_rect;
	OSStatus	err;
	
#ifndef ACTIVEGS
	char	*argptr;
	int	slash_cnt;
	int	i;
	
	/* Prepare argv0 */
	slash_cnt = 0;
	argptr = argv[0];
	for(i = strlen(argptr); i >= 0; i--) {
		if(argptr[i] == '/') {
			slash_cnt++;
			if(slash_cnt == 3) {
				strncpy(&(g_argv0_path[0]), argptr, i);
				g_argv0_path[i] = 0;
			}
		}
	}
	
	printf("g_argv0_path is %s\n", g_argv0_path);
	
	g_mac_argv[0] = argv[0];
	g_mac_argc = 1;
	i = 1;
	while((i < argc) && (g_mac_argc < MAX_MAC_ARGS)) {
		if(!strncmp(argv[i], "-psn", 4)) {
			/* skip this argument */
		} else {
			g_mac_argv[g_mac_argc++] = argv[i];
		}
		i++;
	}
#endif
	
	InitCursor();
	g_event_rgnhandle = NewRgn();
	g_status_font_family = FMGetFontFamilyFromName("\pCourier");
	
	SetRect(&win_rect, 0, 0, X_A2_WINDOW_WIDTH, X_A2_WINDOW_HEIGHT 
			// OG Remove status line from ActiveGS window
#ifndef ACTIVEGS
			+ MAX_STATUS_LINES*16 + 8
#endif
			);
	OffsetRect(&win_rect, 64, 50);
	
	
	// Create a Nib reference passing the name of the nib file
	// CreateNibReference only searches into the application bundle.
	err = CreateNibReference(CFSTR("main"), &nibRef);
	require_noerr( err, CantGetNibRef );
	// Once the nib reference is created, set the menu bar.
	err = SetMenuBarFromNib(nibRef, CFSTR("MenuBar"));
	require_noerr( err, CantSetMenuBar );
	
	
#ifndef ACTIVEGS
	err = CreateNewWindow(kDocumentWindowClass,
						  kWindowStandardDocumentAttributes |
						  kWindowStandardHandlerAttribute,
						  &win_rect, &g_main_window);
	
	err = SetWindowTitleWithCFString(g_main_window, CFSTR("KEGSMAC"));
#else
	err = CreateNewWindow(kDocumentWindowClass,
						  (kWindowCloseBoxAttribute /*| kWindowFullZoomAttribute */| kWindowCollapseBoxAttribute /*| kWindowResizableAttribute*/) /*kWindowStandardDocumentAttributes*/ |
						  kWindowStandardHandlerAttribute,
						  &win_rect, &g_main_window);
	extern CFStringRef activeGSversionSTR;
	err = SetWindowTitleWithCFString(g_main_window, activeGSversionSTR);
#endif
	
	//printf("CreateNewWindow ret: %d, g_main_window: %p\n", (int)err, g_main_window);
	
	
	// We don't need the nib reference anymore.
	DisposeNibReference(nibRef);
	
	SysBeep(120);
	
	handlerUPP = NewEventHandlerUPP( my_cmd_handler );
	
	cmd_event[0].eventClass = kEventClassCommand;
	cmd_event[0].eventKind = kEventProcessCommand;
	InstallWindowEventHandler(g_main_window, handlerUPP, 1, &cmd_event[0],
							  (void *)g_main_window, NULL);
	
	handlerUPP = NewEventHandlerUPP(my_win_handler);
	cmd_event[0].eventClass = kEventClassWindow;
	cmd_event[0].eventKind = kEventWindowDrawContent;
	cmd_event[1].eventClass = kEventClassWindow;
	cmd_event[1].eventKind = kEventWindowUpdate;
	cmd_event[2].eventClass = kEventClassWindow;
	cmd_event[2].eventKind = kEventWindowClose;
	err = InstallWindowEventHandler(g_main_window, handlerUPP, 3,
									&cmd_event[0], (void *)g_main_window, NULL);
	require_noerr(err, CantCreateWindow);

	// Get screen depth
	g_gdhandle = GetGDevice();
	s_video.g_screen_mdepth = (**((**g_gdhandle).gdPMap)).pixelSize;
	
	s_video.g_screen_depth = s_video.g_screen_mdepth;

	if(s_video.g_screen_depth > 16) {
		/* 32-bit display */
		s_video.g_red_mask = 0xff;
		s_video.g_green_mask = 0xff;
		s_video.g_blue_mask = 0xff;
		
		/*
		if (macUsingCoreGraphics)
		{
			g_red_left_shift = 0;
			g_green_left_shift = 8;
			g_blue_left_shift = 16;
		}
		else
		 */
		{
			s_video.g_red_left_shift = 16;
			s_video.g_green_left_shift = 8;
			s_video.g_blue_left_shift = 0;			
			
		}
		
		s_video.g_red_right_shift = 0;
		s_video.g_green_right_shift = 0;
		s_video.g_blue_right_shift = 0;
	} else if(s_video.g_screen_depth > 8) {
		/* 16-bit display */
		s_video.g_red_mask = 0x1f;
		s_video.g_green_mask = 0x1f;
		s_video.g_blue_mask = 0x1f;
		s_video.g_red_left_shift = 10;
		s_video.g_green_left_shift = 5;
		s_video.g_blue_left_shift = 0;
		s_video.g_red_right_shift = 3;
		s_video.g_green_right_shift = 3;
		s_video.g_blue_right_shift = 3;
	}
	
	// show_alert("About to show window", (int)g_main_window);
	update_main_window_size();
	
	update_window();
	

	// The window was created hidden so show it.
	ShowWindow( g_main_window );
	BringToFront( g_main_window );
	
	update_window();
	
	// Make us pop to the front a different way
	err = GetCurrentProcess(&my_psn);
	if(err == noErr) {
		(void)SetFrontProcess(&my_psn);
	}

	// Call the event loop
	temp_run_application_event_loop();
	return 0;
	
CantCreateWindow:
CantSetMenuBar:
CantGetNibRef:
	show_simple_alert("ending", "", "error code", err);
	return err;
}

void
x_redraw_status_lines()
{
	// OG Disable status line
#ifndef ACTIVEGS
	Rect	rect;
	Pattern	white_pattern;
	char	tmp_buf[256];
	char	*buf;
	int	len;
	int	line;
	int	height;
	int	margin;
	
	SetPortWindowPort(g_main_window);
	PenNormal();
	height = 16;
	margin = 0;
	TextFont(g_status_font_family);
	TextFace(normal);
	TextSize(12);
	
	SetRect(&rect, 0, X_A2_WINDOW_HEIGHT + margin, X_A2_WINDOW_WIDTH,
			X_A2_WINDOW_HEIGHT + margin + MAX_STATUS_LINES*height);
	GetQDGlobalsWhite(&white_pattern);
	FillRect(&rect, &white_pattern);
	
	for(line = 0; line < MAX_STATUS_LINES; line++) {
		buf = g_status_ptrs[line];
		if(buf == 0) {
			/* skip it */
			continue;
		}
		MoveTo(10, X_A2_WINDOW_HEIGHT + height*line + margin + height);
		len = MIN(250, strlen(buf));
		strncpy(&tmp_buf[1], buf, len);
		tmp_buf[0] = len;
		DrawString((const unsigned char*)&tmp_buf[0]);
	}
#endif
}

void
x_full_screen(int do_full)
{
	
#if 0
	WindowRef new_window;
	short	width, height;
	OSErr	ret;
	
	width = 640;
	height = 480;
	if(do_full && (g_mac_fullscreen_state == 0)) {
		g_main_window_saved = g_main_window;
		
		GetWindowBounds(g_main_window, kWindowContentRgn,
						&g_main_window_saved_rect);
		ret = BeginFullScreen(&g_mac_fullscreen_state, 0,
							  &width, &height, &new_window, 0, 0);
		printf("Ret beginfullscreen: %d\n", (int)ret);
		printf("New width: %d, new height: %d\n", width, height);
		if(ret == noErr) {
			g_main_window = new_window;
		} else {
			g_mac_fullscreen_state = 0;
		}
	} else if(!do_full && (g_mac_fullscreen_state != 0)) {
		ret = EndFullScreen(g_mac_fullscreen_state, 0);
		printf("ret endfullscreen: %d\n", (int)ret);
		g_main_window = g_main_window_saved;
		g_mac_fullscreen_state = 0;
		//InitCursor();
		SetWindowBounds(g_main_window, kWindowContentRgn,
						&g_main_window_saved_rect);
	}
	
	update_main_window_size();
	
	ShowWindow(g_main_window);
	BringToFront(g_main_window);
	update_window();
#endif
}


void
x_push_done()
{

		CGrafPtr window_port;
		
		SetPortWindowPort(g_main_window);
		window_port = GetWindowPort(g_main_window);
		
		QDFlushPortBuffer(window_port, 0);
	
}

void
mac_warp_mouse()
{
#ifndef ACTIVEGS
	Rect		port_rect;
	Point		win_origin_pt;
	CGPoint		cgpoint;
	CGDisplayErr	cg_err;
	
	GetPortBounds(GetWindowPort(g_main_window), &port_rect);
	SetPt(&win_origin_pt, port_rect.left, port_rect.top);
	LocalToGlobal(&win_origin_pt);
	
	cgpoint = CGPointMake( (float)(win_origin_pt.h + X_A2_WINDOW_WIDTH/2),
						  (float)(win_origin_pt.v + X_A2_WINDOW_HEIGHT/2));
	cg_err = CGDisplayMoveCursorToPoint(kCGDirectMainDisplay, cgpoint);
#endif
}

int x_refresh_video()
{
	void x_refresh_buffer();
	 x_refresh_buffer();
	return 1;
}

#ifdef ACTIVEGS
void x_refresh_panel(enum refreshpanelenum _enum)
{
}
#endif
