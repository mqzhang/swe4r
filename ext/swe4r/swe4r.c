/*
Swe4r :: Swiss Ephemeris for Ruby - A C extension for the Swiss Ephemeris library (http://www.astro.com/swisseph/)
Copyright (C) 2012 Andrew Kirk (andrew.kirk@windhorsemedia.com)
Additional work (C) 2023 David Lowenfels (dfl@alum.mit.edu)

This file is part of Swe4r.

Swe4r is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Swe4r is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Swe4r.  If not, see <http://www.gnu.org/licenses/>.
*/


// https://docs.ruby-lang.org/en/3.0/extension_rdoc.html
#include <ruby.h> 
#include "swephexp.h"

// Module Name
VALUE rb_mSwe4r = Qnil;

/*
 * Set directory path of ephemeris files
 * http://www.astro.com/swisseph/swephprg.htm#_Toc283735481
 * int swe_set_ephe_path(char *path);
 */
static VALUE t_swe_set_ephe_path(VALUE self, VALUE path)
{
	swe_set_ephe_path(StringValuePtr(path));
	return Qnil;
}

/*
 * Set directory path of ephemeris files to work with jpl
 * http://www.astro.com/swisseph/swephprg.htm#_Toc283735481
 * int swe_set_jpl_file(char *fname);
 */
static VALUE t_swe_set_jpl_file(VALUE self, VALUE path)
{
	swe_set_jpl_file(StringValuePtr(path));
	return Qnil;
}

/*
 * Get the Julian day number from year, month, day, hour
 * http://www.astro.com/swisseph/swephprg.htm#_Toc283735468
	double swe_julday(
		int year,
		int month,
		int day,
		double hour,
		int gregflag	// Gregorian calendar: 1, Julian calendar: 0
	);
 */
static VALUE t_swe_julday(VALUE self, VALUE year, VALUE month, VALUE day, VALUE hour)
{
	double julday = swe_julday(NUM2INT(year), NUM2INT(month), NUM2INT(day), NUM2DBL(hour), SE_GREG_CAL);
	return rb_float_new(julday);
}

/*
 * Set the geographic location for topocentric planet computation
 * The longitude and latitude must be in degrees, the altitude in meters.
 * http://www.astro.com/swisseph/swephprg.htm#_Toc283735476
	void swe_set_topo (
		double geolon,      // geographic longitude: eastern longitude is positive, western longitude is negative
		double geolat,      // geographic latitude: northern latitude is positive, southern latitude is negative
		double altitude		// altitude above sea
	);
*/
static VALUE t_swe_set_topo(VALUE self, VALUE lon, VALUE lat, VALUE alt)
{
	swe_set_topo(NUM2DBL(lon), NUM2DBL(lat), NUM2DBL(alt));
	return Qnil;
}

/*
 * Calculation of planets, moon, asteroids, lunar nodes, apogees, fictitious bodies
 * http://www.astro.com/swisseph/swephprg.htm#_Toc283735419
	long swe_calc_ut(
		double tjd_ut,	// Julian day number, Universal Time
		int ipl,		// planet number
		long iflag,		// flag bits
		double *xx,  	// target address for 6 position values: longitude, latitude, distance, long.speed, lat.speed, dist.speed
		char *serr		// 256 bytes for error string
	);
 */
static VALUE t_swe_calc_ut(VALUE self, VALUE julian_ut, VALUE body, VALUE iflag)
{
	double results[6];
	char serr[AS_MAXCH];

	if (swe_calc_ut(NUM2DBL(julian_ut), NUM2INT(body), NUM2LONG(iflag), results, serr) < 0)
		rb_raise(rb_eRuntimeError, serr);

	VALUE output = rb_ary_new();
	for (int i = 0; i < 6; i++)
		rb_ary_push(output, rb_float_new(results[i]));

	return output;
}

/*
 * This function can be used to specify the mode for sidereal computations
 * http://www.astro.com/swisseph/swephprg.htm#_Toc283735478
	void swe_set_sid_mode (
		int32 sid_mode, 	// Mode
		double t0, 		// Reference date
		double ayan_t0	// Initial value of the ayanamsha
	);
 */
