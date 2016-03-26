/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#include <stdio.h>
#include "defc.h"
#include <stdarg.h>

#include "disas.h"
#include "adb.h"
#include "moremem.h"
#include "sim65816.h"
#include "iwm.h"
#include "protos_engine_c.h"

#define LINE_SIZE 160

extern byte *g_memory_ptr;


extern int g_config_control_panel;


int	g_num_breakpoints = 0;
		
struct_breakpoint g_breakpts[MAX_BREAK_POINTS];


#define W_BUF_LEN	128
char w_buff[W_BUF_LEN];

int g_stepping = 0;

word32	list_kpc;
int	hex_line_len;
word32	a1,a2,a3;
word32	g_a4, g_a4bank;
int a1bank, a2bank, a3bank;
char *line_ptr;
int mode,old_mode;
int got_num;

// OG replaced by HALT_WANTTOQUIT
//int	g_quit_sim_now = 0;

int
get_num()
{
	int tmp1;

	a2 = 0;
	got_num = 0;
	while(1) {
		if(mode == 0 && got_num != 0) {
/*
			printf("In getnum, mode =0, setting a1,a3 = a2\n");
			printf("a2: %x\n", a2);
*/
			a3 = a2;
			a3bank = a2bank;
			a1 = a2;
			a1bank = a2bank;
		}
		tmp1 = *line_ptr++ & 0x7f;
		if(tmp1 >= '0' && tmp1 <= '9') {
			a2 = (a2 << 4) + tmp1 - '0';
			got_num = 1;
			continue;
		}
		if(tmp1 >= 'a' && tmp1 <= 'f') {
			a2 = (a2 << 4) + tmp1 - 'a' + 10;
			got_num = 1;
			continue;
		}
		if(tmp1 == '/') {
			a2bank = a2;
			a2 = 0;
			continue;
		}
		return tmp1;
	}
}

void
debugger_help()
{
	printf("KEGS Debugger help (courtesy Fredric Devernay\n");
	printf("General command syntax: [bank]/[address][command]\n");
	printf("e.g. 'e1/0010B' to set a breakpoint at the interrupt jump pt\n");
	printf("Enter all addresses using lower-case\n");
	printf("As with the IIgs monitor, you can omit the bank number after\n");
	printf("having set it: 'e1/0010B' followed by '14B' will set\n");
	printf("breakpoints at e1/0010 and e1/0014\n");
	printf("\n");
	printf("g                       Go\n");
	printf("[bank]/[addr]g          Go from [bank]/[address]\n");
	printf("s                       Step one instruction\n");
	printf("[bank]/[addr]s          Step one instr at [bank]/[address]\n");
	printf("[bank]/[addr]B          Set breakpoint at [bank]/[address]\n");
	printf("B                       Show all breakpoints\n");
	printf("[bank]/[addr]D          Delete breakpoint at [bank]/[address]\n");
	printf("[bank]/[addr1].[addr2]  View memory\n");
	printf("[bank]/[addr]L          Disassemble memory\n");

	printf("P                       Dump the trace to 'pc_log_out'\n");
	printf("Z                       Dump SCC state\n");
	printf("I                       Dump IWM state\n");
	printf("[drive].[track]I        Dump IWM state\n");
	printf("E                       Dump Ensoniq state\n");
	printf("[osc]E                  Dump oscillator [osc] state\n");
	printf("R                       Dump dtime array and events\n");
	printf("T                       Show toolbox log\n");
	printf("[bank]/[addr]T          Dump tools using ptr [bank]/[addr]\n");
	printf("                            as 'tool_set_info'\n");
	printf("[mode]V                 XOR verbose with 1=DISK, 2=IRQ,\n");
	printf("                         4=CLK,8=SHADOW,10=IWM,20=DOC,\n");
	printf("                         40=ABD,80=SCC, 100=TEST, 200=VIDEO\n");
	printf("[mode]H                 XOR halt_on with 1=SCAN_INT,\n");
	printf("                         2=IRQ, 4=SHADOW_REG, 8=C70D_WRITES\n");
	printf("r                       Reset\n");
	printf("[0/1]=m                 Changes m bit for l listings\n");
	printf("[0/1]=x                 Changes x bit for l listings\n");
	printf("[t]=z                   Stops at absolute time t (obsolete)\n");
	printf("S                       show_bankptr_bank0 & smartport errs\n");
	printf("P                       show_pmhz\n");
	printf("A                       show_a2_line_stuff show_adb_log\n");
	printf("Ctrl-e                  Dump registers\n");
	printf("[bank]/[addr1].[addr2]us[file]  Save mem area to [file]\n");
	printf("[bank]/[addr1].[addr2]ul[file]  Load mem area from [file]\n");
	printf("v			Show video information\n");
	printf("q                       Exit Debugger (and KEGS)\n");
}

