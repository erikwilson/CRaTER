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

#ifndef _PIPELINE_HPP_
#define _PIPELINE_HPP_

/*
  Pipeline works by quickly indexing files and record headers into
  a single L0 multiset.  Multiple sources are then read and processed
  simultaneously to produce pri/sec/hk L0, L1, and L2 products.
 */
class Pipeline {
public:
  vector<fstream*> files;
  map<fstream*,string> fileNames;
  multiset<L0*,L0PtrCmp> L0index;
  map<int, map<string,bool> > ydFiles;
  uint64 pipeSize;
  uint64 indexSize;
  string info;

  /* 
     Construct a pipeline with a given configuration.
  */
  Pipeline(const pipeConfig conf) {
    config = conf;

    config.start_t = getSecs(config.yyyyddd) + config.start_delta;
    config.stop_t = config.start_t + 24*60*60 + config.stop_delta;
    
    char* logfile = cratFile(-1,"LOG",config.logDir);
    logof.open(logfile,fstream::out|fstream::trunc);
    if (!logof.good()) throwError("Unable to open " << logfile);
	
    logof << "PDS_VERSION_ID     = PDS3" << endl;
    logof << "RECORD_TYPE        = STREAM" << endl;
    logof << "OBJECT             = TEXT" << endl;
    logof << "NOTE               = \"Pipeline log file.\"" << endl;
    logof << "PUBLICATION_DATE   = " << nowDate(0,false) << endl;
    logof << "END_OBJECT         = TEXT" << endl;
    logof << "END" << endl << endl;


    logof << config.proc << " starting on " << nowDate() << endl << endl;
    // List options in log file
    logof << "Year and day-of-year:        " << config.yyyyddd << " (" << nowDate(getSecs(config.yyyyddd,false),false) << ")" << endl;
    logof << "Product version number:      " << config.version << endl;
    logof << "Mission phase:               " << config.phase << endl;
    logof << "Prior radiation dosage:      " << config.radPrior << endl;
    logof << "Output data file directory:  " << config.outDir << endl;
    logof << "Output log file:             " << logfile << endl;
    logof << "Output gap file:             " << cratFile(-1,"GAPS",config.gapDir) << endl;
    logof << "Output index file:           " << cratFile(-1,"INDEX",config.indxDir) << endl;
    logof << "Appid definitions file:      " << config.appidFile << endl;
    logof << "Phase definitions file:      " << config.phaseFile << endl;
    logof << "Housekeeping parameter file: " << config.hkFile << endl;
    logof << "Minimum gap times:           " 
	  << config.gapVal[0] << " "
	  << config.gapVal[1] << " "
	  << config.gapVal[2] << endl;
    logof << "Instrument serial number:    " << config.serial << endl;
    logof << "Verbose mode:                " << config.verbose << endl;
    logof << "Compression mode (Lvl 1,2):  " << (config.compress ? "on" : "off") << endl;
    logof << "Using SPICE:                 " << (config.useSpice ? "yes" : "no") << endl;
    logof << "Machine byte order:          " << (endian.big ? "big" : "little") << endl;
    logof << "Pipeline build revision:     " << REVISION << endl;
    logof << endl;

    sysInfo(0);

    apps.readInfo(config.appidFile);
    hk_table.readInfo(config.hkFile,config.serial);

    for (int i=0; i<config.spice.size(); i++) 
      addSpice(config.spice[i].c_str());

    addFiles(config.files);

    logof << (info = sysInfo("Ready to index")) << endl;
    if (config.verbose) cout << info << endl;
  }

  /*
    Add a vector of files to be processed.
   */
  void addFiles(vector<string> &files) { 
    for (vector<string>::iterator f = files.begin(); f<files.end(); f++) 
      if (isDir(*f)) addFiles(expandPath(*f));
      else addFile(*f);
  }

  /*
    Add a specific file to be processed, check for data or spice.
   */
  void addFile(const string file) {
    if (endsWith(file,".sci") or 
	endsWith(file,".hk") or 
	endsWith(file,".dat")) addDataFile(file);
    else if (endsWith(file,".bsp") or endsWith(file,".bc")) addSpice(file);
    else {
      stringstream ss;
      ss << "unknown file type " << file << endl;
      logof << ss.str();
      cout << ss.str();
    }
  }

