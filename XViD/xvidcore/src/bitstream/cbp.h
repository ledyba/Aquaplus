/*****************************************************************************
 *
 *  XVID MPEG-4 VIDEO CODEC
 *  - CBP related header  -
 *
 *  Copyright(C) 2002-2003 Edouard Gomez <ed.gomez@free.fr>
 *               2003      Christoph Lampert <gruel@web.de>
 *
 *  This program is free software ; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation ; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY ; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program ; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * $Id$
 *
 ****************************************************************************/

#ifndef _ENCODER_CBP_H_
#define _ENCODER_CBP_H_

#include "../portab.h"

typedef uint32_t(cbpFunc) (const int16_t * codes);

typedef cbpFunc *cbpFuncPtr;

extern cbpFuncPtr calc_cbp;

extern cbpFunc calc_cbp_c;
extern cbpFunc calc_cbp_plain;

#ifdef ARCH_IS_IA32
extern cbpFunc calc_cbp_mmx;
extern cbpFunc calc_cbp_3dne;
extern cbpFunc calc_cbp_sse2;
#endif

#ifdef ARCH_IS_PPC
extern cbpFunc calc_cbp_ppc;
#endif

#ifdef ARCH_IS_ALTIVEC
extern cbpFunc calc_cbp_altivec;
#endif

#endif /* _ENCODER_CBP_H_ */
