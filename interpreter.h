#include<string>
#include<vector>

using namespace std;

extern void executeConfigScript
(
	int& responseCode,
	string& responseText,
	char* confScript,
	string& addedHeaders,
	string& requestPath,
	string& requestParams,
	string& requestContent,
	string& requestSender,
	bool& getChanged,
	bool& postChanged,
	bool& requestPathChanged,
	vector<string>* getKeys,
	vector<string>* getValues,
	vector<string>* postKeys,
	vector<string>* postValues,
	vector<string>* strings,
	vector<string>* regexs,
	vector<string>* inputs,
	string keys[32],
	string values[32],
	
	string& headerContentType,
	string& headerContentDisposition,
	string& headerServer,
	string& headerKeepAlive,
	
	char *directory,
	vector<string>& indexes,
	string& filePath,
	string& fileName
);