  /*
    Add data file, indexing start and stop time.
   */
  void addDataFile(const string &file) {
    fstream fs(file.c_str(),fstream::in);
    clock start, stop;

    if (!fs.is_open()) throwError("Unable to read " << file);
    
    if (fileHeader(fs)) {
      // Check header for start time and end time.
      byte fheader[64];
      fs.read((Sbyte)fheader,64);
      if (!fs.good()) 
	{ throwError("Error reading header in " << file); }
      start = getBits<clock>(fheader,"f_start");
      stop  = getBits<clock>(fheader,"f_stop");
    } else {
      L0 r;
      fs >> r;
      if (!fs.good()) 
	{ throwError("Error reading record in " << file); }
      // Save first record start time.
      start = r.info.time;
      // Search from end for records to locate end time.
      static ushort apps[] = {  32,   48, 100, 101, 106, 0 };
      static ushort size[] = { 328, 1716, 312, 140, 608, 0 };
      ushort *a=apps, *s=size;
      while (*a) {
	fs.seekg(-(*s), ios_base::end);
	if (r.atHeader(fs)) {
	  fs >> r;
	  if (r.info.appid==*a and r.info.length==*s) break;
	}
	a++; s++;
      }
      if (*a) stop = r.info.time;
      else throwError("Unknown stop time in " << file);
    }
    stringstream ss;
    ss << "Found data file " << file << " " 
       << pdsTime(start) << " (" << getYD(start.secs) << ") to " 
       << pdsTime(stop)  << " (" << getYD(stop.secs) << ")" << endl;
    logof << ss.str(); 
    if (config.verbose>1) cout << ss.str();

    for (int yd=getYD(start.secs); yd<=getYD(stop.secs); yd++)
      ydFiles[yd][file]=true;
  }

  /*
    Open only the files which are in our desired time span.
  */
  void openDataFiles( int yd) {
    indexSize=0;
    for (map<string,bool>::iterator f=ydFiles[yd].begin(); f!=ydFiles[yd].end(); f++) {
      fstream* fs = new fstream(f->first.c_str(),fstream::in);
      if (fs->is_open()) {
	fs->seekg(0,ios_base::end);
	indexSize+=fs->tellg();
	fs->seekg(0,ios_base::beg);
	files.push_back(fs);
	fileNames[fs]=f->first;
      } else {
	throwError("Unable to open " << f->first);
      }
    }
  }

  /*
    Reset the L0 index.
  */
  void clearData() {
    for (multiset<L0*>::iterator it=L0index.begin(); it!=L0index.end(); it++) delete *it;
    L0index.clear();
  }
  
  /*
    Clear list of files.
  */
  void clearFiles() {
    for (vector<fstream*>::iterator it=files.begin(); it!=files.end(); it++) {
      (*it)->close();
      delete *it;
    }
    files.clear();
    fileNames.clear();
  }

  /*
    Insert an L0 record to start of queue.
  */
  void prependL0(const L0 *r) {
    L0 *first = new L0();
    *first = *r;
    first->info.time.secs = 0;
    first->info.time.fract = 0;
    L0index.insert(first);    
  }

  /*
    Check for record for valid serial and appid number.
  */
  inline bool validSerial(const L0 *r) {
    if ((r->info.serial != config.serial) and
	(r->info.appid == appId.CRATER_PRI or
	 r->info.appid == appId.CRATER_SEC or
	 r->info.appid == appId.CRATER_HK )) {
      throwError("received invalid serial: "  << r->info.serial << ", asked for : " << config.serial);
    }
    return true;
  }

  /*
    Check record for a desired timestamp.
  */
  inline bool validTime(const L0 *r) {
    return (r->info.time.secs<config.stop_t and
	    (r->info.time.secs+60)>=config.start_t);
  }

