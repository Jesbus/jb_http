#include<string>
#include<boost/regex.hpp>
#include <sys/stat.h>
#include <vector>

#include "definitions.h"
#include "main.h"

using namespace std;

void printRegexError(boost::regex_error &e)
{
	const char* precedeError = "\n         %s";
	switch (e.code())
	{
		case boost::regex_constants::error_collate:
			printf(precedeError, "The expression contained an invalid collating element name.");
		break;
		case boost::regex_constants::error_ctype:
			printf(precedeError, "The expression contained an invalid character class name.");
		break;
		case boost::regex_constants::error_escape:
			printf(precedeError, "The expression contained an invalid escaped character, or a trailing escape.");
		break;
		case boost::regex_constants::error_backref:
			printf(precedeError, "The expression contained an invalid back reference.");
		break;
		case boost::regex_constants::error_brack:
			printf(precedeError, "The expression contained mismatched brackets ([ and ]).");
		break;
		case boost::regex_constants::error_paren:
			printf(precedeError, "The expression contained mismatched parentheses (( and )).");
		break;
		case boost::regex_constants::error_brace:
			printf(precedeError, "The expression contained mismatched braces ({ and }).");
		break;
		case boost::regex_constants::error_badbrace:
			printf(precedeError, "The expression contained an invalid range between braces ({ and }).");
		break;
		case boost::regex_constants::error_range:
			printf(precedeError, "The expression contained an invalid character range.");
		break;
		case boost::regex_constants::error_space:
			printf(precedeError, "There was insufficient memory to convert the expression into a finite state machine.");
		break;
		case boost::regex_constants::error_badrepeat:
			printf(precedeError, "The expression contained a repeat specifier (one of *?+{) that was not preceded by a valid regular expression.");
		break;
		case boost::regex_constants::error_complexity:
			printf(precedeError, "The complexity of an attempted match against a regular expression exceeded a pre-set level.");
		break;
		case boost::regex_constants::error_stack:
			printf(precedeError, "There was insufficient memory to determine whether the regular expression could match the specified character sequence.");
		break;
		default:
			printf(precedeError, "Unknown error :(");
		break;
	}
}

bool replace(string& str, const string& from, const string& to)
{
	size_t start_pos = str.find(from);
	if (start_pos == string::npos) return false;
	str.replace(start_pos, from.length(), to);
	return true;
}

bool replaceByPointers(string& str, string* from, string* to)
{
	if (debug) printf("replaceByPointer(%s,%s,%s)", str.c_str(), from->c_str(), to->c_str());
	//if (debug) printf("\nreplaceByPointers(.., .., ..) #0");
	size_t start_pos = str.find(*from);
	//if (debug) printf("\nreplaceByPointers(.., .., ..) #1");
	if (start_pos == string::npos) return false;
	//if (debug) printf("\nreplaceByPointers(.., .., ..) #2");
	str.replace(start_pos, from->length(), *to);
	//if (debug) printf("\nreplaceByPointers(.., .., ..) #3");
	return true;
}

string bin2hex(const string& input)
{
    string res;
    const char hex[] = "0123456789ABCDEF";
    for(auto sc : input)
    {
        unsigned char c = static_cast<unsigned char>(sc);
        res += hex[c >> 4];
        res += hex[c & 0xf];
    }

    return res;
}
string bin2hex(char c)
{
    string res;
    const char hex[] = "0123456789ABCDEF";
    
    unsigned char c2 = static_cast<unsigned char>(c);
    res += hex[c2 >> 4];
    res += hex[c2 & 0xf];

    return res;
}
string generateParamString(vector<string>* keys, vector<string>* values)
{
	string outputString = string("");
	for (int i=0;i<keys->size();i++)
	{
		outputString.append(keys->at(i));
		outputString += '=';
		outputString.append(values->at(i));
		if (i!=keys->size()-1) outputString += '&';
	}
	return outputString;
}
void parseParamString(string paramString, vector<string>* keys, vector<string>* values)
{
	bool inKey = true;
	
	stringstream buff;
	string emptyStr = string("");
	
	int l = paramString.length();
	
	for (int i=0;i<l;i++)
	{
		if (paramString[i]=='&')
		{
			if (inKey)
			{
				keys->push_back(buff.str());
				values->push_back(emptyStr);
			}
			else
			{
				values->push_back(buff.str());
			}
			inKey = true;
			buff.str("");
		}
		else if (inKey&&(paramString[i]=='='))
		{
			inKey = false;
			keys->push_back(buff.str());
			buff.str("");
		}
		else buff << paramString[i];
	}
	if (inKey)
	{
		keys->push_back(buff.str());
		values->push_back(emptyStr);
	}
	else
	{
		values->push_back(buff.str());
	}
	buff.str("");
}

bool searchForFile(char* directory, string& requestPath, vector<string>& indexes, string& filePath, string& fileName)
{
	bool exists = false;
	
	filePath = (string(directory)+string(requestPath));
	int lastSlash = filePath.rfind('/');
	string filePathOnly = filePath.substr(0, lastSlash+1);
	fileName = filePath.substr(lastSlash+1, requestPath.length()-lastSlash-1);
	int tryingIndex = -1;
	bool triedAddingSlash = false;
	
tryFileAgain:
	lastSlash = filePath.rfind('/');
	filePathOnly = filePath.substr(0, lastSlash+1);
	fileName = filePath.substr(lastSlash+1, requestPath.length()-lastSlash-1);
	struct stat buffer;
	if (stat(filePath.c_str(), &buffer)!=0) // If the file doesn't exist....
	{
		if (tryingIndex<indexes.size()-1)
		{
			tryingIndex++;
			filePath = filePathOnly+indexes.at(tryingIndex);
			goto tryFileAgain;
		}
		exists = false;
	}
	else if (S_ISDIR(buffer.st_mode)) // If it's a directory...
	{
		if (filePath.back()!='/') filePath += "/";
		fileName = "";
		filePath += indexes.at(0);
		tryingIndex = 0;
		if (debug) printf("\nIt's a directory! added a / and cleared the fileName");
		goto tryFileAgain;
	}
	else return true;
	if ((!triedAddingSlash) && (!exists))
	{
		triedAddingSlash = true;
		filePath += "/";
		fileName = "";
		tryingIndex = -1;
		if (debug) printf("\nTrying index at [%i]", tryingIndex);
		goto tryFileAgain;
	}
	
	return exists;
}
