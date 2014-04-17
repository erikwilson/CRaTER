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

#ifndef _FILEUTILS_HPP_
#define _FILEUTILS_HPP_

/*
  Returns true if string is a directory.
*/
bool isDir(const string &file) {
  struct stat sb;    
  return !stat(file.c_str(),&sb) && (sb.st_mode & S_IFMT)==S_IFDIR;
}

/*
  Creats a string vector of directory contents.
*/
vector<string>& expandPath(const string &file) {
  vector<string> *dir = new vector<string>();
  DIR *dp = opendir(file.c_str());
  assert(dp);
  struct dirent *de;
  while (de = readdir(dp)) {
    if (de->d_name[0] != '.')
      dir->push_back(file + "/" + de->d_name);
  }
  return *dir;
}

/*
  Return true if istream points at a file header.
*/
bool fileHeader(istream &is) {
  byte hdr[4] = { 255, 255, 255, 0 };
  streampos p = is.tellg();
  is.read((Sbyte)hdr, 4);
  is.seekg(p);
  if (!hdr[0] and !hdr[1] and !hdr[2] and hdr[3]>=199 and hdr[3]<203)
    return true;
  else
    return false;
}

/*
  Return a byte array header given parameyers.
*/
byte* createHeader(byte* header, const int &fileId, const clock &start, const clock &stop, const char* name) {
  setBits<byte4>(header, apps.id[HEADER]["f_fileid"], fileId);
  setBits(header, apps.id[HEADER]["f_start"], start);
  setBits(header, apps.id[HEADER]["f_stop"], stop);
  setBits(header, apps.id[HEADER]["f_name"], name);
  return header;
}


/*
  Create a gzip output stream to file.
*/
void gzip(string file, fstream *os) {
  if (config.compress and !config.nullData) {
    // use a fifo to write to gzip
    char fifo[24];
    close(mkstemp(strcpy(fifo, "/tmp/crat.XXXXXX")));
    unlink(fifo);
    if (mkfifo(fifo, 0600)) {
      cerr << fifo << ": " << strerror(errno) << endl;
      exit(1);
    }
    // forking a gzip process to compress the output
    char buf[file.size()+32];
    sprintf(buf, "(gzip<%s>%s.gz&)", fifo, file.c_str());
    if (system(buf) == -1) {
      cerr << "system call to gzip failed?" << endl;
    }
    // write to the fifo
    os->open(fifo, ofstream::out);
    // safe to unlink it since gzip has already opened it
    unlink(fifo);
  } else {
    os->open(file.c_str(), ofstream::out);
  }
}


/*
  Returns appropriate file name given parameters.
*/
char *cratFile(const int level, const char *type, const string &outDir, const bool label=false) {
  int len = config.outDir.size()+strlen(type)+28;
  char *file = new char[len];
  assert(file);
  if (level>=0) {
    sprintf(file, "%s/CRAT_L%d_%s_%07d_V%02d.%s",
	    outDir.c_str(), level, type, config.yyyyddd,
	    config.version, (label?"LBL":level?"TAB":"DAT"));
  } else {
    sprintf(file, "%s/CRAT_%07d_V%02d_%s.%s",
	    outDir.c_str(), config.yyyyddd,
	    config.version, type, (string(type)=="LOG"?"TXT":"TAB"));
  }
  return file;
}

/*
  Return filename for output in current directory.
*/
char *cratName(const int level, const char *type) {
  char *name = basename(cratFile(level,type,""));
  *strrchr(name,'.')=0;
  return toUpper(name);
}

#endif //_FILEUTILS_HPP_
