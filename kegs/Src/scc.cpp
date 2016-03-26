/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#include "defc.h"
#include "moremem.h"
#include "sim65816.h"

#ifdef UNDER_CE
#define vsnprintf _vsnprintf
#endif



/* my scc port 0 == channel A = slot 1 = c039/c03b */
/*        port 1 == channel B = slot 2 = c038/c03a */

#include "scc.h"

s_scc g_scc;

#define SCC_R14_DPLL_SOURCE_BRG		0x100
#define SCC_R14_FM_MODE			0x200

#define SCC_DCYCS_PER_PCLK	((DCYCS_1_MHZ) / ((DCYCS_28_MHZ) /8))
#define SCC_DCYCS_PER_XTAL	((DCYCS_1_MHZ) / 3686400.0)

#define SCC_BR_EVENT			1
#define SCC_TX_EVENT			2
#define SCC_RX_EVENT			3
#define SCC_MAKE_EVENT(port, a)		(((a) << 1) + (port))

const int g_baud_table[] = {
	110, 300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200
};


void
scc_init()
{
	Scc	*scc_ptr;
	int	i, j;

	for(i = 0; i < 2; i++) {
		scc_ptr = &(g_scc.scc_stat[i]);
		scc_ptr->accfd = -1;
		scc_ptr->sockfd = -1;
		scc_ptr->socket_state = -1;
		scc_ptr->rdwrfd = -1;
		scc_ptr->state = 0;
		scc_ptr->host_handle = 0;
		scc_ptr->host_handle2 = 0;
		scc_ptr->br_event_pending = 0;
		scc_ptr->rx_event_pending = 0;
		scc_ptr->tx_event_pending = 0;
		scc_ptr->char_size = 8;
		scc_ptr->baud_rate = 9600;
		scc_ptr->telnet_mode = 0;
		scc_ptr->telnet_iac = 0;
		scc_ptr->out_char_dcycs = 0.0;
		scc_ptr->socket_num_rings = 0;
		scc_ptr->socket_last_ring_dcycs = 0;
		scc_ptr->modem_mode = 0;
		scc_ptr->modem_dial_or_acc_mode = 0;
		scc_ptr->modem_plus_mode = 0;
		scc_ptr->modem_s0_val = 0;
		scc_ptr->modem_cmd_len = 0;
		scc_ptr->modem_cmd_str[0] = 0;
		for(j = 0; j < 2; j++) {
			scc_ptr->telnet_local_mode[j] = 0;
			scc_ptr->telnet_remote_mode[j] = 0;
			scc_ptr->telnet_reqwill_mode[j] = 0;
			scc_ptr->telnet_reqdo_mode[j] = 0;
		}
	}

	scc_reset();
}

void
scc_reset()
{
	Scc	*scc_ptr;
	int	i;

	for(i = 0; i < 2; i++) {
		scc_ptr = &(g_scc.scc_stat[i]);

		scc_ptr->port = i;
		scc_ptr->mode = 0;
		scc_ptr->reg_ptr = 0;
		scc_ptr->in_rdptr = 0;
		scc_ptr->in_wrptr = 0;
		scc_ptr->out_rdptr = 0;
		scc_ptr->out_wrptr = 0;
		scc_ptr->dcd = 0;
		scc_ptr->wantint_rx = 0;
		scc_ptr->wantint_tx = 0;
		scc_ptr->wantint_zerocnt = 0;
		scc_ptr->read_called_this_vbl = 0;
		scc_ptr->write_called_this_vbl = 0;
		scc_evaluate_ints(i);
		scc_hard_reset_port(i);
	}
}

void
scc_hard_reset_port(int port)
{
	Scc	*scc_ptr;

	scc_reset_port(port);

	scc_ptr = &(g_scc.scc_stat[port]);
	scc_ptr->reg[14] = 0;		/* zero bottom two bits */
	scc_ptr->reg[13] = 0;
	scc_ptr->reg[12] = 0;
	scc_ptr->reg[11] = 0x08;
	scc_ptr->reg[10] = 0;
	scc_ptr->reg[7] = 0;
	scc_ptr->reg[6] = 0;
	scc_ptr->reg[5] = 0;
	scc_ptr->reg[4] = 0x04;
	scc_ptr->reg[3] = 0;
	scc_ptr->reg[2] = 0;
	scc_ptr->reg[1] = 0;

	/* HACK HACK: */
	g_scc.scc_stat[0].reg[9] = 0;		/* Clear all interrupts */

	scc_evaluate_ints(port);

	scc_regen_clocks(port);
}

void
scc_reset_port(int port)
{
	Scc	*scc_ptr;

	scc_ptr = &(g_scc.scc_stat[port]);
	scc_ptr->reg[15] = 0xf8;
	scc_ptr->reg[14] &= 0x03;	/* 0 most (including >= 0x100) bits */
	scc_ptr->reg[10] = 0;
	scc_ptr->reg[5] &= 0x65;	/* leave tx bits and sdlc/crc bits */
	scc_ptr->reg[4] |= 0x04;	/* Set async mode */
	scc_ptr->reg[3] &= 0xfe;	/* clear receiver enable */
	scc_ptr->reg[1] &= 0xfe;	/* clear ext int enable */

	scc_ptr->br_is_zero = 0;
	scc_ptr->tx_buf_empty = 1;

	scc_ptr->wantint_rx = 0;
	scc_ptr->wantint_tx = 0;
	scc_ptr->wantint_zerocnt = 0;

	scc_ptr->rx_queue_depth = 0;

	scc_evaluate_ints(port);

	scc_regen_clocks(port);

	scc_clr_tx_int(port);
	scc_clr_rx_int(port);
}

