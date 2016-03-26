/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#include "defc.h"
#include "moremem.h"
#include "sim65816.h"
#include "iwm.h"
#include "config.h"
#include "driver.h"

#ifdef _MSC_VER	
#define snprintf _snprintf
#endif

//extern int g_rom_version;
//extern word32 g_mem_size_base;
extern byte *g_rom_fc_ff_ptr;
extern byte *g_rom_cards_ptr;
//extern word32 g_mem_size_exp;
//extern Iwm iwm;
///extern int Verbose;

//extern int	g_highest_smartport_unit;

// was in config.cpp

int g_config_control_panel = 0;
int g_config_kegs_update_needed = 0;
word32	g_cfg_vbl_count = 0;
int	g_user_page2_shadow = 1;
int	g_save_cur_a2_stat = 0;

const byte g_rom_c600_rom01_diffs[256] = {
	0x00, 0x00, 0x00, 0x00, 0xc6, 0x00, 0xe2, 0x00,
	0xd0, 0x50, 0x0f, 0x77, 0x5b, 0xb9, 0xc3, 0xb1,
	0xb1, 0xf8, 0xcb, 0x4e, 0xb8, 0x60, 0xc7, 0x2e,
	0xfc, 0xe0, 0xbf, 0x1f, 0x66, 0x37, 0x4a, 0x70,
	0x55, 0x2c, 0x3c, 0xfc, 0xc2, 0xa5, 0x08, 0x29,
	0xac, 0x21, 0xcc, 0x09, 0x55, 0x03, 0x17, 0x35,
	0x4e, 0xe2, 0x0c, 0xe9, 0x3f, 0x9d, 0xc2, 0x06,
	0x18, 0x88, 0x0d, 0x58, 0x57, 0x6d, 0x83, 0x8c,
	0x22, 0xd3, 0x4f, 0x0a, 0xe5, 0xb7, 0x9f, 0x7d,
	0x2c, 0x3e, 0xae, 0x7f, 0x24, 0x78, 0xfd, 0xd0,
	0xb5, 0xd6, 0xe5, 0x26, 0x85, 0x3d, 0x8d, 0xc9,
	0x79, 0x0c, 0x75, 0xec, 0x98, 0xcc, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x77, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x7b, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x39, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x39, 0x00, 0x35, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x4a, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00,
	0x6c, 0x44, 0xce, 0x4c, 0x01, 0x08, 0x00, 0x00
};

extern void x_config_load_roms();
extern void x_config_load_additional_roms();



