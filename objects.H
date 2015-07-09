#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <csignal>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <ctime>
#include <locale>
#include <cmath>
#include <vector>


#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
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

#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define EARTH_RADIUS 6378.137

#define FBUFFER_TIMEOUT 1800





// Coordinates structure in decimal degree format
typedef struct coords
{
	double lat;
	double lon;
} tCoords;


// Flight buffer element structure.
// Contains pairs of ICAO24 - FlightID with timestamp of last appearance.
// Serves to avoid multiple additions of the very same flight to companyPlot.
// Old records should be removed regularly.
typedef struct flightStamp
{
	std::string hex;
	std::string callsign;
	std::time_t timestamp;
} tFStamp;


// Split string by delimiter into vector of substrings
std::vector<std::string> split(std::string str, char delimiter);


// Convert decimal degree value to decimal radians
double toRadians(double degrees);

// Convert decimal radians value to decimal degrees value
double toDegrees(double radians);


// Calculate distance between two spherical coordinates in km
double getDistance(tCoords first, tCoords second);


// Calculate forward azimuth (initial bearing) between two provided spherical coordinates.
double getBearing(tCoords first, tCoords second);


// Conversions between nautical miles and kilometers
double toKm(double nm);
double toNm(double km);


class data
{
	public:
	std::time_t timestamp;	// last change of file
	std::time_t uptime;		// time of program launch
	
	tCoords ref;		// Reference position for range calculations
	
	// Polar range plot - for each track from center of reference position there is maximum position value (359 values in total)
	std::vector<tCoords> polarRange;
	
	// HeatMap - contains weighted points for each position truncuted to 1/100 of full degree converted to single int to speed up comparing (50.00/16.00 = 50001600)
	std::map<int, int> heatMap;
	
	// Lists recorded companies (airlines) with number of caught aircrafts.
	std::map<std::string, int> companyPlot;	
	
	// Buffer of last appearance of pair ICAO24 - Callsign. If pair is in buffer, company counter in company plot should not be increased, until the pair is removed from buffer.
	// Old records should be removed regularly (~30 mins?)
	std::vector<tFStamp> flightBuffer;
	
	// Check whether a pair hex-callsign in tFStamp stamp is currently in flightBuffer
	bool isInFBuffer(tFStamp stamp);

	// In case of invalid input file, print stderr message and exit program
	void formatError();
	
	
		// Constructor
		// Initialize object from external file
		data(std::string path);

		// Another constructor
		// If init file is not provided, initialize object from scratch.
		// Reference lat/lon is needed in this case.
		data(double lat, double lon);
		
		// Empty constructor (for scope purposes)
		// For real initialization, it is needed to call one of above constructors
		data();
		
		// Export object data to internal representation file
		int exportFile(std::string path);
		
		// Process incoming message -> fill apropriate object data
		int processMessage(std::string message);
		
		// Clear flightBuffer - entries older than 30 minutes are deleted
		int flushFBuffer();
		
		// Interface to get uptime value from object instance
		std::time_t getUptime();
};
