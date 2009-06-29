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

#ifndef _TIME_HPP_
#define _TIME_HPP_


struct clock { 
  byte4 secs;
  byte fract; // CRaTER subseconds is actually 4bits

  bool operator< (const clock &rhs) const
  { return ((secs==rhs.secs and fract<rhs.fract) or secs<rhs.secs);  }
  bool operator== (const clock &rhs) const
  { return (secs==rhs.secs and fract==rhs.fract); }
};

/*
  Standard output of CRaTER clock.
*/
ostream& operator<<(ostream& os, const clock &c) {
  os << fixed << right << setprecision(0) << setfill(' ') 
     << setw(9) << c.secs << "," << setfill('0') 
     << setw(2) << (ushort)(c.fract*100)/16 << setfill(' ');
  return os;
}

/*
  Given a yearday, extract year, month, and day of month.
*/
void convertYD( int yd, int &year, int &month, int &day) {
  int calendar[12]={31,28,31,30,31,30,31,31,30,31,30,31};
  int mdays=0;
  int yday=yd%1000;
  year=yd/1000;
  month = 0; day = 0;  

  if ((year%4==0 and year%100!=0) or year%400==0) calendar[1]=29;
  else calendar[1]=28;

  while (yday>(mdays+calendar[month])) mdays+=calendar[month++];
  year -= 1900;
  day=yday-mdays;
}

/*
  Given yearday, return seconds for start of day.
*/
time_t getSecs(int yd, const bool &LROtime=true) {  
  time_t t1, t2;
  struct tm tm = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  convertYD(yd, tm.tm_year, tm.tm_mon, tm.tm_mday);  
  t1 = mktime(&tm);
  t2 = mktime(gmtime(&t1));
  t1 -= (t2-t1);
  if (LROtime) t1 -= 86400*(31*365+8);
  assert(t1>=0);
  return t1;
}

time_t getSecs(string &t, const bool &LROtime=true) {
  time_t t1, t2;
  vector<string> date = split(t,'-');
  if (date.size() != 3) throwError("Date wrong format! not (year-month-day)");
  struct tm tm = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  readSS(date[0],tm.tm_year);
  readSS(date[1],tm.tm_mon);
  readSS(date[2],tm.tm_mday);
  tm.tm_year -= 1900;
  tm.tm_mon  -= 1;
  t1 = mktime(&tm);
  t2 = mktime(gmtime(&t1));
  t1 -= (t2-t1);
  if (LROtime) t1 -= 86400*(31*365+8);
  assert(t1>=0);
  return t1;  
}

/*
  Given seconds, return corresponding yearday.
*/
time_t getYD(int secs) {
  time_t tt = secs+86400*(31*365+8);
  assert(tt);
  tm *ptm = gmtime(&tt);
  assert(ptm);
  return 1000*(1900+ptm->tm_year)+ptm->tm_yday+1;
}

/*
  Return a string of the time/date given by time_t, or null for current time.
*/
char* nowDate(time_t tt=0, const bool &showTime=true) {
  if (!tt) time(&tt);
  assert(tt);
  tm *ptm = gmtime(&tt);
  assert(ptm);
  static char buf[20];
  assert(buf);
  sprintf(buf, "%04d-%02d-%02dT%02d:%02d:%02d",
	  1900+ptm->tm_year, ptm->tm_mon+1, ptm->tm_mday,
	  ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
  if (!showTime) buf[10]=0;
  return buf;
}

char* nowDate(clock t) {
  return nowDate(t.secs + 86400*(31*365+8));
}

#endif //_TIME_HPP_