void
scc_regen_clocks(int port)
{
	Scc	*scc_ptr;
	double	br_dcycs, tx_dcycs, rx_dcycs;
	double	rx_char_size, tx_char_size;
	double	clock_mult;
	word32	reg4;
	word32	reg14;
	word32	reg11;
	word32	br_const;
	word32	baud;
	word32	max_diff;
	word32	diff;
	int	state;
	int	baud_entries;
	int	pos;
	int	i;

	/*	Always do baud rate generator */
	scc_ptr = &(g_scc.scc_stat[port]);
	br_const = (scc_ptr->reg[13] << 8) + scc_ptr->reg[12];
	br_const += 2;	/* counts down past 0 */

	reg4 = scc_ptr->reg[4];
	clock_mult = 1.0;
	switch((reg4 >> 6) & 3) {
	case 0:		/* x1 */
		clock_mult = 1.0;
		break;
	case 1:		/* x16 */
		clock_mult = 16.0;
		break;
	case 2:		/* x32 */
		clock_mult = 32.0;
		break;
	case 3:		/* x64 */
		clock_mult = 64.0;
		break;
	}

	br_dcycs = 0.01;
	reg14 = scc_ptr->reg[14];
	if(reg14 & 0x1) {
		br_dcycs = SCC_DCYCS_PER_XTAL;
		if(reg14 & 0x2) {
			br_dcycs = SCC_DCYCS_PER_PCLK;
		}
	}

	br_dcycs = br_dcycs * (double)br_const;

	tx_dcycs = 1;
	rx_dcycs = 1;
	reg11 = scc_ptr->reg[11];
	if(((reg11 >> 3) & 3) == 2) {
		tx_dcycs = 2.0 * br_dcycs * clock_mult;
	}
	if(((reg11 >> 5) & 3) == 2) {
		rx_dcycs = 2.0 * br_dcycs * clock_mult;
	}

	tx_char_size = 8.0;
	switch((scc_ptr->reg[5] >> 5) & 0x3) {
	case 0:	// 5 bits
		tx_char_size = 5.0;
		break;
	case 1:	// 7 bits
		tx_char_size = 7.0;
		break;
	case 2:	// 6 bits
		tx_char_size = 6.0;
		break;
	}

	scc_ptr->char_size = (int)tx_char_size;

	switch((scc_ptr->reg[4] >> 2) & 0x3) {
	case 1:	// 1 stop bit
		tx_char_size += 2.0;	// 1 stop + 1 start bit
		break;
	case 2:	// 1.5 stop bit
		tx_char_size += 2.5;	// 1.5 stop + 1 start bit
		break;
	case 3:	// 2 stop bits
		tx_char_size += 3.0;	// 2.0 stop + 1 start bit
		break;
	}

	if(scc_ptr->reg[4] & 1) {
		// parity enabled
		tx_char_size += 1.0;
	}

	if(scc_ptr->reg[14] & 0x10) {
		/* loopback mode, make it go faster...*/
		rx_char_size = 1.0;
		tx_char_size = 1.0;
	}

	rx_char_size = tx_char_size;	/* HACK */

	baud = (int)(DCYCS_1_MHZ / tx_dcycs);
	max_diff = 5000000;
	pos = 0;
	baud_entries = sizeof(g_baud_table)/sizeof(g_baud_table[0]);
	for(i = 0; i < baud_entries; i++) {
		diff = abs((int)(g_baud_table[i] - baud));
		if(diff < max_diff) {
			pos = i;
			max_diff = diff;
		}
	}

	scc_ptr->baud_rate = g_baud_table[pos];

	scc_ptr->br_dcycs = br_dcycs;
	scc_ptr->tx_dcycs = tx_dcycs * tx_char_size;
	scc_ptr->rx_dcycs = rx_dcycs * rx_char_size;

	state = scc_ptr->state;
	if(state == 2) {
		/* real serial ports */
#if defined(DRIVER_IOS) || defined(DRIVER_OSX)
		scc_serial_mac_change_params(port);
#endif
#ifdef _WIN32
		scc_serial_win_change_params(port);
#endif
	} else {
		scc_socket_change_params(port);
	}
}

void
scc_port_init(int port)
{
	int	state;

	state = 0;
	if(g_sim65816.g_raw_serial) {
#if defined(DRIVER_IOS) || defined(DRIVER_OSX)
		state = scc_serial_mac_init(port);
#endif
#ifdef _WIN32
		state = scc_serial_win_init(port);
#endif
	}

	if(state <= 0) {
		scc_socket_init(port);
	}
}

