/* GSL - Generic Sound Layer
 * Copyright (C) 2001-2004 Tim Janik
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#ifndef __GSL_LOADER_H__
#define __GSL_LOADER_H__

#include <bse/gsldefs.h>
#include <bse/gslcommon.h>
#include <bse/gslwavechunk.h>

G_BEGIN_DECLS

/* --- structures --- */
struct _GslWaveFileInfo
{
  guint	   n_waves;
  struct {
    gchar *name;
  }       *waves;

  gchar  **comments;

  /*< private >*/
  gchar     *file_name;
  GslLoader *loader;
  guint      ref_count;
};
struct _GslWaveDsc
{
  gchar		  *name;
  guint	           n_chunks;
  GslWaveChunkDsc *chunks;
  guint            n_channels;
  gchar          **xinfos;
  /*< private >*/
  GslWaveFileInfo *file_info;
};
struct _GslWaveChunkDsc
{
  gfloat	  mix_freq;
  gfloat	  osc_freq;
  gchar         **xinfos;
  /* loader-specific */
  GslLong         loader_offset;
  GslLong         loader_length;
  glong		  loader_num1;
  gpointer	  loader_data1; /* generic pointers for more data */
  gpointer	  loader_data2;
};


/* --- functions --- */
GslWaveFileInfo*      gsl_wave_file_info_load	(const gchar	 *file_name,
						 BseErrorType	 *error);
GslWaveFileInfo*      gsl_wave_file_info_ref	(GslWaveFileInfo *wave_file_info);
void                  gsl_wave_file_info_unref	(GslWaveFileInfo *wave_file_info);
const gchar*	      gsl_wave_file_info_loader	(GslWaveFileInfo *fi);
GslWaveDsc*	      gsl_wave_dsc_load		(GslWaveFileInfo *wave_file_info,
						 guint		  nth_wave,
						 BseErrorType	 *error);
void		      gsl_wave_dsc_free		(GslWaveDsc	 *wave_dsc);
GslDataHandle*	      gsl_wave_handle_create	(GslWaveDsc	 *wave_dsc,
						 guint		  nth_chunk,
						 BseErrorType	 *error);
GslWaveChunk*	      gsl_wave_chunk_create	(GslWaveDsc	 *wave_dsc,
						 guint		  nth_chunk,
						 BseErrorType	 *error);


/* --- loader impl --- */
typedef enum /*< skip >*/
{
  GSL_LOADER_SKIP_PRECEEDING_NULLS = 1 << 0
} GslLoaderFlags;
struct _GslLoader
{
  const gchar *name;		/* format/loader name, e.g. "BseWave" or "WAVE audio, RIFF (little-endian)" */

  /* at least one of the
   * following three must
   * be non-NULL
   */
  const gchar **extensions;	/* e.g.: "mp3", "ogg" or "bsewave" */
  const gchar **mime_types;	/* e.g.: "audio/x-mpg3" or "audio/x-wav" */
  GslLoaderFlags flags;
  const gchar **magic_specs;	/* e.g.: "0 string RIFF\n8 string WAVE\n" or "0 string #BseWave\n" */

  gint   priority;   /* -100=high, +100=low, 0=default */

  /*< private >*/
  gpointer		  data;
  GslWaveFileInfo*	(*load_file_info)	(gpointer	   data,
						 const gchar	  *file_name,
						 BseErrorType	  *error);
  void			(*free_file_info)	(gpointer	   data,
						 GslWaveFileInfo  *file_info);
  GslWaveDsc*		(*load_wave_dsc)	(gpointer	   data,
						 GslWaveFileInfo  *file_info,
						 guint		   nth_wave,
						 BseErrorType	  *error);
  void			(*free_wave_dsc)	(gpointer	   data,
						 GslWaveDsc	  *wave_dsc);
  GslDataHandle*	(*create_chunk_handle)	(gpointer	   data,
						 GslWaveDsc	  *wave_dsc,
						 guint		   nth_chunk,
						 BseErrorType	  *error);
  GslLoader   *next;	/* must be NULL */
};

void	      gsl_loader_register	        (GslLoader	 *loader);
GslLoader*    gsl_loader_match	                (const gchar	 *file_name);
gchar**       bse_xinfos_add_value              (gchar          **xinfos,
                                                 const gchar     *key,
                                                 const gchar     *value);
gchar**       bse_xinfos_add_float              (gchar          **xinfos,
                                                 const gchar     *key,
                                                 gfloat           fvalue);
gchar**       bse_xinfos_add_num                (gchar          **xinfos,
                                                 const gchar     *key,
                                                 SfiNum           num);
const gchar*  bse_xinfos_get_value              (gchar          **xinfos,
                                                 const gchar     *key);
gfloat        bse_xinfos_get_float              (gchar          **xinfos,
                                                 const gchar     *key);
SfiNum        bse_xinfos_get_num                (gchar          **xinfos,
                                                 const gchar     *key);
gchar**       bse_xinfos_del_value              (gchar          **xinfos,
                                                 const gchar     *key);

G_END_DECLS

#endif /* __GSL_LOADER_H__ */
