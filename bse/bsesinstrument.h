/* BSE - Bedevilled Sound Engine
 * Copyright (C) 1999 Tim Janik
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 * bsesinstrument.h: BSE Synthesis network Instrument
 */
#ifndef __BSE_SINSTRUMENT_H__
#define __BSE_SINSTRUMENT_H__

#include <bse/bsesource.h>
#include <bse/bseinstrument.h>
#include <bse/bsevoice.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */





/* --- object type macros --- */
#define BSE_TYPE_SINSTRUMENT              (BSE_TYPE_ID (BseSInstrument))
#define BSE_SINSTRUMENT(object)           (BSE_CHECK_STRUCT_CAST ((object), BSE_TYPE_SINSTRUMENT, BseSInstrument))
#define BSE_SINSTRUMENT_CLASS(class)      (BSE_CHECK_CLASS_CAST ((class), BSE_TYPE_SINSTRUMENT, BseSInstrumentClass))
#define BSE_IS_SINSTRUMENT(object)        (BSE_CHECK_STRUCT_TYPE ((object), BSE_TYPE_SINSTRUMENT))
#define BSE_IS_SINSTRUMENT_CLASS(class)   (BSE_CHECK_CLASS_TYPE ((class), BSE_TYPE_SINSTRUMENT))
#define BSE_SINSTRUMENT_GET_CLASS(object) ((BseSInstrumentClass*) (((BseObject*) (object))->bse_struct.bse_class))


/* --- BseSInstrument source --- */
struct _BseSInstrument
{
  BseSource       parent_object;

  BseInstrument  *instrument;	/* maintained by BseInstrument */
  BseVoice       *voice;	/* maintained by BseVoice */
};
struct _BseSInstrumentClass
{
  BseSourceClass parent_class;
};


/* --- channels --- */
enum
{
  BSE_SINSTRUMENT_ICHANNEL_NONE,
  BSE_SINSTRUMENT_ICHANNEL_MULTI
};
enum
{
  BSE_SINSTRUMENT_OCHANNEL_NONE,
  BSE_SINSTRUMENT_OCHANNEL_STEREO,
  BSE_SINSTRUMENT_OCHANNEL_FREQ
};


/*< private >*/
void	  bse_sinstrument_poke_foreigns (BseSInstrument *sinstrument,
					 BseInstrument  *instrument,
					 BseVoice       *voice);





#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BSE_SINSTRUMENT_H__ */
