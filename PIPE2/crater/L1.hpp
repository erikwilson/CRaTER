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

#ifndef _L1_HPP_
#define _L1_HPP_

/*
  Describes how the packet values are seperated from the packed binary form.
*/
class L1 {
public:

  class packetData {
  public:
    headerInfo* info;
    byte* packet;
    vector<string> names;
    vector<field> fields;
    map<string,byte4> digital;
    map<string,double> analog;
    int appid;

    /*
      Digital to analog mechanism.
    */
    void convertDigital(const byte4 &digital, const field &f) {      
      switch (f.coefType) {
      case  0 :
      case '0':
	break;
      case '1': analog[f.alias] = hk_table.coef[f.alias]*digital;
	break;
      case '2': analog[f.alias] = hk_table.coef[f.alias+'0']*digital - hk_table.coef[f.alias+'1'];
	break;
      case '3': analog[f.alias] = 100*analog["V5plus"] - hk_table.coef[f.alias+'0']*digital - hk_table.coef[f.alias+'1'];
	break;
      case '4': analog[f.alias] = hk_table.coef[f.alias+'0']*((digital+4 - int(hk_table.coef[f.alias+'1']))>>3);
	break;
      default:
	throwError("Unknown Coefficient Type");
      }
    }

    /*
      Reads values from a packet.
    */
    packetData& getValues(headerInfo &thisInfo, byte* thisPacket) {
      info=&thisInfo;
      packet=thisPacket;
      
      for (int i=0; i<names.size(); i++) {
	byte4 d = getNum(packet,fields[i]);
	digital[names[i]]=d;
	convertDigital(d,fields[i]);
      }
      return *this;
    }

    /*
      Define what kind of L1 data this is.
    */
    void setId( const int &id ) {
      appid=id;
      names.clear();
      fields.clear();
      for (int i=0; i<apps.order[appid].size(); i++) {	
	const string &name=apps.order[appid][i];
	const field &f=apps.id[appid][name];
	if (f.printWidth) {
	  names.push_back(name);
	  fields.push_back(f);
	}
      }      
    }

    /*
      Output data according to field specifications.
    */
    void dataOut (const int &level, ostream& os) {
      for (int i=0; i<names.size(); i++) {
	field &f = fields[i];
	if (level >= f.level) {
	  if (f.notation=='F') os << fixed;
	  if (f.notation=='S') os << scientific;
	  if (f.precision!=-1) os << setprecision(f.precision);
	  os << "," << setw(f.printWidth);
	  os << (f.coefType ? analog[f.alias] : digital[names[i]]);
	}
      }  
    }
  };

  /*
    Housekeeping L1.
  */
  class hk:public packetData {
  public:
    hk() {
      setId(appId.CRATER_HK);
    }
  };

  /*
    Secondary Science L1.
  */
  class sec:public packetData {
  public:
    sec() {
      setId(appId.CRATER_SEC);
    }
  };

  /*
    Primary Science L1.
  */
  class pri {
  public:
    headerInfo* info;
    byte* packet;
    vector<unsigned*> amplitude;
    vector<double*> energy;
    ushort nEvents;
    vector<field> bits;
    hk* h1;

    vector<double> detCorOffset, detCorSlope, detCorTGain;

    /*
      Pre-populate our coefficients.
    */
    pri() {
      for (int i=0; i<6; i++) {
	bits.push_back(apps.id[appId.CRATER_PRI][string("CREVTAMPDET")+(char)(i+'1')]);
	detCorOffset.push_back(hk_table.coef[string("DetCorOffset")+(char)(i+'0')]);
	detCorSlope.push_back(hk_table.coef[string("DetCorSlope")+(char)(i+'0')]);
	detCorTGain.push_back(hk_table.coef[string("DetCorTGain")+(char)(i+'0')]);
      }
   }

    /*
      Populate L1 values from L0 data.
    */
    pri& getValues(headerInfo &thisInfo, byte* thisPacket, hk &thisH1) {
      info=&thisInfo;
      packet=thisPacket;
      h1 = &thisH1;

      double dtemp = h1->analog["Ttelescope"] - hk_table.coef["DetCorTZero"];
      vector<field> b = bits;
      nEvents=(info->length-12)/9;

      if (energy.size()<nEvents or amplitude.size()<nEvents) {
	energy.resize(nEvents);
	amplitude.resize(nEvents);
      }

      for (int i=0; i<nEvents; i++) {
	if (!amplitude[i]) amplitude[i] = new unsigned[6];
	if (!energy[i]) energy[i] = new double[6];

	for (int j=0; j<6; j++) {
	  unsigned a = getBits<byte2>(packet,b[j]);
	  double e = detCorOffset[j] + a * (detCorSlope[j] * (1 + (dtemp * detCorTGain[j])));
	  amplitude[i][j]=a;
	  energy[i][j]=(e>0.0?e:0.0);
	  b[j].byte+=9;
	}
      }
      return *this;
    }

    /*
      Output values.
    */
    void dataOut (ostream& os, const int &event) {
      for (int j=0; j<6; j++)
	os << "," << setw(4) << amplitude[event][j];
      os << scientific << setprecision(4);
      for (int j=0; j<6; j++)
	os << "," << setw(10) << energy[event][j];
    }

  };

};

/*
  Output primary science.
 */
ostream& operator<< (ostream& os, L1::pri &p) { 
  for (int i=0;i<p.nEvents;i++) {    
    os << p.info->time << "," << setw(6) << i;
    p.dataOut(os,i);
    os << fixed << setprecision(0) << endl;
  }
  return os;
}

/*
  Output secondary science and housekeeping.
 */
ostream& operator<<(ostream& os, L1::packetData& p)
{
  if ( p.info->time.secs ) {
    os << p.info->time;
    p.dataOut(1,os);
    os << fixed << setprecision(0) << endl;
  }
  return os;
}

#endif //_L1_HPP_
