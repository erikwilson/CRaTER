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


/*
  Return uppercase version of string.
*/
string toUpper(string str) {
  for(unsigned int i=0; str[i]!=0; i++ ) 
    str[i] = toupper(str[i]);
  return str;
}

/*
  Uppercase a string.
*/
char* toUpper(char *s) {
  for (char *n = s; *n; n++) 
    *n = toupper(*n);
  return s;
}

/*
  Comparing ending of string1 with string2.
*/
bool endsWith(const string &s, const string &e)
{ return toUpper(s.substr(s.size()-e.size(),e.size()))==toUpper(e); }


/*
  Methods to vectorize a delimated string.
*/
vector<string> &split(const string &s, const char &delim, vector<string> &elems) {
  stringstream ss(s); string item; 
  while(getline(ss, item, delim)) elems.push_back(item);
  return elems;
}
vector<string> split(const string &s, const char &delim) {
  vector<string> elems; return split(s, delim, elems);
}

/*
  Read data from string A into variable B
*/
stringstream ss;
#define readSS(a,b) { ss.clear(); ss.str(a); ss>>b; }
