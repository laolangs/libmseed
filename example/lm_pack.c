/***************************************************************************
 * A program for libmseed packing tests.
 *
 * This file is part of the miniSEED Library.
 *
 * Copyright (c) 2024 Chad Trabant, EarthScope Data Services
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <libmseed.h>

#define VERSION "[libmseed " LIBMSEED_VERSION " example]"
#define PACKAGE "lm_pack"

static flag verbose       = 0;
static int reclen         = -1;
static double samprate    = 1.0;
static int encoding       = 10;
static int msformat       = 3;
static nstime_t starttime = NSTUNSET;
static char *outfile      = NULL;

static int parameter_proc (int argcount, char **argvec);
static void usage (void);

/****************************************************************************
 * An expanding sinusoid of 500 samples.
 *
 * When truncated to integers:
 *  - Signed differences in samples needing all bit lengths 3-31s.
 *  - ONLY the last difference requires 31 bits, all others are <= 30
 * -  Contain sequences of differences matching all Steim 1 and 2 encoding possibilities
 *
 * Values 1 through up to 220 do not require more than 16-bits.
 ***************************************************************************/
#define SINE_DATA_SAMPLES 500
static float fsinedata[SINE_DATA_SAMPLES] =
    {0.000000,6.109208,10.246826,10.609957,6.764728,-0.075704,-7.409461,-12.346208,
    -12.731430,-8.062958,0.182060,8.985442,14.875067,15.276420,9.609196,-0.328370,
    -10.895428,-17.921131,-18.329336,-11.450576,0.526448,13.209973,21.590023,21.991385,
    13.643140,-0.791247,-16.014492,-26.008907,-26.383901,-16.253504,1.141655,19.412378,
    31.330871,31.652348,19.360848,-1.601465,-23.528777,-37.740204,-37.971107,-23.059260,
    2.200591,28.515156,45.458753,45.549217,27.460529,-2.976569,-34.554817,-54.753559,
    -54.637244,-32.697448,3.976416,41.869576,65.946052,65.535525,38.927729,-5.258928,
    -50.727832,-79.423116,-78.604029,-46.338644,6.897516,61.454325,95.650397,94.274177,
    55.152523,-8.983716,-74.441929,-115.188314,-113.063003,-65.633259,11.631511,
    90.165916,138.711322,135.590105,78.094013,-14.982665,-109.201193,-167.031082,
    -162.597954,-92.906331,19.213290,132.243134,201.124338,194.976222,110.510935,
    -24.541940,-160.132765,-242.166440,-233.790899,-131.430478,31.239561,193.887193,
    291.571675,280.319178,156.284630,-39.641741,-234.736391,-351.041770,-336.091216,
    -185.807912,50.163757,284.167632,422.624233,402.940140,220.870763,-63.319072,
    -343.979194,-508.782510,-483.061913,-262.504429,79.742042,416.345244,612.480366,
    579.087014,311.930343,-100.215795,-503.894220,-737.283353,-694.166245,-370.594813,
    125.706429,609.803540,887.480834,832.073438,440.209940,-157.404955,-737.913996,
    -1068.232708,-997.328409,-522.801872,196.778712,892.867950,1285.745847,1195.344115,
    620.767701,-245.634371,-1080.276240,-1547.486227,-1432.602775,-736.942489,
    306.195102,1306.919790,1862.433994,1716.866654,874.678227,-381.195062,-1580.993078,
    -2241.390126,-2057.430298,-1037.936774,473.995052,1912.398180,2697.345105,2465.422369,
    1231.399220,-588.724017,-2313.099844,-3245.922143,-2954.166800,-1460.594493,
    730.452133,2797.554253,3905.910007,3539.614901,1732.050527,-905.402443,-3383.226729,
    -4699.903513,-4240.862333,-2053.471832,1121.209554,4091.216806,5655.073452,
    5080.767553,2433.947965,-1387.235765,-4947.012887,-6804.092030,-6086.691631,
    -2884.198121,1714.957253,5981.403297,8186.245216,7291.383170,3416.857907,-2118.435721,
    -7231.576094,-9848.769674,-8734.036728,-4046.815355,2614.894255,8742.446660,
    11848.459577,10461.558685,4791.604321,-3225.420222,-10568.260176,-14253.597692,
    -12530.081077,-5671.864737,3975.823020,12774.525771,17146.276092,15006.771888,
    6711.880612,-4897.680529,-15440.350897,-20625.184991,-17971.999652,-7940.208402,
    6029.615451,18661.258563,24808.964001,21521.921578,9390.410233,-7418.851697,
    -22552.587165,-29840.229074,-25771.577789,-11101.908663,9123.111793,27253.593143,
    35890.411153,30858.590420,13120.982075,-11212.929535,-32932.401520,-43165.569941,
    -36947.585456,-15501.922592,13774.468128,39791.979238,51913.378983,44235.478131,
    18308.381438,-16912.953578,-48077.342225,-62431.517747,-52957.790070,-21614.930138,
    20756.856764,58084.250586,75077.753677,63396.198979,25508.869716,-25462.986446,
    -70169.698634,-90282.054065,-75887.560674,-30092.324242,31222.690356,84764.569627,
    108561.135815,90834.689747,35484.659605,-38269.404043,-102388.901147,-130535.943080,
    -108719.240618,-41825.273267,46887.838631,123670.298781,156952.641109,130117.096987,
    49276.805933,-57425.161274,-149366.146307,-188707.832667,-155716.756615,-58028.831480,
    70304.598043,180390.393831,226878.845103,186341.292651,68302.086980,-86041.981200,
    -217845.865904,-272760.106197,-222974.585037,-80353.310040,105265.874749,
    263063.225186,327906.831053,266792.649626,94480.755768,-128742.047924,-317647.960457,
    -394187.487304,-319201.052420,-111030.470003,157403.231090,383537.048821,
    473846.799900,381879.586964,130403.398591,-192385.288422,-463067.280664,
    -569581.409624,-456835.620147,-153063.413757,235071.184227,559057.643966,
    684630.722900,546467.782541,179546.337116,-287144.413901,-674908.656359,-822885.998573,
    -653642.002301,-210470.033463,350653.927224,814722.125712,989021.327043,781782.266485,
    246545.638548,-428093.004245,-983445.533844,-1188650.888674,-934978.952245,
    -288589.965744,522495.068481,1187046.097893,1428517.756157,1118118.116795,
    337539.108116,-637550.058001,-1432720.599829,-1716720.558561,-1337035.786123,
    -394463.210730,777745.745791,1729148.322568,2062985.588205,1598702.057951,
    460582.328770,-948539.335064,-2086795.934450,-2478993.447111,-1911440.758239,
    -537283.204837,1156565.787585,2518284.974613,2978771.147961,2285191.490698,
    626136.686773,-1409890.715316,-3038834.772892,-3579162.765425,-2731822.228828,
    -728915.356799,1718317.328563,3666796.264850,4300394.349825,3265502.159830,
    847610.742696,-2093758.948552,-4424295.326464,-5166751.952850,-3903146.346575,
    -984449.218292,2550691.033230,5338008.063395,6207395.378498,4664945.984099,
    1141905.356234,-3106699.622013,-6440095.078674,-7457334.785992,-5575000.657492,
    -1322711.048944,3783146.686785,7769327.268660,8958602.684417,6662072.138144,
    1529858.136359,-4605977.214406,-9372442.351616,-10761660.350202,-7960482.979376,
    -1766591.537123,5606698.099533,11305779.346088,12927085.482769,9511187.602536,
    2036388.930375,-6823565.288237,-13637247.864482,-15527597.247686,-11363048.833756,
    -2342921.824577,8303023.316062,16448700.704063,18650486.048610,13574359.117581,
    2689991.311114,-10101450.710498,-19838792.204081,-22400528.788313,-16214653.084049,
    -3081429.850680,12287276.018778,23926421.676653,26903486.467518,19366867.002831,
    3520957.976311,-14943542.889821,-28854881.491245,-32310300.258257,-23129911.185536,
    -4011981.690190,18171019.183242,34796853.797034,38802125.311214,27621733.908111,
    4557312.422997,-22091965.102165,-41960429.245394,-46596369.274924,-32982970.290335,
    -5158786.517383,26854699.579994,50596356.438136,55953935.730766,39381287.228525,
    5816755.047390,-32639133.476996,-61006773.393892,-67187912.573574,-47016556.451947,
    -6529407.091211,39663473.627686,73555723.560159,80673993.102082,56127012.680548,
    7291879.963058,-48192344.715084,-88681820.564997,-96862974.793882,-66996583.433046,
    -8095097.920129,58546627.892657,106913500.120816,116295749.303753,79963612.136585,
    8924265.924556,-71115377.915425,-128887386.814358,-139621279.391140,
    -95431237.847443,-9756926.457174,86370256.557591,155370411.011265,167618156.945886,
    113879744.320966,10560464.285645,-104883012.041142,-187286440.462195,
    -201220454.267134,-135881249.803956,-11288915.420918,127346645.407973,
    225748346.861610,241548722.121826,162117178.463787,11878900.943144,
    -154601039.263161,-272096614.921471,-289947157.492990,-193399036.824020,
    -12244462.331299,187663986.962885,327945826.905572,348028166.863781,
    230693116.321629,12270520.408649,-227768756.995135,-395240626.753420,
    -417725794.004062,-275149858.943091,-11804612.586023,276409565.108355,
    476323094.198962,501359772.474058,328138760.150134,10646479.758475,
    -335396614.264246,-574013851.836865,-601712311.942546,-391289845.886545,
    -8534971.317913,406922710.094078,691709700.348455,722120145.317499,
    466542952.987464,5131609.783276,-493643879.751773,-833501145.234545,
    -866584864.231526,-556206270.243475,0.00000};