void
config_load_roms()
{
	int	more_than_8mb;
	int	changed_rom;
	int	i;
	
	// OG load roms
    if (g_driver.x_config_load_roms==NULL)
        x_fatal_exit("missing rom");
    
	g_driver.x_config_load_roms();

	memset(&g_rom_cards_ptr[0], 0, 256*16);

	if (g_sim65816.g_rom_version < 0)
	{
		// No need to
		return ;
	}

	/* initialize c600 rom to be diffs from the real ROM, to build-in */
	/*  Apple II compatibility without distributing ROMs */
	for(i = 0; i < 256; i++) {
		g_rom_cards_ptr[0x600 + i] = g_rom_fc_ff_ptr[0x3c600 + i] ^
				g_rom_c600_rom01_diffs[i];
	}

	if(g_sim65816.g_rom_version >= 3) {
		/* some patches */
		g_rom_cards_ptr[0x61b] ^= 0x40;
		g_rom_cards_ptr[0x61c] ^= 0x33;
		g_rom_cards_ptr[0x632] ^= 0xc0;
		g_rom_cards_ptr[0x633] ^= 0x33;
	}


	x_config_load_additional_roms();

	more_than_8mb = (g_sim65816.g_mem_size_exp > 0x800000);
	/* Only do the patch if users wants more than 8MB of expansion mem */

	changed_rom = 0;
	if(g_sim65816.g_rom_version == 1) {
		/* make some patches to ROM 01 */
#if 0
		/* 1: Patch ROM selftest to not do speed test */
		printf("Patching out speed test failures from ROM 01\n");
		g_rom_fc_ff_ptr[0x3785a] = 0x18;
		changed_rom = 1;
#endif

#if 0
		/* 2: Patch ROM selftests not to do tests 2,4 */
		/* 0 = skip, 1 = do it, test 1 is bit 0 of LSByte */
		g_rom_fc_ff_ptr[0x371e9] = 0xf5;
		g_rom_fc_ff_ptr[0x371ea] = 0xff;
		changed_rom = 1;
#endif

		
		if(more_than_8mb) {
			/* Geoff Weiss patch to use up to 14MB of RAM */
			g_rom_fc_ff_ptr[0x30302] = 0xdf;
			g_rom_fc_ff_ptr[0x30314] = 0xdf;
			g_rom_fc_ff_ptr[0x3031c] = 0x00;
			changed_rom = 1;
		}

		/* Patch ROM selftest to not do ROM cksum if any changes*/
		if(changed_rom) {
			g_rom_fc_ff_ptr[0x37a06] = 0x18;
			g_rom_fc_ff_ptr[0x37a07] = 0x18;
		}
	} else if(g_sim65816.g_rom_version == 3) {
		/* patch ROM 03 */
		printf("Patching ROM 03 smartport bug\n");
		/* 1: Patch Smartport code to fix a stupid bug */
		/*   that causes it to write the IWM status reg into c036, */
		/*   which is the system speed reg...it's "safe" since */
		/*   IWM status reg bit 4 must be 0 (7MHz)..., otherwise */
		/*   it might have turned on shadowing in all banks! */
		g_rom_fc_ff_ptr[0x357c9] = 0x00;
		changed_rom = 1;

#if 0
		/* patch ROM 03 to not to speed test */
		/*  skip fast speed test */
		g_rom_fc_ff_ptr[0x36ad7] = 0x18;
		g_rom_fc_ff_ptr[0x36ad8] = 0x18;
		changed_rom = 1;
#endif

#if 0
		/*  skip slow speed test */
		g_rom_fc_ff_ptr[0x36ae7] = 0x18;
		g_rom_fc_ff_ptr[0x36ae8] = 0x6b;
		changed_rom = 1;
#endif

#if 0
		/* 4: Patch ROM 03 selftests not to do tests 1-4 */
		g_rom_fc_ff_ptr[0x364a9] = 0xf0;
		g_rom_fc_ff_ptr[0x364aa] = 0xff;
		changed_rom = 1;
#endif

		/* ROM tests are in ff/6403-642x, where 6403 = addr of */
		/*  test 1, etc. */

		if(more_than_8mb) {
			/* Geoff Weiss patch to use up to 14MB of RAM */
			g_rom_fc_ff_ptr[0x30b] = 0xdf;
			g_rom_fc_ff_ptr[0x31d] = 0xdf;
			g_rom_fc_ff_ptr[0x325] = 0x00;
			changed_rom = 1;
		}

		if(changed_rom) {
			/* patch ROM 03 selftest to not do ROM cksum */
			g_rom_fc_ff_ptr[0x36cb0] = 0x18;
			g_rom_fc_ff_ptr[0x36cb1] = 0x18;
		}

	}
}



Disk *
cfg_get_dsk_from_slot_drive(int slot, int drive)
{
	Disk	*dsk;
	int	max_drive;

	/* Get dsk */
	max_drive = 2;
	switch(slot) {
	case 5:
		dsk = &(g_iwm.iwm.drive35[drive]);
		break;
	case 6:
		dsk = &(g_iwm.iwm.drive525[drive]);
		break;
	default:
		max_drive = MAX_C7_DISKS;
		dsk = &(g_iwm.iwm.smartport[drive]);
		break;
	}

	if(drive >= max_drive) {
		dsk -= drive;	/* move back to drive 0 effectively */
	}
	dsk->slot = slot;
	dsk->disk = drive+1;
	return dsk;
}


