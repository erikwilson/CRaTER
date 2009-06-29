/*
 The MIT License

 Copyright (c) 2010 Erik Wilson

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

#ifndef _TYPES_HPP_
#define _TYPES_HPP_

#define throwError(msg) { \
    stringstream errss; \
    errss << endl << msg << endl << "ERROR AT [" << __FILE__ << ":" << __LINE__ << "] ABORTING!" << endl << endl; \
    cerr << errss.str(); \
    logof << errss.str(); \
    logof.close(); \
    throw; }

#ifndef REVISION
#define REVISION "unknown"
#endif

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef	double FLOAT;

typedef uchar byte;
typedef char* Sbyte;
typedef ushort byte2;
typedef uint byte4;
typedef unsigned long long uint64;

struct pipeConfig {
  string proc;
  string dirName;
  string outDir;
  string indxDir;
  string gapDir;
  string hkFile;
  string appidFile;
  string phaseFile;
  string logDir;
  string labelNote;
  vector<string> phases;
  vector<string> spice;
  vector<string> files;
  string phase;
  double radPrior;
  int gapVal[3];
  int verbose;
  int serial;
  int version;
  int year;
  int doy;
  int yyyyddd;
  int start_t;
  int stop_t;
  int start_delta;
  int stop_delta;
  bool useSpice;
  bool compress;
  bool nullData;
} config;

fstream logof;

#endif  //_TYPES_HPP_