void
scc_try_to_empty_writebuf(int port, double dcycs)
{
	Scc	*scc_ptr;
	int	state;

	scc_ptr = &(g_scc.scc_stat[port]);
	state = scc_ptr->state;
	if(scc_ptr->write_called_this_vbl) {
		return;
	}

	scc_ptr->write_called_this_vbl = 1;

	if(state == 2) {
#if defined(DRIVER_IOS) || defined(DRIVER_OSX)
		scc_serial_mac_empty_writebuf(port);
#endif
#if defined(_WIN32)
		scc_serial_win_empty_writebuf(port);
#endif
	} else if(state == 1) {
		scc_socket_empty_writebuf(port, dcycs);
	}
}

void
scc_try_fill_readbuf(int port, double dcycs)
{
	Scc	*scc_ptr;
	int	space_used, space_left;
	int	state;

	scc_ptr = &(g_scc.scc_stat[port]);
	state = scc_ptr->state;

	space_used = scc_ptr->in_wrptr - scc_ptr->in_rdptr;
	if(space_used < 0) {
		space_used += SCC_INBUF_SIZE;
	}
	space_left = (7*SCC_INBUF_SIZE/8) - space_used;
	if(space_left < 1) {
		/* Buffer is pretty full, don't try to get more */
		return;
	}

#if 0
	if(scc_ptr->read_called_this_vbl) {
		return;
	}
#endif

	scc_ptr->read_called_this_vbl = 1;

	if(state == 2) {
#if defined(DRIVER_IOS) || defined(DRIVER_OSX)
		scc_serial_mac_fill_readbuf(port, space_left, dcycs);
#endif
#if defined(_WIN32)
		scc_serial_win_fill_readbuf(port, space_left, dcycs);
#endif
	} else if(state == 1) {
		scc_socket_fill_readbuf(port, space_left, dcycs);
	}
}

void
scc_update(double dcycs)
{
	/* called each VBL update */
	g_scc.scc_stat[0].write_called_this_vbl = 0;
	g_scc.scc_stat[1].write_called_this_vbl = 0;
	g_scc.scc_stat[0].read_called_this_vbl = 0;
	g_scc.scc_stat[1].read_called_this_vbl = 0;

	scc_try_to_empty_writebuf(0, dcycs);
	scc_try_to_empty_writebuf(1, dcycs);
	scc_try_fill_readbuf(0, dcycs);
	scc_try_fill_readbuf(1, dcycs);

	g_scc.scc_stat[0].write_called_this_vbl = 0;
	g_scc.scc_stat[1].write_called_this_vbl = 0;
	g_scc.scc_stat[0].read_called_this_vbl = 0;
	g_scc.scc_stat[1].read_called_this_vbl = 0;
}

void
do_scc_event(int type, double dcycs)
{
	Scc	*scc_ptr;
	int	port;

	port = type & 1;
	type = (type >> 1);

	scc_ptr = &(g_scc.scc_stat[port]);
	if(type == SCC_BR_EVENT) {
	//	printf("SCC_BR_EVENT %d\n",port);

		/* baud rate generator counted down to 0 */
		scc_ptr->br_event_pending = 0;
		scc_set_zerocnt_int(port);
		scc_maybe_br_event(port, dcycs);
	} else if(type == SCC_TX_EVENT) {
	//	printf("SCC_TX_EVENT %d\n",port);

		scc_ptr->tx_event_pending = 0;
		scc_ptr->tx_buf_empty = 1;
		scc_handle_tx_event(port, dcycs);
	} else if(type == SCC_RX_EVENT) {
	//		printf("SCC_RX_EVENT %d\n",port);
		scc_ptr->rx_event_pending = 0;
		scc_maybe_rx_event(port, dcycs);
	} else {
		halt_printf("do_scc_event: %08x!\n", type);
	}
	return;
}

