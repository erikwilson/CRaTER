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

#ifndef _ENDIAN_HPP_
#define _ENDIAN_HPP_

struct byteOrder {

  bool big;

  /*
    Test byte order of the machine.
  */
  byteOrder() {
    const byte w[] = {0x87,0x65,0x43,0x21};
    if (sizeof(byte)!=1 || sizeof(byte2)!=2 || sizeof(byte4)!=4)
      { throwError("System type size mismatch!"); }
    else if (*(byte4*)w == 0x87654321) big=true;
    else if (*(byte4*)w == 0x21436587) big=false;
    else throwError("Unknown endian type!");
    //cout << "[" << (big?"big":"little") << " endian detected]" << endl;
  }

  inline void swap(byte* x, int n) const {}

  /*
    Swap 2 bytes if little endian.
  */
  inline void swap(byte2* x, int n) const { 
    if (big) return;
    for (; --n>=0; x++) 
      *x = (*x>>8) | (*x<<8);
  }

  /*
    Swap 4 bytes if little endian.
  */
  inline void swap(byte4* x, int n) const {
    if (big) return;
    for (; --n>=0; x++)
      *x = ((*x>>8) & 0x0000FF00) | (*x>>24) | (*x<<24) | (0x00FF0000 & (*x<<8));
  }

} const endian;

#endif //_ENDIAN_HPP_