static char *textdata =
    "I've seen things you people wouldn't believe. Attack ships on fire off the shoulder of Orion. "
    "I watched C-beams glitter in the dark near the Tannhäuser Gate. All those moments will be lost "
    "in time, like tears...in...rain. Time to die.";

/* Binary I/O for Windows platforms */
#ifdef LMP_WIN
  #include <fcntl.h>
  unsigned int _CRT_fmode = _O_BINARY;
#endif

int
main (int argc, char **argv)
{
  MS3Record *msr = NULL;
  uint32_t flags = 0;
  int32_t sinedata[SINE_DATA_SAMPLES];
  double dsinedata[SINE_DATA_SAMPLES];
  int idx;
  int rv;

  /* Create integer and double sine data sets */
  for (idx = 0; idx < SINE_DATA_SAMPLES; idx++)
  {
    sinedata[idx] = (int32_t)(fsinedata[idx]);
    dsinedata[idx] = (double)(fsinedata[idx]);
  }

  /* Process command line arguments */
  if (parameter_proc (argc, argv) < 0)
    return -1;

  if (starttime == NSTUNSET)
    starttime = ms_timestr2nstime ("2012-01-01T00:00:00Z");

  if (!(msr = msr3_init (msr)))
  {
    ms_log (2, "Could not allocate MS3Record, out of memory?\n");
    return -1;
  }

  /* Set up record parameters */
  strcpy (msr->sid, "FDSN:XX_TEST__X_Y_Z");
  msr->reclen = reclen;
  msr->pubversion = 1;
  msr->starttime = starttime;
  msr->samprate = samprate;
  msr->encoding = encoding;

  if (encoding == DE_TEXT)
  {
    msr->numsamples  = strlen (textdata);
    msr->datasamples = textdata;
    msr->sampletype  = 't';
  }
  else if (encoding == DE_FLOAT32)
  {
    msr->numsamples = SINE_DATA_SAMPLES;
    msr->datasamples = fsinedata;
    msr->sampletype  = 'f';
  }
  else if (encoding == DE_FLOAT64)
  {
    msr->numsamples = SINE_DATA_SAMPLES;
    msr->datasamples = dsinedata;
    msr->sampletype  = 'd';
  }
  else if (encoding == DE_INT16)
  {
    msr->numsamples  = 220; /* The first 220 samples can be represented in 16-bits */
    msr->datasamples = sinedata;
    msr->sampletype  = 'i';
  }
  else if (encoding == DE_INT32)
  {
    msr->numsamples  = SINE_DATA_SAMPLES;
    msr->datasamples = sinedata;
    msr->sampletype  = 'i';
  }
  else if (encoding == DE_STEIM1)
  {
    msr->numsamples  = SINE_DATA_SAMPLES;
    msr->datasamples = sinedata;
    msr->sampletype  = 'i';
  }
  else if (encoding == DE_STEIM2)
  {
    msr->numsamples  = SINE_DATA_SAMPLES - 1; /* Steim-2 can represent all but the last difference */
    msr->datasamples = sinedata;
    msr->sampletype  = 'i';
  }
  else
  {
    ms_log (2, "Unsupported encoding: %d\n", encoding);
    return -1;
  }

  msr->samplecnt = msr->numsamples;

  /* Set data flush flag */
  flags |= MSF_FLUSHDATA;

  /* Set miniSEED v2 if requested */
  if (msformat == 2)
    flags |= MSF_PACKVER2;

  rv = msr3_writemseed (msr, outfile, 1, flags, verbose);

  if (rv < 0)
    ms_log (2, "Error (%d) writing miniSEED to %s\n", rv, outfile);

  /* Make sure everything is cleaned up */
  msr->datasamples = NULL;
  msr3_free (&msr);

  return 0;
} /* End of main() */

