/* GSL - Generic Sound Layer
 * Copyright (C) 2001 Stefan Westerfeld and Tim Janik
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
#include "gslfilter.h"




/* --- common utilities --- */
static inline double
cotan (double x)
{
  return - tan (x + GSL_PI * 0.5);
}

static void
band_filter_common (unsigned int iorder,
		    double       p_freq, /* 0..pi */
		    double       s_freq, /* 0..pi */
		    double       epsilon,
		    GslComplex  *roots,
		    GslComplex  *poles,
		    double      *a,      /* [0..iorder] */
		    double      *b,
		    gboolean     band_pass,
		    gboolean     t1_norm)
{
  unsigned int iorder2 = iorder >> 1;
  GslComplex *poly = g_newa (GslComplex, iorder + 1);
  GslComplex fpoly[2 + 1] = { { 0, }, { 0, }, { 1, 0 } };
  double alpha, norm;
  guint i;
  
  alpha = cos ((s_freq + p_freq) * 0.5) / cos ((s_freq - p_freq) * 0.5);
  
  fpoly[0] = gsl_complex (1, 0);
  fpoly[1] = gsl_complex (1, 0);
  for (i = 0; i < iorder2; i++)
    {
      fpoly[0] = gsl_complex_mul (fpoly[0], gsl_complex_sub (gsl_complex (1, 0), gsl_complex_reciprocal (roots[i])));
      fpoly[1] = gsl_complex_mul (fpoly[1], gsl_complex_sub (gsl_complex (1, 0), gsl_complex_reciprocal (poles[i])));
    }
  norm = gsl_complex_div (fpoly[1], fpoly[0]).re;
  
  if ((iorder2 & 1) == 0)      /* norm is fluctuation minimum */
    norm *= sqrt (1.0 / (1.0 + epsilon * epsilon));
  
  /* z nominator polynomial */
  poly[0] = gsl_complex (norm, 0);
  for (i = 0; i < iorder2; i++)
    {
      GslComplex t, alphac = gsl_complex (alpha, 0);
      
      t = band_pass ? gsl_complex_inv (roots[i]) : roots[i];
      fpoly[1] = gsl_complex_sub (gsl_complex_div (alphac, t), alphac);
      fpoly[0] = gsl_complex_inv (gsl_complex_reciprocal (t));
      gsl_cpoly_mul (poly, i * 2, poly, 2, fpoly);
    }
  for (i = 0; i <= iorder; i++)
    a[i] = poly[i].re;
  
  /* z denominator polynomial */
  poly[0] = gsl_complex (1, 0);
  for (i = 0; i < iorder2; i++)
    {
      GslComplex t, alphac = gsl_complex (alpha, 0);
      
      t = band_pass ? gsl_complex_inv (poles[i]) : poles[i];
      fpoly[1] = gsl_complex_sub (gsl_complex_div (alphac, t), alphac);
      fpoly[0] = gsl_complex_inv (gsl_complex_reciprocal (t));
      gsl_cpoly_mul (poly, i * 2, poly, 2, fpoly);
    }
  for (i = 0; i <= iorder; i++)
    b[i] = poly[i].re;
}

static void
filter_rp_to_z (unsigned int iorder,
		GslComplex  *roots, /* [0..iorder-1] */
		GslComplex  *poles,
		double      *a,     /* [0..iorder] */
		double      *b)
{
  GslComplex *poly = g_newa (GslComplex, iorder + 1);
  guint i;
  
  /* z nominator polynomial */
  poly[0] = gsl_complex (1, 0);
  for (i = 0; i < iorder; i++)
    gsl_cpoly_mul_reciprocal (i + 1, poly, roots[i]);
  for (i = 0; i <= iorder; i++)
    a[i] = poly[i].re;

  /* z denominator polynomial */
  poly[0] = gsl_complex (1, 0);
  for (i = 0; i < iorder; i++)
    gsl_cpoly_mul_reciprocal (i + 1, poly, poles[i]);
  for (i = 0; i <= iorder; i++)
    b[i] = poly[i].re;
}

