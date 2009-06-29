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

#ifndef _L0_HPP_
#define _L0_HPP_

/*
  Header info for a given L0 record.
*/
struct headerInfo {
  ushort appid;
  ushort seqcount;
  ushort length;
  clock time;
  ushort serial;
};

class L0 {
public:
  byte header[12];
  headerInfo info;
  istream* stream;
  streampos position;
  bool valid;
  
  L0() : valid(false) {}

  /*
    Method to compare headers for record indexing.
  */
  bool operator< (const L0& rhs) const {    
    return (info.time < rhs.info.time || 
	    (info.time == rhs.info.time && 
	     appOrder[info.appid] < appOrder[rhs.info.appid]));
  }

  /*
    Check if input stream is at a record.
  */
  bool atHeader(istream &is) {
    byte hdr[2] = { is.get(), is.peek() };
    is.putback(hdr[0]);
    if (hdr[0] == 8 && validAppId(hdr[1]))
      return true;
    else
      return false;
  }

  friend ostream& operator<< (ostream& os, L0 &r);
};

/*
  Compare record contents when comparing pointers.
*/
struct L0PtrCmp {
  bool operator() (const L0* lhs, const L0* rhs) const
  { return (*lhs)<(*rhs); }
};

/*
  Output basic record information.
*/
ostream& operator<< (ostream& os, L0 &r) { 
  os << r.info.appid << ":"
     << r.info.time << ":"
     << r.info.seqcount <<  ":"
     << r.info.length << ":"
     << r.position << ":"
     << r.valid << endl;
  return os;
}

/*
  Read record header info from a file.
*/
istream& operator>> (istream& is, L0 &r) { 
  uint skip=0;
  while (!r.atHeader(is) && is.good()) 
    { is.get(); skip++; }
  if (skip) 
    { cout << "skipped " << skip << " bytes." << endl; }
  if (is.good()) {
    r.stream=&is; r.position=is.tellg(); is.read((Sbyte)r.header,sizeof(r.header));
    if (!is.fail()) {
      r.info = (headerInfo){ getBits<byte2>(r.header,"appid"),
			     getBits<byte2>(r.header,"seqcount"),
			     getBits<byte2>(r.header,"length")+7,
			     getBits<clock>(r.header,"time"),
			     getBits<byte>(r.header,"serial")};
      is.seekg(r.info.length+r.position);
      if (!skipId[r.info.appid]) r.valid=true;
    } else {
      throwError("L0 failed @" << r.position);
    }
  }
  return is;
}

#endif //_L0_HPP_