static VALUE t_swe_set_sid_mode(VALUE self, VALUE mode, VALUE t0, VALUE ayan_t0)
{
	swe_set_sid_mode(NUM2INT(mode), NUM2DBL(t0), NUM2DBL(ayan_t0));
	return Qnil;
}

/*
 * This function computes the ayanamsha, the distance of the tropical vernal point from the sidereal zero point of the zodiac.
 * The ayanamsha is used to compute sidereal planetary positions from tropical ones:
 * pos_sid = pos_trop – ayanamsha
 * Before calling swe_get_ayanamsha(), you have to set the sidereal mode with swe_set_sid_mode, unless you want the default sidereal mode, which is the Fagan/Bradley ayanamsha.
 * http://www.astro.com/swisseph/swephprg.htm#_Toc283735479
 * double swe_get_ayanamsa_ut(double tjd_ut);
 */
static VALUE t_swe_get_ayanamsa_ut(VALUE self, VALUE julian_ut)
{
	double ayanamsa = swe_get_ayanamsa_ut(NUM2DBL(julian_ut));
	return rb_float_new(ayanamsa);
}


// * This function computes the ayanamsha using a Delta T consistent with the ephe_flag specified.
// * https://www.astro.com/swisseph/swephprg.htm#_Toc112949018
// * input variables:
// * tjd_ut = Julian day number in UT
// * (tjd_et = Julian day number in ET/TT)
// * iflag = ephemeris flag (one of SEFLG_SWIEPH, SEFLG_JPLEPH, SEFLG_MOSEPH)
// * plus some other optional SEFLG_...
// * output values
// * daya = ayanamsha value (pointer to double)
// * serr = error message or warning (pointer to string)
// * The function returns either the ephemeris flag used or ERR (-1)

static VALUE t_swe_get_ayanamsa_ex_ut(VALUE self, VALUE julian_ut, VALUE flag )
{
	double ayanamsha;
	char serr[AS_MAXCH];

	// if(TYPE(flag) == T_NIL) { // default to Moshier Ephemeris
	// 	flag = SEFLG_MOSEPH;
	// }

	if (swe_get_ayanamsa_ex_ut(NUM2DBL(julian_ut), NUM2INT(flag), &ayanamsha, serr) < 0)
		rb_raise(rb_eRuntimeError, serr);

	return rb_float_new(ayanamsha);
}


/*
 * This function computes house cusps, ascendant, midheaven, etc
 * http://www.astro.com/swisseph/swephprg.htm#_Toc283735486
	int swe_houses(
		double tjd_ut,      // Julian day number, UT
		double geolat,      // geographic latitude, in degrees
		double geolon,      // geographic longitude, in degrees (eastern longitude is positive, western longitude is negative, northern latitude is positive, southern latitude is negative
		int hsys,           // house method, ascii code of one of the letters PKORCAEVXHTBG
		double *cusps,      // array for 13 doubles
		double *ascmc	    // array for 10 doubles
	);
 * House method codes...
 * ‘P’ 			= Placidus
 * ‘K’     		= Koch
 * ‘O’     		= Porphyrius
 * ‘R’     		= Regiomontanus
 * ‘C’     		= Campanus
 * ‘A’ or ‘E’  	= Equal (cusp 1 is Ascendant)
 * ‘V’     		= Vehlow equal (Asc. in middle of house 1)
 * ‘W’     		= Whole sign
 * ‘X’     		= axial rotation system
 * ‘H’     		= azimuthal or horizontal system
 * ‘T’     		= Polich/Page (“topocentric” system)
 * ‘B’     		= Alcabitus
 * ‘M’     		= Morinus
 * ‘U’     		= Krusinski-Pisa
 * ‘G’     		= Gauquelin sectors
 */

