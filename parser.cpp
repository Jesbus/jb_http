#include<string>
#include<sstream>
#include<chrono>

#include "main.h"
#include "definitions.h"

using namespace std;

void parseRequest
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
)
{
	if (debug) printf("%s", "\nParsing request...");
	auto startTimeParse = std::chrono::system_clock::now();
	if (header.substr(0, 3)=="GET") requestType = "GET";
	else if (header.substr(0, 4)=="POST") requestType = "POST";
	else
	{
		responseCode = 400; // Bad request
		responseText = "Hablo HTTP?";
		return;
	}
	
	bool inLoc = false;
	bool locDone = false;

	bool inHeaders = false;
	bool inKey = false;
	bool inContent = false;
	bool insideContent = false;
	bool seenQuestionMark = false;

	std::stringstream key, value, content, location, locPath, locParams;

	int keyI = 0;
	int headerCount = 0;

	for(int i = 2; i < header.size(); ++i)
	{
		if (!inHeaders)
		{
			if (inLoc)
			{
				if (header[i]==' ') { inLoc = false; locDone = true; continue; }
				location << header[i];
				if (header[i]=='?') seenQuestionMark = true;
				else if (seenQuestionMark) locParams << header[i];
				else
				{
					locPath << header[i];
					if (header[i]=='/' && header[i-1]=='.' && header[i-2]=='.' && header[i-3]=='/')
					{
						responseCode = 403;
						responseText = "You're not allowed to go upwards in the directory tree";
						break;
					}
				}
				location.seekg(0, std::ios::end);
				int size = location.tellg();
				if (size>256)
				{
					responseCode = 414;
					responseText = "Request-URI Too Long";
					break;
				}
			}
			if ((!inLoc)&&(!locDone))
			{
				if (header[i]==' ') inLoc = true;
			}
			if (header[i]=='\n')
			{
				inHeaders = true;
				inKey = true;
				key.str(std::string());
				key.clear();
			}
		}
		else if (inContent)
		{
			if (!insideContent)
			{
				if (header[i]!='\n' && header[i]!='\r')
				{
					insideContent = true;
					content << header[i];
				}
			}
			else
			{
				content << header[i];
				if (headerContentLength!=-1)
				{
					content.seekg(0, std::ios::end);
					int size = content.tellg();
					if (size-1>headerContentLength)
					{
						responseCode = 400;
						responseText = "Received more content than the Content-Length header field announced.";
						break;
					}
				}
			}
		}
		else if (inKey)
		{
			if (header[i]=='\r')
			{
				responseCode = 400;
				responseText = "Carriage returns are not allowed in the middle of a header key.";
				break;
			}
			if (header[i]=='\n')
			{
				responseCode = 400;
				responseText = "Newlines are not allowed in the middle of a header key.";
				break;
			}
			if (header[i]==':' && header[i+1]==' ')
			{
				inKey = false;
				if (headerCount>=32)
				{
					responseCode = 413;
					responseText = "Too many header fields! This server doesn't allow more than 32.";
					break;
				}
				keys[headerCount] = key.str();
				++i;
				value.str(std::string());
				value.clear();
			}
			else
			{
				key << header[i];
				key.seekg(0, std::ios::end);
				int size = key.tellg();
				if (size>256)
				{
					responseCode = 413;
					responseText = "Header key too long! Max is 256 bytes.";
					break;
				}
			}
		}
		else // inHeaders
		{
			if (header[i]=='\r'){}
			else if (header[i]=='\n')
			{
				values[headerCount] = value.str();
				if (keys[headerCount]=="Content-Length")
				{
					headerContentLength = std::stoi(values[headerCount]);
					if (headerTransferEncodingChunked)
					{
						responseCode = 400;
						responseText = "Both Content-Length and Transfer-Encoding: chunked is not allowed.";
						break;
					}
				}
				else if (keys[headerCount]=="Transfer-Encoding")
				{
					headerTransferEncodingChunked = values[headerCount] == "chunked";
					if (headerContentLength!=-1 && headerTransferEncodingChunked)
					{
						responseCode = 400;
						responseText = "Both Content-Length and Transfer-Encoding: chunked is not allowed.";
						break;
					}
					if (headerTransferEncodingChunked)
					{
						responseCode = 500;
						responseText = "Sorry, this server cannot deal with Transfer-Encoding: chunked";
						break;
					}
				}
				else if (keys[headerCount]=="Host")
				{
					headerHost = values[headerCount];
				}
				headerCount++;
				key.str(std::string());
				key.clear();
				if (header[i+1]=='\r' || header[i+1]=='\n')
				{
					inContent = true;
					inHeaders = false;
				}
				else
				{
					inKey = true;
				}
			}
			else
			{
				value << header[i];
				value.seekg(0, std::ios::end);
				int size = value.tellg();
				if (size>256)
				{
					responseCode = 413;
					responseText = "Header value for "+keys[headerCount]+" too long! Max is 256 bytes.";
					break;
				}
			}
		}
	}
	requestLocation = location.str();
	requestPath = locPath.str();
	requestParams = locParams.str();
	requestContent = content.str();
	if (verbose && headerCount!=0)
	{
		auto endTimeParse = std::chrono::system_clock::now();
		auto elapsed = endTimeParse - startTimeParse;
		double ms = double(elapsed.count())/1000000;
		printf("\n%sParsing took: %fms%s", BLU, ms, NRM);
		printf("%s%i%s", "\n##-------- ", headerCount, " header fields:");
		for (int i=0;i<headerCount;i++)
		{
			printf("\n%s%s%s: %s", GRE, keys[i].c_str(), NRM, values[i].c_str());
		}
	}
}
