#include <tau/tau.h>
#include <libmseed.h>
#include <time.h>

TEST (time, nstime2timestr)
{
  char timestr[50];
  nstime_t nstime;

  /* Suppress error and warning messages by accumulating them */
  ms_rloginit (NULL, NULL, NULL, NULL, 10);

  /* General parsing test to nstime_t */
  nstime = ms_timestr2nstime ("2004-05-12T7:8:9.123456788Z");
  CHECK (nstime == 1084345689123456788, "Failed to convert time string: '2004-05-12T7:8:9.123456788Z'");

  /* Format variations */
  ms_nstime2timestr_n (nstime, timestr, sizeof(timestr), ISOMONTHDAY, NANO_MICRO_NONE);
  CHECK_STREQ (timestr, "2004-05-12T07:08:09.123456788");

  ms_nstime2timestr_n (nstime, timestr, sizeof(timestr), ISOMONTHDAY_Z, NANO_MICRO_NONE);
  CHECK_STREQ (timestr, "2004-05-12T07:08:09.123456788Z");

  ms_nstime2timestr_n (nstime, timestr, sizeof(timestr), ISOMONTHDAY_DOY, NANO_MICRO_NONE);
  CHECK_STREQ (timestr, "2004-05-12T07:08:09.123456788 (133)");

  ms_nstime2timestr_n (nstime, timestr, sizeof(timestr), ISOMONTHDAY_DOY_Z, NANO_MICRO_NONE);
  CHECK_STREQ (timestr, "2004-05-12T07:08:09.123456788Z (133)");

  ms_nstime2timestr_n (nstime, timestr, sizeof(timestr), SEEDORDINAL, NANO_MICRO_NONE);
  CHECK_STREQ (timestr, "2004,133,07:08:09.123456788");

  ms_nstime2timestr_n (nstime, timestr, sizeof(timestr), UNIXEPOCH, NANO_MICRO_NONE);
  CHECK_STREQ (timestr, "1084345689.123456788");

  ms_nstime2timestr_n (nstime, timestr, sizeof(timestr), NANOSECONDEPOCH, NANO_MICRO_NONE);
  CHECK_STREQ (timestr, "1084345689123456788");

  /* Subsecond variations */

  /* Nano subseconds */
  nstime = ms_timestr2nstime ("2004-05-12T7:8:9.123456788Z");
  ms_nstime2timestr_n (nstime, timestr, sizeof(timestr), ISOMONTHDAY_Z, NANO);
  CHECK_STREQ (timestr, "2004-05-12T07:08:09.123456788Z");

  ms_nstime2timestr_n (nstime, timestr, sizeof(timestr), ISOMONTHDAY_Z, MICRO);
  CHECK_STREQ (timestr, "2004-05-12T07:08:09.123456Z");

  ms_nstime2timestr_n (nstime, timestr, sizeof(timestr), ISOMONTHDAY_Z, NONE);
  CHECK_STREQ (timestr, "2004-05-12T07:08:09Z");

  /* Micro subseconds */
  nstime = ms_timestr2nstime ("2004-05-12T7:8:9.1234Z");
  ms_nstime2timestr_n (nstime, timestr, sizeof(timestr), ISOMONTHDAY_Z, NANO_MICRO_NONE);
  CHECK_STREQ (timestr, "2004-05-12T07:08:09.123400Z");

  ms_nstime2timestr_n (nstime, timestr, sizeof(timestr), ISOMONTHDAY_Z, NANO_MICRO);
  CHECK_STREQ (timestr, "2004-05-12T07:08:09.123400Z");

  ms_nstime2timestr_n (nstime, timestr, sizeof(timestr), ISOMONTHDAY_Z, MICRO_NONE);
  CHECK_STREQ (timestr, "2004-05-12T07:08:09.123400Z");

  /* No subseconds */
  nstime = ms_timestr2nstime ("2004-05-12T7:8:9Z");
  ms_nstime2timestr_n (nstime, timestr, sizeof(timestr), ISOMONTHDAY_Z, NANO_MICRO_NONE);
  CHECK_STREQ (timestr, "2004-05-12T07:08:09Z");

  ms_nstime2timestr_n (nstime, timestr, sizeof(timestr), ISOMONTHDAY_Z, NANO_MICRO);
  CHECK_STREQ (timestr, "2004-05-12T07:08:09.000000Z");

  ms_nstime2timestr_n (nstime, timestr, sizeof(timestr), ISOMONTHDAY_Z, MICRO_NONE);
  CHECK_STREQ (timestr, "2004-05-12T07:08:09Z");

  /* Negative sub-second times: UNIXEPOCH integer seconds are floored to agree
   * with calendar formats, e.g. -1.5s is second -2, not -1 */
  nstime = -1500000000; /* -1.5 seconds */
  ms_nstime2timestr_n (nstime, timestr, sizeof(timestr), UNIXEPOCH, NONE);
  CHECK_STREQ (timestr, "-2");

  ms_nstime2timestr_n (nstime, timestr, sizeof(timestr), ISOMONTHDAY_Z, NONE);
  CHECK_STREQ (timestr, "1969-12-31T23:59:58Z");

  ms_nstime2timestr_n (nstime, timestr, sizeof(timestr), UNIXEPOCH, NANO);
  CHECK_STREQ (timestr, "-1.500000000");

  nstime = -500000000; /* -0.5 seconds */
  ms_nstime2timestr_n (nstime, timestr, sizeof(timestr), UNIXEPOCH, NANO);
  CHECK_STREQ (timestr, "-0.500000000");

  /* Unset time */
  ms_nstime2timestr_n (NSTUNSET, timestr, sizeof(timestr), ISOMONTHDAY_Z, NANO_MICRO_NONE);
  CHECK_STREQ (timestr, "UNSET");

  /* Error time */
  ms_nstime2timestr_n (NSTERROR, timestr, sizeof(timestr), ISOMONTHDAY_Z, NANO_MICRO_NONE);
  CHECK_STREQ (timestr, "ERROR");
}