void
do_debug_intfc()
{
	char	linebuf[LINE_SIZE];
	int	slot_drive;
	int	track;
	int	osc;
	int	done;
	int	ret_val;

	g_config_control_panel = 1;

	hex_line_len = 0x10;
	a1 = 0; a2 = 0; a3 = 0; g_a4 = 0;
	a1bank = 0; a2bank = 0; a3bank = 0; g_a4bank = 0;
	list_kpc = g_sim65816.engine.kpc;
	g_stepping = 0;
	mode = 0; old_mode = 0;
	done = 0;
	g_sim65816.stop_run_at = -1;

	x_auto_repeat_on(0);
	g_adb.fullscreen = 0;
	x_full_screen(0);

	// OG use HALT_WANTTOQUIT instead of g_quit_sim_now
	if (r_sim65816.should_emulator_terminate()) //g_sim65816.halt_sim&HALT_WANTTOQUIT)
	{
		printf("Exiting immediately\n");
		return;
	}
	

	printf("Type 'h' for help\n");

	while(!done) {
		printf("> "); fflush(stdout);
		if(read_line(linebuf,LINE_SIZE-1) <= 0) {
			done = 1;
			continue;
		}
		line_ptr = linebuf;

/*
		printf("input line: :%s:\n", linebuf);
		printf("mode: %d\n", mode);
*/
		mode = 0;

		while(*line_ptr != 0) {
			ret_val = get_num();
/*
			printf("ret_val: %x, got_num= %d\n", ret_val,
				got_num);
*/
			old_mode = mode;
			mode = 0;
			switch(ret_val) {
			case 'h':
				debugger_help();
				break;
			case 'R':
				show_dtime_array();
				show_all_events();
				break;
			case 'I':
				slot_drive = -1;
				track = -1;
				if(got_num) {
					if(old_mode == '.') {
						slot_drive = a1;
					}
					track = a2;
				}
				iwm_show_track(slot_drive, track);
				iwm_show_stats();
				break;
			case 'E':
				osc = -1;
				if(got_num) {
					osc = a2;
				}
				doc_show_ensoniq_state(osc);
				break;
			case 'T':
				if(got_num) {
					show_toolset_tables(a2bank, a2);
				} else {
					show_toolbox_log();
				}
				break;
			case 'v':
				if(got_num) {
					dis_do_compare();
				} else {
					video_show_debug_info();
				}
				break;
			case 'V':
				printf("g_moremem.g_irq_pending: %05x\n", g_moremem.g_irq_pending);
				printf("Setting g_sim65816.Verbose ^= %04x\n", a1);
				Verbose ^= a1;
				printf("Verbose is now: %04x\n", Verbose);
				break;
			case 'H':
				printf("Setting g_sim65816.Halt_on ^= %04x\n", a1);
				g_sim65816.Halt_on ^= a1;
				printf("Halt_on is now: %04x\n", g_sim65816.Halt_on);
				break;
			case 'r':
				do_reset();
				list_kpc = g_sim65816.engine.kpc;
				break;
			case 'm':
				if(old_mode == '=') {
					if(!a1) {
						g_sim65816.engine.psr &= ~0x20;
					} else {
						g_sim65816.engine.psr |= 0x20;
					}
					if(g_sim65816.engine.psr & 0x100) {
						g_sim65816.engine.psr |= 0x30;
					}
				} else {
					dis_do_memmove();
				}
				break;
			case 'p':
				dis_do_pattern_search();
				break;
			case 'x':
				if(old_mode == '=') {
					if(!a1) {
						g_sim65816.engine.psr &= ~0x10;
					} else {
						g_sim65816.engine.psr |= 0x10;
					}
					if(g_sim65816.engine.psr & 0x100) {
						g_sim65816.engine.psr |= 0x30;
					}
				}
				break;
			case 'z':
				if(old_mode == '=') {
					g_sim65816.stop_run_at = a1;
					printf("Calling add_event for t:%08x\n",
						a1);
					add_event_stop((double)a1);
					printf("set g_sim65816.stop_run_at = %x\n", a1);
				}
				break;
			case 'l': case 'L':
				do_debug_list();
				break;
			case 'Z':
				show_scc_log();
				show_scc_state();
				break;
			case 'S':
				show_bankptrs_bank0rdwr();
				smartport_error();
				break;
			case 'C':
				x_show_color_array();
				break;
			case 'P':
				show_pc_log();
				break;
			case 'M':
				show_pmhz();
				break;
			case 'A':
				show_a2_line_stuff();
				show_adb_log();
				break;
			case 's':
				g_stepping = 1;
				if(got_num) {
					g_sim65816.engine.kpc = (a2bank<<16) + (a2&0xffff);
				}
				mode = 's';
				list_kpc = g_sim65816.engine.kpc;
				break;
			case 'B':
				if(got_num) {
					printf("got_num: %d, a2bank: %x, a2: %x\n", got_num, a2bank, a2);
					set_bp((a2bank << 16) + a2,BRK_ALL,NULL,NULL);
				} else {
					show_bp();
				}
				break;
			case 'D':
				if(got_num) {
					printf("got_num: %d, a2bank: %x, a2: %x\n", got_num, a2bank, a2);
					delete_bp((a2bank << 16) + a2);
				}
				break;
			case 'g':
			case 'G':
				printf("Going..\n");
				g_stepping = 0;
				if(got_num) {
					g_sim65816.engine.kpc = (a2bank<<16) + (a2&0xffff);
				}
				if(ret_val == 'G' && g_sim65816.g_testing_enabled) {
					do_gen_test(got_num, a2);
				} else {
					do_go();
				}
				list_kpc = g_sim65816.engine.kpc;
				break;
			case 'q':
			case 'Q':
				printf("Exiting debugger\n");
				return;
				break;
			case 'u':
				printf("Unix commands\n");
				do_debug_unix();
				break;
			case ':': case '.':
			case '+': case '-':
			case '=': case ',':
				mode = ret_val;
				printf("Setting mode = %x\n", mode);
				break;
			case ' ': case '\t':
				if(!got_num) {
					mode = old_mode;
					break;
				}
				do_blank();
				break;
			case '<':
				g_a4 = a2;
				g_a4bank = a2bank;
				break;
			case 0x05: /* ctrl-e */
				show_regs();
				break;
			case '\n':
				*line_ptr = 0;
				if(old_mode == 's') {
					do_blank();
					break;
				}
				if(line_ptr == &linebuf[1]) {
					a2 = a1 | (hex_line_len - 1);
					show_hex_mem(a1bank,a1,a2bank,a2, -1);
					a1 = a2 + 1;
				} else {
					if(got_num == 1 || mode == 's') {
						do_blank();
					}
				}
				break;
			case 'w':
				read_line(w_buff, W_BUF_LEN);
				break;
			default:
				printf("\nUnrecognized command: %s\n",linebuf);
				*line_ptr = 0;
				break;
			}
		}

	}
	printf("Console closed.\n");

}