void
insert_disk(int slot, int drive, const char *name, int ejected, int force_size,
		const char *partition_name, int part_num)
{
	byte	buf_2img[512];
	Disk	*dsk;


//	char	*part_ptr;
	int	size;
#if 0
//	char	*name_ptr;
	char	 *uncomp_ptr, *system_str;
	int	system_len;
#endif
//	int	part_len;
	int	cmp_o, cmp_p, cmp_dot;
	int	cmp_b, cmp_i, cmp_n;
	int	can_write;
	int	len;
	int	nibs;
	int	unix_pos;
	size_t	name_len;
	int	image_identified;
	int	exp_size;
	int	save_track;
	int	ret;
	int	tmp;
	int	i;

    
	g_config_kegs_update_needed = 1;

	if((slot < 5) || (slot > 7)) {
		fatal_printf("Invalid slot for inserting disk: %d\n", slot);
		return;
	}
	if(drive < 0 || ((slot == 7) && (drive >= MAX_C7_DISKS)) ||
					((slot < 7) && (drive > 1))) {
		fatal_printf("Invalid drive for inserting disk: %d\n", drive);
		return;
	}

	dsk = cfg_get_dsk_from_slot_drive(slot, drive);

	// OG Do not insert same disk at same place
	if (( dsk->name_ptr && name) && !strcmp(dsk->name_ptr,name)) return ;

#if 0
	printf("Inserting disk %s (%s or %d) in slot %d, drive: %d\n", name,
		partition_name, part_num, slot, drive);
#endif

	dsk->just_ejected = 0;
	dsk->force_size = force_size;

	if(dsk->fd >= 0) {
		eject_disk(dsk);
	}

	// OG check for valid name first
	if (!name)	return ;

	/* Before opening, make sure no other mounted disk has this name */
	/* If so, unmount it */
	if(!ejected) {
		for(i = 0; i < 2; i++) {
			eject_named_disk(&g_iwm.iwm.drive525[i], name,partition_name);
			eject_named_disk(&g_iwm.iwm.drive35[i], name, partition_name);
		}
		for(i = 0; i < MAX_C7_DISKS; i++) {
			eject_named_disk(&g_iwm.iwm.smartport[i],name,partition_name);
		}
	}

	/*
	if(dsk->name_ptr != 0) {
		// free old name_ptr 
		free(dsk->name_ptr);
	}
	*/

	name_len = strlen(name);
	strncpy(dsk->name_ptr,name,MAX_DISK_NAME);
	/*
	name_len = strlen(name);
	name_ptr = (char *)malloc(name_len + 1);
	strncpy(name_ptr, name, name_len + 1);
	dsk->name_ptr = name_ptr;
	*/

//	dsk->partition_name = 0;
	dsk->partition_name[0] = 0;

	if(partition_name != 0) {
		/*
		part_len = strlen(partition_name) + 1;
		part_ptr = (char *)malloc(part_len);
		strncpy(part_ptr, partition_name, part_len);
		dsk->partition_name = part_ptr;
		*/
		strncpy(dsk->partition_name,partition_name,MAX_DISK_NAME);
	}
	dsk->partition_num = part_num;

	iwm_printf("Opening up disk image named: %s\n", dsk->name_ptr);

	if(ejected) {
		/* just get out of here */
		dsk->fd = -1;
		return;
	}

	dsk->fd = -1;
    
#ifdef ACTIVEGSKARATEKA
	can_write = 0;
#else
	can_write = 1;
#endif

//OG discard .GZ support
#if 0  
	if((name_len > 3) && (strcmp(&name_ptr[name_len - 3], ".gz") == 0)) {

		/* it's gzip'ed, try to gunzip it, then unlink the */
		/*   uncompressed file */

		can_write = 0;

		uncomp_ptr = (char *)malloc(name_len + 1);
		strncpy(uncomp_ptr, name_ptr, name_len + 1);
		uncomp_ptr[name_len - 3] = 0;

		system_len = 2*name_len + 100;
		system_str = (char *)malloc(system_len + 1);
		snprintf(system_str, system_len,
			"set -o noclobber;gunzip -c %c%s%c > %c%s%c",
			0x22, name_ptr, 0x22,
			0x22, uncomp_ptr, 0x22);
		/* 0x22 are " to allow spaces in filenames */
		printf("I am uncompressing %s into %s for mounting\n",
							name_ptr, uncomp_ptr);
		ret = system(system_str);
		if(ret == 0) {
			/* successfully ran */
			dsk->fd = open(uncomp_ptr, O_RDONLY | O_BINARY, 0x1b6);
			iwm_printf("Opening .gz file %s is fd: %d\n",
							uncomp_ptr, dsk->fd);

			/* and, unlink the temporary file */
			(void)unlink(uncomp_ptr);
		}
		free(system_str);
		free(uncomp_ptr);
		/* Reduce name_len by 3 so that subsequent compares for .po */
		/*  look at the correct chars */
		name_len -= 3;
	}
#endif // OG

	if(dsk->fd < 0 && can_write) {
		dsk->fd = open(dsk->name_ptr, O_RDWR | O_BINARY, 0x1b6);
	}

	if(dsk->fd < 0 /*&& can_write*/) {
		printf("Trying to open %s read-only, errno: %d\n", dsk->name_ptr,
								errno);
		dsk->fd = open(dsk->name_ptr, O_RDONLY | O_BINARY, 0x1b6);
		can_write = 0;
	}

	iwm_printf("open returned: %d\n", dsk->fd);

	if(dsk->fd < 0) {
		fatal_printf("Disk image %s does not exist!\n", dsk->name_ptr);
		return;
	}

	if(can_write != 0) {
		dsk->write_prot = 0;
		dsk->write_through_to_unix = 1;
	} else {
		dsk->write_prot = 1;
		dsk->write_through_to_unix = 0;
	}

	save_track = dsk->cur_qtr_track;	/* save arm position */
	dsk->image_type = DSK_TYPE_PRODOS;
	dsk->image_start = 0;

	/* See if it is in 2IMG format */
	ret = read(dsk->fd, (char *)&buf_2img[0], 512);
	size = force_size;
	if(size <= 0) {
		size = cfg_get_fd_size(dsk->fd) ;
	}

	/* Try to guess that there is a Mac Binary header of 128 bytes */
	/* See if image size & 0xfff = 0x080 which indicates extra 128 bytes */
	if((size & 0xfff) == 0x080) {
		printf("Assuming Mac Binary header on %s\n", dsk->name_ptr);
		dsk->image_start += 0x80;
	}
	image_identified = 0;
	if(buf_2img[0] == '2' && buf_2img[1] == 'I' && buf_2img[2] == 'M' &&
			buf_2img[3] == 'G') {

		// OG Put back Gilles Tschopp hack to deal with corrupted 2IMG files 	
		size -=  64;
		size = size & -0x1000;

		/* It's a 2IMG disk */
#ifdef _DEBUG
		printf("Image named %s is in 2IMG format\n", dsk->name_ptr);
#endif
		image_identified = 1;

		if(buf_2img[12] == 0) {
			printf("2IMG is in DOS 3.3 sector order\n");
			dsk->image_type = DSK_TYPE_DOS33;
		}
		if(buf_2img[19] & 0x80) {
			/* disk is locked */
			printf("2IMG is write protected\n");
			dsk->write_prot = 1;
			dsk->write_through_to_unix = 0;
		}
		if((buf_2img[17] & 1) && (dsk->image_type == DSK_TYPE_DOS33)) {
			dsk->vol_num = buf_2img[16];
			printf("Setting DOS 3.3 vol num to %d\n", dsk->vol_num);
		}

		// OG do not overwrite when 2mg size = 0
		int size2img = (buf_2img[31] << 24) + (buf_2img[30] << 16) +	(buf_2img[29] << 8) + buf_2img[28];
		if (size2img) size = size2img;
		else	printf(".2MG has corrupted size - fixed.\n");

		unix_pos = (buf_2img[27] << 24) + (buf_2img[26] << 16) +
				(buf_2img[25] << 8) + buf_2img[24];
		//	Some 2IMG archives have the size byte reversed
		if(size == 0x800c00) {
			//	Byte reversed 0x0c8000
			size = 0x0c8000;
		}
		dsk->image_start = unix_pos;
		dsk->image_size = size;
	}
	exp_size = 800*1024;
	if(dsk->disk_525) {
		exp_size = 140*1024;
	}
	if(!image_identified) {
		/* See if it might be the Mac diskcopy format */
		tmp = (buf_2img[0x40] << 24) + (buf_2img[0x41] << 16) +
				(buf_2img[0x42] << 8) + buf_2img[0x43];
		if((size >= (exp_size + 0x54)) && (tmp == exp_size)) {
			/* It's diskcopy since data size field matches */
			printf("Image named %s is in Mac diskcopy format\n",
								dsk->name_ptr);
			image_identified = 1;
			dsk->image_start += 0x54;
			dsk->image_size = exp_size;
			dsk->image_type = DSK_TYPE_PRODOS;	/* ProDOS */
		}
	}
	if(!image_identified) {
		/* Assume raw image */
		dsk->image_size = size;
		dsk->image_type = DSK_TYPE_PRODOS;
		if(dsk->disk_525) {
			dsk->image_type = DSK_TYPE_DOS33;

			if(name_len >= 4) {
				cmp_o = dsk->name_ptr[name_len-1];
				cmp_p = dsk->name_ptr[name_len-2];
				cmp_dot = dsk->name_ptr[name_len-3];
				if(cmp_dot == '.' &&
					  (cmp_p == 'p' || cmp_p == 'P') &&
					  (cmp_o == 'o' || cmp_o == 'O')) {
					dsk->image_type = DSK_TYPE_PRODOS;
				}

				cmp_b = dsk->name_ptr[name_len-1];
				cmp_i = dsk->name_ptr[name_len-2];
				cmp_n = dsk->name_ptr[name_len-3];
				cmp_dot = dsk->name_ptr[name_len-4];
				if(cmp_dot == '.' &&
					  (cmp_n == 'n' || cmp_n == 'N') &&
					  (cmp_i == 'i' || cmp_i == 'I') &&
					  (cmp_b == 'b' || cmp_b == 'B')) {
					dsk->image_type = DSK_TYPE_NIB;
					dsk->write_prot = 0;	// OG : Remove WriteProtect on .nib !!!! TO REACTIVATE !!
					dsk->write_through_to_unix = 0;
				}
			}
		}
	}

	dsk->disk_dirty = 0;
	dsk->nib_pos = 0;
	dsk->trks = 0;

	if(dsk->smartport) {
		g_iwm.g_highest_smartport_unit = MAX(dsk->drive,g_iwm.g_highest_smartport_unit);

// OG Disabled partition support
#ifndef ACTIVEGS
		if( (partition_name != NULL && *partition_name ) || part_num >= 0) {
			ret = cfg_partition_find_by_name_or_num(dsk->fd,
						partition_name, part_num, dsk);
			printf("partition %s (num %d) mounted, wr_prot: %d\n",
				partition_name, part_num, dsk->write_prot);

			if(ret < 0) {
				close(dsk->fd);
				dsk->fd = -1;
				return;
			}
		}
#endif
		iwm_printf("adding smartport device[%d], size:%08x, "
			"img_sz:%08x\n", dsk->drive, dsk->trks[0].unix_len,
			dsk->image_size);
	} else if(dsk->disk_525) {
		unix_pos = dsk->image_start;
		size = dsk->image_size;
		disk_set_num_tracks(dsk, 4*35);
		len = 0x1000;
		nibs = NIB_LEN_525;
		if(dsk->image_type == DSK_TYPE_NIB) {
			len = dsk->image_size / 35;;
			nibs = len;
		}
		if(size != 35*len) {
			fatal_printf("Disk 5.25 error: size is %d, not 140K.  "
				"Will try to mount anyway\n", size, 35*len);
		}
		for(i = 0; i < 35; i++) {
			iwm_move_to_track(dsk, 4*i);
			disk_unix_to_nib(dsk, 4*i, unix_pos, len, nibs);
			unix_pos += len;
		}
	} else {
		/* disk_35 */
		unix_pos = dsk->image_start;
		size = dsk->image_size;
		if(size != 800*1024) {
			fatal_printf("Disk 3.5 error: size is %d, not 800K.  "
				"Will try to mount anyway\n", size);
		}
		disk_set_num_tracks(dsk, 2*80);
		for(i = 0; i < 2*80; i++) {
			iwm_move_to_track(dsk, i);
			len = g_track_bytes_35[i >> 5];
			nibs = g_track_nibs_35[i >> 5];
			iwm_printf("Trk: %d.%d = unix: %08x, %04x, %04x\n",
				i>>1, i & 1, unix_pos, len, nibs);
			disk_unix_to_nib(dsk, i, unix_pos, len, nibs);
			unix_pos += len;

			iwm_printf(" trk_len:%05x\n", dsk->trks[i].track_len);
		}
	}

	iwm_move_to_track(dsk, save_track);

	x_notify_disk_insertion(dsk->image_size,dsk->image_start,(dsk->image_type == DSK_TYPE_PRODOS),slot,drive);

}

