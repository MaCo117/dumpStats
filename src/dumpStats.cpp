/* DumpStats - dump1090 feed statistical data collector
 * Copyright (C) 2015 Marcel Kebisek
 * Contact: marcel.kebisek@gmail.com
 * 
 * This file is part of DumpStats.
 * 
 * DumpStats is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * DumpStats is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with DumpStats. If not, see <http://www.gnu.org/licenses/>.
 */

#include "objects.H"

int bsSocket;

// SIGINT handler - closes basestation socket and terminates 
void f_sigint_handler(int s)
{
	close(bsSocket);
	std::cout << "SIGINT caught!\nExiting...\n";
	exit(0);
	return;
}


// Print help message
void printHelp()
{
	std::cout << "\ncollect mode usage: dumpStats [-d] [-l] [-p LAT] [-m LON] [-f FILE] IP PORT\n\n";
	std::cout << "optional arguments:\n -h    show this message and exit\n -d    display incoming messages (verbose)\n -p/-m specify initial receiver position at scratch start\n";
	std::cout << " -f    specify input/output file path in load mode and output file path in scratch mode\n -l    enable logging debug information into logfile at executable directory(logfile can get quite big during long runtime)\n\n\n";
	std::cout << "convert mode usage: dumpStats -c [OUT_DIR] [-t TRESHOLD] FILE_PATH\n\n";
	std::cout << "OUT_DIR   is a directory where JS files will be stored (current directory by default)\n -t       specify number of counts per company, below which (TRESHOLD included) company will not show in chart (useful for crowded chart)\nFILE_PATH is path to load file\n";
	return;
}


// Returns path of executable
std::string get_selfpath()
{
    char buff[256];
    ssize_t len = readlink("/proc/self/exe", buff, 255);
    if (len != -1)
    {
      buff[len] = '\0';
      return std::string(buff);
    }
}


// Returns formatted exact time
std::string getNanoTime()
{
	timespec ts;
	
	char buffer[80];
	char outBuf[128];
	
	clock_gettime(CLOCK_REALTIME, &ts);
	strftime(buffer, 80, "%Y-%m-%d, %H:%M:%S", gmtime(&(ts.tv_sec)));
	sprintf(outBuf, "%s.%Ld", buffer, ts.tv_nsec);
	return std::string(outBuf);
}




