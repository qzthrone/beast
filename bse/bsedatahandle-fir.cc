/* BSE - Bedevilled Sound Engine
 * Copyright (C) 2001, 2003 Tim Janik and Stefan Westerfeld
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
#include "gsldatahandle.h"
#include "gsldatautils.h"
#include "gslfilter.h"
#include <complex>
#include <vector>
#include <string.h>
#include <stdio.h>
#include <math.h>

namespace Bse {

using std::vector;
using std::min;

class DataHandleFir;

struct CDataHandleFir : public GslDataHandle
{
  // back pointer to get casting right, even in presence of C++ vtable:
  DataHandleFir* cxx_dh;
};

class DataHandleFir {
protected:
  CDataHandleFir	m_dhandle;
  GslDataHandle	       *m_src_handle;
  vector<double>        m_a;	      /* FIR coefficients: [0..order] */
  vector<float>         m_input_data;
  int64                 m_input_voffset;
  int64                 m_block_size;
  int64                 m_history;
  bool			m_init_ok;

public:
  DataHandleFir (GslDataHandle *src_handle,
		 guint          order) :
    m_src_handle (src_handle),
    m_a (order + 1),
    m_init_ok (false)
  {
    g_return_if_fail (src_handle != NULL);

    memset (&m_dhandle, 0, sizeof (m_dhandle));
    m_init_ok = gsl_data_handle_common_init (&m_dhandle, NULL);
    if (m_init_ok)
      gsl_data_handle_ref (m_src_handle);
  }

  /* protected destructor: (use reference counting instead) */
  virtual
  ~DataHandleFir()
  {
    if (m_init_ok)
      {
	gsl_data_handle_unref (m_src_handle);
	gsl_data_handle_common_free (&m_dhandle);
      }
  }

  BseErrorType
  open (GslDataHandleSetup *setup)
  {
    BseErrorType error = gsl_data_handle_open (m_src_handle);
    if (error != BSE_ERROR_NONE)
      return error;

    /* !not! m_dhandle.setup; the framework magically ensures that *m_dhandle.setup
     * is initialized by whatever we write into *setup if open is successful
     */
    *setup = m_src_handle->setup; /* copies setup.xinfos by pointer */
    setup->bit_depth = 32;	  /* possibly increased by filtering */

    // since we need overlapping data for consecutive reads we buffer data locally
    m_block_size = 1024 * m_src_handle->setup.n_channels;
    m_history = (m_a.size() + 1) / 2;
    m_input_data.resize (m_block_size + (2 * m_history) * m_src_handle->setup.n_channels);
    m_input_voffset = -2 * m_block_size;

    design_filter_coefficients (gsl_data_handle_mix_freq (m_src_handle));

    return BSE_ERROR_NONE;
  }

  virtual void
  design_filter_coefficients (double mix_freq) = 0;

  void
  close()
  {
    m_dhandle.setup.xinfos = NULL;	/* cleanup pointer reference */
    gsl_data_handle_close (m_src_handle);
  }

  void
  fir_apply (const gfloat *src,
	     const guint   n_samples,
	     gfloat       *dest)
  {
    /* tiny FIR evaluation: not optimized for speed */
    guint i, j;
    const guint iorder = m_a.size();
    for (i = 0; i < n_samples; i++)
      {
	gdouble accu = 0;
	for (j = 0; j <= iorder; j++)
	  {
	    GslLong p = i + j;
	    p -= iorder / 2;
	    accu += m_a[j] * src[p];
	  }
	dest[i] = accu;
      }
  }

  int64
  seek (int64 voffset)
  {
    int64 i = 0;
    g_return_val_if_fail (voffset % m_block_size == 0, -1);

    if (m_input_voffset == voffset - m_block_size)
      {
	int64 overlap_values = 2 * m_history * m_dhandle.setup.n_channels;
	copy (m_input_data.end() - overlap_values, m_input_data.end(), m_input_data.begin());
	i += overlap_values;
      }

    while (i < static_cast<int64> (m_input_data.size()))
      {
	int64 offset = voffset + i - m_history;
	if (offset >= 0 && offset < m_dhandle.setup.n_values)
	  {
	    int64 values_todo = min (static_cast<int64> (m_input_data.size()) - i, m_dhandle.setup.n_values - offset);
	    int64 l = gsl_data_handle_read (m_src_handle, offset, values_todo, &m_input_data[i]);
	    if (l < 0)
	      {
		// invalidate m_input_data
		voffset = -2 * m_block_size;
		return l;
	      }
	    else
	      {
		i += l;
	      }
	  }
	else
	  {
	    m_input_data[i++] = 0;
	  }
      }
    m_input_voffset = voffset;
    return 0;
  }

  int64
  read (int64  voffset,
	int64  n_values,
	float *values)
  {
    int64 ivoffset = voffset;
    ivoffset = ivoffset - ivoffset % m_block_size;

    if (ivoffset != m_input_voffset)
      {
	int64 l = seek (ivoffset);
	if (l < 0)
	  return l;
      }

    g_assert (ivoffset == m_input_voffset);
    vector<float> dest_data (m_input_data.size());
    fir_apply (&m_input_data[m_history], m_block_size, &dest_data[m_history]);
    
    voffset -= ivoffset;
    n_values = min (n_values, m_block_size - voffset);
    voffset += m_history;
    std::copy (&dest_data[voffset], &dest_data[voffset + n_values], values);
    return n_values;
  }

  int64
  get_state_length() const
  {
    int64 source_state_length = gsl_data_handle_get_state_length (m_src_handle);
    // m_src_handle must be opened and have valid state size
    g_return_val_if_fail (source_state_length >= 0, 0);  

    int64 per_channel_state = 0;
    return source_state_length + per_channel_state * m_dhandle.setup.n_channels;
  }

  static GslDataHandle*
  dh_create (DataHandleFir *cxx_dh)
  {
    static GslDataHandleFuncs dh_vtable =
    {
      dh_open,
      dh_read,
      dh_close,
      NULL,
      dh_get_state_length,
      dh_destroy,
    };

    if (cxx_dh->m_init_ok)
      {
	cxx_dh->m_dhandle.vtable = &dh_vtable;
	cxx_dh->m_dhandle.cxx_dh = cxx_dh;	/* make casts work, later on */
	return &cxx_dh->m_dhandle;
      }
    else
      {
	delete cxx_dh;
	return NULL;
      }
  }