word32
dis_get_memory_ptr(word32 addr)
{
	word32	tmp1, tmp2, tmp3;

	tmp1 = get_memory_c(addr, 0);
	tmp2 = get_memory_c(addr + 1, 0);
	tmp3 = get_memory_c(addr + 2, 0);

	return (tmp3 << 16) + (tmp2 << 8) + tmp1;
}

void
show_one_toolset(FILE *toolfile, int toolnum, word32 addr)
{
	word32	rout_addr;
	int	num_routs;
	int	i;

	num_routs = dis_get_memory_ptr(addr);
	fprintf(toolfile, "Tool 0x%02x, table: 0x%06x, num_routs:%03x\n",
		toolnum, addr, num_routs);

	for(i = 1; i < num_routs; i++) {
		rout_addr = dis_get_memory_ptr(addr + 4*i);
		fprintf(toolfile, "%06x = %02x%02x\n", rout_addr, i, toolnum);
	}
}

void
show_toolset_tables(word32 a2bank, word32 addr)
{
	FILE	*toolfile;
	word32	tool_addr;
	int	num_tools;
	int	i;

	addr = (a2bank << 16) + (addr & 0xffff);

	toolfile = fopen("tool_set_info", "w");
	if(toolfile == 0) {
		fprintf(stderr, "fopen of tool_set_info failed: %d\n", errno);
		x_exit(2);
	}

	num_tools = dis_get_memory_ptr(addr);
	fprintf(toolfile, "There are 0x%02x tools using ptr at %06x\n",
			num_tools, addr);

	for(i = 1; i < num_tools; i++) {
		tool_addr = dis_get_memory_ptr(addr + 4*i);
		show_one_toolset(toolfile, i, tool_addr);
	}

	fclose(toolfile);
}


