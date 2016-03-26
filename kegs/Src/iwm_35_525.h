/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#ifdef INCLUDE_IWM_RCSID_C
const char rcsdif_iwm_35_525_h[] = "@(#)$KmKId: iwm_35_525.h,v 1.14 2004-12-01 19:45:02-05 kentd Exp $";
#endif

int
IWM_READ_ROUT (Disk *dsk, int fast_disk_emul, double dcycs)
{
	Trk	*trk;
	double	dcycs_last_read;
	int	pos;
	int	pos2;
	int	size;
	int	next_size;
	int	qtr_track;
	int	skip_nibs;
	int	track_len;
	byte	ret;
	int	shift;
	int	skip;
	int	cycs_this_nib;
	int	cycs_passed;
	double	dcycs_this_nib;
	double	dcycs_next_nib;
	double	dcycs_passed;
	double	track_dcycs;
	double	dtmp;

	g_iwm.iwm.previous_write_bits = 0;

	qtr_track = dsk->cur_qtr_track;

#if IWM_DISK_525
	qtr_track = qtr_track & -4;	/* round to nearest whole trk! */
#endif

	trk = 0;
	track_len = 0;
	if(dsk->trks) {
		trk = &(dsk->trks[qtr_track]);
		track_len = trk->track_len;
	}

	dcycs_last_read = dsk->dcycs_last_read;
	dcycs_passed = dcycs - dcycs_last_read;

	cycs_passed = (int)dcycs_passed;

	if(track_len == 0) {
		ret = (cycs_passed & 0x7f) + 0x80;
		iwm_printf("Reading c0ec, track_len 0, returning %02x\n", ret);
		return ret;
	}

	pos = dsk->nib_pos;
	if(pos >= track_len) {
		/* Arm may have moved from inner 3.5 track to outer one, */
		/*  and so must make pos fit on smaller sized track */
		pos = 0;
	}

    if (pos<0)
    {
        printf("invalid pos");
        pos=0;
    }
	size = trk->nib_area[pos];

	while(size == 0) {
		pos += 2;
		if(pos >= track_len) {
			pos = 0;
		}
		size = trk->nib_area[pos];
	}

	cycs_this_nib = size * (2 * IWM_CYC_MULT);
	dcycs_this_nib = (double)cycs_this_nib;

	if(fast_disk_emul) {
		cycs_passed = cycs_this_nib;
		dcycs_passed = dcycs_this_nib;

		/* pull a trick to make disk motor-on test pass ($bd34 RWTS) */
		/*  if this would be a sync byte, and we didn't just do this */
		/*  then don't return whole byte */
		/* BUT, don't do this if g_fast_disk_unnib, since it will */
		/*  cause the dsk->unix routines to break */
		if(size > 8 && !g_iwm.g_fast_disk_unnib && (g_iwm.g_iwm_fake_fast == 0)) {
			cycs_passed = cycs_passed >> 1;
			dcycs_passed = dcycs_passed * 0.5;
			g_iwm.g_iwm_fake_fast = 1;
		} else {
			g_iwm.g_iwm_fake_fast = 0;
		}
	}

	skip = 0;
	if(cycs_passed >= (cycs_this_nib + 11)) {
		/* skip some bits? */
		skip = 1;
		if(g_iwm.iwm.iwm_mode & 1) {
			/* latch mode */

			pos2 = pos + 2;
			if(pos2 >= track_len) {
				pos2 = 0;
			}
			next_size = trk->nib_area[pos2];
			while(next_size == 0) {
				pos2 += 2;
				if(pos2 >= track_len) {
					pos2 = 0;
				}
				next_size = trk->nib_area[pos2];
			}

			dcycs_next_nib = next_size * (2 * IWM_CYC_MULT);

			if(dcycs_passed < (dcycs_this_nib + dcycs_next_nib)) {
				skip = 0;
			}
		}
	}

	if(skip) {
		iwm_printf("skip since cycs_passed: %f, cycs_this_nib: %f\n",
			dcycs_passed, dcycs_this_nib);

		track_dcycs = IWM_CYC_MULT * (track_len * 8);

		if(dcycs_passed >= track_dcycs) {
			dtmp = (int)(dcycs_passed / track_dcycs);
			dcycs_passed = dcycs_passed -
					(dtmp * track_dcycs);
			dcycs_last_read += (dtmp * track_dcycs);
		}

		if(dcycs_passed >= track_dcycs || dcycs_passed < 0.0) {
			dcycs_passed = 0.0;
		}

		cycs_passed = (int)dcycs_passed;

		skip_nibs = ((word32)cycs_passed) >> (4 + IWM_DISK_525);

		pos += skip_nibs * 2;
		while(pos >= track_len) {
			pos -= track_len;
		}

		dcycs_last_read += (skip_nibs * 16 * IWM_CYC_MULT);

		dsk->dcycs_last_read = dcycs_last_read;

		size = trk->nib_area[pos];
		dcycs_passed = dcycs - dcycs_last_read;
		if(dcycs_passed < 0.0 || dcycs_passed > 64.0) {
			halt_printf("skip, last_read:%f, dcycs:%f, dcyc_p:%f\n",
				dcycs_last_read, dcycs, dcycs_passed);
		}

		while(size == 0) {
			pos += 2;
			if(pos >= track_len) {
				pos = 0;
			}
			size = trk->nib_area[pos];
		}

		cycs_this_nib = size * (2 * IWM_CYC_MULT);
		cycs_passed = (int)dcycs_passed;
		dcycs_this_nib = cycs_this_nib;
	}

	if(cycs_passed < cycs_this_nib) {
		/* partial */
#if 0
		iwm_printf("Disk partial, %f < %f, size: %d\n",
					dcycs_passed, dcycs_this_nib, size);
#endif
		shift = (cycs_passed) >> (1 + IWM_DISK_525);
		ret = trk->nib_area[pos+1] >> (size - shift);
		if(ret & 0x80) {
			halt_printf("Bad shift in partial read: %02x, but "
				"c_pass:%f, this_nib:%f, shift: %d, size: %d\n",
				ret, dcycs_passed, dcycs_this_nib, shift, size);
		}
	} else {
		/* whole thing */
		ret = trk->nib_area[pos+1];
		pos += 2;
		if(pos >= track_len) {
			pos = 0;
		}
		if(!fast_disk_emul) {
			dsk->dcycs_last_read = dcycs_last_read + dcycs_this_nib;
		}
	}

	dsk->nib_pos = pos;
	if(pos < 0 || pos > track_len) {
		halt_printf("I just set nib_pos: %d!\n", pos);
	}

#if 0
	iwm_printf("Disk read, returning: %02x\n", ret);
#endif

	return ret;
}


