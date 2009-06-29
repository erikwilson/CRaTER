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

#ifndef _L2_HPP_
#define _L2_HPP_

class L2 {
public:

  /*
    L2 primary science calculates LLD Energy and LET energy from L1 primary science.
   */
  class pri {
  public:
    vector<double> lldOffset,lldSlope,detThickness;
    vector<double*> let;
    vector<vector<bool>*> flags;
    double dqi;
    L1::pri *p1;

    pri() {
      dqi=0;
      for (int j=0; j<6; j++) {
	lldOffset.push_back(hk_table.coef[string("LLDoffset")+(char)(j+'0')]);
	lldSlope.push_back(hk_table.coef[string("LLDslope")+(char)(j+'0')]);
	detThickness.push_back(hk_table.coef[string("DetThickness")+(char)(j+'0')]);
      }
    }

    /*
      Apply coefficients read from file.
     */
    pri& getValues(L1::pri &thisP1) {
      p1=&thisP1;
      double volts[2] = { p1->h1->analog["LLDThin"], p1->h1->analog["LLDThick"] };

      if (let.size() < p1->nEvents) let.resize(p1->nEvents);
      if (flags.size() < p1->nEvents) flags.resize(p1->nEvents);

      for (int i=0; i<p1->nEvents; i++) {
	if (!let[i]) let[i] = new double[6];
	if (!flags[i]) flags[i] = new vector<bool>(12,false);
	for (int j=0; j<6; j++) {
	  double lldEnergy = lldOffset[j] + volts[j & 1]*lldSlope[j];
	  double thisLET = p1->energy[i][j]/detThickness[j];
	  let[i][j] = (thisLET>0 ? thisLET : 0);
	  (*flags[i])[j] = p1->energy[i][j] >= lldEnergy;
	  (*flags[i])[j+6] = p1->amplitude[i][j] > 3890;
	}
      }
      return *this;
    }
  };


  /*
    L2 secondary science adds moon vector from L1 secondary science time.
   */
  class sec {
  public:
    L1::sec *s1;

    double MoonVec[3];

    sec& getValues(L1::sec &thisS1) {
      s1=&thisS1;

      SpiceDouble lt, et, moonvec[6]={-1,-1,-1,0,0,0};

      if (config.useSpice) {
#ifdef HAVE_SPICE
	et = getEt(s1->info->time);
	spkezr_c(SPACECRAFT, et, "MOON_ME", "NONE", "MOON", moonvec, &lt);
	checkSpice("spkezr_c: MOON->SPACECRAFT (MOON_ME)");
#endif //HAVE_SPICE
      }

      for (int nn = 0; nn < 3; nn++) {
	MoonVec[nn]=moonvec[nn];
      }
      return *this;
    }
  };

  /*
    L2 housekeeping adds bias energies, moon boresight and eclipse flags, and radiation counter.
   */
  class hk {
  public:
    L1::hk *h1;

    double biasEnergy[6];
    bool offMoon, eclipse;
    double radTotal;

    hk& getValues(L1::hk &thisH1) {
      h1 = &thisH1;

      if (h1->info->time.secs) {

	SpiceDouble cmat[3][3], spoint[3], dist, trgepc, 
	  obspos[3], state[6], lt, et;
	SpiceBoolean found;

	for (int nn = 0; nn < 6; nn++) {
	  double volts = ((nn & 1) ? h1->analog["LLDThick"] : h1->analog["LLDThin"] );
	  double energy = hk_table.coef[string("LLDoffset")+(char)(nn+'0')] +
	    volts * hk_table.coef[string("LLDslope")+(char)(nn+'0')];
	  biasEnergy[nn] = (energy>0 ? energy : 0);
	}      

	if (config.useSpice) {
#ifdef HAVE_SPICE
	  et = getEt(h1->info->time);
	  // get boresight intercept point, if any
	  pxform_c("J2000", INSTRUMENT, et, cmat);
	  checkSpice("pxform_c: J2000 -> INSTRUMENT");
	  sincpt_c("Ellipsoid", "MOON", et, "MOON_PA", "NONE", SPACECRAFT, "J2000",
		   cmat[2], spoint, &trgepc, obspos, &found);
	  checkSpice("sincpt_c: SPACECRAFT -> MOON");
	  offMoon = (found==SPICEFALSE);
	  
	  // get S/C to Sun vector and check for lunar eclipse
	  spkezr_c("SUN", et, "J2000", "NONE", SPACECRAFT, state, &lt);
	  checkSpice("spkezr_c: SPACECRAFT -> SUN");
	  sincpt_c("Ellipsoid", "MOON", et, "MOON_PA", "NONE", SPACECRAFT, "J2000",
		   state, spoint, &trgepc, obspos, &found);
	  checkSpice("sincpt_c: SPACECRAFT -> SUN");
	  eclipse = (found==SPICETRUE);
#endif //HAVE_SPICE
	} else {
	  offMoon=SPICEFALSE;
	  eclipse=SPICETRUE;
	}

	radTotal = ( config.radPrior +
		     h1->analog["RadHighSens"] + 
		     h1->analog["RadMedSens"] + 
		     h1->analog["RadLowSens"] );
      }      
      return *this;
    }
  };
};

/*
  Format to output L2 primary science.
 */
ostream& operator<< (ostream& os, L2::pri &p) {
  char *date = spiceDate(p.p1->info->time);
  date[19] = 0;

  for (int i=0;i<p.p1->nEvents;i++) {
    os << p.p1->info->time
       << ",\"" << date << "\""
       << "," << setw(6) << i;
    p.p1->dataOut(os,i);
    for (int j=0; j<6; j++)
      os << "," << setw(10) << p.let[i][j];
    os << "," << setw(10) << p.dqi;
    for (int j=0; j<(*p.flags[i]).size(); j++)
      os << "," << (*p.flags[i])[j];
    os << fixed << setprecision(0) << endl;
  }
  return os;
}


/*
  Format to output L2 secondary science.
 */
ostream& operator<< (ostream& os, L2::sec &s) {
  char *date = spiceDate(s.s1->info->time);
  date[19] = 0;
  
  os << s.s1->info->time
     << ",\"" << date << "\"";

  s.s1->dataOut(2,os);

  os << scientific << setprecision(4);
  for (int nn = 0; nn < 3; nn++) {
    os << "," << setw(11) << s.MoonVec[nn];
  }

  return os << fixed << setprecision(0) << endl;
}


/*
  Format to output L2 housekeeping.
 */
ostream& operator<< (ostream& os, L2::hk &h) {
  if ( h.h1->info->time.secs ) {
    char *date = spiceDate(h.h1->info->time);
    date[19] = 0;
  
    os << h.h1->info->time
       << ",\"" << date << "\"";

    h.h1->dataOut(2,os);
  
    os << "," << setw(10) << h.radTotal;

    for (int nn = 0; nn < 6; nn++) {
      os << "," << setw(10) << h.biasEnergy[nn];
    }

    os << "," << setw(1) << h.offMoon;
    os << "," << setw(1) << h.eclipse;
    
    os << fixed << setprecision(0) << endl;
  }
  return os;
}

#endif //_L2_HPP_