static void
filter_lp_invert (unsigned int iorder,
		  double      *a,     /* [0..iorder] */
		  double      *b)
{
  guint i;

  for (i = 1; i <= iorder; i +=2)
    {
      a[i] = -a[i];
      b[i] = -b[i];
    }
}


/* --- butterworth filter --- */
void
gsl_filter_butter_rp (unsigned int iorder,
		      double       freq, /* 0..pi */
		      double       epsilon,
		      GslComplex  *roots,    /* [0..iorder-1] */
		      GslComplex  *poles)
{
  double pi = GSL_PI, order = iorder;
  double beta_mul = pi / (2.0 * order);
  /* double kappa = gsl_trans_freq2s (freq); */
  double kappa = gsl_trans_freq2s (freq) * pow (epsilon, -1.0 / order);
  GslComplex root;
  unsigned int i;

  /* construct poles for butterworth filter */
  for (i = 1; i <= iorder; i++)
    {
      double t = (i << 1) + iorder - 1;
      double beta = t * beta_mul;

      root.re = kappa * cos (beta);
      root.im = kappa * sin (beta);
      poles[i - 1] = gsl_trans_s2z (root);
    }

  /* z nominator polynomial */
  for (i = 0; i < iorder; i++)
    roots[i] = gsl_complex (-1, 0);
}


/* --- tschebyscheff type 1 filter --- */
static double
tschebyscheff_eval (unsigned int degree,
		    double       x)
{
  double td = x, td_m_1 = 1;
  unsigned int d = 1;

  /* eval polynomial for a certain x */
  if (degree == 0)
    return 1;

  while (d < degree)
    {
      double td1 = 2 * x * td - td_m_1;

      td_m_1 = td;
      td = td1;
      d++;
    }
  return td;
}

void
gsl_filter_tscheb1_rp (unsigned int iorder,
		       double       freq,  /* 1..pi */
		       double       epsilon,
		       GslComplex  *roots, /* [0..iorder-1] */
		       GslComplex  *poles)
{
  double pi = GSL_PI, order = iorder;
  double alpha = asinh (1.0 / epsilon) / order;
  double beta_mul = pi / (2.0 * order);
  double kappa = gsl_trans_freq2s (freq);
  GslComplex root;
  unsigned int i;

  /* construct poles polynomial from tschebyscheff polynomial */
  for (i = 1; i <= iorder; i++)
    {
      double t = (i << 1) + iorder - 1;
      double beta = t * beta_mul;

      root.re = kappa * sinh (alpha) * cos (beta);
      root.im = kappa * cosh (alpha) * sin (beta);
      poles[i - 1] = gsl_trans_s2z (root);
    }

  /* z nominator polynomial */
  for (i = 0; i < iorder; i++)
    roots[i] = gsl_complex (-1, 0);
}


/* --- tschebyscheff type 2 filter --- */
void
gsl_filter_tscheb2_rp (unsigned int iorder,
		       double       c_freq, /* 1..pi */
		       double       r_freq, /* 1..pi */
		       double       epsilon,
		       GslComplex  *roots,  /* [0..iorder-1] */
		       GslComplex  *poles)
{
  double pi = GSL_PI, order = iorder;
  double kappa_c = gsl_trans_freq2s (c_freq);
  double kappa_r = gsl_trans_freq2s (r_freq);
  double tepsilon = epsilon * tschebyscheff_eval (iorder, kappa_r / kappa_c);
  double alpha = asinh (tepsilon) / order;
  double beta_mul = pi / (2.0 * order);
  GslComplex root;
  unsigned int i;

  /* construct poles polynomial from tschebyscheff polynomial */
  for (i = 1; i <= iorder; i++)
    {
      double t = (i << 1) + iorder - 1;
      double beta = t * beta_mul;
      
      root.re = sinh (alpha) * cos (beta);
      root.im = cosh (alpha) * sin (beta);
      root = gsl_complex_div (gsl_complex (kappa_r, 0), root);
      root = gsl_trans_s2z (root);
      poles[i - 1] = root;
    }
  
  /* construct roots polynomial from tschebyscheff polynomial */
  for (i = 1; i <= iorder; i++)
    {
      double t = (i << 1) - 1;
      GslComplex root = gsl_complex (0, cos (t * beta_mul));
      
      if (fabs (root.im) > 1e-14)
	{
	  root = gsl_complex_div (gsl_complex (kappa_r, 0), root);
	  root = gsl_trans_s2z (root);
	}
      else
	root = gsl_complex (-1, 0);
      roots[i - 1] = root;
    }
}