void
IWM_WRITE_ROUT (Disk *dsk, word32 val, int fast_disk_emul, double dcycs)
{
	double	dcycs_last_read;
	int	bits_read; // OG must be signed to detect errors
	word32	mask;
	word32	prev_val;
	double	dcycs_this_nib;
	double	dcycs_passed;
	int	sdiff;
	int	prev_bits;

	if(dsk->fd < 0 || dsk->trks == 0) {
		halt_printf("Tried to write to type: %d, drive: %d, fd: %d!\n",
			IWM_DISK_525, dsk->drive, dsk->fd, dsk->trks);
		return;
	}

	dcycs_last_read = dsk->dcycs_last_read;

	dcycs_passed = dcycs - dcycs_last_read;

	prev_val = g_iwm.iwm.previous_write_val;
	prev_bits = g_iwm.iwm.previous_write_bits;
	mask = 0x100;
	iwm_printf("Iwm write: prev: %x,%d, new:%02x\n", prev_val, prev_bits,
							val);

	if(IWM_DISK_525) {
		/* Activate slow write emulation mode */
		g_iwm.g_dcycs_end_emul_wr = dcycs + 64.0;
		if(!g_iwm.g_slow_525_emul_wr) {
			set_halt(HALT_EVENT);
			g_iwm.g_slow_525_emul_wr = 1;
		}
	} else {
		/* disable slow writes on 3.5" drives */
		if(g_iwm.g_slow_525_emul_wr) {
			set_halt(HALT_EVENT);
			printf("HACK3: g_slow_525_emul_wr set to 0\n");
			g_iwm.g_slow_525_emul_wr = 0;
		}
	}

	if(g_iwm.iwm.iwm_mode & 2) {
		/* async mode = 3.5" default */
		bits_read = 8;
	} else {
		/* sync mode, 5.25" drives */
		bits_read = ((int)dcycs_passed) >> (1 + IWM_DISK_525);
		if(bits_read < 8) {
			bits_read = 8;
		}
	}

	if(fast_disk_emul) {
		bits_read = 8;
	}
	
	dcycs_this_nib = bits_read * (2 * IWM_CYC_MULT);

	if(fast_disk_emul) {
		dcycs_passed = dcycs_this_nib;
	}

	if(prev_bits > 0) {
		while((prev_val & 0x80) == 0 && bits_read > 0) {
			/* previous byte needs some bits */
			mask = mask >> 1;
			prev_val = (prev_val << 1) + ((val & mask) !=0);
			prev_bits++;
			bits_read--;
		}
	}

	val = val & (mask - 1);
	if(prev_bits) {
		/* force out prev_val if it had many bits before */
		/*  this prevents writes of 0 from messing us up */
		if(((prev_val & 0x80) == 0) && (prev_bits < 10)) {
			/* special case: we still don't have enough to go */
			iwm_printf("iwm_write: zip2: %02x, %d, left:%02x,%d\n",
					prev_val, prev_bits, val,bits_read);
			val = prev_val;
			bits_read = prev_bits;
		} else {
			iwm_printf("iwm_write: prev: %02x, %d, left:%02x, %d\n",
				prev_val, prev_bits, val, bits_read);
			disk_nib_out(dsk, prev_val, prev_bits);
		}
	} else if(val & 0x80) {
		iwm_printf("iwm_write: new: %02x, %d\n", val,bits_read);
		disk_nib_out(dsk, val, bits_read);
		bits_read = 0;
	} else {
		iwm_printf("iwm_write: zip: %02x, %d, left:%02x,%d\n",
			prev_val, prev_bits, val,bits_read);
	}

	g_iwm.iwm.previous_write_val = val;
	g_iwm.iwm.previous_write_bits = bits_read;
	if(bits_read < 0) {
		halt_printf("iwm, bits_rd:%d, val:%08x, prev:%02x, prevb:%d\n",
			bits_read, val, prev_val, prev_bits);
	}

	sdiff = (int)( dcycs - dcycs_last_read );
	if(sdiff < (dcycs_this_nib) || (sdiff > (2*dcycs_this_nib)) ) {
		dsk->dcycs_last_read = dcycs;
	} else {
		dsk->dcycs_last_read = dcycs_last_read + dcycs_this_nib;
	}
}
