#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>

#include "main.h"
#include "compiler.h"
#include "parser.h"
#include "definitions.h"

using namespace std; // this is a commit test
// this is a second commit test

string compileConfigScript(string confFile, vector<string>* strings, vector<string>* regexs, vector<string>* inputs)
{
	auto startTimeCompile = std::chrono::system_clock::now();
	std::stringstream byteCode;
	// Prepiling (lol.)
	std::stringstream curThing;
	
	// Compiling (lol.)
	std::stringstream theThing;
	vector<unsigned char>* operatorStack = new vector<unsigned char>();
	vector<unsigned char>* dataStack = new vector<unsigned char>();
	
	unsigned char bcStringCount = 0;
	unsigned char bcRegexCount = 0;
	unsigned char bcInputCount = 0;
		
	bool inString = false;
	bool inRegex = false;
	bool inStatement = false;
	bool inSubstatement = false;
	bool beginningOfLine = true;
	bool inCommandStatement = false; // e.g. Redirect
	
	int inBrackets = 0;
	
	int line = 1;
	int col = 0;
	
	for(int i=0; i<confFile.length(); i++)
	{
		if (beginningOfLine)
		{
			col = 0;
			if (inBrackets!=0)
			{
				if (inBrackets==1) printf("\n%s[Notice]%s One bracket was left unclosed on line %i.\n", BLU, NRM, line-1);
				else if (inBrackets>1) printf("\n%s[Notice]%s %i brackets were left unclosed on line %i.\n", BLU, NRM, inBrackets, line-1);
				inBrackets = 0;
			}
		ignoreStartingSpaces:
			while (confFile[i]=='\n' || confFile[i]=='\t' || confFile[i]=='\r' || confFile[i]==' ')
			{
				col++;
				i++;
			}
			if (confFile[i]=='#')
			{
				while (confFile[i]!='\n')
				{
					i++;
				}
				col = 0;
				i++;
				goto ignoreStartingSpaces;
			}
		}
		col++;
		// Brackets :] :) :} :{ :( :[
		if ((!inRegex)&&(!inString)&&(confFile[i]=='(')) // Enter bracketed part
		{
			if (debug) printf("\nENTER BRACKET");
			dataStack->push_back((unsigned char)0x26);
			operatorStack->push_back((unsigned char)0x26);
			inBrackets++;
		}
		else if ((!inRegex)&&(!inString)&&(confFile[i]==')')) // Exit bracketed part
		{
			// Search back for 0x26's and delete em
			for (int j=dataStack->size()-1;j>=0;j--)
			{
				if (dataStack->at(j)==0x26)
				{
					dataStack->erase(dataStack->begin()+j);
					break;
				}
				if (j==0)
				{
					printf("\n%s[Notice]%s [Line %i;Col %i] Unmatched closing bracket ignored.", BLU, NRM, line, col);
					goto endOfRemoving0x26;
				}
			}
			for (int j=operatorStack->size()-1;j>=0;j--)
			{
				if (operatorStack->at(j)==0x26)
				{
					operatorStack->erase(operatorStack->begin()+j);
					break;
				}
				if (j==0)
				{
					printf("\n%s[Severe Error] [Line %i;Col %i] OMFG THE STRANGEST ERROR EVER CONTACT JESBUS IMMEDIATELY. THIS COULD SHOULD NEVER BE REACHED. WTF. HELP. HALF LIFE 3 CONFIRMED. END OF THE WORLD 6/6/2066. MAYA CALENDAR FOUND IN INCA EMPIRE PYRAMID DURING SPANISH INQUISITION. CHEMTRAILS DISABLE FREE 3N3RGY DEVICES. V14GRA C1ALIS FREE SAMPLES.%s", RED, line, col, NRM);
					return string("");
				}
			}
			endOfRemoving0x26:
			inBrackets--;
		}
		else if ((!inRegex)&&(!inString)&&(confFile[i]=='"')) // Enter string
		{
			inString = true;
		}
		else if (inString&&(confFile[i]=='"')) // Exit string
		{
			inString = false;
			string s = curThing.str();
			strings->push_back(s);//
			if (inCommandStatement && inBrackets==0){}else dataStack->push_back(0x20);
			byteCode << (unsigned char)0x20;
			bcStringCount++;
			byteCode << (unsigned char)bcStringCount;
			curThing.str("");
		}
		else if ((!inRegex)&&(!inString)&&(confFile[i]=='/')) // Enter regex
		{
			inRegex = true;
		}
		else if (inRegex&&(confFile[i]=='/'))//(confFile[i]==' ' || confFile[i]==':' || confFile[i]=='\n' || confFile[i]=='\r' || confFile[i]=='\t')) // Exit regex
		{
			inRegex = false;
			string s = curThing.str();
			regexs->push_back(string(s));
			if (inCommandStatement && inBrackets==0){}else dataStack->push_back(0x21);
			byteCode << (unsigned char)0x21;
			bcRegexCount++;
			byteCode << (unsigned char)bcRegexCount;
			curThing.str("");
		}
		else if (inRegex||inString) // Buffer regex & string
		{
			curThing << confFile[i];
		}
		else if ((!inStatement)&&beginningOfLine) // Enter statement
		{
			inStatement = true;
		}
		else if (inStatement&&(confFile[i]==' ' || confFile[i]==':' || confFile[i]=='\n' || confFile[i]=='\r' || confFile[i]=='\t')) // Exit statement
		{
			inStatement = false;
			std::string s = curThing.str();
			curThing.str("");
			if (s=="if") byteCode << (unsigned char)0x92;
			else if (s=="end")
			{
				byteCode << (unsigned char)0x91;
			}
			else if (s=="else") byteCode << (unsigned char)0x93;
			else if (s=="elseif") byteCode << (unsigned char)0x94;
			else if (s=="exit") byteCode << (unsigned char)0x95;
			else if (s=="Redirect")
			{
				byteCode << (unsigned char)0x70;
				inCommandStatement = true;
			}
			else if (s=="ResponseCode")
			{
				byteCode << (unsigned char)0x71;
				inCommandStatement = true;
			}
			else if (s=="ResponseStatus")
			{
				byteCode << (unsigned char)0x72;
				inCommandStatement = true;
			}
			else if (s=="Header")
			{
				byteCode << (unsigned char)0x73;
				inCommandStatement = true;
			}
			else if (s=="Rewrite")
			{
				byteCode << (unsigned char)0x74;
				inCommandStatement = true;
			}
			else if (s=="Nop")
			{
				byteCode << (unsigned char)0x77;
			}
			else if (s=="SetPath")
			{
				byteCode << (unsigned char)0x78;
				inCommandStatement = true;
			}
			else if (s=="ReplacePath")
			{
				byteCode << (unsigned char)0x79;
				inCommandStatement = true;
			}
			else if (s=="Recurse")
			{
				byteCode << (unsigned char)0x7A;
			}
			else if (s=="Set")
			{
				byteCode << (unsigned char)0x7B;
				inCommandStatement = true;
			}
			else if (s=="Remove")
			{
				byteCode << (unsigned char)0x7C;
				inCommandStatement = true;
			}
			else if (s=="RemoveAllGet")
			{
				byteCode << (unsigned char)0x7D;
			}
			else if (s=="RemoveAllPost")
			{
				byteCode << (unsigned char)0x7E;
			}
			else if (s=="Log")
			{
				byteCode << (unsigned char)0x7F;
				inCommandStatement = true;
			}
			else
			{
				printf("\n%s[Error]%s [Line %i;Col %i-%i] Wtf is '%s'", RED, NRM, line, col-s.length(), col, s.c_str());
				byteCode.str("");
				return string("");
			}
		}
		else if ((!inSubstatement)&&(!inStatement)&&!(confFile[i]==' ' || confFile[i]==':' || confFile[i]=='\n' || confFile[i]=='\r' || confFile[i]=='\t')) // Enter substatement
		{
			inSubstatement = true;
		}
		else if (inSubstatement&&(confFile[i]==' ' || confFile[i]==':' || confFile[i]=='\n' || confFile[i]=='\r' || confFile[i]=='\t' || confFile[i]==')')) // Exit substatement
		{
			inSubstatement = false;
			string s = curThing.str();
			curThing.str("");
			
			if (s=="not") operatorStack->push_back(0x40);
			else if (s=="then") byteCode << (unsigned char)0x96;
			else if (s=="equals") operatorStack->push_back(0x41);
			else if (s=="contains") operatorStack->push_back(0x42);
			else if (s=="concat" || s=="&" || s==".") operatorStack->push_back(0x49);
			else if (s=="and") operatorStack->push_back(0x43);
			else if (s=="or") operatorStack->push_back(0x44);
			else if (s=="nand") operatorStack->push_back(0x45);
			else if (s=="nor") operatorStack->push_back(0x46);
			else if (s=="xor") operatorStack->push_back(0x47);
			else if (s=="xnor") operatorStack->push_back(0x48);
			else if (s=="startswith") operatorStack->push_back(0x4A);
			else if (s=="endswith") operatorStack->push_back(0x4B);
			
			else if (s=="in") operatorStack->push_back(0x4C);
			else if (s=="replace") operatorStack->push_back(0x4D);
			else if (s=="with") operatorStack->push_back(0x4E);
			
			else if (s=="path" || s=="content" || s=="sourceip" || s=="getparams" || s=="postparams" || s=="requesttype" || s=="responsecode")
			{
				inputs->push_back(string(s));
				dataStack->push_back(0x22);
				byteCode << (unsigned char)0x22;
				bcInputCount++;
				byteCode << (unsigned char)bcInputCount;
			}
			else if (s.length()>7 && s.substr(0, 7)=="header[")
			{
				inputs->push_back(s.substr(7, s.length()-8));
				if (inCommandStatement && inBrackets==0){}else dataStack->push_back(0x25);
				byteCode << (unsigned char)0x25;
				bcInputCount++;
				byteCode << (unsigned char)bcInputCount;
			}
			else if (s.length()>4 && s.substr(0, 4)=="get[")
			{
				inputs->push_back(s.substr(4, s.length()-5));
				if (inCommandStatement && inBrackets==0){}else dataStack->push_back(0x27);
				byteCode << (unsigned char)0x27;
				bcInputCount++;
				byteCode << (unsigned char)bcInputCount;
			}
			else if (s.length()>5 && s.substr(0, 5)=="post[")
			{
				inputs->push_back(s.substr(5, s.length()-6));
				if (inCommandStatement && inBrackets==0){}else dataStack->push_back(0x28);
				byteCode << (unsigned char)0x28;
				bcInputCount++;
				byteCode << (unsigned char)bcInputCount;
			}
			else if (s.length()>4 && s.substr(0, 4)=="var[")
			{
				inputs->push_back(s.substr(4, s.length()-5));
				if (inCommandStatement && inBrackets==0){}else dataStack->push_back(0x29);
				byteCode << (unsigned char)0x29;
				bcInputCount++;
				byteCode << (unsigned char)bcInputCount;
			}
			else
			{
				printf("\n%s[Error]%s [Line %i;Col %i-%i] Wtf is '%s'", RED, NRM, line, col-s.length(), col, s.c_str());
				byteCode.str("");
				return string("");
			}
		}
		int oss = -1;
		for (int k=0;k<1;k++)
		{
			if ((oss=operatorStack->size())!=0)
			{
				char lastOp = operatorStack->back();
				int dsSize = dataStack->size();
				if (lastOp==0x41 ||
					lastOp==0x42 ||
					lastOp==0x43 ||
					lastOp==0x44 ||
					lastOp==0x45 ||
					lastOp==0x46 ||
					lastOp==0x47 ||
					lastOp==0x48 ||
					lastOp==0x49 ||
					lastOp==0x4A ||
					lastOp==0x4B) // Equals || Contains || Logicops || concat || startswith || endswith
				{
					if (dsSize>=2)
					{
						char d2 = dataStack->back();
						if (d2==0x26)
						{
							break;
						}
						dataStack->pop_back();
						char d1 = dataStack->back();
						if (d1==0x26)
						{
							dataStack->push_back(d2);
							break;
						}
						dataStack->pop_back();
						operatorStack->pop_back();
						
						byteCode << (unsigned char)lastOp;
						dataStack->push_back(0x23);
					}
				}
				else if (lastOp==0x40) // Not
				{
					if (dsSize>=1)
					{
						unsigned char d1 = dataStack->back();
						if (d1==0x23)
						{
							dataStack->pop_back();
							operatorStack->pop_back();
							byteCode << (unsigned char)0x40;
							dataStack->push_back(0x23);
						}
					}
				}
				else
				{
					int osSize = operatorStack->size();
					if (osSize>=3)
					{
						unsigned char o1 = operatorStack->at(osSize-3);
						unsigned char o2 = operatorStack->at(osSize-2);
						unsigned char o3 = operatorStack->at(osSize-1);
						if ((o1==0x4C) && (o2==0x4D) && (o3==0x4E)) // replace
						{
							unsigned char d3 = dataStack->back();
							if (d3==0x26)
							{
								break;
							}
							dataStack->pop_back();
							unsigned char d2 = dataStack->back();
							if (d2==0x26)
							{
								dataStack->push_back(d3);
								break;
							}
							dataStack->pop_back();
							unsigned char d1 = dataStack->back();
							if (d1==0x26)
							{
								dataStack->push_back(d2);
								dataStack->push_back(d3);
								break;
							}
							dataStack->pop_back();
							
							operatorStack->pop_back();
							operatorStack->pop_back();
							operatorStack->pop_back();

							byteCode << (unsigned char)0x4D;
							dataStack->push_back(0x23);
						}
					}
				}
			}
		}
	bufferAndStuffer:
		if (inStatement||inSubstatement) curThing << confFile[i]; // Buffer statement
		beginningOfLine = confFile[i]=='\n' || confFile[i]=='\r' || ((!inString)&&(!inRegex)&&(confFile[i]==':'));
		if (beginningOfLine)
		{
			if (dataStack->size()!=0)
			{
				unsigned char d1 = dataStack->back();
				if (d1==0x20) // String
				{
					byteCode << (unsigned char)d1;
					bcStringCount++;
					byteCode << (unsigned char)bcStringCount;
				}
			}
			else if (operatorStack->size()!=0)
			{
				printf("\n%s[Error]%s [Line %i;Col %i] Insufficient data for operator.", RED, NRM, line, col);
				return string("");
			}
			operatorStack->clear();
			dataStack->clear();
			line++;
			inCommandStatement = false;
		}
	}
	byteCode << (unsigned char)0x77;
	byteCode << (unsigned char)0xFF;
	
	auto endTimeCompile = std::chrono::system_clock::now();
	auto elapsed = endTimeCompile - startTimeCompile;
	double ms = double(elapsed.count())/1000000;
	if (verbose) printf("\n%sCompilation took: %fms%s", BLU, ms, NRM);
	
	if (verbose) printf("%s", "\nDone with compiling config script");
	return byteCode.str();
}