  /*
    Index records in L0 data files.
  */
  void indexFiles() {
    openDataFiles(config.yyyyddd);

    byte fheader[64];
    uint64 lastPos=0;
    uint64 readSize=0;
    pipeSize=0;

    if (files.size()==0) {
      string err="ERROR: No valid files to index.";
      logof << err << endl;
      cerr << err << endl;
      exit(1);
    }

    if (config.verbose) cout << "Indexing files:\n";
    progressBar pb(indexSize);

    // Foreach L0 file
    for (unsigned fileno=0; fileno<files.size(); fileno++) {
      fstream &is = *files[fileno];      
      is.seekg(0);
      lastPos=0;
      if (fileHeader(is)) {
	is.read((Sbyte)fheader,64);
      }
      int pCount=0, aCount=0;
      // Read all the record headers
      while (is.good()) {
	int pos = is.tellg();
	readSize+=(pos-lastPos);
	lastPos=pos;
	if (config.verbose) pb.update(readSize);
	L0 *r = new L0();
	is >> *r;
	// And insert valid record headers in L0 index
	if (r->valid and validSerial(r) and validTime(r)) {
	  aCount++;
	  L0index.insert(r);
	  pipeSize += r->info.length;
	} else delete r;		
	if (!is.eof()) pCount++;	
      }
      logof << fileNames[files[fileno]] << ": read " << pCount << " records, " << aCount << " accepted." << endl;
    }
    logof << (info = sysInfo("Indexed files")) << endl;
    if (config.verbose) cout << info << endl;

    if (pipeSize==0) {
      string err="ERROR: No valid packets found.";
      logof << err << endl;
      cerr << err << endl;
      exit(1);
    }
  }

