#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>
#include <boost/regex.hpp>

#include "definitions.h"
#include "main.h"
#include "interpreter.h"

using namespace std;

void executeConfigScript
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
)
{
	auto startTimeBytecode = std::chrono::system_clock::now();
	if (strlen(confScript)<=2) return;
	int byteCodeExecutionCount = 0;
	
	vector<string>* varKeys   = new vector<string>();
	vector<string>* varValues = new vector<string>();
	
byteCodeAgain:
	byteCodeExecutionCount++;
	string emptyStr  = string("" );
	string boolTrue  = string("Y");
	string boolFalse = string("" );
	string strGET    = string("GET");
	string strPOST   = string("POST");
	
	vector<string>* datas = new vector<string>();
	vector<unsigned char>* dataTypes = new vector<unsigned char>();
	bool inIf = false;
	bool inSetvar = false;
	
	
	string* setvarVar = NULL;
	int curLevel = 0;
	int searchForElseOrEndAtLevel = -1;
	int searchForEndAtLevel = -1;
	unsigned char executingCommand = 0x00;
	
	if (responseCode==200 || responseCode==404)
	{
		for (int i=0;i<strlen(confScript);i++)
		{
			if (byteCodeExecutionCount>16)
			{
				printf("\n%s", "[Error] Config script recursion limit is 16. aborted");
				break;
			}
			unsigned char c = confScript[i];
			/////// HERE WAS THE SETVAR EXECUTING STUFF
			if (debug) printf("\n#####confScript[%i]=%s", i, bin2hex(c).c_str());
		
			// Take care of level
			if (i==0 || (i>0 && confScript[i-1]!=0x20 && confScript[i-1]!=0x21 && confScript[i-1]!=0x22))
			{
				switch (c)
				{
					case 0x92: curLevel++; break;
					case 0x91: curLevel--; break;
				}
			}
		
			// Take care of searching (partially =[)
			if (searchForEndAtLevel!=-1)
			{
				if (c==0x91)
				{
					if (curLevel==searchForEndAtLevel)
					{
						searchForEndAtLevel = -1;
					}
				}
				else continue;
			}
			if (searchForElseOrEndAtLevel!=-1)
			{
				if (c==0x91)
				{
					if (curLevel==searchForElseOrEndAtLevel)
					{
						searchForElseOrEndAtLevel = -1;
					}
				}
				else if (c!=0x91 && c!=0x93 && c!=0x94)
				{
					continue;
				}
			}
			/*************************************/
			if ((c>=0x91 && c<=0x9F)||(c>=0x70 && c<=0x7F))
			{
			if (executingCommand!=0x00) // IMPORTANTMOD2141
			{
				/*if (executingCommand==0x70)
				{
					if (datas->size()>=1)
					{
						addedHeaders += "Location: ";
						addedHeaders += datas->back();
						addedHeaders += "\r\n";
						printf("\n%s%s", "addedHeaders=", addedHeaders.c_str());
						responseCode = 302;
						responseText = "Redirect";
					}
					else continue;
				}
				else */
				if (executingCommand==0x71)
				{
					if (datas->size()>=1)
					{
						responseCode = std::stoi(datas->back());
					}
					else continue;
				}
				else if (executingCommand==0x7F)
				{
					if (datas->size()>=1)
					{
						printf("\n%s%s%s", BLU, datas->back().c_str(), NRM);
					}
					else continue;
				}
				else if (executingCommand==0x72)
				{
					if (datas->size()>=1)
					{
						responseText = datas->back();
					}
					else continue;
				}
				else if (executingCommand==0x73)
				{
					if (datas->size()>=1)
					{
						string h = datas->back();
						     if (h.substr(0, 13)=="Content-Type:") headerContentType = h.substr(14, h.length()-14);
						else if (h.substr(0, 20)=="Content-Disposition:") headerContentDisposition = h.substr(21, h.length()-21);
						else if (h.substr(0,  7)=="Server:") headerServer = h.substr(8, h.length()-8);
						else if (h.substr(0, 11)=="Keep-Alive:") headerKeepAlive = h.substr(12, h.length()-12);
						else addedHeaders += h + "\r\n";
					}
					else continue;
				}
				else if (executingCommand==0x74)
				{
					if (datas->size()>=2)
					{
						printf("\n%s", "[Error] The Rewrite command is currently unused. please remove it from your config file!");
					}
					else continue;
				}
				else if (executingCommand==0x77)
				{
					//printf("\n%s", "NOP");
				}
				else if (executingCommand==0x78) // SetPath
				{
					if (datas->size()>=1)
					{
						requestPath = datas->back();
						requestPathChanged = true;
					}
					else continue;
				}
				else if (executingCommand==0x79) // CheckFileExistance
				{
					if (!searchForFile(directory, requestPath, indexes, filePath, fileName))
					{
						responseCode = 404;
						responseText = "File not found";
					}
					else
					{
						responseCode = 200;
						responseText = "OK";
					}
				}
				else if (executingCommand==0x7A) // Recurse
				{
					goto byteCodeAgain;
				}
				else if (executingCommand==0x7B) // Set
				{
					if (debug) printf("\nexecCom=0x7B");
					if (datas->size()>=2)
					{
						int s = dataTypes->size();
						unsigned char dt1 = dataTypes->at(s-2);
						unsigned char dt2 = dataTypes->at(s-1);
						if (dt1==0x27 && dt2==0x20) // get[...]
						{
							if (debug) printf("\nexecCom=0x7B datas->size()>=2 0x27 0x20");
							string inputName = datas->at(s-2);
							
							for (int j=0;j<getKeys->size();j++)
							{
								if (getKeys->at(j)==inputName)
								{
									getValues->at(j) = datas->back();
									goto foundGetAndSet;
								}
							}
							getKeys->push_back(inputName);
							getValues->push_back(datas->back());
						foundGetAndSet:
							getChanged = true;
						}
						else if (dt1==0x28 && dt2==0x20) // post[...]
						{
							string inputName = datas->at(s-2);
							
							for (int j=0;j<postKeys->size();j++)
							{
								if (postKeys->at(j)==inputName)
								{
									postValues->at(j) = datas->back();
									goto foundPostAndSet;
								}
							}
							postKeys->push_back(inputName);
							postValues->push_back(datas->back());
						foundPostAndSet:
							postChanged = true;
						}
						else if (dt1==0x29 && dt2==0x20) // var[...]
						{
							string inputName = datas->at(s-2);
							
							for (int j=0;j<varKeys->size();j++)
							{
								if (varKeys->at(j)==inputName)
								{
									varValues->at(j) = datas->back();
									goto foundVarAndSet;
								}
							}
							varKeys->push_back(inputName);
							varValues->push_back(datas->back());
						foundVarAndSet:;
						}
						else continue;
					}
					else continue;
				}
				else if (executingCommand==0x7C) // Remove
				{
					if (datas->size()>=1)
					{
						unsigned char dt = dataTypes->back();
						if (dt==0x27) // get[...]
						{
							string inputName = datas->back();
							for (int j=0;j<getKeys->size();j++)
							{
								if (getKeys->at(j)==inputName)
								{
									getKeys  ->erase(getKeys->begin()+j);
									getValues->erase(getValues->begin()+j);
									getChanged = true;
									break;
								}
							}
						}
						else if (dt==0x28) // post[...]
						{
							string inputName = datas->back();
							for (int j=0;j<postKeys->size();j++)
							{
								if (postKeys->at(j)==inputName)
								{
									postKeys  ->erase(postKeys  ->begin()+j);
									postValues->erase(postValues->begin()+j);
									postChanged = true;
									break;
								}
							}
						}
						else if (dt==0x29) // var[...]
						{
							string inputName = datas->back();
							for (int j=0;j<varKeys->size();j++)
							{
								if (varKeys->at(j)==inputName)
								{
									varKeys  ->erase(varKeys  ->begin()+j);
									varValues->erase(varValues->begin()+j);
									break;
								}
							}
						}
						else continue;
					}
					else continue;
				}
				else if (executingCommand==0x7D) // RemoveAllGet
				{
					getKeys->clear();
					getValues->clear();
					getChanged = true;
				}
				else if (executingCommand==0x7E) // RemoveAllPost
				{
					postKeys->clear();
					postValues->clear();
					postChanged = true;
				}
				else printf("\n%s", "error enderman");
				datas->clear();
				dataTypes->clear();
				executingCommand = 0x00;
			}
			}
			/*************************************/
			switch (c)
			{
			case 0x92: // if
				if (!inIf) { inIf = true; break; }
			case 0x96: // then
				if (inIf) { inIf = false; break; }
			case 0x93:{ // else
				if (searchForElseOrEndAtLevel==-1)
				{
					searchForEndAtLevel = curLevel-1;
				}
				else if (curLevel-1==searchForElseOrEndAtLevel)
				{
					searchForElseOrEndAtLevel = -1;
				}
			}break;
			case 0x97: // setvar
				i++;
				setvarVar = &(inputs->at(confScript[i]-1));
				if (!inSetvar)
				{
					inSetvar = true;
					goto afterActuallySettingVar;
				}
			break;
			case 0x20:{ // string
				dataTypes->push_back(c);
				i++;
				string asd = strings->at(confScript[i]-1);
				datas->push_back(asd);
				if (debug) printf("\n#####confScript[%i]=%s=strid=%s", i, bin2hex(confScript[i]).c_str(), asd.c_str());
			}break;
			case 0x21:{ // regex
				dataTypes->push_back(c);
				i++;
				if (debug) printf("\n#####confScript[%i]=%s", i, bin2hex(confScript[i]).c_str());
				datas->push_back(regexs->at(confScript[i]-1));
			}break;
			case 0x22:{ // input
				dataTypes->push_back(0x20);
				i++;
				if (debug) printf("\n#####confScript[%i]=%s", i, bin2hex(confScript[i]).c_str());
				string* inputName = &inputs->at(confScript[i]-1);
				if (*inputName=="path")
				{
					datas->push_back(requestPath);
				}
				else if (*inputName=="content")
				{
					datas->push_back(requestContent);
				}
				else if (*inputName=="sourceip")
				{
					datas->push_back(requestSender);
				}
				else if (*inputName=="requesttype")
				{
					if (postKeys->size()==0) datas->push_back(strGET);
					else datas->push_back(strPOST);
				}
				else if (*inputName=="responsecode")
				{
					datas->push_back(to_string(responseCode));
				}
				else if (*inputName=="getparams")
				{
					if (getChanged) { getChanged = false; requestParams = generateParamString(getKeys, getValues); }
					datas->push_back(requestParams);
				}
				else if (*inputName=="postparams")
				{
					if (postChanged) { postChanged = false; requestContent = generateParamString(postKeys, postValues); }
					datas->push_back(requestContent);
				}
				else printf("\n%s%s", "Could not interpret input ", inputName->c_str());
			}break;
			case 0x25:{ // header[...]
				i++;
				string* inputName = &inputs->at(confScript[i]-1);
				for (int j=0;j<32;j++)
				{
					if (keys[j]==*inputName)
					{
						dataTypes->push_back(0x20);
						datas->push_back(values[j]);
						break;
					}
					if (j==31)
					{
						if (verbose) printf("\n%s%s%s", "Notice: Header '", inputName->c_str(), "' not present!");
						dataTypes->push_back(0x20);
						datas->push_back(emptyStr);
					}
				}
			}break;
			case 0x27:{ // get[...]
				if (datas->size()==0 && (executingCommand==0x7B || executingCommand==0x7C))
				{
					if (debug) printf("\nadded a 0x27 onto datas stack");
					dataTypes->push_back(0x27);
					i++;
					datas->push_back(inputs->at(confScript[i]-1));
					break;
				}
				if (debug) printf("\n0x27 get[...]\n");
				dataTypes->push_back(0x20);
				i++;
				string* inputName = &inputs->at(confScript[i]-1);
				for (int j=0;j<getKeys->size();j++)
				{
					if (getKeys->at(j)==*inputName)
					{
						if (debug) printf("\nCP01\n");
						datas->push_back(getValues->at(j));
						goto foundGet;
					}
				}
				if (verbose) printf("\n%s%s%s", "Notice: Get parameter '", inputName->c_str(), "' not present!");
				datas->push_back(emptyStr);
			foundGet:;
			}break;
			case 0x28:{ // post[...]
				if (datas->size()==0 && (executingCommand==0x7B || executingCommand==0x7C))
				{
					dataTypes->push_back(0x28);
					i++;
					datas->push_back(inputs->at(confScript[i]-1));
					break;
				}
				dataTypes->push_back(0x20);
				i++;
				string* inputName = &inputs->at(confScript[i]-1);
				for (int j=0;j<postKeys->size();j++)
				{
					if (postKeys->at(j)==*inputName)
					{
						datas->push_back(postValues->at(j));
						goto foundPost;
					}
				}
				if (verbose) printf("\n%s%s%s", "Notice: Post parameter '", inputName->c_str(), "' not present!");
				datas->push_back(emptyStr);
			foundPost:;
			}break;
			case 0x29:{ // var[...]
				if (datas->size()==0 && (executingCommand==0x7B || executingCommand==0x7C))
				{
					dataTypes->push_back(0x29);
					i++;
					datas->push_back(inputs->at(confScript[i]-1));
					break;
				}
				dataTypes->push_back(0x20);
				i++;
				string* inputName = &inputs->at(confScript[i]-1);
				for (int j=0;j<varKeys->size();j++)
				{
					if (varKeys->at(j)==*inputName)
					{
						datas->push_back(varValues->at(j));
						goto foundVar;
					}
				}
				printf("\n%s%s%s", "[Error] Variable '", inputName->c_str(), "' does not exist!");
				goto bytecodeDone;
				datas->push_back(emptyStr);
			foundVar:;
			}break;
			case 0x4D:{ // replace
				unsigned char d1 = dataTypes->at(dataTypes->size()-3);
				unsigned char d2 = dataTypes->at(dataTypes->size()-2);
				unsigned char d3 = dataTypes->back();
				if (d1==0x20 && d2==0x20 && d3==0x20)
				{
					string s = string(datas->at(datas->size()-3));
					if (debug) printf("\nReplacing in s=%s", s.c_str());
					int dsSize = datas->size();
					replaceByPointers(s, &(datas->at(dsSize-2)), &(datas->back()));
					dataTypes->pop_back();
					dataTypes->pop_back();
					dataTypes->pop_back();
					datas->pop_back();
					datas->pop_back();
					datas->pop_back();
					dataTypes->push_back((unsigned char)0x20);
					datas->push_back(s);
				}
				else if (d1==0x20 && d2==0x21 && d3==0x20)
				{
					printf("\nRegex replace is not supported yet.");
				}
				else printf("\n%s", "Note: Did not execute replace yet, didn't find three strings");
			}break;
			case 0x41:{ // equals
				unsigned char d1 = dataTypes->at(dataTypes->size()-2);
				unsigned char d2 = dataTypes->back();
				if (d1==0x20 && d2==0x20) // two strings
				{
					string s2 = datas->back();
					datas->pop_back();
					dataTypes->pop_back();
					string s1 = datas->back();
					datas->pop_back();
					dataTypes->pop_back();
				
					if (s1==s2)
					{ // True
						dataTypes->push_back(0x24);
						datas->push_back(boolTrue);
					}
					else
					{ // False
						dataTypes->push_back(0x24);
						datas->push_back(boolFalse);
					}
				}
				else if (d1==0x20 && d2==0x21) // string & regex
				{
					string s2 = datas->back();
					datas->pop_back();
					dataTypes->pop_back();
					string s1 = datas->back();
					datas->pop_back();
					dataTypes->pop_back();
					try
					{
						if (boost::regex_match(s1.c_str(), boost::regex(s2.c_str())))
						{ // True
							dataTypes->push_back(0x24);
							datas->push_back(boolTrue);
						}
						else
						{ // False
							dataTypes->push_back(0x24);
							datas->push_back(boolFalse);
						}
					}
					catch (boost::regex_error& e)
					{
						dataTypes->push_back(0x24);
						datas->push_back(boolFalse);
						printf("\n%s%s%s%s%s", "[Notice] Regex error in /", s2.c_str(), "/ while matching \"", s1.c_str(), "\":");
						printRegexError(e);
					}
				}
				else printf("\n%s", "Note: Did not execute equals yet, didn't find two strings");
			}
			break;
			case 0x42: // contains
			{
				unsigned char d2 = dataTypes->back();
				unsigned char d1 = dataTypes->at(dataTypes->size()-2);
				if (d1==0x20 && d2==0x20) // two strings
				{
					string s2 = datas->back();
					datas->pop_back();
					dataTypes->pop_back();
					string s1 = datas->back();
					datas->pop_back();
					dataTypes->pop_back();
				
					if (s1.find(s2) != std::string::npos)
					{ // True
						dataTypes->push_back(0x24);
						datas->push_back(boolTrue);
					}
					else
					{ // False
						dataTypes->push_back(0x24);
						datas->push_back(boolFalse);
					}
				}
				else if (d1==0x20 && d2==0x21)
				{
					
					string s2 = datas->back();
					datas->pop_back();
					dataTypes->pop_back();
					string s1 = datas->back();
					datas->pop_back();
					dataTypes->pop_back();
					try
					{
						boost::smatch m;
						if (boost::regex_search(
							s1,
							m,
							boost::regex(s2.c_str())
						))
						{ // True
							dataTypes->push_back(0x24);
							datas->push_back(boolTrue);
						}
						else
						{ // False
							dataTypes->push_back(0x24);
							datas->push_back(boolFalse);
						}
					}
					catch (boost::regex_error& e)
					{
						dataTypes->push_back(0x24);
						datas->push_back(boolFalse);
						printf("\n%s%s%s%s%s", "[Notice] Regex error in /", s2.c_str(), "/ while matching \"", s1.c_str(), "\":");
						printRegexError(e);
					}
				}
				else printf("\n%s", "Note: Did not execute contains yet, didn't find two strings");
			}break;
			case 0x4A:{ // startswith
				unsigned char d2 = dataTypes->back();
				unsigned char d1 = dataTypes->at(dataTypes->size()-2);
				if (d1==0x20 && d2==0x20) // two strings
				{
					string s2 = datas->back();
					datas->pop_back();
					dataTypes->pop_back();
					string s1 = datas->back();
					datas->pop_back();
					dataTypes->pop_back();
					if (s2.length()>s1.length())
					{ // False
						dataTypes->push_back(0x24);
						datas->push_back(boolFalse);
					}
					else if (s1.substr(0, s2.length())==s2)
					{ // True
						dataTypes->push_back(0x24);
						datas->push_back(boolTrue);
					}
					else
					{ // False
						dataTypes->push_back(0x24);
						datas->push_back(boolFalse);
					}
				}
				else printf("\n%s", "Note: Did not execute startswith yet, didn't find two strings");
			}break;
			case 0x4B:{ // endswith
				unsigned char d2 = dataTypes->back();
				unsigned char d1 = dataTypes->at(dataTypes->size()-2);
				if (d1==0x20 && d2==0x20) // two strings
				{
					string s2 = datas->back();
					datas->pop_back();
					dataTypes->pop_back();
					string s1 = datas->back();
					datas->pop_back();
					dataTypes->pop_back();
					if (s2.length()>s1.length())
					{ // False
						dataTypes->push_back(0x24);
						datas->push_back(boolFalse);
					}
					else if (s1.substr(s1.length()-s2.length(), s2.length())==s2)
					{ // True
						dataTypes->push_back(0x24);
						datas->push_back(boolTrue);
					}
					else
					{ // False
						dataTypes->push_back(0x24);
						datas->push_back(boolFalse);
					}
				}
				else printf("\n%s", "Note: Did not execute startswith yet, didn't find two strings");
			}break;
			case 0x49:{ // concat
				unsigned char d2 = dataTypes->back();
				unsigned char d1 = dataTypes->at(dataTypes->size()-2);
				if (d1==0x20 && d2==0x20) // two strings
				{
					string allStrings = "";
					for (int k=0;k<strings->size();k++)
					{
						allStrings += strings->at(k).c_str();
						allStrings += '-';
					}
					if (debug) printf("\nallStrings=%s", allStrings.c_str());
					
					string s2 = datas->back();
					datas->pop_back();
					dataTypes->pop_back();
					
					string s1 = datas->back();
					datas->pop_back();
					dataTypes->pop_back();
					
					dataTypes->push_back(0x20);
					
					string asd = string(s1+s2);
					datas->push_back(asd);
					
					if (debug) printf("\nconcatinated str=%s", asd.c_str());
				}
				else if (debug) printf("\n%s", "Note: Did not execute concat yet, didn't find two strings");
			}break;
			default:
			if (c==0x43 || c==0x44 || c==0x45 || c==0x46 || c==0x47 || c==0x48)
			{
				unsigned char d1 = dataTypes->back();
				unsigned char d2 = dataTypes->at(dataTypes->size()-2);
				if (d1==0x24 && d2==0x24)
				{
					bool b1 = datas->back()==boolTrue;
					bool b2 = datas->at(datas->size()-2)==boolTrue;
					dataTypes->pop_back();
					datas->pop_back();
					datas->pop_back();
					bool result = false;
					if (c==0x43) result = b1&&b2; // And
					if (c==0x44) result = b1||b2; // Or
					if (c==0x45) result = !(b1&&b2); // Nand
					if (c==0x46) result = !(b1||b2); // Nor
					if (c==0x47) result = b1^b2; // Xor
					if (c==0x48) result = !(b1^b2); // Xnor
					if (result) datas->push_back(boolTrue);
					else datas->push_back(boolFalse);
				}
				else printf("\n%s", "Note: Did not execute logic operator yet, didn't find two booleans");
			}
			else if (c==0x40) // Not
			{
				if (datas->back()=="") // False -> True
				{
					datas->pop_back();
					datas->push_back(boolTrue);
				}
				else // True -> False
				{
					datas->pop_back();
					datas->push_back(boolFalse);
				}
			}
			else
			{
				if (inIf)
				{
					inIf = false;
					if (datas->back()=="") // False
					{
						datas->clear();
						dataTypes->clear();
						searchForElseOrEndAtLevel = curLevel-1;
						continue;
					}
					datas->clear();
					dataTypes->clear();
				}
			}
			// HERE WAS THE STUFF UNDER IMPORTANTMOD2141
			if (c>=0x70 && c<=0x7F)
			{
				executingCommand = c;
			}
			else if (c==0x95)
			{
				goto bytecodeDone;
			}
			// End of default:
			} // End of switch(){
			if (inSetvar&&((c>=0x91&&c<=0x9F)||(c>=0x70&&c<=0x7F)))
			{
				if (c!=0x97) inSetvar = false;
				for (int j=0;j<varKeys->size();j++)
				{
					if (varKeys->at(j)==*setvarVar)
					{
						varValues->at(j) = datas->back();
						goto foundVarAndSet2;
					}
				}
				varKeys->push_back(*setvarVar);
				varValues->push_back(datas->back());
			foundVarAndSet2:
				datas->clear();
				dataTypes->clear();
			}
		afterActuallySettingVar:;
			
		}
	}
	if (curLevel!=0) printf("\n%s", "[Notice] Config script missing an \"end\"");
bytecodeDone:
	datas->clear();
	dataTypes->clear();
	if (verbose)
	{
		auto endTimeBytecode = std::chrono::system_clock::now();
		auto elapsed = endTimeBytecode - startTimeBytecode;
		double ms = double(elapsed.count())/1000000;
		printf("\n%sExecuting config script took: %fms%s", BLU, ms, NRM);
	}
}
