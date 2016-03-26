/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#pragma once

#ifdef INCLUDE_RCSID_C
const char rcsid_scc_h[] = "@(#)$KmKId: scc.h,v 1.17 2004-12-03 14:03:12-05 kentd Exp $";
#endif

#include <ctype.h>

#ifdef _WIN32
# include <winsock2.h>
#else
# include <sys/socket.h>
# include <netinet/in.h>
# include <netdb.h>
#endif

#if defined(HPUX) || defined(__linux__) || defined(SOLARIS) || defined(DRIVER_IOS) || defined(DRIVER_OSX) || defined(__MACH__) || defined(_WIN32)
# define SCC_SOCKETS
#endif


/* my scc port 0 == channel A, port 1 = channel B */

#define	SCC_INBUF_SIZE		512		/* must be a power of 2 */
#define	SCC_OUTBUF_SIZE		512		/* must be a power of 2 */

#define SCC_MODEM_MAX_CMD_STR	128

#ifndef SOCKET
# define SOCKET		word32		/* for non-windows */
#endif


#ifdef _WIN32
#pragma pack(push,PRAGMA_PACK_SIZE)
#endif

STRUCT(Scc) {
	int	port;
	int	state;
	int	accfd;
	SOCKET	sockfd;
	int	socket_state;
	int	rdwrfd;
	void	*host_handle;
	void	*host_handle2;
	int	host_aux1;
	int	read_called_this_vbl;
	int	write_called_this_vbl;

	int	mode;
	int	reg_ptr;
	int	reg[16];

	int	rx_queue_depth;
	byte	rx_queue[4];

	int	in_rdptr;
	int	in_wrptr;
	byte	in_buf[SCC_INBUF_SIZE];

	int	out_rdptr;
	int	out_wrptr;
	byte	out_buf[SCC_OUTBUF_SIZE];

	int	br_is_zero;
	int	tx_buf_empty;
	int	wantint_rx;
	int	wantint_tx;
	int	wantint_zerocnt;
	int	dcd;

	_ALIGNED(8) double	br_dcycs;
	_ALIGNED(8) double	tx_dcycs ;
	_ALIGNED(8) double	rx_dcycs;

	int	br_event_pending;
	int	rx_event_pending;
	int	tx_event_pending;

	int	char_size;
	int	baud_rate;
	_ALIGNED(8) double	out_char_dcycs;

	int	socket_num_rings;
	int	socket_last_ring_dcycs;
	word32	modem_mode;
	int	modem_dial_or_acc_mode;
	int	modem_plus_mode;
	int	modem_s0_val;
	int	telnet_mode;
	int	telnet_iac;
	word32	telnet_local_mode[2];
	word32	telnet_remote_mode[2];
	word32	telnet_reqwill_mode[2];
	word32	telnet_reqdo_mode[2];
	int	modem_cmd_len;
	byte	modem_cmd_str[SCC_MODEM_MAX_CMD_STR + 5];
} XCODE_PRAGMA_PACK ;




#define SCCMODEM_NOECHO		0x0001
#define SCCMODEM_NOVERBOSE	0x0002


class s_scc : public serialized
{
public:
	Scc	scc_stat[2];
	int g_scc_overflow ;
	int	g_wsastartup_called;

	s_scc()
	{
		INIT_SERIALIZED();
	}


#define SERIALIZE_SOCKET(X,N) int old_##N = X.sockfd; X.sockfd=0;
#define UNSERIALIZE_SOCKET(X,N) X.sockfd = old_##N ;

	void in(serialize* _s)				
	{									
		byte* ptr = (byte*)_s->data;	
		int ssize = *(int*)ptr;			
		if (ssize!= sizeof(*this))		
			ALIGNMENT_ERROR("scc",ssize,sizeof(*this));	
		ptr += sizeof(ssize);					
		memcpy(this,ptr,sizeof(*this));		

		// TODO UNSERIALIZE SOCKET
	}					

	int out(serialize* _s, int _fastalloc)	
	{										

		int ssize = sizeof(*this);			
		int size = ssize + sizeof(ssize);	
		_s->size = size;					
		_s->fastalloc = _fastalloc;			
		_s->data = (void*)x_malloc(size,_fastalloc); 
		byte* ptr = (byte*) _s->data;	

		memcpy(ptr,&ssize,sizeof(ssize));	
		ptr+=sizeof(ssize);	
		
		SERIALIZE_SOCKET(scc_stat[0],stat0);
		SERIALIZE_SOCKET(scc_stat[1],stat1);

		memcpy(ptr,this,sizeof(*this));	

		UNSERIALIZE_SOCKET(scc_stat[0],stat0);
		UNSERIALIZE_SOCKET(scc_stat[1],stat1);

		return size;	
	}

} XCODE_PRAGMA_PACK;

#ifdef _WIN32
#pragma pack(pop)
#endif

extern s_scc g_scc;