int main(int argc, char **argv)
{
	// Argument parsing
	bool scratch = false;
	bool load = false;
	bool convert = false;
	bool logging = false;
	int comp_treshold = 0;
	double refLat;
	double refLon;
	std::string filePath;
	char *hostname;
	char *portStr;
	std::string jsDir;
	std::string logFile;
	
	bool dFlag = false;
	bool pFlag = false;
	char *pVal = nullptr;    /* handle error condition */
	bool mFlag = false;
	char *mVal = nullptr;
	bool fFlag = false;
	char *fVal = nullptr;
	bool cFlag = false;
	bool lFlag = false;
	char *lVal = nullptr;
	bool tFlag = false;
	char *tVal = nullptr;
	
	int optIndex;
	int c;
	
	while ((c = getopt(argc, argv, "hl:cdp:m:f:t:")) != -1)
	{
		switch(c)
		{
			case 'h':
				printHelp();
				exit(1);
			
			case 'l':
				lFlag = true;
				lVal = optarg;
				break;
				
			case 'c':
				cFlag = true;
				break;
				
			case 'd':
				dFlag = true;
				break;
			
			case 'p':
				pFlag = true;
				pVal = optarg;
				break;
				
			case 'm':
				mFlag = true;
				mVal = optarg;
				break;
			
			case 'f':
				fFlag = true;
				fVal = optarg;
				break;
			
			case 't':
				tFlag = true;
				tVal = optarg;
				break;
				
			case '?':
				if (optopt == 'c')
				{
					fprintf (stderr, "Option -%c requires an argument.\n", optopt);
				}
				else
				{
					fprintf (stderr, "Unknown option `-%c'.\n", optopt);
				}
				return 1;
				break;
			
			default:
				abort();
		}
	}
		
	std::vector<char*> nonOptions;
	for (optIndex = optind; optIndex < argc; optIndex++)
	{
		nonOptions.push_back(argv[optIndex]);
	}
	
	if (cFlag)
	{
		if (pFlag || mFlag || fFlag || dFlag || lFlag)
		{
			fprintf(stderr, "Invalid argument usage! Convert mode does not accept other options.\n");
			exit(1);
		}
		
		if (tFlag)
		{
			comp_treshold = atoi(tVal);
			if (comp_treshold == 0)
			{
				fprintf(stderr, "Invalid value of -t TRESHOLD parameter! (Zero is implicit and cannot be processed).\n");
				exit(1);
			}
		}
		if (nonOptions.size() == 2)
		{
			jsDir = std::string(nonOptions[0]);
			filePath = std::string(nonOptions[1]);
			convert = true;
		}
		else if (nonOptions.size() == 1)
		{
			jsDir = std::string("./");
			filePath = std::string(nonOptions[0]);
			convert = true;
		}
		else
		{
			fprintf(stderr, "Invalid number of values for convert mode!\n");
			exit(1);
		}
	}
	else
	{
		if (pFlag || mFlag)
		{
			if ((pFlag && !mFlag) || (!pFlag && mFlag))
			{
				fprintf(stderr, "Invalid argument usage! Another position coordinate is required, if starting from scratch.\n");
				exit(1);
			}
			
			if (pFlag && mFlag)
			{
				scratch = true;
				refLat = atof(pVal);
				refLon = atof(mVal);
			}
			
			if (fFlag)
			{
				filePath = std::string(fVal);
			}
			else
			{
				filePath = std::string("./stats.out");
			}
			
			if (lFlag)
			{
				logging = true;
				logFile = std::string(lVal);
			}
		}
		else
		{
			if (!fFlag)
			{
				fprintf(stderr, "Invalid argument usage! Load file or initial position is required.\n");
				exit(1);
			}
			else
			{
				load = true;
				filePath = std::string(fVal);
			}
			
			if (lFlag)
			{
				logging = true;
				logFile = std::string(lVal);
			}
		}
		
		if (nonOptions.size() < 2)
		{
			fprintf(stderr, "Missing arguments! Source IP (127.0.0.1 if on localhost) and port are required!\n");
			exit(1);
		}
		else
		{
			hostname = nonOptions[0];
			portStr = nonOptions[1];
		}
	}
	
	std::string execDir = get_selfpath();
		
	execDir = execDir.substr(0, execDir.size() - 10);
	
	// Convert mode
	if (convert)
	{
		data stats = data(filePath);
		
		if (stats.createJS(jsDir, execDir, comp_treshold) == 0)
		{
			std::cout << "Converting successfull.\n";
		}
		
		return 0;
	}		
	
	std::ofstream logf;
	if (logging)
	{				
		logf.open(logFile);
		if (! logf.is_open())
		{
			fprintf(stderr, "ERROR: Unable to open logfile!\n");
			exit(1);
		}
		
		logf << "[ " << getNanoTime() << " ] Arguments successfully parsed: ";
		if (convert)
		{
			logf << "convert mode, ";
			if (jsDir != "")
			{
				logf << "script dir is " << jsDir;
			}
			
			logf << ", loading from " << filePath;
		}
		else
		{
			logf << "collect mode, ";
			if (load)
			{
				logf << "loading start from " << filePath;
			}
			else
			{
				logf << "scratch start at " << refLat << ", " << refLon;
			}
			
			if (dFlag)
			{
				logf << ", display messages";
			}
			else
			{
				logf << ", no display";
			}
			
			logf << ", listening at " << hostname << ":" << portStr << "\n";
		}
	}

	
	// array of file descriptors
	int fds[2];
	// Create a pipe - both ends of pipe in fds
	pipe(fds);
	
	if (logging)
	{
		logf << "[ " << getNanoTime() << " ] Pipe created.\n";
	}
	
	
	// Forking
	pid_t pid;
	pid = fork();
	if (pid == (pid_t) 0)
	{
		// Child - processor
		
		
		// Calling different constructor based on number of provided arguments. Ternary operator used.

		
		data stats = load ? data(filePath) : data(refLat, refLon);
		
		if (logging)
		{
			logf << "[ " << getNanoTime() << " ] Created stats object.\n";
		}
		
		// Child - data processing
		FILE* stream;
		
		// Close write end of pipe
		close (fds[1]);
		
		// Convert read-end-of-pipe file descriptor to FILE object
		stream = fdopen (fds[0], "r");
		
		
		// Read from pipe
		char buffer[128];
		int result;
		if (logging)
		{
			logf << "[ " << getNanoTime() << " ] Starting pipe reading..\n";
		}
		
		int lastDiskOp;		// last disk operation in minutes (file write)
		
		while (!feof(stream))
		{			
			// get line from pipe
			fgets(buffer, sizeof(buffer), stream);
			
			// convert line to std::string
			std::string message = buffer;
			if (dFlag)
			{
				std::cout << message;
			}
			
			// process line
			result = stats.processMessage(message);
			
			if (logging)
			{
				if (result != 0)
				{
					logf << "[ " << getNanoTime() << " ] Logged type " << result << " message.\n";
				}
				else
				{
					logf << "[ " << getNanoTime() << " ] Discarded message.\n";
				}
			}
			
			// every 1 minute:
			//	* write data to outfile
			//	* clear old entries from flightBuffer
			if (((std::time(nullptr) - stats.getUptime()) % 60 == 0) && ((std::time(nullptr) / 60) != lastDiskOp))
			{
				lastDiskOp = std::time(nullptr) / 60;
				result = stats.exportFile(filePath);
				if (logging)
				{
					if (result == 0)
					{
						logf << "[ " << getNanoTime() << " ] File successfully written.\n";
					}
				}
	
				result = stats.flushFBuffer();
				if (logging)
				{
					logf << "[ " << getNanoTime() << " ] FlightBuffer flushed ( " << result << " entries deleted ).\n";
				}
			}
			
		}
		if (logging)
		{
			logf << "[ " << getNanoTime() << " ] Stream ended.\nProgram is correctly ending.";
		}
		fclose(stream);
		return 0;
	}
	
	
	
	else
	{
		// Parent - transceiver
		FILE* stream;
		
		//Close read end of pipe
		close (fds[0]);
		
		// Convert write-end-of-pipe file descriptor to FILE object
		stream = fdopen (fds[1], "w");
		
		// Initialization
		int n;
		struct sockaddr_in sin;
		struct hostent *hptr;
		
		struct sigaction sigIntHandler;
		sigIntHandler.sa_handler = f_sigint_handler;
		sigemptyset(&sigIntHandler.sa_mask);
		sigIntHandler.sa_flags = 0;
		
		
		char buffer[1];
		
		
		// Create socket
		if ((bsSocket = socket (PF_INET, SOCK_STREAM, 0)) < 0)
		{
			fprintf(stderr, "ERROR creating socket!\n");
			return -1;
		}

		sin.sin_family = PF_INET;		// Set protocol family to internet
		sin.sin_port = htons(atoi(portStr));	// Set port number
		if ((hptr = gethostbyname(hostname)) == NULL)
		{
			fprintf(stderr, "ERROR Gethostname error!\n");
			return -1;
		}

		memcpy(&sin.sin_addr, hptr->h_addr, hptr->h_length);
		
		// Connect
		if (connect(bsSocket, (struct sockaddr*)&sin, sizeof(sin)) < 0)
		{
			fprintf(stderr, "ERROR Connect error!\n");
			return -1;
		}
		
		// Read periodically
		sigaction(SIGINT, &sigIntHandler, NULL);
		int counter = 1;
		std::string message ("");
		while (true)
		{
			if ((n = read(bsSocket, buffer, sizeof(buffer))) < 0)
			{
				printf("ERROR Read error!\n");
				return -1;
			}
			
			if (buffer[0] == '\n')
			{			
				fprintf(stream, "%s\n", message.c_str());
				fflush(stream);
				message = "";
			}
			else
			{
				message += buffer[0];
			}
		}
		fclose(stream);
		return 0;
	}
}