void
show_scc_state()
{
	Scc	*scc_ptr;
	int	i, j;

	for(i = 0; i < 2; i++) {
		scc_ptr = &(g_scc.scc_stat[i]);
		printf("SCC port: %d\n", i);
		for(j = 0; j < 16; j += 4) {
			printf("Reg %2d-%2d: %02x %02x %02x %02x\n", j, j+3,
				scc_ptr->reg[j], scc_ptr->reg[j+1],
				scc_ptr->reg[j+2], scc_ptr->reg[j+3]);
		}
		printf("state: %d, accfd: %d, rdwrfd: %d, host:%p, host2:%p\n",
			scc_ptr->state, scc_ptr->accfd, scc_ptr->rdwrfd,
			scc_ptr->host_handle, scc_ptr->host_handle2);
		printf("in_rdptr: %04x, in_wr:%04x, out_rd:%04x, out_wr:%04x\n",
			scc_ptr->in_rdptr, scc_ptr->in_wrptr,
			scc_ptr->out_rdptr, scc_ptr->out_wrptr);
		printf("rx_queue_depth: %d, queue: %02x, %02x, %02x, %02x\n",
			scc_ptr->rx_queue_depth, scc_ptr->rx_queue[0],
			scc_ptr->rx_queue[1], scc_ptr->rx_queue[2],
			scc_ptr->rx_queue[3]);
		printf("want_ints: rx:%d, tx:%d, zc:%d\n",
			scc_ptr->wantint_rx, scc_ptr->wantint_tx,
			scc_ptr->wantint_zerocnt);
		printf("ev_pendings: rx:%d, tx:%d, br:%d\n",
			scc_ptr->rx_event_pending,
			scc_ptr->tx_event_pending,
			scc_ptr->br_event_pending);
		printf("br_dcycs: %f, tx_dcycs: %f, rx_dcycs: %f\n",
			scc_ptr->br_dcycs, scc_ptr->tx_dcycs,
			scc_ptr->rx_dcycs);
		printf("char_size: %d, baud_rate: %d, mode: %d\n",
			scc_ptr->char_size, scc_ptr->baud_rate,
			scc_ptr->mode);
		printf("modem_dial_mode:%d, telnet_mode:%d iac:%d, "
			"modem_cmd_len:%d\n", scc_ptr->modem_dial_or_acc_mode,
			scc_ptr->telnet_mode, scc_ptr->telnet_iac,
			scc_ptr->modem_cmd_len);
		printf("telnet_loc_modes:%08x %08x, telnet_rem_motes:"
			"%08x %08x\n", scc_ptr->telnet_local_mode[0],
			scc_ptr->telnet_local_mode[1],
			scc_ptr->telnet_remote_mode[0],
			scc_ptr->telnet_remote_mode[1]);
		printf("modem_mode:%08x plus_mode: %d, out_char_dcycs: %f\n",
			scc_ptr->modem_mode, scc_ptr->modem_plus_mode,
			scc_ptr->out_char_dcycs);
	}

}

#define LEN_SCC_LOG	50
STRUCT(Scc_log) {
	int	regnum;
	word32	val;
	double	dcycs;
};

Scc_log	g_scc_log[LEN_SCC_LOG];
int	g_scc_log_pos = 0;

#define SCC_REGNUM(wr,port,reg) ((wr << 8) + (port << 4) + reg)

void
scc_log(int regnum, word32 val, double dcycs)
{
	int	pos;

	pos = g_scc_log_pos;
	g_scc_log[pos].regnum = regnum;
	g_scc_log[pos].val = val;
	g_scc_log[pos].dcycs = dcycs;
	pos++;
	if(pos >= LEN_SCC_LOG) {
		pos = 0;
	}
	g_scc_log_pos = pos;
}

void
show_scc_log(void)
{
	double	dcycs;
	int	regnum;
	int	pos;
	int	i;

	pos = g_scc_log_pos;
	dcycs = g_sim65816.g_cur_dcycs;
	printf("SCC log pos: %d, cur dcycs:%f\n", pos, dcycs);
	for(i = 0; i < LEN_SCC_LOG; i++) {
		pos--;
		if(pos < 0) {
			pos = LEN_SCC_LOG - 1;
		}
		regnum = g_scc_log[pos].regnum;
		printf("%d:%d: port:%d wr:%d reg: %d val:%02x at t:%f\n",
			i, pos, (regnum >> 4) & 0xf, (regnum >> 8),
			(regnum & 0xf),
			g_scc_log[pos].val,
			g_scc_log[pos].dcycs - dcycs);
	}
}

word32
scc_read_reg(int port, double dcycs)
{
	Scc	*scc_ptr;
	word32	ret;
	int	regnum;

	scc_ptr = &(g_scc.scc_stat[port]);
	scc_ptr->mode = 0;
	regnum = scc_ptr->reg_ptr;

	/* port 0 is channel A, port 1 is channel B */
	switch(regnum) {
	case 0:
	case 4:
		ret = 0x60;	/* 0x44 = no dcd, no cts,0x6c = dcd ok, cts ok*/
		if(scc_ptr->dcd) {
			ret |= 0x08;
		}
		ret |= 0x8;	/* HACK HACK */
		if(scc_ptr->rx_queue_depth) {
			ret |= 0x01;
		}
		if(scc_ptr->tx_buf_empty) {
			ret |= 0x04;
		}
		if(scc_ptr->br_is_zero) {
			ret |= 0x02;
		}
		//printf("Read scc[%d] stat: %f : %02x\n", port, dcycs, ret);
		break;
	case 1:
	case 5:
		/* HACK: residue codes not right */
		ret = 0x07;	/* all sent */
		break;
	case 2:
	case 6:
		if(port == 0) {
			ret = scc_ptr->reg[2];
		} else {

			halt_printf("Read of RR2B...stopping\n");
			ret = 0;
#if 0
			ret = g_scc.scc_stat[0].reg[2];
			wr9 = g_scc.scc_stat[0].reg[9];
			for(i = 0; i < 8; i++) {
				if(ZZZ){};
			}
			if(wr9 & 0x10) {
				/* wr9 status high */
				
			}
#endif
		}
		break;
	case 3:
	case 7:
		if(port == 0) {
			ret = (g_moremem.g_irq_pending & 0x3f);
		} else {
			ret = 0;
		}
		break;
	case 8:
		ret = scc_read_data(port, dcycs);
		break;
	case 9:
	case 13:
		ret = scc_ptr->reg[13];
		break;
	case 10:
	case 14:
		ret = 0;
		break;
	case 11:
	case 15:
		ret = scc_ptr->reg[15];
		break;
	case 12:
		ret = scc_ptr->reg[12];
		break;
	default:
		halt_printf("Tried reading c03%x with regnum: %d!\n", 8+port,
			regnum);
		ret = 0;
	}

	scc_ptr->reg_ptr = 0;
	scc_printf("Read c03%x, rr%d, ret: %02x\n", 8+port, regnum, ret);
	if(regnum != 0 && regnum != 3) {
		scc_log(SCC_REGNUM(0,port,regnum), ret, dcycs);
	}

	return ret;
}