  /*
    Write L0/L1/L2 files.
  */
  void printRecords() {
  
    static const int bufSize=1024;
    static byte buf[bufSize];
    static const char* types[] = { "PRI", "SEC", "HK" };

    bool foundHK=false, foundSC=false, filesOpen=false;
    int skip=0, readSize=0;
    byte4 PWCRI=0, PWBUSV=0;
    byte fheader[64];

    double lastRad = 0;

    memset((Sbyte)fheader,0,sizeof(fheader));

    multiset<L0*>::iterator it=L0index.begin();

    L1::pri p1;  L1::sec s1;  L1::hk h1;    
    L2::pri p2;  L2::sec s2;  L2::hk h2;

    clock first[3], last[3];
    int rawCount[3] = { 0, 0, 0 };
    int eventCount=0;

    map<int, map<char,fstream*> > out;

    // Create output streams.
    for (int l=0;l<3;l++) {
      for (int t=0;t<3;t++) {
	out[l][types[t][0]] = new fstream();
      }
    }

    char* gapfile = cratFile(-1,"GAPS",config.gapDir);
    fstream gapof(gapfile,fstream::out|fstream::trunc);
    if (!gapof.good()) throwError("Unable to open " << gapfile);

    // Make sure our data is primed with instrument power info      
    while (it!=L0index.end() and (!foundHK or !foundSC)) {
      bool isHK = (*it)->info.appid == appId.CRATER_HK;
      bool isSC = (*it)->info.appid == appId.SC_HK_100;

      if ((isHK and !foundHK) or (isSC and !foundSC)) {	
	int tdiff=(int)(*it)->info.time.secs - config.start_t;
	if (tdiff>0) {
	  stringstream ss;
	  ss << "WARNING: Found " << (isSC?"SC":"") << "HK after " << skip << " records at " 
	     << tdiff << " seconds... prepending" << endl;
	  logof << ss.str();
	  cerr << ss.str();
	  prependL0(*it);
	  pipeSize += (*it)->info.length;
	}
	( isHK ? foundHK=true : foundSC=true );
      }
      skip++; it++;
    }

    // Warn if not
    if (!foundHK or !foundSC) {
      stringstream ss;
      ss << "WARNING!: " << (!foundHK?"NO HK":"") << (!foundHK and !foundSC?" and ":"") << (!foundSC?"NO SC":"")
	 << " PACKETS FOUND FOR PRIMING" << endl;
      logof << ss.str();
      cerr << ss.str();
    }

    if (config.verbose) cout << "Running pipeline:\n";

    progressBar pb(pipeSize);

    for (it=L0index.begin(); it!=L0index.end(); it++) {
           
      L0 &r = **it;
      clock &rTime = r.info.time;

      ushort &appid = r.info.appid;
      istream &is = *(r.stream);

      is.clear();
      is.seekg(r.position);

      // Read entire record
      int readLen=r.info.length;
      if (readLen>bufSize) throwError("Buffer too small");
      is.read((Sbyte)buf,readLen);
      if (is.fail()) throwError("Problem reading data!");

      readSize += readLen;
      if (config.verbose) pb.update(readSize);
      if (rTime.secs>config.stop_t) continue;

      // Extract CRaTER power info from spacecraft housekeeping data
      if (appid == appId.SC_HK_100) {
	PWCRI = getNum(buf,appId.SC_HK_100,"PWCRI");
	PWBUSV = getNum(buf,appId.SC_HK_100,"PWBUSV");
	continue;
      }

      bool record = rTime.secs>=config.start_t and rTime.secs<config.stop_t;
      
      // Open output file if not open, leaving space for the header.
      if (!filesOpen and record and !config.nullData) {
	filesOpen = true;
	for (int l=0;l<3;l++) {
	  for (int t=0;t<3;t++) {
	    char* file = cratFile(l,types[t],config.outDir);
	    if (l==0) {
	      out[l][types[t][0]]->open(file,fstream::out|fstream::trunc);
	      out[l][types[t][0]]->write((Sbyte)fheader,sizeof(fheader));
	      first[t] = last[t] = (clock){0,0};
	    } else {
	      gzip(file,out[l][types[t][0]]);
	    }
	    if (!out[l][types[t][0]]->good()) {
	      throwError("Unable to write output file " << file);
	    }
	  }
	}
      }      

      const int rType = appid - appId.CRATER_PRI;

      // Check for data gaps.
      if (record and rType>=0 and rType<3) {
	if (rawCount[rType]) {
	  unsigned t = last[rType].secs + config.gapVal[rType];
	  if (rTime.secs>t or (rTime.secs==t and rTime.fract>last[rType].fract)) {
	    gapof << left << setfill(' ') << '"' << setw(31) << string(cratName(0,types[rType]))+".DAT";
	    gapof << "\"," << setw(23) << spiceDate(last[rType]);
	    gapof << "," << setw(23) << spiceDate(rTime);
	    gapof << ",\"" << pdsTime(last[rType]) << "\",\"" << pdsTime(rTime);
	    gapof << "\"," << right << setw(10) << rTime.secs-last[rType].secs << left << endl;
	  }
	}
	
	if (!first[rType].secs) first[rType]=rTime;
	last[rType]=rTime;
	rawCount[rType]++;
      }

      // Write out data if a primary CRaTER packet
      if (appid == appId.CRATER_PRI) {
	out[0]['P']->write((Sbyte)buf,readLen);
        *out[1]['P'] << p1.getValues(r.info,buf,h1);
	*out[2]['P'] << p2.getValues(p1);
	if (record) eventCount += p1.nEvents;
	continue;
      } 

      // Write out data if a secondary CRaTER packet
      if (appid == appId.CRATER_SEC) {
	out[0]['S']->write((Sbyte)buf,readLen);
	*out[1]['S'] << s1.getValues(r.info,buf);
	*out[2]['S'] << s2.getValues(s1);
	continue;
      } 

      // Write out data if a house keeping CRaTER packet
      if (appid == appId.CRATER_HK) {
	out[0]['H']->write((Sbyte)buf,readLen);
	*out[1]['H'] << h1.getValues(r.info,buf);
	h1.convertDigital(PWBUSV,apps.id[appId.CRATER_HK]["SCV28bus"]);
	h1.convertDigital(PWCRI,apps.id[appId.CRATER_HK]["SCI28bus"]);
	h1.analog["SCP28bus"] = h1.analog["SCV28bus"] * h1.analog["SCI28bus"];
	*out[2]['H'] << h2.getValues(h1);
	lastRad = h2.radTotal;
	continue;
      }

      throwError("Unknown appid " << appid);
    }

    logof << (info = sysInfo("Processed files")) << endl;
    if (config.verbose) cout << info << endl;

    // Fill in the file header
    static const int fileid[] = { 200, 202, 201 };    
    for (int l=0;l<3;l++) {
      for (int t=0;t<3;t++) {
	if (l==0) {
	  out[l][types[t][0]]->seekg(0);
	  createHeader(fheader,fileid[t],first[t],last[t],cratName(l,types[t]));
	  out[l][types[t][0]]->write((Sbyte)fheader,sizeof(fheader));

	  stringstream ss;
	  ss << types[t] << " : " << rawCount[t] << " packets processed" << endl;
	  logof << ss.str();
	  if (config.verbose>1) cout << ss.str();
	}
	out[l][types[t][0]]->close();
      }
    }

    // Write log info and close files
    stringstream ss;
    ss << endl << "Ending radiation dose: " << lastRad << " rads " << endl;
    ss << endl << "Finished " << config.proc << " on " << nowDate() << endl;
    logof << ss.str();
    if (config.verbose>1) cout << ss.str();

    writePdsLabels(rawCount,eventCount,first,last);

    gapof.close();

    clearData();
    clearFiles();
  }

};

#endif //_PIPELINE_HPP_