/* --- lowpass filters --- */
/**
 * gsl_filter_butter_lp
 * @iorder:   filter order
 * @freq:     cutoff frequency (0..pi)
 * @epsilon:  fall off at cutoff frequency (0..1)
 * @a:        root polynomial coefficients a[0..iorder]
 * @b:        pole polynomial coefficients b[0..iorder]
 * Butterworth lowpass filter.
 */
void
gsl_filter_butter_lp (unsigned int iorder,
		      double       freq, /* 0..pi */
		      double       epsilon,
		      double      *a,    /* [0..iorder] */
		      double      *b)
{
  GslComplex *roots = g_newa (GslComplex, iorder);
  GslComplex *poles = g_newa (GslComplex, iorder);
  double norm;
  
  g_return_if_fail (freq > 0 && freq < GSL_PI);

  gsl_filter_butter_rp (iorder, freq, epsilon, roots, poles);
  filter_rp_to_z (iorder, roots, poles, a, b);

  /* scale maximum to 1.0 */
  norm = gsl_poly_eval (iorder, b, 1) / gsl_poly_eval (iorder, a, 1);
  gsl_poly_scale (iorder, a, norm);
}

/**
 * gsl_filter_tscheb1_lp
 * @iorder:   filter order
 * @freq:     cutoff frequency (0..pi)
 * @epsilon:  fall off at cutoff frequency (0..1)
 * @a:        root polynomial coefficients a[0..iorder]
 * @b:        pole polynomial coefficients b[0..iorder]
 * Tschebyscheff type 1 lowpass filter.
 */
void
gsl_filter_tscheb1_lp (unsigned int iorder,
		       double       freq, /* 0..pi */
		       double       epsilon,
		       double      *a,    /* [0..iorder] */
		       double      *b)
{
  GslComplex *roots = g_newa (GslComplex, iorder);
  GslComplex *poles = g_newa (GslComplex, iorder);
  double norm;

  g_return_if_fail (freq > 0 && freq < GSL_PI);

  gsl_filter_tscheb1_rp (iorder, freq, epsilon, roots, poles);
  filter_rp_to_z (iorder, roots, poles, a, b);

  /* scale maximum to 1.0 */
  norm = gsl_poly_eval (iorder, b, 1) / gsl_poly_eval (iorder, a, 1);
  if ((iorder & 1) == 0)      /* norm is fluctuation minimum */
    norm *= sqrt (1.0 / (1.0 + epsilon * epsilon));
  gsl_poly_scale (iorder, a, norm);
}

/**
 * gsl_filter_tscheb2_lp
 * @iorder:    filter order
 * @freq:      passband cutoff frequency (0..pi)
 * @steepness: frequency steepness (c_freq * (1 + steepness) < pi)
 * @epsilon:   fall off at passband frequency (0..1)
 * @a:         root polynomial coefficients a[0..iorder]
 * @b:         pole polynomial coefficients b[0..iorder]
 * Tschebyscheff type 2 lowpass filter.
 */