void
scc_write_reg(int port, word32 val, double dcycs)
{
	Scc	*scc_ptr;
	word32	old_val;
	word32	changed_bits;
	word32	irq_mask;
	int	regnum;
	int	mode;
	int	tmp1;

	scc_ptr = &(g_scc.scc_stat[port]);
	regnum = scc_ptr->reg_ptr & 0xf;
	mode = scc_ptr->mode;

	if(mode == 0) {
		if((val & 0xf0) == 0) {
			/* Set reg_ptr */
			scc_ptr->reg_ptr = val & 0xf;
			regnum = 0;
			scc_ptr->mode = 1;
		} else {
			scc_log(SCC_REGNUM(1,port,0), val, dcycs);
		}
	} else {
		scc_ptr->reg_ptr = 0;
		scc_ptr->mode = 0;
	}

	if(regnum != 0) {
		scc_log(SCC_REGNUM(1,port,regnum), val, dcycs);
	}

	changed_bits = (scc_ptr->reg[regnum] ^ val) & 0xff;

	/* Set reg reg */
	switch(regnum) {
	case 0: /* wr0 */
		tmp1 = (val >> 3) & 0x7;
		switch(tmp1) {
		case 0x0:
		case 0x1:
			break;
		case 0x2:	/* reset ext/status ints */
			/* should clear other ext ints */
			scc_clr_zerocnt_int(port);
			break;
		case 0x5:	/* reset tx int pending */
			scc_clr_tx_int(port);
			break;
		case 0x6:	/* reset rr1 bits */
			break;
		case 0x7:	/* reset highest pri int pending */
			irq_mask = g_moremem.g_irq_pending;
			if(port == 0) {
				/* Move SCC0 ints into SCC1 positions */
				irq_mask = irq_mask >> 3;
			}
			if(irq_mask & IRQ_PENDING_SCC1_RX) {
				scc_clr_rx_int(port);
			} else if(irq_mask & IRQ_PENDING_SCC1_TX) {
				scc_clr_tx_int(port);
			} else if(irq_mask & IRQ_PENDING_SCC1_ZEROCNT) {
				scc_clr_zerocnt_int(port);
			}
			break;
		case 0x4:	/* enable int on next rx char */
		default:
			halt_printf("Wr c03%x to wr0 of %02x, bad cmd cd:%x!\n",
				8+port, val, tmp1);
		}
		tmp1 = (val >> 6) & 0x3;
		switch(tmp1) {
		case 0x0:	/* null code */
			break;
		case 0x1:	/* reset rx crc */
		case 0x2:	/* reset tx crc */
			printf("Wr c03%x to wr0 of %02x!\n", 8+port, val);
			break;
		case 0x3:	/* reset tx underrun/eom latch */
			/* if no extern status pending, or being reset now */
			/*  and tx disabled, ext int with tx underrun */
			/* ah, just do nothing */
			break;
		}
		return;
	case 1: /* wr1 */
		/* proterm sets this == 0x10, which is int on all rx */
		scc_ptr->reg[regnum] = val;
		return;
	case 2: /* wr2 */
		/* All values do nothing, let 'em all through! */
		scc_ptr->reg[regnum] = val;
		return;
	case 3: /* wr3 */
		if((val & 0x1e) != 0x0) {
			halt_printf("Wr c03%x to wr3 of %02x!\n", 8+port, val);
		}
		scc_ptr->reg[regnum] = val;
		return;
	case 4: /* wr4 */
		if((val & 0x30) != 0x00 || (val & 0x0c) == 0) {
			halt_printf("Wr c03%x to wr4 of %02x!\n", 8+port, val);
		}
		scc_ptr->reg[regnum] = val;
		if(changed_bits) {
			scc_regen_clocks(port);
		}
		return;
	case 5: /* wr5 */
		if((val & 0x15) != 0x0) {
			halt_printf("Wr c03%x to wr5 of %02x!\n", 8+port, val);
		}
		scc_ptr->reg[regnum] = val;
		if(changed_bits & 0x60) {
			scc_regen_clocks(port);
		}
		return;
	case 6: /* wr6 */
		if(val != 0) {
			halt_printf("Wr c03%x to wr6 of %02x!\n", 8+port, val);
		}
		scc_ptr->reg[regnum] = val;
		return;
	case 7: /* wr7 */
		if(val != 0) {
			halt_printf("Wr c03%x to wr7 of %02x!\n", 8+port, val);
		}
		scc_ptr->reg[regnum] = val;
		return;
	case 8: /* wr8 */
		scc_write_data(port, val, dcycs);
		return;
	case 9: /* wr9 */
		if((val & 0xc0)) {
			if(val & 0x80) {
				scc_reset_port(0);
			}
			if(val & 0x40) {
				scc_reset_port(1);
			}
			if((val & 0xc0) == 0xc0) {
				scc_hard_reset_port(0);
				scc_hard_reset_port(1);
			}
		}
		if((val & 0x35) != 0x00) {
			printf("Write c03%x to wr9 of %02x!\n", 8+port, val);
			halt_printf("val & 0x35: %02x\n", (val & 0x35));
		}
		old_val = g_scc.scc_stat[0].reg[9];
		g_scc.scc_stat[0].reg[regnum] = val;
		scc_evaluate_ints(0);
		scc_evaluate_ints(1);
		return;
	case 10: /* wr10 */
		if((val & 0xff) != 0x00) {
			printf("Wr c03%x to wr10 of %02x!\n", 8+port, val);
		}
		scc_ptr->reg[regnum] = val;
		return;
	case 11: /* wr11 */
		scc_ptr->reg[regnum] = val;
		if(changed_bits) {
			scc_regen_clocks(port);
		}
		return;
	case 12: /* wr12 */
		scc_ptr->reg[regnum] = val;
		if(changed_bits) {
			scc_regen_clocks(port);
		}
		return;
	case 13: /* wr13 */
		scc_ptr->reg[regnum] = val;
		if(changed_bits) {
			scc_regen_clocks(port);
		}
		return;
	case 14: /* wr14 */
		old_val = scc_ptr->reg[regnum];
		val = val + (old_val & (~0xff));
		switch((val >> 5) & 0x7) {
		case 0x0:
		case 0x1:
		case 0x2:
		case 0x3:
			break;
		
		case 0x4:	/* DPLL source is BR gen */
			val |= SCC_R14_DPLL_SOURCE_BRG;
			break;
		default:
			halt_printf("Wr c03%x to wr14 of %02x, bad dpll cd!\n",
				8+port, val);
		}
		if((val & 0x0c) != 0x0) {
			halt_printf("Wr c03%x to wr14 of %02x!\n", 8+port, val);
		}
		scc_ptr->reg[regnum] = val;
		if(changed_bits) {
			scc_regen_clocks(port);
		}
		scc_maybe_br_event(port, dcycs);
		return;
	case 15: /* wr15 */
		/* ignore all accesses since IIgs self test messes with it */
		if((val & 0xff) != 0x0) {
			scc_printf("Write c03%x to wr15 of %02x!\n", 8+port,
				val);
		}
		if((g_scc.scc_stat[0].reg[9] & 0x8) && (val != 0)) {
			printf("Write wr15:%02x and master int en = 1!\n",val);
			/* set_halt(1); */
		}
		scc_ptr->reg[regnum] = val;
		scc_maybe_br_event(port, dcycs);
		scc_evaluate_ints(port);
		return;
	default:
		halt_printf("Wr c03%x to wr%d of %02x!\n", 8+port, regnum, val);
		return;
	}
}