static VALUE t_swe_houses(VALUE self, VALUE julian_day, VALUE latitude, VALUE longitude, VALUE house_system)
{
	double cusps[13];
	double ascmc[10];
	char serr[AS_MAXCH];

	if (swe_houses(NUM2DBL(julian_day), NUM2DBL(latitude), NUM2DBL(longitude), NUM2CHR(house_system), cusps, ascmc) < 0)
		rb_raise(rb_eRuntimeError, serr);

	VALUE _cusps = rb_ary_new();
	for (int i = 0; i < 13; i++)
		rb_ary_push(_cusps, rb_float_new(cusps[i]));

	VALUE _ascmc = rb_ary_new();
	for (int i = 0; i < 10; i++)
		rb_ary_push(_ascmc, rb_float_new(ascmc[i]));

	VALUE output = rb_ary_new();
	rb_ary_push(output, _cusps);
	rb_ary_push(output, _ascmc);
	return output;
}

// This function is better than swe_houses and returns speeds as well
// https://www.astro.com/swisseph/swephprg.htm#_Toc112949026
// int swe_houses_ex2(
// double tjd_ut,      /* Julian day number, UT */
// int32 iflag,        /* 0 or SEFLG_SIDEREAL or SEFLG_RADIANS or SEFLG_NONUT */
// double geolat,      /* geographic latitude, in degrees */
// double geolon,      /* geographic longitude, in degrees
//                     * eastern longitude is positive,
//                     * western longitude is negative,
//                     * northern latitude is positive,
//                     * southern latitude is negative */
// int hsys,           /* house method, one-letter case sensitive code (list, see further below) */
// double *cusps,      /* array for 13 (or 37 for hsys G) doubles, explained further below */
// double *ascmc,      /* array for 10 doubles, explained further below */
// double *cusp_speed,  /* like cusps */
// double *ascmc_speed, /* like ascmc */
// char *serr);       


static VALUE t_swe_houses_ex2(VALUE self, VALUE julian_day, VALUE flag, VALUE latitude, VALUE longitude, VALUE house_system)
{
	double cusps[13];
	double ascmc[10];
	double cusps_speed[13];
	double ascmc_speed[10];
	char serr[AS_MAXCH];

	if (swe_houses_ex2(NUM2DBL(julian_day), NUM2INT(flag), NUM2DBL(latitude), NUM2DBL(longitude), NUM2CHR(house_system), cusps, ascmc, cusps_speed, ascmc_speed, serr) < 0)
		rb_raise(rb_eRuntimeError, serr);


	VALUE _cusps = rb_ary_new();
	for (int i = 0; i < 13; i++)
		rb_ary_push(_cusps, rb_float_new(cusps[i]));

	VALUE _ascmc = rb_ary_new();
	for (int i = 0; i < 10; i++)
		rb_ary_push(_ascmc, rb_float_new(ascmc[i]));

	VALUE _cusps_speed = rb_ary_new();
	for (int i = 0; i < 13; i++)
		rb_ary_push(_cusps_speed, rb_float_new(cusps_speed[i]));

	VALUE _ascmc_speed = rb_ary_new();
	for (int i = 0; i < 10; i++)
		rb_ary_push(_ascmc_speed, rb_float_new(ascmc_speed[i]));

	VALUE output = rb_ary_new();
	rb_ary_push(output, _cusps);
	rb_ary_push(output, _ascmc);
	rb_ary_push(output, _cusps_speed);
	rb_ary_push(output, _ascmc_speed);
	return output;
}

// int32 swe_rise_trans(
// double tjd_ut,      /* search after this time (UT) */
// int32 ipl,               /* planet number, if planet or moon */
// char *starname,     /* star name, if star; must be NULL or empty, if ipl is used */
// int32 epheflag,     /* ephemeris flag */
// int32 rsmi,              /* integer specifying that rise, set, or one of the two meridian transits is wanted. see definition below */
// double *geopos,     /* array of three doubles containing
//                         * geograph. long., lat., height of observer */
// double atpress      /* atmospheric pressure in mbar/hPa */
// double attemp,      /* atmospheric temperature in deg. C */
// double *tret,            /* return address (double) for rise time etc. */
// char *serr);             /* return address for error message */

