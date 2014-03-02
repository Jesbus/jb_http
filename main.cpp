#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <string>
#include <vector>
#include <errno.h>
#include <iomanip>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/types.h>
#include <fcntl.h>
#include <chrono>

#include "compiler.h"
#include "parser.h"
#include "main.h"
#include "interpreter.h"
#include "definitions.h"

using namespace std;

bool verbose = false, teapot = false, php = false, python = false, perl = false, quiet = false, debug = false;

bool is_dir(const char* path)
{
	struct stat buf;
	stat(path, &buf);
	return S_ISDIR(buf.st_mode);
}
void sigchld_handler(int s)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
int main2(int argc, char* argv[]);
int main(int argc, char* argv[])
{
	main2(argc, argv);
	printf("\n");
	return 0;
}
int main2(int argc, char* argv[])
{
	char* port = "80";
	char* directory = ".";
	vector<string> indexes;
	bool addingIndexes = false;
	int backlog = 10;
	for (int i=1;i<argc;i++)
	{
		std::string arg = std::string(argv[i]);
		
		if (addingIndexes)
		{
			if (argv[i][0]!='-')
			{
				indexes.push_back(arg);
				continue;
			}
			else addingIndexes = false;
		}
		
			 if (arg=="-v" || arg=="--verbose") verbose = true;
		else if (arg=="-q" || arg=="--quiet") quiet = true;
		else if (arg=="-t" || arg=="--teapot") teapot = true;
		else if (arg=="-php" || arg=="--php") php = true;
		else if (arg=="-py" || arg=="--python") python = true;
		else if (arg=="-perl" || arg=="--perl") perl = true;
		else if (arg=="--debug") { debug = true; verbose = true; }
		else if (arg=="-d" || arg=="--directory") { directory = argv[i+1]; i++; }
		else if (arg=="-i" || arg=="--index") { addingIndexes = true; }
		else if (arg=="-b" || arg=="--backlog") { backlog = std::stoi(std::string(argv[i+1])); i++; }
		else if (arg=="-p" || arg=="--port")
		{
			int iPort = std::stoi(std::string(port));
			if (std::to_string(iPort)!=port)
			{
				printf("\nPort has to be an int");
				return 0;
			}
			if (iPort<0 || iPort>=65536)
			{
				printf("\nPort has to be from 0 to 65535");
			}
			port = argv[i+1];
			i++;
		}
		else if (arg=="-pyh" || arg=="--pythonh")
		{
			printf(  "%s###########################%s", YEL, NRM);
			printf("\n%s## %sJesbus' HTTP server python help%s", YEL, BLU, NRM);
			printf("\n%s##%s Python scripts are executed when having the extension .py", YEL, NRM);
			printf("\n%s##%s GET parameters  = ast.literal_eval(sys.argv[1])", YEL, NRM);
			printf("\n%s##%s POST parameters = ast.literal_eval(sys.argv[2])", YEL, NRM);
			printf("\n%s##%s print command can be used to output stuff", YEL, NRM);
			printf("\n%s##%s That's it.. Good fortune on the road.", YEL, NRM);
			printf("\n%s###########################%s\n", YEL, NRM);
			return 0;
		}
		else if (arg=="-peh" || arg=="--perlh")
		{
			printf(  "%s###########################", YEL);
			printf("\n%s## %sJesbus' HTTP server perl help%s", YEL, BLU, NRM);
			printf("\n%s##%s Perl scripts are executed when having the extension .pl", YEL, NRM);
			printf("\n%s##%s GET parameters  : $ARGV[0]", YEL, NRM);
			printf("\n%s##%s POST parameters : $ARGV[1]", YEL, NRM);
			printf("\n%s##%s You'll have to parse them yourself.", YEL, NRM);
			printf("\n%s###########################%s\n", YEL, NRM);
			return 0;
		}
		else if (arg=="--phphelp")
		{
			printf(  "%s###########################", YEL);
			printf("\n%s## %sJesbus' HTTP server PHP help%s", YEL, BLU, NRM);
			printf("\n%s##%s PHP scripts are executed when having the extension .php", YEL, NRM);
			printf("\n%s##%s GET parameters  : parse_str($_SERVER['argv'][1], $_GET)", YEL, NRM);
			printf("\n%s##%s POST parameters : parse_str($_SERVER['argv'][2], $_POST)", YEL, NRM);
			printf("\n%s##%s That's it.. Good fortune on the road.", YEL, NRM);
			printf("\n%s###########################%s\n", YEL, NRM);
			return 0;
		}
		else if (arg=="-h" || arg=="--help" || arg=="help" || arg=="-help" || arg=="?")
		{
			printf("%s###########################", YEL);
			printf("\n%s## %sJesbus' HTTP server help%s", YEL, BLU, NRM);
			printf("\n%s## %sShort   Long        Meaning                     Default%s", YEL, RED, NRM);
			printf("\n%s##%s -h      --help      Show this help              Nope"      , YEL, NRM);
			printf("\n%s##%s -v      --verbose   Enable advanced logging     Disabled"  , YEL, NRM);
			printf("\n%s##%s -q      --quiet     Disable connection logging  Disabled"  , YEL, NRM);
			printf("\n%s##%s -p      --port      Set the port to listen to   80"        , YEL, NRM);
			printf("\n%s##%s -d      --directory Set the directory           ./"        , YEL, NRM);
			printf("\n%s##%s -t      --teapot    Enable teapot mode          Disabled"  , YEL, NRM);
			printf("\n%s##%s -php    --php       Enable PHP                  Disabled"  , YEL, NRM);
			printf("\n%s##%s         --phphelp   Show help for PHP           Disabled"  , YEL, NRM);
			printf("\n%s##%s -py     --python    Enable Python               Disabled"  , YEL, NRM);
			printf("\n%s##%s -pyh    --pythonh   Show help for Python        Nope"      , YEL, NRM);
			printf("\n%s##%s -perl   --perl      Enable Perl                 Disabled"  , YEL, NRM);
			printf("\n%s##%s -peh    --perlh     Show help for Perl          Nope"      , YEL, NRM);
			printf("\n%s##%s -i      --index     Set the index filenames     index.html ~.htm ~.php", YEL, NRM);
			printf("\n%s##%s -b      --backlog   Set the max. connections    10"        , YEL, NRM);
			printf("\n%s##%s -ch     --confhelp  Show help for jb_http.conf  Nope"      , YEL, NRM);
			printf("\n%s##%s         --debug     Enable debug mode           Nope"      , YEL, NRM);
			printf("\n%s###########################%s\n", YEL, NRM);
			return 0;
		}
		else if (arg=="-ch" || arg=="--confighelp")
		{
			printf("%s################################################################", YEL);
			printf("\n%s##%s Jesbus' HTTP server help for jb_http.conf scripting%s", YEL, BLU, NRM);
			printf("\n%s##%s An omnipotent example can say more than a 1000-page manual.%s", YEL, GRE, NRM);
			printf("\n%s##%s if not header[User-Agent] contains \"Firefox\"", YEL, NRM);
			printf("\n%s##%s 	Header \"Location: http://google.com/chrome\"", YEL, NRM);
			printf("\n%s##%s 	ResponseCode 302\"", YEL, NRM);
			printf("\n%s##%s 	exit", YEL, NRM);
			printf("\n%s##%s end", YEL, NRM);
			printf("\n%s##%s setvar var[testvar] ( get[hi] concat \"-\" concat get[bye] )", YEL, NRM);
			printf("\n%s##%s if ( header[host] contains /([a-z]{5})/ ) xor ( get[bla] equals \"hi\" )", YEL, NRM);
			printf("\n%s##%s 	SetPath \"/test.php\"", YEL, NRM);
			printf("\n%s##%s 	ResponseCode 200", YEL, NRM);
			printf("\n%s##%s else", YEL, NRM);
			printf("\n%s##%s 	if ( post[goto] equals \"Google\" )", YEL, NRM);
			printf("\n%s##%s 		Log ( \"Google goooog elgooG~\" concat var[testvar] )", YEL, NRM);
			printf("\n%s##%s 		RemoveAllGet", YEL, NRM);
			printf("\n%s##%s 		RemoveAllPost", YEL, NRM);
			printf("\n%s##%s 	end", YEL, NRM);
			printf("\n%s##%s end", YEL, NRM);
			printf("\n%s################################################################%s\n", YEL, NRM);
			return 0;
		}
		else
		{
			printf("\nIncomprehended argument: %s", arg.c_str());
			return 0;
		}
	}
	if (indexes.size()==0)
	{
		indexes.push_back(string("index.html"));
		indexes.push_back(string("index.htm"));
		indexes.push_back(string("index.php"));
	}
	
	if (teapot) verbose = false;
	
	if (!is_dir(directory))
	{
		printf("\nDirectory %s didn't exist, created it.", directory);
		mkdir(directory, 0777);
	}
	
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0)
	{
		fprintf(stderr, "Error: %s\n", gai_strerror(rv));
		return 1;
	}
	
	// loop through all the results and bind to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next)
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
		{
			perror("server: socket");
			continue;
		}
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
		{
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(sockfd);
			perror("\nFailed to bind to port");
			continue;
		}

		break;
	}

	if (p == NULL) 
	{
		//fprintf(stderr, "Failed to bind to port\n");
		printf("Failed to bind to port %i\n", port);
		return 2;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (listen(sockfd, backlog) == -1)
	{
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1)
	{
		perror("sigaction");
		exit(1);
	}
	
	
	vector<string>* strings = new vector<string>();
	vector<string>* regexs  = new vector<string>();
	vector<string>* inputs  = new vector<string>();
	
	if ((!verbose)&&(!quiet))
	{
					printf("%s##########################", RED);
		if (teapot)	printf("\n%s###%s Starting teapot... %s###", RED, BLU, RED);
		else		printf("\n%s###%s Starting server... %s###", RED, BLU, RED);
					printf("\n%s##########################%s", RED, NRM);
	}
	
	string byteCode;
	
	struct stat buffer;
	if (stat("jb_http.conf", &buffer)==0)
	{
		if (verbose) printf("\njb_http.conf file found! Reading and compiling it...");

		std::ifstream t("jb_http.conf");
		std::stringstream buffer;
		buffer << t.rdbuf();
		t.close();
		std::string confFile = buffer.str();
		
		byteCode = compileConfigScript(confFile, strings, regexs, inputs);
	}
	int byteCodeLength = byteCode.length();
	char* confScript = new char[byteCodeLength+1];
	for (int i=0;i<byteCodeLength;i++){confScript[i]='a';}
	confScript[byteCodeLength] = (char)0x00;
	
	strncpy(confScript, byteCode.c_str(), byteCodeLength);
	
	if (!quiet)
	{
		if (!teapot)
		{
			if (verbose) printf("\nServer is running!\n");
			else
			{
				printf("\n%s###%s Server is running! %s###", RED, BLU, RED);
				printf("\n%s##########################%s\n", RED, NRM);
			}
		}
		else
		{
			if (verbose) printf("\nTeapot is running!\n");
			else
			{
				printf("\n%s###%s Teapot is running! %s###", RED, BLU, RED);
				printf("\n%s##########################%s\n", RED, NRM);
			}
		}
	}
	while(1)
	{
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1)
		{
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
		
		
		if (!fork()) // Child process/thread (idk lol, im such a n00b)
		{
			close(sockfd); // child doesn't need the listener
			
			auto startTimeEverything = std::chrono::system_clock::now();
			
			// Request vars
			char* requestType;
			std::string requestLocation, requestContent, requestSender=s, requestPath, requestParams;
			
			// Reponse vars
			int responseCode = 200;
			std::string responseText = "OK";
			
			if (verbose) printf("\n%s######################################%s\nIncoming connection from %s%s%s", RED, NRM, GRE, requestSender.c_str(), NRM);
			else if (!quiet) printf("%s%s%s%s%s", "\n", GRE, requestSender.c_str(), NRM, ": ");

			

			////////////////////////////////////////////////
			// #### -------------- Read request
			
			int headerContentLength = -1;
			bool headerTransferEncodingChunked = false;
			string headerHost;
			
			std::string keys  [32];
			std::string values[32];
			
			{
				auto startTimeReceive = std::chrono::system_clock::now();
				char recvBuff[256];
				int amountReceived = 0;
				if (debug) printf("\nReceiving request...");
			
				std::stringstream headerSS;
				int blocksReceived = 0;
				bool shouldRequestBeParsed = true;
				if (teapot)
				{
					shouldRequestBeParsed = false;
					responseCode = 418;
					responseText = "I'm a teapot";
				}
				while ((amountReceived=recv(new_fd, recvBuff, 256, 0))>0)
				{
					if (shouldRequestBeParsed)
					{
						std::string str = std::string(recvBuff);
						headerSS << str.substr(0, amountReceived);
					}
					blocksReceived++;
					if (blocksReceived>8)
					{
						if (shouldRequestBeParsed)
						{
							shouldRequestBeParsed = false;
							responseCode = 413;
							headerSS.str("");
							headerSS.clear();
						}
					}
					else if (blocksReceived>128)
					{
						printf("\nRequest size is way too big. Connection cut off");
						close(new_fd);
						exit(0);
					}
					if (amountReceived!=256) break;
				}
				if (responseCode==413)
				{
					responseText = "Request exceeds 2 kiB! In fact, it's ";
					responseText += std::to_string(blocksReceived*256+amountReceived);
					responseText += " bytes!";
				}
				std::string header = headerSS.str();
			
				//printf("%s%s%s", "\n##----------\n", header.c_str(), "\n##------------\n");
			
				const char* headerCs = header.c_str();
			
				if (verbose)
				{
					printf("\nReceived request of %i bytes", blocksReceived*256+amountReceived);
					auto endTimeReceive = std::chrono::system_clock::now();
					auto elapsed = endTimeReceive - startTimeReceive;
					double ms = double(elapsed.count())/1000000;
					printf("\n%sReceiving request took: %fms%s", BLU, ms, NRM);
				}

				
				////////////////////////////////////////////////
				// #### -------------- Parse request
				
				if (shouldRequestBeParsed)
				{
					parseRequest
					(
						keys,
						values,
						requestType,
						requestLocation,
						requestContent,
						requestSender,
						requestPath,
						requestParams,
						responseCode,
						responseText,
						headerContentLength,
						headerTransferEncodingChunked,
						headerHost,
						header
					);
				}
				
			}
			
			////////////////////////////////////////////////
			// #### -------------- Parse get params
			
			vector<string>* getKeys   = new vector<string>();
			vector<string>* getValues = new vector<string>();
			
			parseParamString(requestParams, getKeys, getValues);
			
			////////////////////////////////////////////////
			// #### -------------- Parse post params
			
			vector<string>* postKeys = new vector<string>();
			vector<string>* postValues = new vector<string>();
			
			parseParamString(requestContent, postKeys, postValues);
			
			if (requestLocation.length()!=0)
			{
				if (verbose)
				{
					if (requestType[0]=='G') printf("\n##-------- Request is: %sGET%s to %s%s%s", GRE, NRM, GRE, requestLocation.c_str(), NRM);
					else printf("\n##-------- Request is: %sPOST%s to %s%s%s\nPOST data: \n%s", GRE, NRM, GRE, requestLocation.c_str(), NRM, requestContent.c_str());
				}
				else if (!quiet)
				{
					if (requestType[0]=='G') printf("\nGET %s%s%s%s", GRE, headerHost.c_str(), requestLocation.c_str(), NRM);
					else printf("%sPOST%s %s\nPost data: %s", GRE, NRM, requestLocation.c_str(), requestContent.c_str());
				}
			}
			
			
			////////////////////////////////////////////////
			// #### -------------- Search for requested file
			
			string filePath, fileName;
			
			if (!searchForFile(directory, requestPath, indexes, filePath, fileName))
			{
				responseCode = 404;
				responseText = "File not found";
			}
			
			string headerContentType = "text/plain";
			string headerContentDisposition = "";
			string headerServer = "Jesbus' server";
			string headerKeepAlive = "300";
			
			////////////////////////////////////////////////
			// #### -------------- Set some headers according to file extension
			
			bool isPhpFile = false, isPythonFile = false, isPerlFile = false;
			if (responseCode==200)
			{
				int fpLength = filePath.length();
				string ext3 = string(""), ext2 = string(""), ext4 = string("");
				if (fpLength>=3) ext2 = filePath.substr(fpLength-3, 3);
				if (fpLength>=4) ext3 = filePath.substr(fpLength-4, 4);
				if (fpLength>=5) ext4 = filePath.substr(fpLength-5, 5);
				if (ext3==".php")
				{
					isPhpFile = true;
					if (php) headerContentType = "text/html";
					else headerContentType = "text/plain";
				}
				else if (ext2==".py")
				{
					isPythonFile = true;
					if (python) headerContentType = "text/html";
					else headerContentType = "text/plain";
				}
				else if (ext2==".pl")
				{
					isPerlFile = true;
					if (perl) headerContentType = "text/html";
					else headerContentType = "text/plain";
				}
				else if (ext3==".htm" || ext4==".html") headerContentType = "text/html";
				else if (ext3==".txt") headerContentType = "text/plain";
				else if (ext3==".pdf") headerContentType = "application/pdf";
				else if (ext3==".ogg") headerContentType = "application/ogg";
				else if (ext3==".xml") headerContentType = "application/xml";
				else if (ext3==".xsl") headerContentType = "application/xml";
				else if (ext3==".zip") headerContentType = "application/zip";
				else if (ext4==".midi") headerContentType = "audio/midi";
				else if (ext4==".mpeg") headerContentType = "audio/mpeg";
				else if (ext3==".wav") headerContentType = "audio/x-wav";
				else if (ext3==".bmp") headerContentType = "image/bmp";
				else if (ext3==".gif") headerContentType = "image/gif";
				else if (ext3==".jpg" || ext4==".jpeg") headerContentType = "image/jpeg";
				else if (ext3==".png") headerContentType = "image/png";
				else if (ext3==".tif" || ext4==".tiff") headerContentType = "image/tiff";
				else if (ext3==".ico") headerContentType = "image/x-icon";
				else if (ext3==".css") headerContentType = "text/css";
				else if (ext3==".mpg" || ext4=="mpeg") headerContentType = "video/mpeg";
				else if (ext3==".avi") headerContentType = "video/x-msvideo";
				else
				{
					headerContentType = string("application/octet-stream");
					headerContentDisposition = string("Content-Disposition: attachment; filename=")+fileName+string(";");
				}
			}

			////////////////////////////////////////////////
			// #### -------------- Send response
			{
			
				// Execute bytecode script :) :( :S
				if (debug) printf("\nExecuting config script...");
				
				bool getChanged = false;
				bool postChanged = false;
				bool requestPathChanged = false;
				string addedHeaders = "";
				
				executeConfigScript
				(
					responseCode,
					responseText,
					confScript,
					addedHeaders,
					requestPath,
					requestParams,
					requestContent,
					requestSender,
					getChanged,
					postChanged,
					requestPathChanged,
					getKeys,
					getValues,
					postKeys,
					postValues,
					strings,
					regexs,
					inputs,
					keys,
					values,
					
					headerContentType,
					headerContentDisposition,
					headerServer,
					headerKeepAlive
				);
				
				if (requestPathChanged) filePath = (string(directory)+string(requestPath));
			
				// Use vector GET and POST values to reflect configScript's changes in requestContent & requestParams
				if (getChanged)
				{
					requestParams = generateParamString(getKeys, getValues);
				}
				if (postChanged)
				{
					requestContent = generateParamString(postKeys, postValues);
				}
				
				auto startTimeResponse = std::chrono::system_clock::now();
				
				std::string header;
				
				header  = "HTTP/1.1 "+std::to_string(responseCode)+" "+responseText+"\r\n";
				header += "Server: "+headerServer+"\r\n";
				header += "Keep-Alive: "+headerKeepAlive+"\r\n";
				header += "Transfer-Encoding: chunked\r\n";
				header += "Connection: close\r\n";
				header += "Content-Type: "+headerContentType+"\r\n";
				if (headerContentDisposition.length()!=0) header += "Content-Disposition: "+headerContentDisposition+"\r\n";
				header += "\r\n";
				
				if (debug) printf("\nSending response header:\n#---\n%s\n#---", header.c_str());
				
				send(new_fd, header.c_str(), header.length(), 0);
				
				FILE* fp;
				char result [256];
				if (php&&isPhpFile)
				{
					if (debug) printf("\nExecuting PHP script");
					fp = popen((std::string("php -f ")+filePath+std::string(" \"")+requestParams+std::string("\" \"")+requestContent+std::string("\"")).c_str(), "r");
				}
				else if (perl&&isPerlFile)
				{
					if (debug) printf("\nExecuting perl script");
					fp = popen((string("perl ")+filePath+string(" \"")+requestParams+string("\" \"")+requestContent+string("\"")).c_str(),"r");
				}
				else if (python&&isPythonFile)
				{
					if (debug) printf("\nExecuting python script");
					stringstream getArgs;
					getArgs << '{';
					int i = 0;
					if (requestParams.length()!=0) getArgs << "\\\"";
					bool seenEquals = false;
					for(i=0; i<requestParams.length(); i++)
					{
						switch(requestParams[i])
						{
							case '"':
								getArgs << "\\\\\\\"";
							break;
							case '&':
								if (!seenEquals) getArgs << "\\\":\\\"";
								getArgs << "\\\",\\\"";
								seenEquals = false;
							break;
							case '=':
								getArgs << "\\\":\\\"";
								seenEquals = true;
							break;
							default:
								getArgs << requestParams[i];
							break;
						}
					}
					if (i!=0)
					{
						if (!seenEquals) getArgs << "\\\":\\\"";
						getArgs << "\\\"";
					}
					getArgs << '}';
				
				
					stringstream postArgs;
					postArgs << '{';
					i = 0;
					if (requestContent.length()!=0) postArgs << "\\\"";
					seenEquals = false;
					for(i=0; i<requestContent.length(); i++)
					{
						switch(requestContent[i])
						{
							case '"':
								postArgs << "\\\\\\\"";
							break;
							case '&':
								if (!seenEquals) postArgs << "\\\":\\\"";
								postArgs << "\\\",\\\"";
								seenEquals = false;
							break;
							case '=':
								seenEquals = true;
								postArgs << "\\\":\\\"";
							break;
							default:
								postArgs << requestContent[i];
							break;
						}
					}
					if (i!=0)
					{
						if (!seenEquals) postArgs << "\\\":\\\"";
						postArgs << "\\\"";
					}
					postArgs << '}';
					fp = popen((string("python ")+filePath+string(" ")+getArgs.str()+string(" ")+postArgs.str()).c_str(),"r");
				}
				else
				{
					if (debug) printf("\nfopen(%s,r)", filePath.c_str());
					fp = fopen(filePath.c_str(), "r");
				}
				
				if (debug) printf("\nWriting content...");
				int resultSize = 0;
				while ((resultSize=(int)fread(result,1,256,fp))!=0)
				{
					std::stringstream strStream;
					strStream << std::hex << resultSize;
					std::string chunkSize(strStream.str());
		
					send(new_fd, chunkSize.c_str(), chunkSize.length(), 0);
					send(new_fd, "\r\n", 2, 0);
					send(new_fd, result, resultSize, 0);
					send(new_fd, "\r\n", 2, 0);
				}
				std::string endingChunk = "0\r\n\r\n";
				send(new_fd, endingChunk.c_str(), endingChunk.length(), 0);
				fclose(fp);
				if (verbose)
				{
					auto endTimeResponse = std::chrono::system_clock::now();
					auto elapsed = endTimeResponse - startTimeResponse;
					double ms = double(elapsed.count())/1000000;
					printf("\n%sGenerating & sending response took: %fms%s", BLU, ms, NRM);
				}
			}
			close(new_fd);
			if (!verbose && !quiet)
			{
				if (responseCode!=200)
				{
					printf("\nResponse: %i %s", responseCode, responseText.c_str());
				}
			}
			else if (verbose)
			{
				printf("\n##-------- Response status: %i %s", responseCode, responseText.c_str());
			}
			if (!verbose && !quiet) printf("\n##-----------------------");
			auto endTimeEverything = std::chrono::system_clock::now();
			auto elapsed = endTimeEverything - startTimeEverything;
			double ms = double(elapsed.count())/1000000;
			if (verbose) printf("\n%sComplete request took: %fms%s\n", BLU, ms, NRM);
			exit(0);
		}
		close(new_fd);  // parent doesn't need this
	}

	return 0;
}
