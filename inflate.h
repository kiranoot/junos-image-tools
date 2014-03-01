/*-
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <phk@FreeBSD.org> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Poul-Henning Kamp
 * ----------------------------------------------------------------------------
 *
 * $FreeBSD$
 *
 */
#ifndef	_SYS_INFLATE_H_
#define	_SYS_INFLATE_H_

#define GZ_EOF -1

#define GZ_WSIZE 0x8000

/*
 * Global variables used by inflate and friends.
 * This structure is used in order to make inflate() reentrant.
 */

typedef unsigned char u_char;
typedef unsigned long u_long;
typedef unsigned short u_short;
struct inflate {
	/* Public part */

	/* This pointer is passed along to the two functions below */
	void           *gz_private;

	/* Fetch next character to be uncompressed */
	int             (*gz_input)(void *);

	/* Dispose of uncompressed characters */
	int             (*gz_output)(void *, unsigned char *, unsigned long);

	/* Private part */
	unsigned long          gz_bb;	/* bit buffer */
	unsigned int       gz_bk;	/* bits in bit buffer */
	unsigned int       gz_hufts;	/* track memory usage */
	struct huft    *gz_fixed_tl;	/* must init to NULL !! */
	struct huft    *gz_fixed_td;
	int             gz_fixed_bl;
	int             gz_fixed_bd;
	unsigned char         *gz_slide;
	unsigned int       gz_wp;
};

//int inflate(struct inflate *);
int xinflate(struct inflate *);

#endif	/* ! _SYS_INFLATE_H_ */