private:
/* for the "C" API (vtable) */
  static DataHandleFir*
  dh_cast (GslDataHandle *dhandle)
  {
    return static_cast<CDataHandleFir *> (dhandle)->cxx_dh;
  }
  static BseErrorType
  dh_open (GslDataHandle *dhandle, GslDataHandleSetup *setup)
  {
    return dh_cast (dhandle)->open (setup);
  }
  static void
  dh_close (GslDataHandle *dhandle)
  {
    dh_cast (dhandle)->close();
  }
  static void
  dh_destroy (GslDataHandle *dhandle)
  {
    delete dh_cast (dhandle);
  }
  static int64
  dh_read (GslDataHandle *dhandle,
	   int64          voffset,
	   int64          n_values,
	   gfloat        *values)
  {
    return dh_cast (dhandle)->read (voffset, n_values, values);
  }
  static int64
  dh_get_state_length (GslDataHandle *dhandle)
  {
    return dh_cast (dhandle)->get_state_length();
  }
};

class DataHandleFirHighpass : public DataHandleFir
{
protected:
  gdouble m_cutoff_freq;

public:
  DataHandleFirHighpass (GslDataHandle *src_handle,
			 gdouble        cutoff_freq,
			 guint          order) :
    DataHandleFir (src_handle, order),
    m_cutoff_freq (cutoff_freq)
  {
    if (m_init_ok)
      m_dhandle.name = g_strconcat (m_src_handle->name, "// #highpass /", NULL);
  }

  virtual void
  design_filter_coefficients (double mix_freq)
  {
    const guint transfer_func_length = 4;
    double transfer_func_freqs[transfer_func_length];
    double transfer_func_values[transfer_func_length];

    transfer_func_freqs[0]  = 0;
    transfer_func_values[0] = 0;

    transfer_func_freqs[1]  = m_cutoff_freq / mix_freq * 2 * M_PI;
    transfer_func_values[1] = 0;

    transfer_func_freqs[2]  = m_cutoff_freq / mix_freq * 2 * M_PI;
    transfer_func_values[2] = 1.0; // 0 dB

    transfer_func_freqs[3]  = PI;
    transfer_func_values[3] = 1.0; // 0 dB

    gsl_filter_fir_approx (m_a.size() - 1, &m_a[0],
                           transfer_func_length, transfer_func_freqs, transfer_func_values,
			   false); // interpolate dB
  }
};

}

#if 0 // debugging
  gfloat freq;
  for (freq = 0; freq < PI; freq += 0.01)
    {
      complex z = cexp (I * freq);
      complex r = 0;

      guint i;
      for (i = 0; i <= iorder; i++)
	{
	  r /= z;
	  r += a[i];
	}
      printf ("%f %f\n", freq, cabs (r));
    }
#endif


using namespace Bse;

/*
 *           __________
 *          /
 *         /
 *        /
 *  _____/
 *         |
 *    cutoff_freq
 *
 * @cutoff_freq: cutoff frequency in Hz in intervall [0..SR/2]
 * @order:       number of filter coefficients
 */
extern "C" GslDataHandle*
bse_data_handle_new_fir_highpass (GslDataHandle *src_handle,
				  gdouble        cutoff_freq,
				  guint          order)
{
  DataHandleFir *cxx_dh = new DataHandleFirHighpass (src_handle, cutoff_freq, order);
  return DataHandleFir::dh_create (cxx_dh);
}