void
gsl_filter_tscheb2_lp (unsigned int iorder,
		       double       freq,   /* 0..pi */
		       double       steepness,
		       double       epsilon,
		       double      *a,      /* [0..iorder] */
		       double      *b)
{
  GslComplex *roots = g_newa (GslComplex, iorder);
  GslComplex *poles = g_newa (GslComplex, iorder);
  double norm;

  g_return_if_fail (freq > 0 && freq < GSL_PI);
  g_return_if_fail (freq * (1.0 + steepness) < GSL_PI);

  gsl_filter_tscheb2_rp (iorder, freq, freq * (1.0 + steepness), epsilon, roots, poles);
  filter_rp_to_z (iorder, roots, poles, a, b);
  
  /* scale maximum to 1.0 */
  norm = gsl_poly_eval (iorder, b, 1) / gsl_poly_eval (iorder, a, 1); /* H(z=0):=1, e^(j*omega) for omega=0 => e^0==1 */
  gsl_poly_scale (iorder, a, norm);
}


/* --- highpass filters --- */
/**
 * gsl_filter_butter_hp
 * @iorder:   filter order
 * @freq:     passband frequency (0..pi)
 * @epsilon:  fall off at passband frequency (0..1)
 * @a:        root polynomial coefficients a[0..iorder]
 * @b:        pole polynomial coefficients b[0..iorder]
 * Butterworth highpass filter.
 */
void
gsl_filter_butter_hp (unsigned int iorder,
		      double       freq, /* 0..pi */
		      double       epsilon,
		      double      *a,    /* [0..iorder] */
		      double      *b)
{
  g_return_if_fail (freq > 0 && freq < GSL_PI);

  freq = GSL_PI - freq;
  gsl_filter_butter_lp (iorder, freq, epsilon, a, b);
  filter_lp_invert (iorder, a, b);
}

/**
 * gsl_filter_tscheb1_hp
 * @iorder:   filter order
 * @freq:     passband frequency (0..pi)
 * @epsilon:  fall off at passband frequency (0..1)
 * @a:        root polynomial coefficients a[0..iorder]
 * @b:        pole polynomial coefficients b[0..iorder]
 * Tschebyscheff type 1 highpass filter.
 */
void
gsl_filter_tscheb1_hp (unsigned int iorder,
		       double       freq, /* 0..pi */
		       double       epsilon,
		       double      *a,    /* [0..iorder] */
		       double      *b)
{
  g_return_if_fail (freq > 0 && freq < GSL_PI);

  freq = GSL_PI - freq;
  gsl_filter_tscheb1_lp (iorder, freq, epsilon, a, b);
  filter_lp_invert (iorder, a, b);
}

/**
 * gsl_filter_tscheb2_hp
 * @iorder:    filter order
 * @freq:      stopband frequency (0..pi)
 * @steepness: frequency steepness
 * @epsilon:   fall off at passband frequency (0..1)
 * @a:         root polynomial coefficients a[0..iorder]
 * @b:         pole polynomial coefficients b[0..iorder]
 * Tschebyscheff type 2 highpass filter.
 */
void
gsl_filter_tscheb2_hp   (unsigned int iorder,
			 double       freq,
			 double       steepness,
			 double       epsilon,
			 double      *a,      /* [0..iorder] */
			 double      *b)
{
  g_return_if_fail (freq > 0 && freq < GSL_PI);

  freq = GSL_PI - freq;
  gsl_filter_tscheb2_lp (iorder, freq, steepness, epsilon, a, b);
  filter_lp_invert (iorder, a, b);
}


/* --- bandpass filters --- */
/**
 * gsl_filter_butter_bp
 * @iorder:   filter order (must be even)
 * @freq1:    stopband end frequency (0..pi)
 * @freq2:    passband end frequency (0..pi)
 * @epsilon:  fall off at passband frequency (0..1)
 * @a:        root polynomial coefficients a[0..iorder]
 * @b:        pole polynomial coefficients b[0..iorder]
 * Butterworth bandpass filter.
 */