#ifndef TEST65
void
do_gen_test(int got_num, int base_seed)
{
	/* dummy */
}
#endif

void
set_bp(word32 _addr,enum mode_breakpoint _mode, const char* _patch, const char* _check)
{
	int	count;

	//printf("About to set BP at %06x\n", _addr);
	count = g_num_breakpoints;
	if(count >= MAX_BREAK_POINTS) {
		printf("Too many (0x%02x) breakpoints set!\n", count);
		return;
	}

	g_breakpts[count].addr = _addr;
	g_breakpts[count].mode = _mode;
	g_breakpts[count].patch = _patch;
	g_breakpts[count].check = _check;
	g_num_breakpoints = count + 1;
	fixup_brks();
}

void
show_bp()
{
	int i;

	printf("Showing breakpoints set\n");
	for(i = 0; i < g_num_breakpoints; i++)
	{
		printf("bp:%02x: %06x ", i, g_breakpts[i].addr);
		if (g_breakpts[i].mode&BRK_READ)
			printf("R");
		if (g_breakpts[i].mode&BRK_WRITE)
			printf("W");
		if (g_breakpts[i].mode&BRK_X)
			printf("X");
		if (g_breakpts[i].mode&BRK_PATCH)
			printf("P");
		if (g_breakpts[i].mode&BRK_STOP)
			printf("B");
		if (g_breakpts[i].mode&BRK_SLOWSPEED)
			printf("S");
		if (g_breakpts[i].mode&BRK_DEFAULTSPEED)
			printf("D");
		if (g_breakpts[i].mode&BRK_MESSAGE)
			printf("M:%s",g_breakpts[i].patch);
		
		if (g_breakpts[i].check && g_breakpts[i].check[0])
			printf(" *");
			
		printf("\n");
	}
}

void
delete_bp(word32 addr)
{
	int	count;
	int	hit;
	int	i;

	printf("About to delete BP at %06x\n", addr);
	count = g_num_breakpoints;

	hit = -1;
	for(i = 0; i < count; i++) {
		if(g_breakpts[i].addr == addr) {
			hit = i;
			break;
		}
	}

	if(hit < 0) {
		printf("Breakpoint not found!\n");
	} else {
		printf("Deleting brkpoint #0x%02x\n", hit);
		for(i = hit+1; i < count; i++) {
			g_breakpts[i-1] = g_breakpts[i];
		}
		g_num_breakpoints = count - 1;
		setup_pageinfo();
	}

	show_bp();
}

void
do_blank()
{
	int tmp, i;

	switch(old_mode) {
	case 's':
		tmp = a2;
		if(tmp == 0) tmp = 1;
		g_sim65816.enter_debug = 0;
		for(i = 0; i < tmp; i++) {
			g_stepping = 1;
			do_step();
			if(g_sim65816.enter_debug || g_sim65816.halt_sim != 0) {
				if(g_sim65816.halt_sim != HALT_EVENT) {
					break;
				}
			}
		}
		list_kpc = g_sim65816.engine.kpc;
		/* video_update_through_line(262); */
		break;
	case ':':
		set_memory_c(((a3bank << 16) + a3), a2, 0);
		a3++;
		mode = old_mode;
		break;
	case '.':
	case 0:
		xam_mem(-1);
		break;
	case ',':
		xam_mem(16);
		break;
	case '+':
		printf("%x\n", a1 + a2);
		break;
	case '-':
		printf("%x\n", a1 - a2);
		break;
	default:
		printf("Unknown mode at space: %d\n", old_mode);
		break;
	}
}

void
do_go()
{

	#ifdef ACTIVEGS
	{
		extern void disableConsole();
		disableConsole();
	}
	#endif

	/* also called by do_step */

	g_config_control_panel = 0;
	clear_halt();

	run_prog();
	show_regs();
	g_config_control_panel = 1;
}

