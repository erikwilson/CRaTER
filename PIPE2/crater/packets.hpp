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

#ifndef _PACKETS_HPP_
#define _PACKETS_HPP_

const int HEADER=0;

map<ushort,ushort> appOrder;
map<ushort,bool> skipId;

/*
  Create a structure of appid names and populate with info.
*/
struct _names {      // CRATER'S CCSDS packet APID values 
  ushort SC_HK_32;   // SpaceCraft HouseKeeping packets
  ushort SC_HK_48;
  ushort SC_HK_100;
  ushort SC_HK_101;
  ushort SC_HK_106;
  ushort CRATER_HK;  // housekeeping
  ushort CRATER_SEC; // secondary science
  ushort CRATER_PRI; // primary science
  ushort _END;

  _names(const ushort *w, const ushort *sm) {
    ushort* t=(ushort*)this;
    _END=0xffff;
    for (int i=0;w[i];i++) {
      if (t[i]==_END) throwError("Premature name structure end.");
      appOrder[w[i]]=i+1;
      t[i]=w[i];
      skipId[w[i]]=sm[i];
    }
  }
} const appId((ushort[]){ 32, 48, 100, 101, 106, 122, 121, 120, 0 },
	      (ushort[]){  1,  1,   0,   1,   1,   0,   0,   0, 0 });

/*
  Resource to extract basic information from headers or packets.
*/
struct packets {
  map<int, map<string,field> > id;
  map<int, vector<string> > order;
  friend istream& operator>> (istream& is, packets &p);

  packets() {
    id[HEADER]["version"]  = (field){ '1',  0, 0,  3 };    
    id[HEADER]["type"]     = (field){ '1',  0, 3,  1 };
    id[HEADER]["secflag"]  = (field){ '1',  0, 4,  1 };
    id[HEADER]["appid"]    = (field){ '2',  0, 5, 11 };
    id[HEADER]["segflag"]  = (field){ '1',  2, 0,  2 };
    id[HEADER]["seqcount"] = (field){ '2',  2, 2, 14 };
    id[HEADER]["length"]   = (field){ '2',  4, 0, 16 };
    id[HEADER]["time"]     = (field){ 'T',  6, 0, 48 };
    id[HEADER]["serial"]   = (field){ '1', 11, 3,  5 };

    id[HEADER]["f_fileid"] = (field){ '4',  0, 0, 32 };
    id[HEADER]["f_spare"]  = (field){ '4',  4, 0, 32 };
    id[HEADER]["f_start"]  = (field){ 'T',  8, 0, 48 };
    id[HEADER]["f_stop"]   = (field){ 'T', 16, 0, 48 };
    id[HEADER]["f_name"]   = (field){ 'S', 24, 0, 40 };
  }    

  void readInfo(string file) {
    fstream is(file.c_str(),fstream::in);
    if (!is.good()) throwError("can't read pack info file " << file);
    is >> *this;
  }
} apps;

/*
  Read data input/output structures from configuration file.
*/
istream& operator>> (istream& is, packets &p) {
  while (is.good()) {
    vector<string> sv;
    string line;
    field f = {0,0,0,0,0,-1,0,0};
    getline(is,line);
    if (line[0] == '#') continue;
    sv = split(line,'|');
    if (sv.size()==0) continue;
    readSS(sv[0],f.appid);
    readSS(sv[1],f.name);
    readSS(sv[2],f.format);
    readSS(sv[3],f.byte);
    readSS(sv[4],f.bit);
    readSS(sv[5],f.width);
    readSS(sv[6],f.printWidth);
    readSS(sv[7],f.precision);
    readSS(sv[8],f.notation);
    readSS(sv[9],f.coefType);
    readSS(sv[10],f.level);

    if (sv.size()>11)
      readSS(sv[11],f.alias);
    if (f.appid and f.name != "") {
      p.id[f.appid][f.name]=f;
      p.order[f.appid].push_back(f.name);
    };
  }
  return is;
}

/*
  Return value in data from a given field name.
*/
template <class T>
T getBits( const byte* data, const string &name ) {
  return getBits<T>( data, apps.id[HEADER][name] );
}
template <class T>
T getBits( const byte* data, const ushort &appid, const string &name ) {
  return getBits<T>( data, apps.id[appid][name] );
}
uint getNum( const byte* data, const ushort &appid, const string &name ) {
  field &f = apps.id[appid][name];
  return getNum(data,f);
}

