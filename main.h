#include<string>
#include<vector>

using namespace std;

extern bool verbose, teapot, php, python, perl, quiet, debug;
extern string generateParamString(vector<string>* keys, vector<string>* values);
extern void parseParamString(string paramString, vector<string>* keys, vector<string>* values);
