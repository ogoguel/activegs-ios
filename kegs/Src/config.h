/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#ifdef INCLUDE_RCSID_C
const char rcsid_config_h[] = "@(#)$KmKId: config.h,v 1.9 2004-11-12 23:10:28-05 kentd Exp $";
#endif

#include "iwm.h"

#define CONF_BUF_LEN		1024
#define COPY_BUF_SIZE		4096
#define CFG_PRINTF_BUFSIZE	2048

#define CFG_PATH_MAX		1024

#define CFG_NUM_SHOWENTS	16

#define CFGTYPE_MENU		1
#define CFGTYPE_INT		2
#define CFGTYPE_DISK		3
#define CFGTYPE_FUNC		4
#define CFGTYPE_FILE		5
/* CFGTYPE limited to just 4 bits: 0-15 */

/* Cfg_menu, Cfg_dirent and Cfg_listhdr are defined in defc.h */

STRUCT(Cfg_defval) {
	Cfg_menu *menuptr;
	int	intval;
	char	*strval;
};

/* config.c */
void config_init_menus(Cfg_menu *menuptr);
void config_init(void);
void cfg_exit(void);
void cfg_toggle_config_panel(void);
void cfg_text_screen_dump(void);
void config_vbl_update(int doit_3_persec);
void config_parse_option(char *buf, int pos, int len, int line);
void config_parse_bram(char *buf, int pos, int len);
void config_load_roms(void);
void config_parse_config_kegs_file(void);
Disk *cfg_get_dsk_from_slot_drive(int slot, int drive);
void config_generate_config_kegs_name(char *outstr, int maxlen, Disk *dsk, int with_extras);
void config_write_config_kegs_file(void);
void insert_disk(int slot, int drive, const char *name, int ejected, int force_size, const char *partition_name, int part_num);
void eject_named_disk(Disk *dsk, const char *name, const char *partition_name);
void eject_disk_by_num(int slot, int drive);
void eject_disk(Disk *dsk);
int cfg_get_fd_size(int fd);
int cfg_partition_read_block(int fd, void *buf, int blk, int blk_size);
int cfg_partition_find_by_name_or_num(int fd, const char *partnamestr, int part_num, Disk *dsk);
int cfg_partition_make_list(int fd);
int cfg_maybe_insert_disk(int slot, int drive, const char *namestr);
int cfg_stat(char *path, struct stat *sb);
void cfg_htab_vtab(int x, int y);
void cfg_home(void);
void cfg_cleol(void);
void cfg_putchar(int c);
void cfg_printf(const char *fmt, ...);
void cfg_print_num(int num, int max_len);
void cfg_get_disk_name(char *outstr, int maxlen, int type_ext, int with_extras);
void cfg_parse_menu(Cfg_menu *menuptr, int menu_pos, int highlight_pos, int change);
void cfg_get_base_path(char *pathptr, const char *inptr, int go_up);
void cfg_file_init(void);
void cfg_free_alldirents(Cfg_listhdr *listhdrptr);
void cfg_file_add_dirent(Cfg_listhdr *listhdrptr, const char *nameptr, int is_dir, int size, int image_start, int part_num);
int cfg_dirent_sortfn(const void *obj1, const void *obj2);
int cfg_str_match(const char *str1, const char *str2, int len);
void cfg_file_readdir(const char *pathptr);
char *cfg_shorten_filename(const char *in_ptr, int maxlen);
void cfg_fix_topent(Cfg_listhdr *listhdrptr);
void cfg_file_draw(void);
void cfg_partition_selected(void);
void cfg_file_update_ptr(char *str);
void cfg_file_selected(void);
void cfg_file_handle_key(int key);
void config_control_panel(void);
