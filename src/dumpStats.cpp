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
	std::cout << "\nusage: dumpStats [-d] [-p LAT] [-m LON] [-f FILE] IP PORT\n\n";
	std::cout << "optional arguments:\n -h    show this message and exit\n -d    display incoming messages (verbose)\n -p/-m specify initial receiver position at scratch start\n";
	std::cout << " -f    specify input/output file path in load mode and output file path in scratch mode\n";
	return;
}




int main(int argc, char **argv)
{
	// Argument parsing
	bool scratch = false;
	bool load = false;
	double refLat;
	double refLon;
	std::string filePath;
	char *hostname;
	char *portStr;
	
	bool dFlag = false;
	bool pFlag = false;
	char *pVal = nullptr;
	bool mFlag = false;
	char *mVal = nullptr;
	bool fFlag = false;
	char *fVal = nullptr;
	
	int optIndex;
	int c;
	
	while ((c = getopt(argc, argv, "hdp:m:f:")) != -1)
	{
		switch(c)
		{
			case 'h':
				printHelp();
				exit(1);
				
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
		
		
	
	
		
	
	
	
	// array of file descriptors
	int fds[2];
	// Create a pipe - both ends of pipe in fds
	pipe(fds);
	
	
	// Forking
	pid_t pid;
	pid = fork();
	if (pid == (pid_t) 0)
	{
		// Child - processor
		
		
		// Calling different constructor based on number of provided arguments. Ternary operator used.

		
		data stats = load ? data(filePath) : data(refLat, refLon);
		
		
		// Child - data processing
		FILE* stream;
		
		// Close write end of pipe
		close (fds[1]);
		
		// Convert read-end-of-pipe file descriptor to FILE object
		stream = fdopen (fds[0], "r");
		
		
		// Read from pipe
		char buffer[128];
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
			stats.processMessage(message);
			
			// every 1 minute:
			//	* write data to outfile
			//	* clear old entries from flightBuffer
			if ((std::time(nullptr) - stats.getUptime()) % 60 == 0)
			{
				stats.exportFile(filePath);
				stats.flushFBuffer();
			}
			
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
