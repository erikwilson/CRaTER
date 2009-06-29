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

#ifndef _PDS_HPP_
#define _PDS_HPP_

#define CRATER_VERSION "1.0"

/*
  Return a string in PDS time formate from a given clock.
*/
string pdsTime(const clock& sc) {
  stringstream ss;
  ss << setfill('0') << right << setw(10) << sc.secs
     << '.' << setw(2) << (ushort)(sc.fract*100)/16 << left << setfill(' ');
  return ss.str();
}

/*
  Write the Level 0/1/2 PDS labels
*/
void writePdsLabels(const int *rawCount, const int eventCount, const clock *first, const clock *last)
{


  
  static const char* productIds[] = { "LRO-L-CRAT-2-EDR-RAWDATA-V1.0",
				      "LRO-L-CRAT-3-CDR-CALIBRATED-V1.0",
				      "LRO-L-CRAT-3/4-DDR-PROCESSED-V1.0" };

  static const char* names[] = { "LRO MOON CRATER EDR RAWDATA V1.0",
				 "LRO MOON CRATER 3 CALIBRATED ENERGY DATA V1.0",
				 "LRO MOON CRATER 3/4 CALIBRATED LET DATA V1.0" };

  /*
    Rows are level (0/1/2) and columns are type (pri/sec/hk).
  */

  static const char* dataIds[][3] = { { "CRAT_L0_PRI", "CRAT_L0_SEC", "CRAT_L0_HK" },
				      { "CRAT_L1_PRI", "CRAT_L1_SEC", "CRAT_L1_HK" },
				      { "CRAT_L2_PRI", "CRAT_L2_SEC", "CRAT_L2_HK" } };

  static const int lrecl[][3] = { { 444, 46, 64 },
				  { 117, 136, 202 },
				  { 240, 194, 329 } };

  static const int columns[][3] = { { 6, 9, 22 },
				    { 5, 17, 20 },
				    { 10, 19, 28 } };
  
  static const char* types[3] = { "PRI", "SEC", "HK" };
					
  char *indxFile = cratFile(-1,"INDEX",config.indxDir);
  fstream indxof(indxFile,fstream::out|fstream::trunc);
  if (!indxof.good()) throwError("Unable to open " << indxFile);      

  static char buf[32];
  for (int l=0;l<3;l++) {
    for (int t=0;t<3;t++) {

      stringstream indxFile;
      indxFile << "DATA/" << config.year << "/" << config.yyyyddd << "/" <<
	basename(toUpper(cratFile(l,types[t],config.outDir)));
      char *date = nowDate(); date[10]=0;
      indxof << left << setfill(' ') << '"' << setw(31) << string(cratName(l,types[t]))+(l?".TAB":".DAT") << "\"";
      indxof << ",\"" << setw(50) << indxFile.str() << "\"";
      indxof << "," << setw(23) << spiceDate(first[t]);
      indxof << "," << setw(23) << spiceDate(last[t]);
      indxof << ",\"" << setw(12) << dataIds[l][t] << "\"";
      indxof << ",\"" << setw(40) << productIds[l] << "\"";
      indxof << ",\"" << setw(10) << date << "\"" << endl;

      char *lblFile = cratFile(l,types[t],config.outDir,true);

      fstream lblof(lblFile,fstream::out|fstream::trunc);
      if (!lblof.good()) throwError("Unable to open " << lblFile);

      lblof << "PDS_VERSION_ID               = PDS3" << endl;
      lblof << "DATA_SET_ID                  = \"" << productIds[l] << '"' << endl;
      lblof << "DATA_SET_NAME                = \"" << names[l] << '"' << endl;
      lblof << "STANDARD_DATA_PRODUCT_ID     = \"" << dataIds[l][t] << '"' << endl;
      lblof << "PRODUCT_ID                   = \"" << cratName(l,types[t]) << '"' << endl;
      lblof << "PRODUCT_TYPE                 = " << (l?"RDR":"EDR") << endl;
      lblof << "PRODUCT_VERSION_ID           = \"" << CRATER_VERSION << "\"" << endl;
      lblof << "PRODUCT_CREATION_TIME        = " << nowDate() << endl;
      lblof << "MISSION_PHASE_NAME           = " << config.phase << endl;
      lblof << endl;
      lblof << "RECORD_TYPE                  = " << (l or t?"FIXED_LENGTH":"UNDEFINED") << endl;
      if (l or t)
	lblof << "RECORD_BYTES                 = " << lrecl[l][t] << endl;
      lblof << "FILE_RECORDS                 = " << (l&&t==0?eventCount:rawCount[t])+(l?0:(t==1?2:t==2?1:0)) << endl;
      lblof << endl;
      lblof << "START_TIME                   = " << spiceDate(first[t]) << endl;
      lblof << "STOP_TIME                    = " << spiceDate(last[t]) << endl;
      lblof << "SPACECRAFT_CLOCK_START_COUNT = \"" << pdsTime(first[t]) << '"' << endl;
      lblof << "SPACECRAFT_CLOCK_STOP_COUNT  = \"" << pdsTime(last[t]) << '"' << endl;
      lblof << endl;
      lblof << "INSTRUMENT_HOST_NAME         = \"Lunar Reconnaissance Orbiter\"" << endl;
      lblof << "INSTRUMENT_HOST_ID           = \"LRO\"" << endl;
      lblof << "INSTRUMENT_NAME              = \"Cosmic Ray Telescope for the Effects of" << endl;
      lblof << "                               Radiation\"" << endl;
      lblof << "INSTRUMENT_ID                = \"CRAT\"" << endl;
      lblof << "INSTRUMENT_SERIAL_NUMBER     = " << config.serial << endl;
      lblof << "DESCRIPTION                  = \"The Cosmic Ray Telescope for the Effects of" << endl;
      lblof << "                               Radiation (CRaTER) is a stacked detector-" << endl;
      lblof << "                               absorber cosmic-ray telescope designed to" << endl;
      lblof << "                               answer key questions to enable future human" << endl;
      lblof << "                               exploration of the Solar System.  CRaTER's" << endl;
      lblof << "                               primary measurement goal is to measure" << endl;
      lblof << "                               directly the lineal energy transfer (LET or" << endl;
      lblof << "                               'y') spectra caused by space radiation" << endl;
      lblof << "                               penetrating and interacting with shielding" << endl;
      lblof << "                               material.  Such measured LET spectra are" << endl;
      lblof << "                               frequently unavailable.  In the absence of" << endl;
      lblof << "                               measurements, numerical models are used to" << endl;
      lblof << "                               provide estimates of LET--the reliability of" << endl;
      lblof << "                               the models require experimental measurements" << endl;
      lblof << "                               to provide a ground truth.\"" << endl;
      lblof << endl;

      //add_pdslabel_note(os, str);			// add an optional note
      
      if (l==0) {
	lblof << "^TABLE                       = \"" << cratName(l,types[t]) << ".DAT\"" << endl;
	lblof << "OBJECT                       = TABLE" << endl;
	lblof << "    NAME                     = LROHDR" << endl;
	lblof << "    INTERCHANGE_FORMAT       = BINARY" << endl;
	lblof << "    ROW_BYTES                = 64" << endl;
	lblof << "    ROWS                     = 1" << endl;
	lblof << "    COLUMNS                  = 7" << endl;
	lblof << "    ^STRUCTURE               = \"LROHDR.FMT\"" << endl;
	lblof << "    DESCRIPTION              = \"LRO standard 64-byte header.\"" << endl;
	lblof << "END_OBJECT                   = TABLE" << endl;
	lblof << endl;
	lblof << "^TABLE                       = (\"" << cratName(l,types[t]) << ".DAT\",";
	lblof << (t==0?"64 <BYTES>":(t==1?"3":(t==2?"2":""))) << ")" << endl;
      } else {
	lblof << "^TABLE                       = \"" << cratName(l,types[t]) << ".TAB\"" << endl;
      }

      lblof << "OBJECT                       = TABLE" << endl;
      lblof << "    NAME                     = " << dataIds[l][t] << endl;
      lblof << "    INTERCHANGE_FORMAT       = " << (l?"ASCII":"BINARY") << endl;
      lblof << "    ROWS                     = " << (l&&t==0?eventCount:rawCount[t]) << endl;
      lblof << "    COLUMNS                  = " << columns[l][t] << endl;
      lblof << "    ROW_BYTES                = " << lrecl[l][t] << endl;
      lblof << "    ^STRUCTURE               = \"" << dataIds[l][t] << ".FMT\"" << endl;
      lblof << "    DESCRIPTION              = ";

      lblof << "\"CRaTER Instrument";
      lblof << (l?string(" Level ") + (char)(l+'0') + " ":" ")
	 << (t==0?"Primary Science":"") 
	 << (t==1?"Secondary Science":"") 
	 << (t==2?"Housekeeping":"") 
	 << (l?".":" packets.")
	 << (l or t?"\"":"") << endl;

      if (l==0 and t==0) {
	lblof << "                               The byte length of each varying-length packet" << endl;
	lblof << "                               is 7 plus the value of the fourth 16-bit" << endl;
	lblof << "                               unsigned MSB integer in the packet.\"" << endl;
      }

      lblof << "END_OBJECT                   = TABLE" << endl;
      lblof << endl;
      lblof << "END" << endl;
	
      lblof.close();
    }
  }
  indxof.close();
}

#endif //_PDS_HPP_
