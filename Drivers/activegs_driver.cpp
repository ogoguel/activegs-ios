/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#include "../kegs/Src/driver.h"

extern void default_config_load_roms();
void activegs_driver()
{
	g_driver.x_config_load_roms  = default_config_load_roms;
#if !defined(ACTIVEGSKARATEKA) && defined(DRIVER_IOS)
    extern void x_notify_eject();
    g_driver.x_notify_eject = x_notify_eject;
#endif
}