/***************************************************************************
 * parameter_proc:
 *
 * Process the command line arguments.
 *
 * Returns 0 on success, and -1 on failure
 ***************************************************************************/
static int
parameter_proc (int argcount, char **argvec)
{
  int optind;

  /* Process all command line arguments */
  for (optind = 1; optind < argcount; optind++)
  {
    if (strcmp (argvec[optind], "-V") == 0)
    {
      ms_log (1, "%s version: %s\n", PACKAGE, VERSION);
      exit (0);
    }
    else if (strcmp (argvec[optind], "-h") == 0)
    {
      usage ();
      exit (0);
    }
    else if (strncmp (argvec[optind], "-v", 2) == 0)
    {
      verbose += strspn (&argvec[optind][1], "v");
    }
    else if (strcmp (argvec[optind], "-F") == 0)
    {
      msformat = strtol (argvec[++optind], NULL, 10);
    }
    else if (strcmp (argvec[optind], "-r") == 0)
    {
      reclen = strtol (argvec[++optind], NULL, 10);
    }
    else if (strcmp (argvec[optind], "-R") == 0)
    {
      samprate = strtod (argvec[++optind], NULL);
    }
    else if (strcmp (argvec[optind], "-e") == 0)
    {
      encoding = strtol (argvec[++optind], NULL, 10);
    }
    else if (strcmp (argvec[optind], "-s") == 0)
    {
      starttime = ms_timestr2nstime (argvec[++optind]);

      if (starttime == NSTERROR)
      {
        ms_log (2, "Invalid start time: %s\n", argvec[optind]);
        exit (1);
      }
    }
    else if (strcmp (argvec[optind], "-o") == 0)
    {
      outfile = argvec[++optind];
    }
    else
    {
      ms_log (2, "Unknown option: %s\n", argvec[optind]);
      exit (1);
    }
  }

  /* Make sure an outfile was specified */
  if (!outfile)
  {
    ms_log (2, "No output file was specified\n\n");
    ms_log (1, "Try %s -h for usage\n", PACKAGE);
    exit (1);
  }

  if (msformat != 2 && msformat != 3)
  {
    ms_log (1, "Specified format must be 2 or 3, version %d is not supported\n", msformat);
    exit (1);
  }

  /* Report the program version */
  if (verbose)
    ms_log (1, "%s version: %s\n", PACKAGE, VERSION);

  return 0;
} /* End of parameter_proc() */

/***************************************************************************
 * usage:
 * Print the usage message and exit.
 ***************************************************************************/
static void
usage (void)
{
  fprintf (stderr, "%s version: %s\n\n", PACKAGE, VERSION);
  fprintf (stderr, "Usage: %s [options] -o outfile\n\n", PACKAGE);
  fprintf (stderr,
           " ## Options ##\n"
           " -V             Report program version\n"
           " -h             Show this usage message\n"
           " -v             Be more verbose, multiple flags can be used\n"
           " -F format      Specify miniSEED version format, default is v3\n"
           " -r bytes       Specify maximum record length in bytes, default 4096\n"
           " -R samprate    Specify sample rate, default is 1.0\n"
           " -e encoding    Specify encoding format\n"
           " -s starttime   Specify the start time, default is 2012-01-01T00:00:00Z\n"
           "\n"
           " -o outfile     Specify the output file, required\n"
           "\n"
           "This program packs static, test data into miniSEED\n"
           "\n");
} /* End of usage() */
