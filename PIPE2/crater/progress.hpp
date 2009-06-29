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

#ifndef _PROGRESS_HPP_
#define _PROGRESS_HPP_

/*
  Displays file indexing and output progress on the command line.
  Also displays estimated time to completion.
*/
class progressBar {
public:
  uint64 progSize;
  double lastProg;
  timeval firstTime;
  timeval lastTime;
  char buf[101];

  progressBar( uint64 size ) {
    progSize = size;
    lastProg = 0;
    gettimeofday(&firstTime, NULL);
    memset(buf,0,101);
  }

  int tSubtract (timeval *result, timeval *x, timeval *y)
  {
    timeval z = *y;
    /* Perform the carry for the later subtraction by updating z. */
    if (x->tv_usec < z.tv_usec) {
      int nsec = (z.tv_usec - x->tv_usec) / 1000000 + 1;
      z.tv_usec -= 1000000 * nsec;
      z.tv_sec += nsec;
    }
    if (x->tv_usec - z.tv_usec > 1000000) {
      int nsec = (z.tv_usec - x->tv_usec) / 1000000;
      z.tv_usec += 1000000 * nsec;
      z.tv_sec -= nsec;
    }
    result->tv_sec = x->tv_sec - z.tv_sec;
    result->tv_usec = x->tv_usec - z.tv_usec;

    return x->tv_sec < z.tv_sec;
  }

  double tDouble( timeval *t ) {
    return t->tv_sec + t->tv_usec/1000000.0;    
  }

  void update( uint64 p ) {
    
    double prog=(p*50.0)/(progSize*1.0);
    timeval thisTime, t;
    gettimeofday(&thisTime, NULL);
    tSubtract(&t, &thisTime, &firstTime);
    if (prog>100 or prog<0) throwError( "Got prog: " << (p>progSize?1:0) << ":" << prog );
    if (int(lastProg*20)==int(prog*20) and (thisTime.tv_sec==lastTime.tv_sec and p!=progSize)) return;
    cout << "\r";
    sprintf(buf," %5.1f%% ",prog*2);
    cout << buf << "[";
    //prog/=2;
    memset(buf,'0',(int)prog);
    memset(buf+(int)prog,'.',50-(int)prog);
    if ( (prog-(int)prog)>=0.5 ) memset(buf+(int)prog,'o',1);
    cout << buf;
    cout << "]";
    if (prog>0) { 
      time_t tot=(int)(tDouble(&t)*progSize/p);
      cout << "  e:" << t.tv_sec << "  t:" << tot << "  r: " << tot-t.tv_sec << "   ";
    }
    if (p==progSize) cout << endl;
    cout.flush();
    lastProg=prog;
    lastTime=thisTime;    
  }  
};

#endif //_PROGRESS_HPP_
