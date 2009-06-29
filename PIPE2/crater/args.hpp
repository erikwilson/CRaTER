/*
 The MIT License

 Copyright (c) 2010 Erik Wilson, Peter Ford

 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following
 conditions:

 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef _ARGS_HPP_
#define _ARGS_HPP_

#include <libgen.h>

// Extract and test caller's arguments
pipeConfig checkArgs(int argc, char * const argv[]) {

  pipeConfig config;

  char sw, ch;
  string dataDir;
  
  config.proc=basename(argv[0]);
  config.dirName=dirname(argv[0]);

  config.yyyyddd = -1;
  config.serial = -1;
  config.radPrior = 0;
  config.version = 1;
  config.phase = "";
  config.gapVal[0] = 2;
  config.gapVal[1] = 2;
  config.gapVal[2] = 20;
  config.compress = false;
  config.nullData = false;
  config.verbose = 0;

  opterr = 1; // write argument errors to stderr	
  config.appidFile = config.dirName+"/conf/appids.txt";
  config.hkFile = config.dirName+"/conf/crat_hk_table_v02.txt";
  config.phaseFile = config.dirName+"/conf/phases.txt";
  config.outDir = config.gapDir = config.logDir = config.indxDir = ".";

  config.start_delta = 0;
  config.stop_delta = 0;

#ifdef HAVE_SPICE
  config.useSpice = true;
#else
  config.useSpice = false;
#endif //HAVE_SPICE

  // examine optional arguments
  while ((sw = getopt(argc, argv, "b:e:a:d:D:g:G:h:l:L:n:N:p:P:r:s:SV:vx:y:zZ?")) != -1) {
    switch (sw) {
    case 'b':
      if (sscanf(optarg, "%d%c", &config.start_delta, &ch) != 1) {
	cerr << config.proc << ": bad start_delta: " << optarg << endl;
	exit(1);
      }
      continue;
    case 'e':
      if (sscanf(optarg, "%d%c", &config.stop_delta, &ch) != 1) {
	cerr << config.proc << ": bad stop_delta: " << optarg << endl;
	exit(1);
      }
      continue;
    case 'a': config.appidFile = optarg;
      continue;
    case 'd': config.outDir = optarg;
      continue;
    case 'D': dataDir = optarg;
      continue;
    case 'g': config.gapDir = optarg;
      continue;
    case 'G': // minimum gap time
      if (sscanf(optarg, "%u,%u,%u%c", &config.gapVal[0], &config.gapVal[1], &config.gapVal[2], &ch) != 3) {
	cerr << config.proc << ": bad gap(s): " << optarg << endl;
	exit(1);
      }
      continue;
    case 'h': config.hkFile = optarg;
      continue;
    case 'l': config.logDir = optarg;
      continue;
    case 'L': config.gapDir = config.indxDir = config.logDir = optarg;
      continue;
    case 'n': // instrument serial number
      if (sscanf(optarg, "%u%c", &config.serial, &ch) != 1 || config.serial < 1 || config.serial > 99) {
	cerr << config.proc << ": bad serial number: " << optarg << endl;
	exit(1);
      }
      continue;
      /*
    case 'N': config.labelNote += ' ' + optarg;
      continue;
      */
    case 'p': config.phaseFile = optarg;
      continue;
    case 'P': config.phases.push_back(optarg);
      continue;
    case 'r': // total radiation dosage
      if (sscanf(optarg, "%f%c", &config.radPrior, &ch) != 1) {
	cerr << config.proc << ": bad prior rad total: " << optarg << endl;
	exit(1);
      }
      continue;
    case 's': config.spice.push_back(optarg);
      continue;
    case 'S': config.useSpice = false;
      continue; 

    case 'V': // version number
      if (sscanf(optarg, "%d%c", &config.version, &ch) != 1 || config.version < 0 || config.version > 99) {
	cerr << config.proc << ": bad version: " << optarg << endl;
	exit(1);
      }
      continue;
    case 'v': config.verbose++;
      continue;
    case 'x': config.indxDir = optarg;
      continue;
    case 'y': // year and day-of-year
      if (sscanf(optarg, "%4d%3d%c", &config.year, &config.doy, &ch) != 2) {
	cerr << config.proc << ": bad year or doy: " << optarg << endl;
      } else if (config.year < 2000 || config.year > 2020) {
	cerr << config.proc << ": bad year: " << optarg << endl;
      } else if (config.doy < 1 || config.doy > 366 || (config.doy == 366 && (config.year % 4) != 0)) {
	cerr << config.proc << ": bad doy: " << optarg << endl;
      } else {
	config.yyyyddd = 1000 * config.year + config.doy;
      }
      continue;
    case 'z': config.compress = true;
      continue;
    case 'Z': config.nullData = true;
      continue;
    case '?': // unknown option
      cerr << "Usage: " << config.proc
	   << " [-v] [-z] [-G n,n,n] [-d OutDir] "
	   << "	[-l LogDir] [-g GapDir] [-x IndexDir] [-L LGIDir] "
	   << "	[-S] [-s SpiceFile] [-a AppidFile] [-h CoefFile] [-p PhaseFile] "
	   << "	[-P phase] [-n serial#] [-r rads] [-V version] "
	   << "	[-D DataDir] -y yyyyddd pathname [pathname ...] " << endl;
      exit(1);
    }
  }

  if (!config.useSpice) {
    cerr << "WARNING: SPICE toolkit is disabled." << endl;
  }

  // Test required parameters
  if (config.yyyyddd == -1) {
    cerr << config.proc << ": missing \"-y yyyyddd\" value" << endl;
    exit(1);
  }

  //  Create a phase name array from files or command line  
  time_t current = getSecs(config.yyyyddd,false) + 12*60*60;
  fstream pf(config.phaseFile.c_str(),fstream::in);
  while (pf.good()) {
    string line;
    vector<string> items;
    vector<string> date;
    time_t t1, t2;
    if (line[0] == '#') continue;
    getline(pf,line);
    items = split(line,':');
    if (items.size()==0) continue;
    if (items.size()!=3) throwError("Phase file wrong format! not (date:date:name)");
    time_t start = getSecs(items[0],false);
    time_t stop  = getSecs(items[1],false) + 24*60*60;
    if (current>start and current<stop)
      config.phases.push_back(items[2]);
  }
  if (config.phases.size()==0) config.phases.push_back("UNKNOWN");
  if (config.phases.size()>1) config.phase += "{";
  for (int i=0; i<config.phases.size(); i++) {
    config.phase += '"' + config.phases[i] + '"';
    if (i!=config.phases.size()-1) config.phase += ", ";    
  }
  if (config.phases.size()>1) config.phase += "}";

  // Add command line files and previous/current/next days if given a data directory
  config.files = vector<string>(argv+optind,argv+argc);
  if (dataDir!="") {
    int s = getSecs(config.yyyyddd);
    char buf[16];

    sprintf(buf,"%d",getYD(s-24*60*60));
    config.files.push_back(dataDir+"/"+buf);

    sprintf(buf,"%d",config.yyyyddd);
    config.files.push_back(dataDir+"/"+buf);

    sprintf(buf,"%d",getYD(s+24*60*60));
    config.files.push_back(dataDir+"/"+buf);
  }

  // return remaining arguments, assumed to be input files or directories
  return config;
}

#endif //_ARGS_HPP_
