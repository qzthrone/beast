/* BSW-SCM - Bedevilled Sound Engine Scheme Wrapper
 * Copyright (C) 2002 Tim Janik
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
#ifndef __BSW_SCM_HANDLE_H__
#define __BSW_SCM_HANDLE_H__

#include        <bsw/bsw.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


typedef struct _BswSCMHandle BswSCMHandle;
typedef struct _BswSCMWire   BswSCMWire;


BswSCMHandle*	bsw_scm_handle_alloc	(void);
void		bsw_scm_handle_set_proc	(BswSCMHandle	*handle,
					 const gchar	*proc_name);
void		bsw_scm_handle_clean	(BswSCMHandle	*handle);
void		bsw_scm_handle_destroy	(BswSCMHandle	*handle);
void		bsw_scm_handle_putunset	(BswSCMHandle	*handle,
					 GValue		*value);
BswErrorType	bsw_scm_handle_eval	(BswSCMHandle	*handle);
GValue*		bsw_scm_handle_peekret	(BswSCMHandle	*handle,
					 GType		 type);
void		bsw_scm_handle_set_wire	(BswSCMWire	*wire);


BswSCMWire*	bsw_scm_wire_from_pipe	(const gchar	*ident,
					 gint		 remote_input,
					 gint		 remote_output);
gchar*		bsw_scm_wire_do_request	(BswSCMWire	*wire,
					 const gchar	*request_msg);
void		bsw_scm_wire_serve	(BswSCMWire	*wire);
void		bsw_scm_wire_died	(BswSCMWire	*wire);

void		bsw_scm_send_register	(const gchar    *name,
					 const gchar    *category,
					 const gchar    *blurb,
					 const gchar    *help,
					 const gchar    *author,
					 const gchar    *copyright,
					 const gchar    *date,
					 GSList         *params);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BSW_SCM_HANDLE_H__ */