static VALUE t_swe_rise_trans(VALUE self, VALUE julian_day, VALUE body, VALUE flag, VALUE rmsi, VALUE lat, VALUE lon, VALUE height, VALUE pressure, VALUE temp)
{
	double geopos[3];
	geopos[0] = NUM2DBL(lat);
	geopos[1] = NUM2DBL(lon);
	geopos[2] = NUM2DBL(height);
	int ipl;
	char *starname;
	if( TYPE(body) == T_STRING ) {
		starname = StringValuePtr(body);
		ipl = 0;
	} else {
		ipl = NUM2INT(body);
		starname = NULL;
	}
	char serr[AS_MAXCH];
	double retval;

	if (swe_rise_trans(NUM2DBL(julian_day), ipl, starname, NUM2INT(flag), NUM2INT(rmsi), geopos, NUM2DBL(pressure), NUM2DBL(temp), &retval, serr) < 0)
		rb_raise(rb_eRuntimeError, serr);
	return rb_float_new(retval);
}

// int32 swe_rise_trans_true_hor(
// double tjd_ut,      /* search after this time (UT) */
// int32 ipl,               /* planet number, if planet or moon */
// char *starname,     /* star name, if star; must be NULL or empty, if ipl is used */
// int32 epheflag,     /* ephemeris flag */
// int32 rsmi,              /* integer specifying that rise, set, or one of the two meridian transits is wanted. see definition below */
// double *geopos,     /* array of three doubles containing
//                         * geograph. long., lat., height of observer */
// double atpress,     /* atmospheric pressure in mbar/hPa */
// double attemp,      /* atmospheric temperature in deg. C */
// double horhgt,      /* height of local horizon in deg at the point where the body rises or sets */
// double *tret,       /* return address (double) for rise time etc. */
// char *serr);        /* return address for error message */

static VALUE t_swe_rise_trans_true_hor(VALUE self, VALUE julian_day, VALUE body, VALUE flag, VALUE rmsi, VALUE lat, VALUE lon, VALUE height, VALUE pressure, VALUE temp, VALUE hor_height)
{
	double geopos[3];
	geopos[0] = NUM2DBL(lat);
	geopos[1] = NUM2DBL(lon);
	geopos[2] = NUM2DBL(height);
	int ipl;
	char *starname;
	if( TYPE(body) == T_STRING ) {
		starname = StringValuePtr(body);
		ipl = 0;
	} else {
		ipl = NUM2INT(body);
		starname = NULL;
	}
	char serr[AS_MAXCH];
	double retval;

	if (swe_rise_trans_true_hor(NUM2DBL(julian_day), ipl, starname, NUM2INT(flag), NUM2INT(rmsi), geopos, NUM2DBL(pressure), NUM2DBL(temp), NUM2DBL(hor_height), &retval, serr) < 0)
		rb_raise(rb_eRuntimeError, serr);
	return rb_float_new(retval);
}

/* TODO: bind swe_house_pos(), what is an ARMC?? */

