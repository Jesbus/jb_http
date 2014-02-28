#include<string>

using namespace std;

extern void parseRequest
(
	string keys[32],
	string values[32],
	char*& requestType,
	string& requestLocation,
	string& requestContent,
	string& requestSender,
	string& requestPath,
	string& requestParams,
	int& responseCode,
	string& responseText,
	int& headerContentLength,
	bool& headerTransferEncodingChunked,
	string& headerHost,
	string& header
);