void
do_step()
{
	int size;
	int size_mem_imm, size_x_imm;

	do_go();

	size_mem_imm = 2;
	if(g_sim65816.engine.psr & 0x20) {
		size_mem_imm = 1;
	}
	size_x_imm = 2;
	if(g_sim65816.engine.psr & 0x10) {
		size_x_imm = 1;
	}
	size = do_dis(stdout, g_sim65816.engine.kpc, size_mem_imm, size_x_imm, 0, 0);
}

void
xam_mem(int count)
{
	show_hex_mem(a1bank, a1, a2bank, a2, count);
	a1 = a2 + 1;
}

void
show_hex_mem(int startbank, word32 start, int endbank, word32 end, int count)
{
	char	ascii[MAXNUM_HEX_PER_LINE];
	word32	i;
	int	val, offset;

	if(count < 0) {
		count = 16 - (start & 0xf);
	}

	offset = 0;
	ascii[0] = 0;
	printf("Showing hex mem: bank: %x, start: %x, end: %x\n",
		startbank, start, end);
	for(i = start; i <= end; i++) {
		if( (i==start) || (count == 16) ) {
			printf("%04x:",i);
		}
		printf(" %02x", get_memory_c((startbank <<16) + i, 0));
		val = get_memory_c((startbank << 16) + i, 0) & 0x7f;
		if(val < 32 || val >= 0x7f) {
			val = '.';
		}
		ascii[offset++] = val;
		ascii[offset] = 0;
		count--;
		if(count <= 0) {
			printf("   %s\n", ascii);
			offset = 0;
			ascii[0] = 0;
			count = 16;
		}
	}
	if(offset > 0) {
		printf("   %s\n", ascii);
	}
}


int
read_line(char *buf, int len)
{
	int	space_left;
	int	ret;
#ifndef _WIN32
	int	flags, flags_save;

	/* Unix */
	flags = fcntl(0, F_GETFL, 0);
	flags_save = flags;
	if(flags == -1) {
		return 0;
	}
	ret = fcntl(0, F_SETFL, flags | O_NONBLOCK);
	if(ret == -1) {
		return 0;
	}
#endif
	space_left = len;

	buf[0] = 0;
	ret = 0;
	while(space_left > 0) {
#ifdef _WIN32
		ret = win_nonblock_read_stdin(0, buf, 1);
#else
		/* Unix */
		ret = read(0, buf, 1);
#endif
		if(ret <= 0) {
			micro_sleep(15.0/60.0);
			if(errno == EAGAIN) {
				/* it would block, so no chars--do update */
				video_update();
				ret = 0;
				continue;
			}
			printf("read ret %d, errno: %d\n", ret, errno);
			if(errno == EAGAIN || errno == EINTR) {
				ret = 0;
				continue;
			}
			break;
		}
		space_left -= ret;
		if(buf[ret-1] == 0x0a) {
			break;
		}
		buf = &buf[ret];
	}
#ifndef _WIN32
	(void)fcntl(0, F_SETFL, flags_save);
#endif

	return (len-space_left);
}

void
do_debug_list()
{
	int i;
	int size;
	int size_mem_imm, size_x_imm;

	if(got_num) {
		list_kpc = (a2bank << 16) + (a2 & 0xffff);
	}
	printf("%d=m %d=x %d=LCBANK\n", (g_sim65816.engine.psr >> 5)&1,
		(g_sim65816.engine.psr >> 4) & 1, (g_moremem.g_c068_statereg & 0x4) >> 2);
	
	size_mem_imm = 2;
	if(g_sim65816.engine.psr & 0x20) {
		size_mem_imm = 1;
	}
	size_x_imm = 2;
	if(g_sim65816.engine.psr & 0x10) {
		size_x_imm = 1;
	}
	for(i=0;i<20;i++) {
		size = do_dis(stdout, list_kpc, size_mem_imm,
			size_x_imm, 0, 0);
		list_kpc += size;
	}
}

void
dis_do_memmove()
{
	word32	val;

	printf("Memory move from %02x/%04x.%04x to %02x/%04x\n", a1bank, a1, a2, g_a4bank, g_a4);
	while(a1 <= (a2 & 0xffff)) {
		val = get_memory_c((a1bank << 16) + a1, 0);
		set_memory_c((g_a4bank << 16) + g_a4, val, 0);
		a1++;
		g_a4++;
	}
	a1 = a1 & 0xffff;
	g_a4 = g_a4 & 0xffff;
}

void
dis_do_pattern_search()
{
	printf("Memory pattern search for %04x in %02x/%04x.%04x\n", g_a4, a1bank, a1, a2);
}