void
scc_maybe_br_event(int port, double dcycs)
{
	Scc	*scc_ptr;
	double	br_dcycs;

	scc_ptr = &(g_scc.scc_stat[port]);

	if(((scc_ptr->reg[14] & 0x01) == 0) || scc_ptr->br_event_pending) {
		return;
	}
	/* also, if ext ints not enabled, don't do baud rate ints */
	if((scc_ptr->reg[15] & 0x02) == 0) {
		return;
	}

	br_dcycs = scc_ptr->br_dcycs;
	if(br_dcycs < 1.0) {
		halt_printf("br_dcycs: %f!\n", br_dcycs);
	}

	scc_ptr->br_event_pending = 1;
	add_event_scc(dcycs + br_dcycs, SCC_MAKE_EVENT(port, SCC_BR_EVENT));
}

void
scc_evaluate_ints(int port)
{
	Scc	*scc_ptr;
	word32	irq_add_mask, irq_remove_mask;
	int	mie;

	scc_ptr = &(g_scc.scc_stat[port]);
	mie = g_scc.scc_stat[0].reg[9] & 0x8;			/* Master int en */

	if(!mie) {
		/* There can be no interrupts if MIE=0 */
		remove_irq(IRQ_PENDING_SCC1_RX | IRQ_PENDING_SCC1_TX |
						IRQ_PENDING_SCC1_ZEROCNT |
			IRQ_PENDING_SCC0_RX | IRQ_PENDING_SCC0_TX |
						IRQ_PENDING_SCC0_ZEROCNT);
		return;
	}

	irq_add_mask = 0;
	irq_remove_mask = 0;
	if(scc_ptr->wantint_rx) {
		irq_add_mask |= IRQ_PENDING_SCC1_RX;
	} else {
		irq_remove_mask |= IRQ_PENDING_SCC1_RX;
	}
	if(scc_ptr->wantint_tx) {
		irq_add_mask |= IRQ_PENDING_SCC1_TX;
	} else {
		irq_remove_mask |= IRQ_PENDING_SCC1_TX;
	}
	if(scc_ptr->wantint_zerocnt) {
		irq_add_mask |= IRQ_PENDING_SCC1_ZEROCNT;
	} else {
		irq_remove_mask |= IRQ_PENDING_SCC1_ZEROCNT;
	}
	if(port == 0) {
		/* Port 1 is in bits 0-2 and port 0 is in bits 3-5 */
		irq_add_mask = irq_add_mask << 3;
		irq_remove_mask = irq_remove_mask << 3;
	}
	if(irq_add_mask) {
		add_irq(irq_add_mask);
	}
	if(irq_remove_mask) {
		remove_irq(irq_remove_mask);
	}
}


