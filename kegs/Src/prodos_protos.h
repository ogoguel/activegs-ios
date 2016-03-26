/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

void flush_disk(ProDisk *disk);
void close_file(ProDisk *disk);
ProDisk *allocate_memdisk(char *out_name, int size);
void format_memdisk(ProDisk *ptr, char *name);
void disk_write_data(ProDisk *disk, int blk_num, byte *buf, int size);
void disk_read_data(ProDisk *disk, int blk_num, byte *buf, int size);
Directory *disk_read_dir(ProDisk *disk, int blk_num);
void disk_write_dir(ProDisk *disk, int blk_num);
void create_new_file(ProDisk *disk, int dir_block, int storage_type,
	char *name, int file_type, word32 creation_time, int version,
	int min_version, int access, int aux_type, word32 last_mod, word32 eof);
int pro_write_file(ProDisk *disk, byte *in_buf, int pos, int size);
int get_disk_block(ProDisk *disk, int pos, int create);
void get_new_ind_block(ProDisk *disk);
void write_ind_block(ProDisk *disk);
void get_new_master_ind_block(ProDisk *disk);
void write_master_ind_block(ProDisk *disk);
int find_next_free_block(ProDisk *disk);
void set_bitmap_used(byte *ptr, int i);
void set_bitmap_free(byte *ptr, int i);
void set_file_entry(File_entry *entry, int storage_type_name_len,
	char *file_name, int file_type, int key_pointer, int blocks_used,
	int eof, word32 creation, int version, int min_version, int access,
        int aux_type, word32 last_mod, int header_pointer);
void set_l2byte(L2byte *ptr, int val);
void set_l3byte(L3byte *ptr, int val);
void set_pro_time(Pro_time *ptr, word32 val);
int get_l2byte(L2byte *ptr);
int get_l3byte(L3byte *ptr);
void inc_l2byte(L2byte *ptr);