void
gsl_filter_butter_bp (unsigned int iorder,
		      double       freq1, /* 0..pi */
		      double       freq2, /* 0..pi */
		      double       epsilon,
		      double      *a,      /* [0..iorder] */
		      double      *b)
{
  unsigned int iorder2 = iorder >> 1;
  GslComplex *roots = g_newa (GslComplex, iorder2);
  GslComplex *poles = g_newa (GslComplex, iorder2);
  double theta;

  g_return_if_fail ((iorder & 0x01) == 0);
  g_return_if_fail (freq1 > 0);
  g_return_if_fail (freq1 < freq2);
  g_return_if_fail (freq2 < GSL_PI);

  theta = 2. * atan2 (1., cotan ((freq2 - freq1) * 0.5));

  gsl_filter_butter_rp (iorder2, theta, epsilon, roots, poles);
  band_filter_common (iorder, freq1, freq2, epsilon, roots, poles, a, b, TRUE, FALSE);
}

/**
 * gsl_filter_tscheb1_bp
 * @iorder:   filter order (must be even)
 * @freq1:    stopband end frequency (0..pi)
 * @freq2:    passband end frequency (0..pi)
 * @epsilon:  fall off at passband frequency (0..1)
 * @a:        root polynomial coefficients a[0..iorder]
 * @b:        pole polynomial coefficients b[0..iorder]
 * Tschebyscheff type 1 bandpass filter.
 */
void
gsl_filter_tscheb1_bp (unsigned int iorder,
		       double       freq1, /* 0..pi */
		       double       freq2, /* 0..pi */
		       double       epsilon,
		       double      *a,      /* [0..iorder] */
		       double      *b)
{
  unsigned int iorder2 = iorder >> 1;
  GslComplex *roots = g_newa (GslComplex, iorder2);
  GslComplex *poles = g_newa (GslComplex, iorder2);
  double theta;

  g_return_if_fail ((iorder & 0x01) == 0);
  g_return_if_fail (freq1 > 0);
  g_return_if_fail (freq1 < freq2);
  g_return_if_fail (freq2 < GSL_PI);
  
  theta = 2. * atan2 (1., cotan ((freq2 - freq1) * 0.5));

  gsl_filter_tscheb1_rp (iorder2, theta, epsilon, roots, poles);
  band_filter_common (iorder, freq1, freq2, epsilon, roots, poles, a, b, TRUE, TRUE);
}

/**
 * gsl_filter_tscheb2_bp
 * @iorder:    filter order (must be even)
 * @freq1:     stopband end frequency (0..pi)
 * @freq2:     passband end frequency (0..pi)
 * @steepness: frequency steepness factor
 * @epsilon:   fall off at passband frequency (0..1)
 * @a:         root polynomial coefficients a[0..iorder]
 * @b:         pole polynomial coefficients b[0..iorder]
 * Tschebyscheff type 2 bandpass filter.
 */
void
gsl_filter_tscheb2_bp (unsigned int iorder,
		       double       freq1, /* 0..pi */
		       double       freq2, /* 0..pi */
		       double       steepness,
		       double       epsilon,
		       double      *a,      /* [0..iorder] */
		       double      *b)
{
  unsigned int iorder2 = iorder >> 1;
  GslComplex *roots = g_newa (GslComplex, iorder2);
  GslComplex *poles = g_newa (GslComplex, iorder2);
  double theta;

  g_return_if_fail ((iorder & 0x01) == 0);
  g_return_if_fail (freq1 > 0);
  g_return_if_fail (freq1 < freq2);
  g_return_if_fail (freq2 < GSL_PI);
  
  theta = 2. * atan2 (1., cotan ((freq2 - freq1) * 0.5));

  gsl_filter_tscheb2_rp (iorder2, theta, theta * (1.0 + steepness), epsilon, roots, poles);
  band_filter_common (iorder, freq1, freq2, epsilon, roots, poles, a, b, TRUE, FALSE);
}