void
dis_do_compare()
{
	word32	val1, val2;

	printf("Memory Compare from %02x/%04x.%04x with %02x/%04x\n", a1bank, a1, a2, g_a4bank, g_a4);
	while(a1 <= (a2 & 0xffff)) {
		val1 = get_memory_c((a1bank << 16) + a1, 0);
		val2 = get_memory_c((g_a4bank << 16) + g_a4, 0);
		if(val1 != val2) {
			printf("%02x/%04x: %02x vs %02x\n", a1bank, a1, val1, val2);
		}
		a1++;
		g_a4++;
	}
	a1 = a1 & 0xffff;
	g_a4 = g_a4 & 0xffff;
}

void
do_debug_unix()
{
	char	localbuf[LINE_SIZE];
	word32	offset, len;
	int	fd, ret;
	int	load, save;
	int	i;

	load = 0; save = 0;
	switch(*line_ptr++) {
	case 'l': case 'L':
		printf("Loading..");
		load = 1;
		break;
	case 's': case 'S':
		printf("Saving...");
		save = 1;
		break;
	default:
		printf("Unknown unix command: %c\n", *(line_ptr-1));
		*line_ptr = 0;
		return;
	}
	while(*line_ptr == ' ' || *line_ptr == '\t') {
		line_ptr++;
	}
	i = 0;
	while(i < LINE_SIZE) {
		localbuf[i++] = *line_ptr++;
		if(*line_ptr==' ' || *line_ptr=='\t' || *line_ptr == '\n') {
			break;
		}
	}
	localbuf[i] = 0;
	

	printf("About to open: %s,len: %d\n", localbuf, (int)strlen(localbuf));
	if(load) {
		fd = open(localbuf,O_RDONLY | O_BINARY);
	} else {
		fd = open(localbuf,O_WRONLY | O_CREAT | O_BINARY, 0x1b6);
	}
	if(fd < 0) {
		printf("Open %s failed: %d\n", localbuf, fd);
		printf("errno: %d\n", errno);
		return;
	}
	if(load) {
		offset = a1 & 0xffff;
		len = 0x20000 - offset;
	} else {
		if(old_mode == '.') {
			len = a2 - a1 + 1;
		} else {
			len = 0x100;
		}
	}
	if(load) {
		if(a1bank >= 0xe0 && a1bank < 0xe2) {
			ret = read(fd,(char*)&g_slow_memory_ptr[((a1bank & 1)<<16)+a1],len);
		} else {
			ret = read(fd,(char*)&g_memory_ptr[(a1bank << 16) + a1],len);
		}
	} else {
		if(a1bank >= 0xe0 && a1bank < 0xe2) {
			ret = write(fd,(char*)&g_slow_memory_ptr[((a1bank & 1)<<16)+a1],len);
		} else {
			ret = write(fd,(char*)&g_memory_ptr[(a1bank << 16) + a1],len);
		}
	}
	printf("Read/write: addr %06x for %04x bytes, ret: %x bytes\n",
		(a1bank << 16) + a1, len, ret);
	if(ret < 0) {
		printf("errno: %d\n", errno);
	}
	a1 = a1 + ret;
}

void
do_debug_load()
{
	printf("Sorry, can't load now\n");
}


