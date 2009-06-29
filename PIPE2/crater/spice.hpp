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

#ifndef _SPICE_HPP_
#define _SPICE_HPP_

#ifdef HAVE_SPICE
#include "SpiceUsr.h"
#include "SpiceZfc.h"
#include "SpiceZmc.h"
#else
typedef double SpiceDouble;
typedef bool SpiceBoolean;
#define SPICEFALSE false;
#define SPICETRUE true;
#endif //HAVE_SPICE

#define SPACECRAFT "LRO"
#define INSTRUMENT "LRO_CRATER"
#define SPACECRAFTID -85

/*
  Setup default spice error reporting.
*/
struct _spice {
  _spice() {
#ifdef HAVE_SPICE
    erract_c("SET",0,"RETURN");
    errprt_c("SET",0,"NONE");
#endif //HAVE_SPICE
  } 
} spiceDefaults;

/*
  Remove spaces from a string.
*/
char * despace(char *str, int len) {
  while (--len >= 0 && str[len] == ' ') str[len] = 0;
  return str;
}

/*
  Validate a spice command.
*/
void checkSpice(const string &str) {
#ifdef HAVE_SPICE
  if (failed_c() == SPICETRUE) {
    char smsg[1024], lmsg[1024], trace[1024];
    getmsg_c("SHORT", sizeof(smsg), smsg);
    getmsg_c("LONG", sizeof(lmsg), lmsg);
    qcktrc_(trace, sizeof(trace));
    stringstream ss;
    ss << endl;
    ss << str << ": traceback: " << despace(trace, sizeof(trace)) << endl;
    ss << despace(smsg, sizeof(smsg)) << ": ";
    ss << despace(lmsg, sizeof(lmsg)) << endl;
    ss << str << ": " << smsg << endl;
    throwError(ss.str());
    /*
      The spice extends life.
      The spice expands consciousness.
      The spice is vital to space travel.
      The spice exists on only one planet in the entire universe.
    */
  }
#endif //HAVE_SPICE
}

/*
  Furnish a spice file.
*/
void addSpice(const string &file) {
#ifdef HAVE_SPICE
  furnsh_c(file.c_str());
  checkSpice(file);
  stringstream ss;
  ss << "Furnished spice file " << file << endl;
  logof << ss.str();
  if (config.verbose>1) cout << ss.str();
#endif //HAVE_SPICE
}

/*
  Get the spice ephemeris time from a given clock.
*/
SpiceDouble getEt(const clock &t) {
  SpiceDouble et;
#ifdef HAVE_SPICE
  sct2e_c(SPACECRAFTID, (t.secs+t.fract/16.0)*65536.0, &et);
  checkSpice("str2et_c");
#endif //HAVE_SPICE
  return et;
}

/*
  Return a spice string representation of a given clock.
*/
char* spiceDate(const clock &t) {
  if (!config.useSpice) return nowDate(t);
  static char buf[24];
  assert(buf);
#ifdef HAVE_SPICE
  et2utc_c(getEt(t),"ISOC",2,24,buf);
  checkSpice("et2utc");
#endif //HAVE_SPICE
  return buf;
}

#endif //_SPICE_HPP_