/* --- bandstop filters --- */
/**
 * gsl_filter_butter_bs
 * @iorder:   filter order (must be even)
 * @freq1:    passband end frequency (0..pi)
 * @freq2:    stopband end frequency (0..pi)
 * @epsilon:  fall off at passband frequency (0..1)
 * @a:        root polynomial coefficients a[0..iorder]
 * @b:        pole polynomial coefficients b[0..iorder]
 * Butterworth bandstop filter.
 */
void
gsl_filter_butter_bs (unsigned int iorder,
		      double       freq1, /* 0..pi */
		      double       freq2, /* 0..pi */
		      double       epsilon,
		      double      *a,      /* [0..iorder] */
		      double      *b)
{
  unsigned int iorder2 = iorder >> 1;
  GslComplex *roots = g_newa (GslComplex, iorder2);
  GslComplex *poles = g_newa (GslComplex, iorder2);
  double theta;

  g_return_if_fail ((iorder & 0x01) == 0);
  g_return_if_fail (freq1 > 0);
  g_return_if_fail (freq1 < freq2);
  g_return_if_fail (freq2 < GSL_PI);

  theta = 2. * atan2 (1., tan ((freq2 - freq1) * 0.5));

  gsl_filter_butter_rp (iorder2, theta, epsilon, roots, poles);
  band_filter_common (iorder, freq1, freq2, epsilon, roots, poles, a, b, FALSE, FALSE);
}

/**
 * gsl_filter_tscheb1_bs
 * @iorder:   filter order (must be even)
 * @freq1:    passband end frequency (0..pi)
 * @freq2:    stopband end frequency (0..pi)
 * @epsilon:  fall off at passband frequency (0..1)
 * @a:        root polynomial coefficients a[0..iorder]
 * @b:        pole polynomial coefficients b[0..iorder]
 * Tschebyscheff type 1 bandstop filter.
 */
void
gsl_filter_tscheb1_bs (unsigned int iorder,
		       double       freq1, /* 0..pi */
		       double       freq2, /* 0..pi */
		       double       epsilon,
		       double      *a,      /* [0..iorder] */
		       double      *b)
{
  unsigned int iorder2 = iorder >> 1;
  GslComplex *roots = g_newa (GslComplex, iorder2);
  GslComplex *poles = g_newa (GslComplex, iorder2);
  double theta;

  g_return_if_fail ((iorder & 0x01) == 0);
  g_return_if_fail (freq1 > 0);
  g_return_if_fail (freq1 < freq2);
  g_return_if_fail (freq2 < GSL_PI);
  
  theta = 2. * atan2 (1., tan ((freq2 - freq1) * 0.5));

  gsl_filter_tscheb1_rp (iorder2, theta, epsilon, roots, poles);
  band_filter_common (iorder, freq1, freq2, epsilon, roots, poles, a, b, FALSE, TRUE);
}

/**
 * gsl_filter_tscheb2_bs
 * @iorder:    filter order (must be even)
 * @freq1:     passband end frequency (0..pi)
 * @freq2:     stopband end frequency (0..pi)
 * @steepness: frequency steepness factor
 * @epsilon:   fall off at passband frequency (0..1)
 * @a:         root polynomial coefficients a[0..iorder]
 * @b:         pole polynomial coefficients b[0..iorder]
 * Tschebyscheff type 2 bandstop filter.
 */
void
gsl_filter_tscheb2_bs (unsigned int iorder,
		       double       freq1, /* 0..pi */
		       double       freq2, /* 0..pi */
		       double       steepness,
		       double       epsilon,
		       double      *a,      /* [0..iorder] */
		       double      *b)
{
  unsigned int iorder2 = iorder >> 1;
  GslComplex *roots = g_newa (GslComplex, iorder2);
  GslComplex *poles = g_newa (GslComplex, iorder2);
  double theta;

  g_return_if_fail ((iorder & 0x01) == 0);
  g_return_if_fail (freq1 > 0);
  g_return_if_fail (freq1 < freq2);
  g_return_if_fail (freq2 < GSL_PI);
  
  theta = 2. * atan2 (1., tan ((freq2 - freq1) * 0.5));

  gsl_filter_tscheb2_rp (iorder2, theta, theta * (1.0 + steepness), epsilon, roots, poles);
  band_filter_common (iorder, freq1, freq2, epsilon, roots, poles, a, b, FALSE, FALSE);
}