TEST (time, timestr2nstime)
{
  nstime_t nstime;

  nstime = ms_timestr2nstime ("2004");
  CHECK (nstime == 1072915200000000000, "Failed to convert time string: '2004'");

  nstime = ms_timestr2nstime ("2004-2-9");
  CHECK (nstime == 1076284800000000000, "Failed to convert time string: '2004-2-9'");

  nstime = ms_timestr2nstime ("2004-05-12T7:8:9.12345Z");
  CHECK (nstime == 1084345689123450000, "Failed to convert time string: '2004-05-12T7:8:9.12345Z'");

  nstime = ms_timestr2nstime ("2004-05-12T7:8:9.12345");
  CHECK (nstime == 1084345689123450000, "Failed to convert time string: '2004-05-12T7:8:9.12345'");

  nstime = ms_timestr2nstime ("2004-05-12T7:8:9.123456788");
  CHECK (nstime == 1084345689123456788, "Failed to convert time string: '2004-05-12T7:8:9.123456788'");

  nstime = ms_timestr2nstime ("1084345689.123456788");
  CHECK (nstime == 1084345689123456788, "Failed to convert time string: '1084345689.123456788'");

  nstime = ms_timestr2nstime ("1969,201,20,17,40.98");
  CHECK (nstime == -14182939020000000, "Failed to convert time string: '1969,201,20,17,40.98'");

  nstime = ms_timestr2nstime ("1969-201T20:17:40.987654321");
  CHECK (nstime == -14182939012345679, "Failed to convert time string: '1969-201T20:17:40.987654321'");

  nstime = ms_timestr2nstime ("-14182939.012345679");
  CHECK (nstime == -14182939012345679, "Failed to convert time string: '-14182939.012345679'");

  /* Near-boundary value that remains in range */
  nstime = ms_timestr2nstime ("+9223372036.5");
  CHECK (nstime == 9223372036500000000, "Failed to convert time string: '+9223372036.5'");

  /* Parsing error tests */
  nstime = ms_timestr2nstime ("this is not a time string");
  CHECK (nstime == NSTERROR, "Failed to produce error for time string: 'this is not a time string'");

  /* Out-of-range seconds */
  nstime = ms_timestr2nstime ("+99999999999");
  CHECK (nstime == NSTERROR, "Failed to produce error for time string: '+99999999999'");

  /* In-range seconds that overflow once the fraction is added */
  nstime = ms_timestr2nstime ("+9223372036.99999999");
  CHECK (nstime == NSTERROR, "Failed to produce error for time string: '+9223372036.99999999'");

  /* In-range seconds that underflow once the fraction is subtracted */
  nstime = ms_timestr2nstime ("-9223372036.99999999");
  CHECK (nstime == NSTERROR, "Failed to produce error for time string: '-9223372036.99999999'");

  nstime = ms_timestr2nstime ("0000-00-00");
  CHECK (nstime == NSTERROR, "Failed to produce error for time string: '0000-00-00'");

  nstime = ms_timestr2nstime ("5000-00-00");
  CHECK (nstime == NSTERROR, "Failed to produce error for time string: '5000-00-00'");

  nstime = ms_timestr2nstime ("20040512T000000");
  CHECK (nstime == NSTERROR, "Failed to produce error for time string: '20040512T000000'");
}

TEST (time, systemtime)
{
  time_t timeval;
  nstime_t currenttime;
  nstime_t difference;

  currenttime = lmp_systemtime();
  timeval = time (NULL);

  CHECK (currenttime > 0, "lmp_systemtime() failed to get current time");

  /* Check that the current time is within 1 second of the system time */
  difference = currenttime - (nstime_t)timeval * NSTMODULUS;

  CHECK (difference < 1 * NSTMODULUS, "lmp_systemtime() is not within 1 second of system time");
}