void
scc_maybe_rx_event(int port, double dcycs)
{
	Scc	*scc_ptr;
	double	rx_dcycs;
	int	in_rdptr, in_wrptr;
	int	depth;

	scc_ptr = &(g_scc.scc_stat[port]);

	if(scc_ptr->rx_event_pending) {
	
		/* one pending already, wait for the event to arrive */
		return;
	}

	in_rdptr = scc_ptr->in_rdptr;
	in_wrptr = scc_ptr->in_wrptr;
	depth = scc_ptr->rx_queue_depth;
	if((in_rdptr == in_wrptr) || (depth >= 3)) {
		/* no more chars or no more space, just get out */
		return;
	}

	if(depth < 0) {
		depth = 0;
	}

	/* pull char from in_rdptr into queue */
	scc_ptr->rx_queue[depth] = scc_ptr->in_buf[in_rdptr];
	scc_ptr->in_rdptr = (in_rdptr + 1) & (SCC_INBUF_SIZE - 1);
	scc_ptr->rx_queue_depth = depth + 1;
	scc_maybe_rx_int(port, dcycs);
	rx_dcycs = scc_ptr->rx_dcycs;
	scc_ptr->rx_event_pending = 1;
	add_event_scc(dcycs + rx_dcycs, SCC_MAKE_EVENT(port, SCC_RX_EVENT));
}

void
scc_maybe_rx_int(int port, double dcycs)
{
	Scc	*scc_ptr;
	int	depth;
	int	rx_int_mode;

	scc_ptr = &(g_scc.scc_stat[port]);

	depth = scc_ptr->rx_queue_depth;
	if(depth <= 0) {
		/* no more chars, just get out */
		scc_clr_rx_int(port);
		return;
	}
	rx_int_mode = (scc_ptr->reg[1] >> 3) & 0x3;
	if(rx_int_mode == 1 || rx_int_mode == 2) {
		scc_ptr->wantint_rx = 1;
	}
	scc_evaluate_ints(port);
}

void
scc_clr_rx_int(int port)
{
	g_scc.scc_stat[port].wantint_rx = 0;
	scc_evaluate_ints(port);
}

void
scc_handle_tx_event(int port, double dcycs)
{
	Scc	*scc_ptr;
	int	tx_int_mode;

	scc_ptr = &(g_scc.scc_stat[port]);

	/* nothing pending, see if ints on */
	tx_int_mode = (scc_ptr->reg[1] & 0x2);
	if(tx_int_mode) {
		scc_ptr->wantint_tx = 1;
	}
	scc_evaluate_ints(port);
}

void
scc_maybe_tx_event(int port, double dcycs)
{
	Scc	*scc_ptr;
	double	tx_dcycs;

	scc_ptr = &(g_scc.scc_stat[port]);

	if(scc_ptr->tx_event_pending) {
		/* one pending already, tx_buf is full */
		scc_ptr->tx_buf_empty = 0;
	} else {
		/* nothing pending, see ints on */
		scc_evaluate_ints(port);
		tx_dcycs = scc_ptr->tx_dcycs;
		scc_ptr->tx_event_pending = 1;
		add_event_scc(dcycs + tx_dcycs,
				SCC_MAKE_EVENT(port, SCC_TX_EVENT));
	}
}

void
scc_clr_tx_int(int port)
{
	g_scc.scc_stat[port].wantint_tx = 0;
	scc_evaluate_ints(port);
}

void
scc_set_zerocnt_int(int port)
{
	Scc	*scc_ptr;

	scc_ptr = &(g_scc.scc_stat[port]);

	if(scc_ptr->reg[15] & 0x2) {
		scc_ptr->wantint_zerocnt = 1;
	}
	scc_evaluate_ints(port);
}

void
scc_clr_zerocnt_int(int port)
{
	g_scc.scc_stat[port].wantint_zerocnt = 0;
	scc_evaluate_ints(port);
}

void
scc_add_to_readbuf(int port, word32 val, double dcycs)
{
	Scc	*scc_ptr;
	int	in_wrptr;
	int	in_wrptr_next;
	int	in_rdptr;

	scc_ptr = &(g_scc.scc_stat[port]);

	in_wrptr = scc_ptr->in_wrptr;
	in_rdptr = scc_ptr->in_rdptr;
	in_wrptr_next = (in_wrptr + 1) & (SCC_INBUF_SIZE - 1);
	if(in_wrptr_next != in_rdptr) {
		scc_ptr->in_buf[in_wrptr] = val;
		scc_ptr->in_wrptr = in_wrptr_next;
		scc_printf("scc in port[%d] add char 0x%02x, %d,%d != %d\n",
				scc_ptr->port, val,
				in_wrptr, in_wrptr_next, in_rdptr);
		printf("sending to modem :%c\n",val);
		g_scc.g_scc_overflow = 0;
	} else {
		if(g_scc.g_scc_overflow == 0) {
			g_sim65816.g_code_yellow++;
			printf("scc inbuf overflow port %d\n", port);
		}
		g_scc.g_scc_overflow = 1;
	}

	scc_maybe_rx_event(port, dcycs);
}