void Init_swe4r()
{
	// Module
	rb_mSwe4r = rb_define_module("Swe4r");

	// Module Functions
	rb_define_module_function(rb_mSwe4r, "swe_set_ephe_path", t_swe_set_ephe_path, 1);
	rb_define_module_function(rb_mSwe4r, "swe_set_jpl_file", t_swe_set_jpl_file, 1);
	rb_define_module_function(rb_mSwe4r, "swe_julday", t_swe_julday, 4);
	rb_define_module_function(rb_mSwe4r, "swe_set_topo", t_swe_set_topo, 3);
	rb_define_module_function(rb_mSwe4r, "swe_calc_ut", t_swe_calc_ut, 3);
	rb_define_module_function(rb_mSwe4r, "swe_set_sid_mode", t_swe_set_sid_mode, 3);
	rb_define_module_function(rb_mSwe4r, "swe_get_ayanamsa_ut", t_swe_get_ayanamsa_ut, 1);
	rb_define_module_function(rb_mSwe4r, "swe_houses", t_swe_houses, 4);
	rb_define_module_function(rb_mSwe4r, "swe_houses_ex2", t_swe_houses_ex2, 5);
	rb_define_module_function(rb_mSwe4r, "swe_get_ayanamsa_ex_ut", t_swe_get_ayanamsa_ex_ut, 2);
	rb_define_module_function(rb_mSwe4r, "swe_rise_trans", t_swe_rise_trans, 9);
	rb_define_module_function(rb_mSwe4r, "swe_rise_trans_true_hor", t_swe_rise_trans_true_hor, 10);

	// Constants

	rb_define_const(rb_mSwe4r, "SE_SUN", INT2FIX(SE_SUN));
	rb_define_const(rb_mSwe4r, "SE_MOON", INT2FIX(SE_MOON));
	rb_define_const(rb_mSwe4r, "SE_MERCURY", INT2FIX(SE_MERCURY));
	rb_define_const(rb_mSwe4r, "SE_VENUS", INT2FIX(SE_VENUS));
	rb_define_const(rb_mSwe4r, "SE_MARS", INT2FIX(SE_MARS));
	rb_define_const(rb_mSwe4r, "SE_JUPITER", INT2FIX(SE_JUPITER));
	rb_define_const(rb_mSwe4r, "SE_SATURN", INT2FIX(SE_SATURN));
	rb_define_const(rb_mSwe4r, "SE_URANUS", INT2FIX(SE_URANUS));
	rb_define_const(rb_mSwe4r, "SE_NEPTUNE", INT2FIX(SE_NEPTUNE));
	rb_define_const(rb_mSwe4r, "SE_PLUTO", INT2FIX(SE_PLUTO));
	rb_define_const(rb_mSwe4r, "SE_MEAN_NODE", INT2FIX(SE_MEAN_NODE));
	rb_define_const(rb_mSwe4r, "SE_TRUE_NODE", INT2FIX(SE_TRUE_NODE));
	rb_define_const(rb_mSwe4r, "SE_MEAN_APOG", INT2FIX(SE_MEAN_APOG));
	rb_define_const(rb_mSwe4r, "SE_OSCU_APOG", INT2FIX(SE_OSCU_APOG));
	rb_define_const(rb_mSwe4r, "SE_EARTH", INT2FIX(SE_EARTH));
	rb_define_const(rb_mSwe4r, "SE_CHIRON", INT2FIX(SE_CHIRON));
	rb_define_const(rb_mSwe4r, "SE_PHOLUS", INT2FIX(SE_PHOLUS));
	rb_define_const(rb_mSwe4r, "SE_CERES", INT2FIX(SE_CERES));
	rb_define_const(rb_mSwe4r, "SE_PALLAS", INT2FIX(SE_PALLAS));
	rb_define_const(rb_mSwe4r, "SE_JUNO", INT2FIX(SE_JUNO));
	rb_define_const(rb_mSwe4r, "SE_VESTA", INT2FIX(SE_VESTA));
	rb_define_const(rb_mSwe4r, "SE_CUPIDO", INT2FIX(SE_CUPIDO));
	rb_define_const(rb_mSwe4r, "SE_HADES", INT2FIX(SE_HADES));
	rb_define_const(rb_mSwe4r, "SE_ZEUS", INT2FIX(SE_ZEUS));
	rb_define_const(rb_mSwe4r, "SE_KRONOS", INT2FIX(SE_KRONOS));
	rb_define_const(rb_mSwe4r, "SE_APOLLON", INT2FIX(SE_APOLLON));
	rb_define_const(rb_mSwe4r, "SE_ADMETOS", INT2FIX(SE_ADMETOS));
	rb_define_const(rb_mSwe4r, "SE_VULKANUS", INT2FIX(SE_VULKANUS));
	rb_define_const(rb_mSwe4r, "SE_POSEIDON", INT2FIX(SE_POSEIDON));

	rb_define_const(rb_mSwe4r, "SE_INTP_APOG", INT2FIX(SE_INTP_APOG));
	rb_define_const(rb_mSwe4r, "SE_INTP_PERG", INT2FIX(SE_INTP_PERG));

	rb_define_const(rb_mSwe4r, "SEFLG_JPLEPH", INT2FIX(SEFLG_JPLEPH));
	rb_define_const(rb_mSwe4r, "SEFLG_SWIEPH", INT2FIX(SEFLG_SWIEPH));
	rb_define_const(rb_mSwe4r, "SEFLG_MOSEPH", INT2FIX(SEFLG_MOSEPH));
	rb_define_const(rb_mSwe4r, "SEFLG_HELCTR", INT2FIX(SEFLG_HELCTR));
	rb_define_const(rb_mSwe4r, "SEFLG_TRUEPOS", INT2FIX(SEFLG_TRUEPOS));
	rb_define_const(rb_mSwe4r, "SEFLG_J2000", INT2FIX(SEFLG_J2000));
	rb_define_const(rb_mSwe4r, "SEFLG_NONUT", INT2FIX(SEFLG_NONUT));
	rb_define_const(rb_mSwe4r, "SEFLG_SPEED3", INT2FIX(SEFLG_SPEED3));
	rb_define_const(rb_mSwe4r, "SEFLG_SPEED", INT2FIX(SEFLG_SPEED));
	rb_define_const(rb_mSwe4r, "SEFLG_NOGDEFL", INT2FIX(SEFLG_NOGDEFL));
	rb_define_const(rb_mSwe4r, "SEFLG_NOABERR", INT2FIX(SEFLG_NOABERR));
	rb_define_const(rb_mSwe4r, "SEFLG_EQUATORIAL", INT2FIX(SEFLG_EQUATORIAL));
	rb_define_const(rb_mSwe4r, "SEFLG_XYZ", INT2FIX(SEFLG_XYZ));
	rb_define_const(rb_mSwe4r, "SEFLG_RADIANS", INT2FIX(SEFLG_RADIANS));
	rb_define_const(rb_mSwe4r, "SEFLG_BARYCTR", INT2FIX(SEFLG_BARYCTR));
	rb_define_const(rb_mSwe4r, "SEFLG_TOPOCTR", INT2FIX(SEFLG_TOPOCTR));
	rb_define_const(rb_mSwe4r, "SEFLG_SIDEREAL", INT2FIX(SEFLG_SIDEREAL));
	rb_define_const(rb_mSwe4r, "SEFLG_ICRS", INT2FIX(SEFLG_ICRS));

	rb_define_const(rb_mSwe4r, "SE_SIDM_FAGAN_BRADLEY", INT2FIX(SE_SIDM_FAGAN_BRADLEY));
	rb_define_const(rb_mSwe4r, "SE_SIDM_LAHIRI", INT2FIX(SE_SIDM_LAHIRI));
	rb_define_const(rb_mSwe4r, "SE_SIDM_DELUCE", INT2FIX(SE_SIDM_DELUCE));
	rb_define_const(rb_mSwe4r, "SE_SIDM_RAMAN", INT2FIX(SE_SIDM_RAMAN));
	rb_define_const(rb_mSwe4r, "SE_SIDM_USHASHASHI", INT2FIX(SE_SIDM_USHASHASHI));
	rb_define_const(rb_mSwe4r, "SE_SIDM_KRISHNAMURTI", INT2FIX(SE_SIDM_KRISHNAMURTI));
	rb_define_const(rb_mSwe4r, "SE_SIDM_DJWHAL_KHUL", INT2FIX(SE_SIDM_DJWHAL_KHUL));
	rb_define_const(rb_mSwe4r, "SE_SIDM_YUKTESHWAR", INT2FIX(SE_SIDM_YUKTESHWAR));
	rb_define_const(rb_mSwe4r, "SE_SIDM_JN_BHASIN", INT2FIX(SE_SIDM_JN_BHASIN));
	rb_define_const(rb_mSwe4r, "SE_SIDM_BABYL_KUGLER1", INT2FIX(SE_SIDM_BABYL_KUGLER1));
	rb_define_const(rb_mSwe4r, "SE_SIDM_BABYL_KUGLER2", INT2FIX(SE_SIDM_BABYL_KUGLER2));
	rb_define_const(rb_mSwe4r, "SE_SIDM_BABYL_KUGLER3", INT2FIX(SE_SIDM_BABYL_KUGLER3));
	rb_define_const(rb_mSwe4r, "SE_SIDM_BABYL_HUBER", INT2FIX(SE_SIDM_BABYL_HUBER));
	rb_define_const(rb_mSwe4r, "SE_SIDM_BABYL_ETPSC", INT2FIX(SE_SIDM_BABYL_ETPSC));
	rb_define_const(rb_mSwe4r, "SE_SIDM_ALDEBARAN_15TAU", INT2FIX(SE_SIDM_ALDEBARAN_15TAU));
	rb_define_const(rb_mSwe4r, "SE_SIDM_HIPPARCHOS", INT2FIX(SE_SIDM_HIPPARCHOS));
	rb_define_const(rb_mSwe4r, "SE_SIDM_SASSANIAN", INT2FIX(SE_SIDM_SASSANIAN));
	rb_define_const(rb_mSwe4r, "SE_SIDM_GALCENT_0SAG", INT2FIX(SE_SIDM_GALCENT_0SAG));
	rb_define_const(rb_mSwe4r, "SE_SIDM_J2000", INT2FIX(SE_SIDM_J2000));
	rb_define_const(rb_mSwe4r, "SE_SIDM_J1900", INT2FIX(SE_SIDM_J1900));
	rb_define_const(rb_mSwe4r, "SE_SIDM_B1950", INT2FIX(SE_SIDM_B1950));
	rb_define_const(rb_mSwe4r, "SE_SIDM_USER", INT2FIX(SE_SIDM_USER));

	rb_define_const(rb_mSwe4r, "SE_CALC_RISE", INT2FIX(SE_CALC_RISE));
	rb_define_const(rb_mSwe4r, "SE_CALC_SET", INT2FIX(SE_CALC_SET));
	rb_define_const(rb_mSwe4r, "SE_CALC_MTRANSIT", INT2FIX(SE_CALC_MTRANSIT));
	rb_define_const(rb_mSwe4r, "SE_CALC_ITRANSIT", INT2FIX(SE_CALC_ITRANSIT));
	rb_define_const(rb_mSwe4r, "SE_BIT_DISC_CENTER", INT2FIX(SE_BIT_DISC_CENTER));
	rb_define_const(rb_mSwe4r, "SE_BIT_DISC_BOTTOM", INT2FIX(SE_BIT_DISC_BOTTOM));
	rb_define_const(rb_mSwe4r, "SE_BIT_GEOCTR_NO_ECL_LAT", INT2FIX(SE_BIT_GEOCTR_NO_ECL_LAT));
	rb_define_const(rb_mSwe4r, "SE_BIT_NO_REFRACTION", INT2FIX(SE_BIT_NO_REFRACTION));
	rb_define_const(rb_mSwe4r, "SE_BIT_CIVIL_TWILIGHT", INT2FIX(SE_BIT_CIVIL_TWILIGHT));
	rb_define_const(rb_mSwe4r, "SE_BIT_NAUTIC_TWILIGHT", INT2FIX(SE_BIT_NAUTIC_TWILIGHT));
	rb_define_const(rb_mSwe4r, "SE_BIT_ASTRO_TWILIGHT", INT2FIX(SE_BIT_ASTRO_TWILIGHT));
	rb_define_const(rb_mSwe4r, "SE_BIT_FIXED_DISC_SIZE", INT2FIX(SE_BIT_FIXED_DISC_SIZE));
	rb_define_const(rb_mSwe4r, "SE_BIT_HINDU_RISING", INT2FIX(SE_BIT_HINDU_RISING));
}
