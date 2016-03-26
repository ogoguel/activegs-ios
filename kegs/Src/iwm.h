/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#pragma once

#ifdef INCLUDE_RCSID_C
const char rcsid_iwm_h[] = "@(#)$KmKId: iwm.h,v 1.14 2004-10-20 17:29:38-04 kentd Exp $";
#endif

#define MAX_TRACKS	(2*80)
#define MAX_C7_DISKS	8

#define NIB_LEN_525		0x1900		/* 51072 bits per track */
#define NIBS_FROM_ADDR_TO_DATA	20

#define DSK_TYPE_PRODOS		0
#define DSK_TYPE_DOS33		1
#define DSK_TYPE_NIB		2

typedef struct _Disk Disk;

#ifdef _WIN32
#pragma pack(push,PRAGMA_PACK_SIZE)
#endif

STRUCT(Trk) {
	Disk	*dsk;
	byte	*nib_area;
	int	track_dirty;
	int	overflow_size;
	int	track_len;
	int	unix_pos;
	int	unix_len;
} XCODE_PRAGMA_PACK;

#define MAX_DISK_NAME 512
struct _Disk {
	_ALIGNED(8)	double	dcycs_last_read;
	char	name_ptr[MAX_DISK_NAME];
	char	partition_name[MAX_DISK_NAME];
	
	int	partition_num;
	int	fd;
	int	force_size;
	int	image_start;
	int	image_size;
	int	smartport;
	int	disk_525;
	int	drive;
	int	cur_qtr_track;
	int	image_type;
	int	vol_num;
	int	write_prot;
	int	write_through_to_unix;
	int	disk_dirty;
	int	just_ejected;
	int	last_phase;
	int	nib_pos;
	int	num_tracks;
	Trk	*trks;
	int slot;
	int disk;
} XCODE_PRAGMA_PACK;


STRUCT(Iwm) {
	Disk	drive525[2];
	Disk	drive35[2];
	Disk	smartport[MAX_C7_DISKS];
	int	motor_on;
	int	motor_off;
	int	motor_off_vbl_count;
	int	motor_on35;
	int	head35;
	int	step_direction35;
	int	iwm_phase[4];
	int	iwm_mode;
	int	drive_select;
	int	q6;
	int	q7;
	int	enable2;
	int	reset;

	word32	previous_write_val;
	int	previous_write_bits;
} XCODE_PRAGMA_PACK;


STRUCT(Driver_desc) {
	word16	sig;
	word16	blk_size;
	word32	blk_count;
	word16	dev_type;
	word16	dev_id;
	word32	data;
	word16	drvr_count;
} XCODE_PRAGMA_PACK;

STRUCT(Part_map) {
	word16	sig;
	word16	sigpad;
	word32	map_blk_cnt;
	word32	phys_part_start;
	word32	part_blk_cnt;
	char	part_name[32];
	char	part_type[32];
	word32	data_start;
	word32	data_cnt;
	word32	part_status;
	word32	log_boot_start;
	word32	boot_size;
	word32	boot_load;
	word32	boot_load2;
	word32	boot_entry;
	word32	boot_entry2;
	word32	boot_cksum;
	char	processor[16];
	char	junk[128];
} XCODE_PRAGMA_PACK;

class s_iwm : public serialized
{
public:
	int	g_fast_disk_emul ;
	int	g_slow_525_emul_wr ;
	_ALIGNED(8)	double	g_dcycs_end_emul_wr ;
	int	g_fast_disk_unnib ;
	int	g_iwm_fake_fast ;
	int	from_disk_byte[256];
	int	from_disk_byte_valid ;
	Iwm	iwm;
	int	g_iwm_motor_on ;
	int	g_check_nibblization ;
	int	g_highest_smartport_unit ;
	word32 g_cycs_in_io_read;
	int g_cnt_enable2_handshake ;


