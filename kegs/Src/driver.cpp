/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#include "driver.h"

s_driver g_driver;

void s_driver::init(void (*_driver)())
{
	if (_driver!=NULL)
		_driver();
}