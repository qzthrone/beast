/* BSE - Bedevilled Sound Engine
 * Copyright (C) 2006 Tim Janik
 *
 * This software is provided "as is"; redistribution and modification
 * is permitted, provided that the following disclaimer is retained.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * In no event shall the authors or contributors be liable for any
 * direct, indirect, incidental, special, exemplary, or consequential
 * damages (including, but not limited to, procurement of substitute
 * goods or services; loss of use, data, or profits; or business
 * interruption) however caused and on any theory of liability, whether
 * in contract, strict liability, or tort (including negligence or
 * otherwise) arising in any way out of the use of this software, even
 * if advised of the possibility of such damage.
 */
#include "bsefilter.h"
#include <birnet/birnet.hh>

using namespace Birnet;

extern "C" {

const gchar*
bse_iir_filter_kind_string (BseIIRFilterKind fkind)
{
  switch (fkind)
    {
    case BSE_IIR_FILTER_BUTTERWORTH:    return "Butterworth";
    case BSE_IIR_FILTER_BESSEL:         return "Bessel";
    case BSE_IIR_FILTER_CHEBYSHEV1:     return "Chebyshev1";
    case BSE_IIR_FILTER_CHEBYSHEV2:     return "Chebyshev2";
    case BSE_IIR_FILTER_ELLIPTIC:       return "Cauer";
    default:                            return "?unknown?";
    }
}

const gchar*
bse_iir_filter_type_string (BseIIRFilterType ftype)
{
  switch (ftype)
    {
    case BSE_IIR_FILTER_LOW_PASS:       return "Low-pass";
    case BSE_IIR_FILTER_BAND_PASS:      return "Band-pass";
    case BSE_IIR_FILTER_HIGH_PASS:      return "High-pass";
    case BSE_IIR_FILTER_BAND_STOP:      return "Band-stop";
    default:                            return "?unknown?";
    }
}

gchar*
bse_iir_filter_request_string (const BseIIRFilterRequest *ifr)
{
  String s;
  s += bse_iir_filter_kind_string (ifr->kind);
  s += " ";
  s += bse_iir_filter_type_string (ifr->type);
  s += " order=" + string_from_int (ifr->order);
  s += " sample-rate=" + string_from_float (ifr->sampling_frequency);
  s += " passband-edge=" + string_from_float (ifr->passband_edge);
  if (ifr->kind == BSE_IIR_FILTER_CHEBYSHEV1 || ifr->kind == BSE_IIR_FILTER_ELLIPTIC)
    s += " passband-ripple-db=" + string_from_float (ifr->passband_ripple_db);
  if (ifr->type == BSE_IIR_FILTER_BAND_PASS || ifr->type == BSE_IIR_FILTER_BAND_STOP)
    s += " passband-edge2=" + string_from_float (ifr->passband_edge2);
  if (ifr->kind == BSE_IIR_FILTER_ELLIPTIC && ifr->stopband_edge > 0)
    s += " stopband-edge=" + string_from_float (ifr->stopband_edge);
  if (ifr->kind == BSE_IIR_FILTER_ELLIPTIC && ifr->stopband_db < 0)
    s += " stopband-db=" + string_from_float (ifr->stopband_db);
  return g_strdup (s.c_str());
}

gchar*
bse_iir_filter_design_string (const BseIIRFilterDesign *fid)
{
  String s;
  s += "order=" + string_from_int (fid->order);
  s += " sample-rate=" + string_from_float (fid->sampling_frequency);
  s += " gain=" + string_from_double (fid->gain);
  s += " n_zeros=" + string_from_int (fid->n_zeros);
  s += " n_poles=" + string_from_int (fid->n_poles);
  for (uint i = 0; i < fid->n_zeros; i++)
    {
      String u ("Zero:");
      u += " " + string_from_double (real (fid->zz[i]));
      u += " + " + string_from_double (imag (fid->zz[i])) + "*i";
      s += "\n" + u;
    }
  for (uint i = 0; i < fid->n_poles; i++)
    {
      String u ("Pole:");
      u += " " + string_from_double (real (fid->zp[i]));
      u += " + " + string_from_double (imag (fid->zp[i])) + "*i";
      s += "\n" + u;
    }
  String u;
  uint o = fid->order;
  u = string_from_double (fid->zn[o]);
  while (o--)
    u = "(" + u + ") * z + " + string_from_double (fid->zn[o]);
  s += "\nNominator: " + u;
  o = fid->order;
  u = string_from_double (fid->zd[o]);
  while (o--)
    u = "(" + u + ") * z + " + string_from_double (fid->zd[o]);
  s += "\nDenominator: " + u;
  return g_strdup (s.c_str());
}

bool
bse_iir_filter_design (const BseIIRFilterRequest  *filter_request,
                       BseIIRFilterDesign         *filter_design)
{
  if (filter_request->kind == BSE_IIR_FILTER_BUTTERWORTH ||
      filter_request->kind == BSE_IIR_FILTER_CHEBYSHEV1 ||
      filter_request->kind == BSE_IIR_FILTER_ELLIPTIC)
    return _bse_filter_design_ellf (filter_request, filter_design);
  return false;
}

} // C