/* --- tschebyscheff type 1 via generic root-finding --- */
static void
tschebyscheff_poly (unsigned int degree,
		    double      *v)
{
  /* construct all polynomial koefficients */
  if (degree == 0)
    v[0] = 1;
  else if (degree == 1)
    {
      v[1] = 1; v[0] = 0;
    }
  else
    {
      double *u = g_newa (double, 1 + degree);
      
      u[degree] = 0; u[degree - 1] = 0;
      tschebyscheff_poly (degree - 2, u);
      
      v[0] = 0;
      tschebyscheff_poly (degree - 1, v + 1);
      gsl_poly_scale (degree - 1, v + 1, 2);
      
      gsl_poly_sub (degree, v, u);
    }
}

static void
gsl_filter_tscheb1_test	(unsigned int iorder,
			 double       zomega,
			 double       epsilon,
			 double      *a,    /* [0..iorder] */
			 double      *b)
{
  GslComplex *roots = g_newa (GslComplex, iorder * 2), *r;
  GslComplex *zf = g_newa (GslComplex, 1 + iorder);
  double *vk = g_newa (double, 1 + iorder), norm;
  double *q = g_newa (double, 2 * (1 + iorder));
  double O = gsl_trans_freq2s (zomega);
  unsigned int i;
  
  /* calc Vk() */
  tschebyscheff_poly (iorder, vk);
  
  /* calc q=1+e^2*Vk()^2 */
  gsl_poly_mul (q, iorder >> 1, vk, iorder >> 1, vk);
  iorder *= 2;
  gsl_poly_scale (iorder, q, epsilon * epsilon);
  q[0] += 1;

  /* find roots, fix roots by 1/(jO) */
  gsl_poly_complex_roots (iorder, q, roots);
  for (i = 0; i < iorder; i++)
    roots[i] = gsl_complex_mul (roots[i], gsl_complex (0, O));
  
  /* choose roots from the left half-plane */
  if (0)
    g_print ("zhqr-root:\n%s\n", gsl_complex_list (iorder, roots, "  "));
  r = roots;
  for (i = 0; i < iorder; i++)
    if (roots[i].re < 0)
      {
	r->re = roots[i].re;
	r->im = roots[i].im;
	r++;
      }
  iorder /= 2;
  
  /* assert roots found */
  if (!(r - roots == iorder))
    {
      g_print ("ERROR: n_roots=%u != iorder=%u\n", r - roots, iorder);
      abort ();
    }
  
  /* s => z */
  for (i = 0; i < iorder; i++)
    roots[i] = gsl_trans_s2z (roots[i]);
  
  /* z denominator polynomial */
  gsl_cpoly_from_roots (iorder, zf, roots);
  for (i = 0; i <= iorder; i++)
    b[i] = zf[i].re;
  
  /* z nominator polynomial */
  for (i = 0; i < iorder; i++)
    {
      roots[i].re = -1;
      roots[i].im = 0;
    }
  gsl_cpoly_from_roots (iorder, zf, roots);
  for (i = 0; i <= iorder; i++)
    a[i] = zf[i].re;
  
  /* scale for b[0]==1.0 */
  gsl_poly_scale (iorder, b, 1.0 / b[0]);

  /* scale maximum to 1.0 */
  norm = gsl_poly_eval (iorder, a, 1) / gsl_poly_eval (iorder, b, 1);
  if ((iorder & 0x01) == 0)	/* norm is fluctuation minimum */
    norm /= sqrt (1.0 / (1.0 + epsilon * epsilon));
  gsl_poly_scale (iorder, a, 1.0 / norm);
}