int
do_dis(FILE *outfile, word32 kpc, int accsize, int xsize,
	int op_provided, word32 instr)
{
	char	buffer[150];
	const char *out;
	int	args, type;
	int	opcode;
	word32	val;
	word32	oldkpc;
	word32	dtype;
	int	signed_val;

	oldkpc = kpc;
	if(op_provided) {
		opcode = (instr >> 24) & 0xff;
	} else {
		opcode = (int)get_memory_c(kpc, 0) & 0xff;
	}

	kpc++;

	dtype = disas_types[opcode];
	out = disas_opcodes[opcode];
	type = dtype & 0xff;
	args = dtype >> 8;

	if(args > 3) {
		if(args == 4) {
			args = accsize;
		} else if(args == 5) {
			args = xsize;
		}
	}

	val = -1;
	switch(args) {
	case 0:
		val = 0;
		break;
	case 1:
		if(op_provided) {
			val = instr & 0xff;
		} else {
			val = get_memory_c(kpc, 0);
		}
		break;
	case 2:
		if(op_provided) {
			val = instr & 0xffff;
		} else {
			val = get_memory16_c(kpc, 0);
		}
		break;
	case 3:
		if(op_provided) {
			val = instr & 0xffffff;
		} else {
			val = get_memory24_c(kpc, 0);
		}
		break;
	default:
		fprintf(stderr, "args out of rang: %d, opcode: %08x\n",
			args, opcode);
		break;
	}
	kpc += args;

	if(!op_provided) {
		instr = (opcode << 24) | (val & 0xffffff);
	}

	switch(type) {
	case ABS:
		if(args != 2) {
			printf("arg # mismatch for opcode %x\n", opcode);
		}
		sprintf(buffer,"%s\t$%04x",out,val);
		show_line(outfile, oldkpc,instr,args+1,buffer);
		break;
	case ABSX:
		if(args != 2) {
			printf("arg # mismatch for opcode %x\n", opcode);
		}
		sprintf(buffer,"%s\t$%04x,X",out,val);
		show_line(outfile, oldkpc,instr,args+1,buffer);
		break;
	case ABSY:
		if(args != 2) {
			printf("arg # mismatch for opcode %x\n", opcode);
		}
		sprintf(buffer,"%s\t$%04x,Y",out,val);
		show_line(outfile, oldkpc,instr,args+1,buffer);
		break;
	case ABSLONG:
		if(args != 3) {
			printf("arg # mismatch for opcode %x\n", opcode);
		}
		sprintf(buffer,"%s\t$%06x",out,val);
		show_line(outfile, oldkpc,instr,args+1,buffer);
		break;
	case ABSIND:
		if(args != 2) {
			printf("arg # mismatch for opcode %x\n", opcode);
		}
		sprintf(buffer,"%s\t($%04x)",out,val);
		show_line(outfile, oldkpc,instr,args+1,buffer);
		break;
	case ABSXIND:
		if(args != 2) {
			printf("arg # mismatch for opcode %x\n", opcode);
		}
		sprintf(buffer,"%s\t($%04x,X)",out,val);
		show_line(outfile, oldkpc,instr,args+1,buffer);
		break;
	case IMPLY:
		if(args != 0) {
			printf("arg # mismatch for opcode %x\n", opcode);
		}
		sprintf(buffer,"%s",out);
		show_line(outfile, oldkpc,instr,args+1,buffer);
		break;
	case ACCUM:
		if(args != 0) {
			printf("arg # mismatch for opcode %x\n", opcode);
		}
		sprintf(buffer,"%s",out);
		show_line(outfile, oldkpc,instr,args+1,buffer);
		break;
	case IMMED:
		if(args == 1) {
			sprintf(buffer,"%s\t#$%02x",out,val);
		} else if(args == 2) {
			sprintf(buffer,"%s\t#$%04x",out,val);
		} else {
			printf("arg # mismatch for opcode %x\n", opcode);
		}
		show_line(outfile, oldkpc,instr,args+1,buffer);
		break;
	case JUST8:
		if(args != 1) {
			printf("arg # mismatch for opcode %x\n", opcode);
		}
		sprintf(buffer,"%s\t$%02x",out,val);
		show_line(outfile, oldkpc,instr,args+1,buffer);
		break;
	case DLOC:
		if(args != 1) {
			printf("arg # mismatch for opcode %x\n", opcode);
		}
		sprintf(buffer,"%s\t$%02x",out,val);
		show_line(outfile, oldkpc,instr,args+1,buffer);
		break;
	case DLOCX:
		if(args != 1) {
			printf("arg # mismatch for opcode %x\n", opcode);
		}
		sprintf(buffer,"%s\t$%02x,X",out,val);
		show_line(outfile, oldkpc,instr,args+1,buffer);
		break;
	case DLOCY:
		if(args != 1) {
			printf("arg # mismatch for opcode %x\n", opcode);
		}
		sprintf(buffer,"%s\t$%02x,Y",out,val);
		show_line(outfile, oldkpc,instr,args+1,buffer);
		break;
	case LONG:
		if(args != 3) {
			printf("arg # mismatch for opcode %x\n", opcode);
		}
		sprintf(buffer,"%s\t$%06x",out,val);
		show_line(outfile, oldkpc,instr,args+1,buffer);
		break;
	case LONGX:
		if(args != 3) {
			printf("arg # mismatch for opcode %x\n", opcode);
		}
		sprintf(buffer,"%s\t$%06x,X",out,val);
		show_line(outfile, oldkpc,instr,args+1,buffer);
		break;
	case DLOCIND:
		if(args != 1) {
			printf("arg # mismatch for opcode %x\n", opcode);
		}
		sprintf(buffer,"%s\t($%02x)",out,val);
		show_line(outfile, oldkpc,instr,args+1,buffer);
		break;
	case DLOCINDY:
		if(args != 1) {
			printf("arg # mismatch for opcode %x\n", opcode);
		}
		sprintf(buffer,"%s\t($%02x),Y",out,val);
		show_line(outfile, oldkpc,instr,args+1,buffer);
		break;
	case DLOCXIND:
		if(args != 1) {
			printf("arg # mismatch for opcode %x\n", opcode);
		}
		sprintf(buffer,"%s\t($%02x,X)",out,val);
		show_line(outfile, oldkpc,instr,args+1,buffer);
		break;
	case DLOCBRAK:
		if(args != 1) {
			printf("arg # mismatch for opcode %x\n", opcode);
		}
		sprintf(buffer,"%s\t[$%02x]",out,val);
		show_line(outfile, oldkpc,instr,args+1,buffer);
		break;
	case DLOCBRAKY:
		if(args != 1) {
			printf("arg # mismatch for opcode %x\n", opcode);
		}
		sprintf(buffer,"%s\t[$%02x],y",out,val);
		show_line(outfile, oldkpc,instr,args+1,buffer);
		break;
	case DISP8:
		if(args != 1) {
			printf("arg # mismatch for opcode %x\n", opcode);
		}
		signed_val = (signed char)val;
		sprintf(buffer,"%s\t$%04x",out,
				(word32)(kpc+(signed_val)) & 0xffff);
		show_line(outfile, oldkpc,instr,args+1,buffer);
		break;
	case DISP8S:
		if(args != 1) {
			printf("arg # mismatch for opcode %x\n", opcode);
		}
		sprintf(buffer,"%s\t$%02x,S",out,(word32)(byte)(val));
		show_line(outfile, oldkpc,instr,args+1,buffer);
		break;
	case DISP8SINDY:
		if(args != 1) {
			printf("arg # mismatch for opcode %x\n", opcode);
		}
		sprintf(buffer,"%s\t($%02x,S),Y",out,(word32)(byte)(val));
		show_line(outfile, oldkpc,instr,args+1,buffer);
		break;
	case DISP16:
		if(args != 2) {
			printf("arg # mismatch for opcode %x\n", opcode);
		}
		sprintf(buffer,"%s\t$%04x", out,
				(word32)(kpc+(signed)(word16)(val)) & 0xffff);
		show_line(outfile, oldkpc,instr,args+1,buffer);
		break;
	case MVPMVN:
		if(args != 2) {
			printf("arg # mismatch for opcode %x\n", opcode);
		}
		sprintf(buffer,"%s\t$%02x,$%02x",out,val&0xff,val>>8);
		show_line(outfile, oldkpc,instr,args+1,buffer);
		break;
	case SEPVAL:
	case REPVAL:
		if(args != 1) {
			printf("arg # mismatch for opcode %x\n", opcode);
		}
		sprintf(buffer,"%s\t#$%02x",out,val);
		show_line(outfile, oldkpc,instr,args+1,buffer);
		break;
	default:
		printf("argument type: %d unexpected\n", type);
		break;
	}

	return(args+1);
}

void
show_line(FILE *outfile, word32 kaddr, word32 operand, int size,
	char *string)
{
	int i;
	int opcode;

//	fprintf(outfile, "%02x/%04x: ", kaddr >> 16, kaddr & 0xffff);
    printf("%02x/%04x: ", kaddr >> 16, kaddr & 0xffff);
    
	opcode = (operand >> 24) & 0xff;
//	fprintf(outfile,"%02x ", opcode);
	printf("%02x ", opcode);

	for(i=1;i<size;i++) {
	//	fprintf(outfile, "%02x ", operand & 0xff);
		printf("%02x ", operand & 0xff);
		operand = operand >> 8;
	}
	for(;i<5;i++) {
	//	fprintf(outfile, "   ");
        printf("   ");
	}
	//fprintf(outfile,"%s\n", string);
    printf("%s\n", string);
    
}

void
halt_printf(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
#ifdef _DEBUG
//	set_halt(HALT_WANTTOBRK);
#endif
	set_halt(1);
}

void
halt2_printf(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);

	set_halt(2);
}