bool validAppId(const int &id) { return appOrder.find(id) != appOrder.end(); }

/*
  Structure for storing calibration information.
*/
struct coefficients {
  map<string,double> coef;
  map<string,string> unit;
  int serial;
  friend istream& operator>> (istream& is, coefficients &p);

  void readInfo(string file, int aSerial) {
    fstream is(file.c_str(),fstream::in);
    if (!is.good()) throwError("can't read pack info file " << file);
    serial=aSerial;
    is >> *this;
  }
} hk_table;

bool bracketFilter(char c){ return c=='[' or ']'==c; }

/*
  Required calibration data.
*/
static const char* hk_required[] = { 
  "SCV28bus0", "SCV28bus1", "SCI28bus0", "SCI28bus1", "V5digital", "V5plus", "V5neg", 
  "BiasCurrent0", "BiasCurrent1", "BiasCurrent2", "BiasCurrent3", "BiasCurrent4", "BiasCurrent5", 
  "BiasVoltThin", "BiasVoltThick", "CalAmp", "LLDThin0", "LLDThin1", "LLDThick0", "LLDThick1", 
  "Ttelescope0", "Ttelescope1", "Tanalog0", "Tanalog1", "Tdigital0", "Tdigital1", "Tpower0", "Tpower1", 
  "Tref0", "Tref1", "RadHighSens0", "RadHighSens1", "RadMedSens0", "RadMedSens1", "RadLowSens0", "RadLowSens1", 
  "DetCorOffset0", "DetCorOffset1", "DetCorOffset2", "DetCorOffset3", "DetCorOffset4", "DetCorOffset5", 
  "DetCorSlope0", "DetCorSlope1", "DetCorSlope2", "DetCorSlope3", "DetCorSlope4", "DetCorSlope5", "DetCorTZero", 
  "DetCorTGain0", "DetCorTGain1", "DetCorTGain2", "DetCorTGain3", "DetCorTGain4", "DetCorTGain5", 
  "DetThickness0", "DetThickness1", "DetThickness2", "DetThickness3", "DetThickness4", "DetThickness5", 
  "LLDoffset0", "LLDoffset1", "LLDoffset2", "LLDoffset3", "LLDoffset4", "LLDoffset5", 
  "LLDslope0", "LLDslope1", "LLDslope2", "LLDslope3", "LLDslope4", "LLDslope5", 0 };

/*
  Read calibration file.
*/
istream& operator>> (istream& is, coefficients &c) {
  int nSerials=0,colSerial=0,tmpSerial=0;
  map<string,bool> found;

  while (is.good()) {
    vector<string> sv;
    stringstream ss;
    string line, s;
    getline(is,line);    
    if (line[0] == '#') continue;
    ss.str(line);
    while (ss.good()) {
      ss >> s;
      sv.push_back(s);
    }
    if (!nSerials) {
      if (sv.size()>=2 and sv[0]=="Serials") {
	readSS(sv[1],nSerials);
      }
    } else if (sv.size()>=(nSerials+2)) {
      if (!colSerial) {
	if (sv[0]=="SerialNumber") {
	  for (int i=0;i<nSerials;i++) {
	    readSS(sv[2+i],tmpSerial);
	    if (tmpSerial==c.serial) {
	      colSerial=i+2;
	      break;
	    }
	  }
	}
      } else {
	sv[0].resize( std::remove_if(sv[0].begin(), sv[0].end(), bracketFilter) - sv[0].begin() );
	found[sv[0]] = true;
	readSS(sv[colSerial],c.coef[sv[0]]);
	c.unit[sv[0]]=sv[1];
      }
    }
  }
  if (!colSerial) throwError("Serial " << c.serial << " not found in hk_table");
  string missing = "";
  for ( const char **r = hk_required; *r; r++ )
    if (!found[*r]) missing += " " + string(*r);  
  if (missing != "") 
    { throwError("Missing from hk_table:" << missing); }
  return is;
}

#endif //_PACKETS_HPP_
