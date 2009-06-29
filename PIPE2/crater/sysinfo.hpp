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

#ifndef _SYSINFO_HPP_
#define _SYSINFO_HPP_

#include <sys/resource.h>
#include <sys/time.h>

/*
  Returns some information about system resource usage.
*/
string sysInfo(char *str) {
  
  static struct {
    struct	timeval	tv;	// time-of-day
    int	utime[2];	// user CPU time
    int	stime[2];	// system CPU time
    int	rss1[2];	// memory usage
    int	io[2];		// i/o blocks
    int	pages[2];	// page faults
    int	rss2[3];	// memory usage
  } R0, R1;
  
  struct rusage r1, r2;		// user and system CPU time
  
  // First time through, save date and return
  if (!str) {
    gettimeofday(&R0.tv, 0);
    return string("");
  }
  
  // Get system and user resource records
  if (getrusage(RUSAGE_SELF, &r1)==-1) throwError("WHO AM I");
  if (getrusage(RUSAGE_CHILDREN, &r2)==-1) throwError("CHILDLESS!");
  R1.utime[0] = r1.ru_utime.tv_sec + r2.ru_utime.tv_sec;
  R1.utime[1] = r1.ru_utime.tv_usec + r2.ru_utime.tv_usec;
  R1.stime[0] = r1.ru_stime.tv_sec + r2.ru_stime.tv_sec;
  R1.stime[1] = r1.ru_stime.tv_usec + r2.ru_stime.tv_usec;
  
  // Open an internal read/write stream
  stringstream ss(stringstream::in | stringstream::out);
  
  // Add output title
  ss << right << setfill('0') << str << ": ";
  
  // Report user CPU time
  int sec = R1.utime[0]-R0.utime[0];
  int usec = R1.utime[1]-R0.utime[1];
  if (usec < 0) sec--, usec += 1000000;
  else if (usec > 1000000) sec++, usec -= 1000000;
  ss << sec << "." << usec/100000 << "s ";
  
  // Report system CPU time
  sec = R1.stime[0]-R0.stime[0];
  usec = R1.stime[1]-R0.stime[1];
  if (usec < 0) sec--, usec += 1000000;
  else if (usec > 1000000) sec++, usec -= 1000000;
  ss << sec << "." << usec/100000 << "s ";
  
  // Report elapsed time
  gettimeofday(&R1.tv, 0);
  int ms = (R1.tv.tv_sec-R0.tv.tv_sec)*100 + (R1.tv.tv_usec-R0.tv.tv_usec)/10000;
  
  int ll = ms/100;
  int ii = ll/3600;
  if (ii && ms > 0) {
    ii = ll % 3600;
    ss << ms/360000 << ":" << setw(2) << (ii/60) % 100;
  } else {
    ii = ll;
    ss << ii/60;
  }
  ss << ":" << setw(2) << (ii % 60) % 100 << " ";	
  int tt = (R1.utime[0] + R1.stime[0] - R0.utime[0] - R0.stime[0])*100+
    (R1.utime[1] + R1.stime[1] - R0.utime[1] - R0.stime[1])/10000;
  ss << int(tt*100 / ((ms ? ms : 1))) << "% ";
  
  // Report shared/unshared memory size
  R1.rss1[0] = r1.ru_ixrss + r2.ru_ixrss;
  R1.rss1[1] = r1.ru_isrss + r2.ru_isrss;
  ss << (tt ? (R1.rss1[0]-R0.rss1[0])/tt : 0) << "+";
  ss << (tt ? (R1.rss1[0]-R0.rss1[0]+R1.rss1[1]-R0.rss1[1])/tt : 0) << "k ";
  
  // Report input/output block count
  R1.io[0] = r1.ru_inblock + r2.ru_inblock;
  R1.io[1] = r1.ru_oublock + r2.ru_oublock;
  ss << R1.io[0]-R0.io[0] << "+" << R1.io[1]-R0.io[1] << "io ";
  
  // Report page faults/swaps
  R1.pages[0] = r1.ru_majflt + r2.ru_majflt;
  R1.pages[1] = r1.ru_nswap + r2.ru_nswap;
  ss << R1.pages[0]-R0.pages[0] << "pf+" << R1.pages[1]-R0.pages[1] << "s ";
  
  // Report total used/maximum memory and reclaims
  R1.rss2[0] = R1.rss1[0] + R1.rss1[1] + r1.ru_idrss + r2.ru_idrss;
  R1.rss2[1] = r1.ru_maxrss;
  R1.rss2[2] = r1.ru_minflt + r2.ru_minflt;
  ss << (tt ? (R1.rss2[0]-R0.rss2[0])/tt : 0);
  ss << "ss+" << (R1.rss2[1]-R0.rss2[1])/2 << "m+" << R1.rss2[2]-R0.rss2[2] << "r" << endl;

  // Remember current stats
  R0 = R1;

  return ss.str();
}

#endif //_SYSINFO_HPPP_
