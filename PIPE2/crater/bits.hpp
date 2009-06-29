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

#ifndef _BITS_HPP_
#define _BITS_HPP_

/*
  Input/output information for a field of data.
*/
struct field {
  char format;
  ushort byte;
  ushort bit;
  ushort width;
  ushort printWidth;
  int precision;
  char notation;
  char coefType;
  int level;  
  int appid;
  string name;
  string alias;
};

#define formatError(f) \
  throwError("Unknown format[" << f.format << "] for name[" << f.name << "], appid[" << f.appid << "] in " << config.appidFile );

/*
  How to extract basic data types given a data pointer and field description.
*/
template <class T>
T getBits( const byte* data, const field &f ) {
  T v;
  data += f.byte;
  ushort ts = sizeof(T);
  ushort bw = ts*8;
  if (ts!=(int)f.format-'0') 
    throwError("Requested type size and bitfield type size don't match!");
  switch (f.format) {
  case '1':
  case '2':
  case '4': {
    memcpy(&v,data,ts);
    endian.swap(&v,1);
    v<<=f.bit;
    v>>=(bw-f.width);
    break;
  }
  default:
    formatError(f);
  }
  return v;
}


void print(const byte* data, int n) {
  for (int i=0;i<n;i++)
    cout << hex << (int)data[i] << dec << ":";
  cout << endl;
}

/*
  How to set basic data types given a data pointer and field description.
*/
template <class T>
void setBits( byte* data, const field &f, const T& value) {
  T v;
  data += f.byte;
  ushort ts = sizeof(T);
  ushort bw = ts*8;
  if (ts!=(int)f.format-'0') 
    throwError("Requested type size and bitfield type size don't match!");
  switch (f.format) {
  case '1':
  case '2':
  case '4': {
    v=*(T*)data;
    endian.swap(&v,1);
    v<<=f.bit;
    v>>=(bw-f.width);
    v<<=(bw-(f.bit+f.width));
    endian.swap(&v,1);
    *(T*)data^=v;
    if (value) {      
      v=value;
      v<<=(bw-f.width);
      v>>=f.bit;
      endian.swap(&v,1);
      *(T*)data|=v;
    }
    break;
  }
  default:
    formatError(f);
  }
}

/*
  How to extract string data types.
*/
template <>
char* getBits<char*>( const byte* data, const field &f ) {
  static char v[1024];
  if (f.width > sizeof(v)) throwError("string too big for buffer");
  data += f.byte;
  switch (f.format) {
  case 'S': {
    memcpy(v,data,f.width);
    v[f.width]=0;
    break;
  }
  default:
    formatError(f);
  }
  return v;
}

/*
  How to set string data types.
*/
void setBits( byte* data, const field &f, const char* s ) {
  data += f.byte;
  int size=strlen(s);
  switch (f.format) {
  case 'S': {
    if (size>f.width) throwError("setbits string too big");
    memcpy(data,s,size);
    memset(data+size,0,f.width-size);
    break;
  }
  default:
    formatError(f);
  }
}

/*
  How to extract clock data types.
*/
template <>
clock getBits<clock>( const byte* data, const field &f ) {
  clock v;
  data += f.byte;
  switch (f.format) {
  case 'T': {
    memcpy(&v.secs,data,4);
    endian.swap(&v.secs,1);
    v.fract = *(data+4)>>4;
    break;
  }
  default:
    formatError(f);
  }
  return v;
}


/*
  How to set clock data types.
*/
void setBits( byte* data, const field &f, const clock &c ) {
  data += f.byte;
  switch (f.format) {
  case 'T': {
    memcpy(data,&c.secs,4);
    endian.swap((byte4*)data,1);
    data+=4;
    *data = c.fract<<4 | (*data<<4)>>4;
    break;
  }
  default:
    formatError(f);
  }
}


/*
  How to extract generic number types.
*/
uint getNum( const byte* data, const field &f ) {
  switch (f.format) {
  case '1': return getBits<byte>( data, f );
  case '2': return getBits<byte2>( data, f );
  case '4': return getBits<byte4>( data, f );
  default:
    formatError(f);
  }
}

#endif // _BITS_HPP_
