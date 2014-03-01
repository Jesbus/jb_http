#include<string>
#include<boost/regex.hpp>

using namespace std;

#define NRM  "\x1B[0m"
#define RED  "\x1B[31m"
#define GRE  "\x1B[32m"
#define YEL  "\x1B[33m"
#define BLU  "\x1B[34m"
#define PUR  "\x1B[35m"
#define CYA  "\x1B[36m"
#define WHI  "\x1B[37m"

extern void printRegexError(boost::regex_error &e);
extern bool replace(std::string& str, const std::string& from, const std::string& to);
extern bool replaceByPointers(std::string& str, std::string* from, std::string* to);
extern std::string bin2hex(const std::string& input);
extern std::string bin2hex(char c);
extern string generateParamString(vector<string>* keys, vector<string>* values);
extern void parseParamString(string paramString, vector<string>* keys, vector<string>* values);
bool searchForFile(char* directory, string& requestPath, vector<string>& indexes, string& filePath, string& fileName);
