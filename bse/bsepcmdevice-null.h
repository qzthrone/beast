/* BSE - Bedevilled Sound Engine
 * Copyright (C) 2004 Tim Janik
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
 */
#ifndef __BSE_PCM_DEVICE_NULL_H__
#define __BSE_PCM_DEVICE_NULL_H__

#include        <bse/bsepcmdevice.h>

G_BEGIN_DECLS

/* --- object type macros --- */
#define BSE_TYPE_PCM_DEVICE_NULL              (BSE_TYPE_ID (BsePcmDeviceNull))
#define BSE_PCM_DEVICE_NULL(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), BSE_TYPE_PCM_DEVICE_NULL, BsePcmDeviceNull))
#define BSE_PCM_DEVICE_NULL_CLASS(class)      (G_TYPE_CHECK_CLASS_CAST ((class), BSE_TYPE_PCM_DEVICE_NULL, BsePcmDeviceNullClass))
#define BSE_IS_PCM_DEVICE_NULL(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), BSE_TYPE_PCM_DEVICE_NULL))
#define BSE_IS_PCM_DEVICE_NULL_CLASS(class)   (G_TYPE_CHECK_CLASS_TYPE ((class), BSE_TYPE_PCM_DEVICE_NULL))
#define BSE_PCM_DEVICE_NULL_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), BSE_TYPE_PCM_DEVICE_NULL, BsePcmDeviceNullClass))


/* --- BsePcmDeviceNull object --- */
typedef struct _BsePcmDeviceNull      BsePcmDeviceNull;
typedef struct _BsePcmDeviceNullClass BsePcmDeviceNullClass;
struct _BsePcmDeviceNull
{
  BsePcmDevice parent_object;
};
struct _BsePcmDeviceNullClass
{
  BsePcmDeviceClass parent_class;
};

G_END_DECLS

#endif /* __BSE_PCM_DEVICE_NULL_H__ */