void
scc_add_to_readbufv(int port, double dcycs, const char *fmt, ...)
{
	va_list	ap;
	char	*bufptr;
	int	ret, len, c;
	int	i;

	va_start(ap, fmt);
	bufptr = (char*)malloc(4096);	// OG cast added
	bufptr[0] = 0;
	ret = vsnprintf(bufptr, 4090, fmt, ap);
	len = strlen(bufptr);
	for(i = 0; i < len; i++) {
		c = bufptr[i];
		if(c == 0x0a) {
			scc_add_to_readbuf(port, 0x0d, dcycs);
		}
		scc_add_to_readbuf(port, c, dcycs);
	}
	va_end(ap);
}

void
scc_transmit(int port, word32 val, double dcycs)
{
	Scc	*scc_ptr;
	int	out_wrptr;
	int	out_rdptr;

	scc_ptr = &(g_scc.scc_stat[port]);

	/* See if port initialized, if not, do so now */
	if(scc_ptr->state == 0) {
		scc_port_init(port);
	}
	if(scc_ptr->state < 0) {
		/* No working serial port, just toss it and go */
		return;
	}

	if(!scc_ptr->tx_buf_empty) {
		/* toss character! */
		printf("Tossing char\n");
		return;
	}

	out_wrptr = scc_ptr->out_wrptr;
	out_rdptr = scc_ptr->out_rdptr;
	if(scc_ptr->tx_dcycs < 1.0) {
		if(out_wrptr != out_rdptr) {
			/* do just one char, then get out */
			printf("tx_dcycs < 1\n");
			return;
		}
	}
	if(g_sim65816.g_serial_out_masking) {
		val = val & 0x7f;
	}

	scc_add_to_writebuf(port, val, dcycs);
}

void
scc_add_to_writebuf(int port, word32 val, double dcycs)
{
	Scc	*scc_ptr;
	int	out_wrptr;
	int	out_wrptr_next;
	int	out_rdptr;

	scc_ptr = &(g_scc.scc_stat[port]);

	/* See if port initialized, if not, do so now */
	if(scc_ptr->state == 0) {
		scc_port_init(port);
	}
	if(scc_ptr->state < 0) {
		/* No working serial port, just toss it and go */
		return;
	}

	out_wrptr = scc_ptr->out_wrptr;
	out_rdptr = scc_ptr->out_rdptr;

	out_wrptr_next = (out_wrptr + 1) & (SCC_OUTBUF_SIZE - 1);
	if(out_wrptr_next != out_rdptr) {
		scc_ptr->out_buf[out_wrptr] = val;
		scc_ptr->out_wrptr = out_wrptr_next;
		scc_printf("scc wrbuf port %d had char 0x%02x added\n",
			scc_ptr->port, val);
		g_scc.g_scc_overflow = 0;
	} else {
		if(g_scc.g_scc_overflow == 0) {
			g_sim65816.g_code_yellow++;
			// OG Too many warning with DiversiTune
			scc_printf("scc outbuf overflow port %d\n", port);
		}
		g_scc.g_scc_overflow = 1;
	}
}

word32
scc_read_data(int port, double dcycs)
{
	Scc	*scc_ptr;
	word32	ret;
	int	depth;
	int	i;

	scc_ptr = &(g_scc.scc_stat[port]);

	scc_try_fill_readbuf(port, dcycs);

	depth = scc_ptr->rx_queue_depth;

	ret = 0;
	if(depth != 0) {
		ret = scc_ptr->rx_queue[0];
		for(i = 1; i < depth; i++) {
			scc_ptr->rx_queue[i-1] = scc_ptr->rx_queue[i];
		}
		scc_ptr->rx_queue_depth = depth - 1;
		scc_maybe_rx_event(port, dcycs);
		scc_maybe_rx_int(port, dcycs);
	}

	scc_printf("SCC read %04x: ret %02x, depth:%d\n", 0xc03b-port, ret,
			depth);

	scc_log(SCC_REGNUM(0,port,8), ret, dcycs);

//	printf("scc send %d\n",ret);
	return ret;
}


void
scc_write_data(int port, word32 val, double dcycs)
{
	Scc	*scc_ptr;

	scc_printf("SCC write %04x: %02x\n", 0xc03b-port, val);
	scc_log(SCC_REGNUM(1,port,8), val, dcycs);

	scc_ptr = &(g_scc.scc_stat[port]);
	if(scc_ptr->reg[14] & 0x10) {
		/* local loopback! */
		scc_add_to_readbuf(port, val, dcycs);
	} else {
		scc_transmit(port, val, dcycs);
	}
	scc_try_to_empty_writebuf(port, dcycs);

	scc_maybe_tx_event(port, dcycs);
}