	s_iwm()
	{
		INIT_SERIALIZED();

		g_fast_disk_emul = 1;
		g_highest_smartport_unit = -1;
	
	}
#define WRITE_TRK(X) \
	if (X.trks)		\
		{			\
			memcpy(ptr,X.trks,X.num_tracks * sizeof(Trk));	\
			ptr += X.num_tracks * sizeof(Trk);	\
			for(int t=0;t<X.num_tracks;t++) \
			{	\
				if (X.trks[t].nib_area)	\
				{\
					memcpy(ptr,X.trks[t].nib_area,X.trks[t].track_len);\
					ptr+=X.trks[t].track_len;\
				}\
			}\
		}
	
#define FREE_TRK(X) \
		if (X.trks) \
		{			\
			for(int t=0;t<X.num_tracks;t++) \
			{	\
				if (X.trks[t].nib_area)	\
				{\
					free(X.trks[t].nib_area); \
					X.trks[t].nib_area=NULL; \
				}\
			}\
			free(X.trks);	\
			X.trks=NULL;	\
		}

#define ALLOC_TRK(X) \
	if (X.trks)		\
		{			\
			X.trks = (Trk *)malloc(X.num_tracks * sizeof(Trk));	\
            X.dcycs_last_read = get_dtime(); \
			memcpy(X.trks,ptr,X.num_tracks * sizeof(Trk));	\
			ptr += X.num_tracks * sizeof(Trk);	\
			for(int t=0;t<X.num_tracks;t++) \
			{	\
                X.trks[t].dsk = &X; \
				if (X.trks[t].nib_area)	\
				{\
					X.trks[t].nib_area = (byte*)malloc(X.trks[t].track_len);\
					memcpy(X.trks[t].nib_area,ptr,X.trks[t].track_len);\
					ptr+=X.trks[t].track_len;\
				}\
			}\
		}

#define SIZE_TRK(X) \
		if (X.trks)	\
		{			\
			for(int t=0;t<X.num_tracks;t++)\
				size += sizeof(Trk) + (X.trks[t].nib_area? X.trks[t].track_len : 0 ); \
		}

#define SAVE_TRK_PTR(X,N) Trk* old_##N = X.trks; int old_fd_##N = X.fd; X.trks=NULL; X.fd=-1;
#define RESTORE_TRK_PTR(X,N) X.trks = old_##N; X.fd = old_fd_##N ;

	void in(serialize* _s)
	{

		SAVE_TRK_PTR(iwm.drive525[0],drive5251);
		SAVE_TRK_PTR(iwm.drive525[1],drive5252);
		SAVE_TRK_PTR(iwm.drive35[0],drive351);
		SAVE_TRK_PTR(iwm.drive35[1],drive352);
		SAVE_TRK_PTR(iwm.smartport[0],smartport0);
		SAVE_TRK_PTR(iwm.smartport[1],smartport1);
		SAVE_TRK_PTR(iwm.smartport[2],smartport2);
		SAVE_TRK_PTR(iwm.smartport[3],smartport3);

			
		byte* ptr = (byte*)_s->data;
		int ssize = *(int*)ptr;
		if (ssize!= sizeof(*this))
			ALIGNMENT_ERROR("iwm",ssize,sizeof(*this));
		ptr += sizeof(ssize);
		memcpy(this,ptr,sizeof(*this));

		RESTORE_TRK_PTR(iwm.drive525[0],drive5251);
		RESTORE_TRK_PTR(iwm.drive525[1],drive5252);
		RESTORE_TRK_PTR(iwm.drive35[0],drive351);
		RESTORE_TRK_PTR(iwm.drive35[1],drive352);
		RESTORE_TRK_PTR(iwm.smartport[0],smartport0);
		RESTORE_TRK_PTR(iwm.smartport[1],smartport1);
		RESTORE_TRK_PTR(iwm.smartport[2],smartport2);
		RESTORE_TRK_PTR(iwm.smartport[3],smartport3);
        
      

	}