void
eject_named_disk(Disk *dsk, const char *name, const char *partition_name)
{

	if(dsk->fd < 0) {
		return;
	}

	/* If name matches, eject the disk! */
	if(!strcmp(dsk->name_ptr, name)) {
		/* It matches, eject it */
		if((partition_name != 0) && (dsk->partition_name != 0)) {
			/* If both have partitions, and they differ, then */
			/*  don't eject.  Otherwise, eject */
			if(strcmp(dsk->partition_name, partition_name) != 0) {
				/* Don't eject */
				return;
			}
		}
		eject_disk(dsk);
	}
}

void
eject_disk_by_num(int slot, int drive)
{
	Disk	*dsk;

	dsk = cfg_get_dsk_from_slot_drive(slot, drive);

	eject_disk(dsk);
}

void
eject_disk(Disk *dsk)
{
	int	motor_on;
	int	i;

	if(dsk->fd < 0) {
		return;
	}

	g_config_kegs_update_needed = 1;

	motor_on = g_iwm.iwm.motor_on;
	if(g_moremem.g_c031_disk35 & 0x40) {
		motor_on = g_iwm.iwm.motor_on35;
	}
	if(motor_on) {
		halt_printf("Try eject dsk:%s, but motor_on!\n", dsk->name_ptr);
	}

	iwm_flush_disk_to_unix(dsk);

	printf("Ejecting disk: %s\n", dsk->name_ptr);

	/* Free all memory, close file */

	/* free the tracks first */
	if(dsk->trks != 0) {
		for(i = 0; i < dsk->num_tracks; i++) {
			if(dsk->trks[i].nib_area) {
				free(dsk->trks[i].nib_area);
			}
			dsk->trks[i].nib_area = 0;
			dsk->trks[i].track_len = 0;
		}
		free(dsk->trks);
	}
	dsk->num_tracks = 0;
	dsk->trks = 0;

	/* close file, clean up dsk struct */
	close(dsk->fd);

	dsk->image_start = 0;
	dsk->image_size = 0;
	dsk->nib_pos = 0;
	dsk->disk_dirty = 0;
	dsk->write_through_to_unix = 0;
	dsk->write_prot = 1;
	dsk->fd = -1;
	dsk->just_ejected = 1;

	/* Leave name_ptr valid */		// OG WHY?

	/*
	if (dsk->name_ptr)
	{
		free(dsk->name_ptr);
		dsk->name_ptr = NULL;
	}
	*/
	dsk->name_ptr[0]=0;
}


int
cfg_get_fd_size(int fd)
{
	struct stat stat_buf;
	int	ret;

	ret = fstat(fd, &stat_buf);
	if(ret != 0) {
		fprintf(stderr,"fstat returned %d on fd %d, errno: %d\n",
			ret, fd, errno);
		stat_buf.st_size = 0;
	}

	return (int)stat_buf.st_size;
}