	int out(serialize* _s, int _fastalloc)
	{
		// calcule la taille totale de la structure 

		SAVE_TRK_PTR(iwm.drive525[0],drive5251);
		SAVE_TRK_PTR(iwm.drive525[1],drive5252);
		SAVE_TRK_PTR(iwm.drive35[0],drive351);
		SAVE_TRK_PTR(iwm.drive35[1],drive352);
		SAVE_TRK_PTR(iwm.smartport[0],smartport0);
		SAVE_TRK_PTR(iwm.smartport[1],smartport1);
		SAVE_TRK_PTR(iwm.smartport[2],smartport2);
		SAVE_TRK_PTR(iwm.smartport[3],smartport3);

		// copie la structure de base

		int ssize = sizeof(*this);
		int size = ssize + sizeof(ssize);
		_s->size = size;
		_s->fastalloc = _fastalloc;
		_s->data = (void*)x_malloc(size,_fastalloc);
		byte* ptr = (byte*) _s->data;
		memcpy(ptr,&ssize,sizeof(ssize));
		ptr+=sizeof(ssize);
		memcpy(ptr,this,sizeof(*this));

		RESTORE_TRK_PTR(iwm.drive525[0],drive5251);
		RESTORE_TRK_PTR(iwm.drive525[1],drive5252);
		RESTORE_TRK_PTR(iwm.drive35[0],drive351);
		RESTORE_TRK_PTR(iwm.drive35[1],drive352);
		RESTORE_TRK_PTR(iwm.smartport[0],smartport0);
		RESTORE_TRK_PTR(iwm.smartport[1],smartport1);
		RESTORE_TRK_PTR(iwm.smartport[2],smartport2);
		RESTORE_TRK_PTR(iwm.smartport[3],smartport3);

		return size;
	}
} XCODE_PRAGMA_PACK;
#ifdef _WIN32
#pragma pack(pop)
#endif

extern s_iwm g_iwm;
extern  int g_track_bytes_35[];
extern  int g_track_nibs_35[];

void iwm_init_drive(Disk *dsk, int smartport, int drive, int disk_525);
void disk_set_num_tracks(Disk *dsk, int num_tracks);
void iwm_init(void);
void iwm_shut(void);	//OG
void iwm_reset(void);
void draw_iwm_status(int line, char *buf);
void iwm_flush_disk_to_unix(Disk *dsk);
void iwm_vbl_update(int doit_3_persec);
void iwm_show_stats(void);
void iwm_touch_switches(int loc, double dcycs);
void iwm_move_to_track(Disk *dsk, int new_track);
void iwm525_phase_change(int drive, int phase);
int iwm_read_status35(double dcycs);
void iwm_do_action35(double dcycs);
int iwm_read_c0ec(double dcycs);
int read_iwm(int loc, double dcycs);
void write_iwm(int loc, int val, double dcycs);
int iwm_read_enable2(double dcycs);
int iwm_read_enable2_handshake(double dcycs);
void iwm_write_enable2(int val, double dcycs);
int iwm_read_data(Disk *dsk, int fast_disk_emul, double dcycs);
void iwm_write_data(Disk *dsk, word32 val, int fast_disk_emul, double dcycs);
void sector_to_partial_nib(byte *in, byte *nib_ptr);
int disk_unnib_4x4(Disk *dsk);
int iwm_denib_track525(Disk *dsk, Trk *trk, int qtr_track, byte *outbuf);
int iwm_denib_track35(Disk *dsk, Trk *trk, int qtr_track, byte *outbuf);
int disk_track_to_unix(Disk *dsk, int qtr_track, byte *outbuf);
void show_hex_data(byte *buf, int count);
void disk_check_nibblization(Disk *dsk, int qtr_track, byte *buf, int size);
void disk_unix_to_nib(Disk *dsk, int qtr_track, int unix_pos, int unix_len, int nib_len);
void iwm_nibblize_track_nib525(Disk *dsk, Trk *trk, byte *track_buf, int qtr_track);
void iwm_nibblize_track_525(Disk *dsk, Trk *trk, byte *track_buf, int qtr_track);
void iwm_nibblize_track_35(Disk *dsk, Trk *trk, byte *track_buf, int qtr_track);
void disk_4x4_nib_out(Disk *dsk, word32 val);
void disk_nib_out(Disk *dsk, byte val, int size);
void disk_nib_end_track(Disk *dsk);
void iwm_show_track(int slot_drive, int track);
void iwm_show_a_track(Trk *trk);


