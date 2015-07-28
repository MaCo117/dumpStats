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

/**
 * In case of invalid input file, print stderr message and exit program.
 */
void data::formatError()
{
	fprintf(stderr, "ERROR: Invalid format of init file.\n");
	exit(1);
}



/**
 * Function converts decimal degree value into decimal radians value
 * @param degrees - degree value to be converted in double
 * @return converted radians value in double
 */
double toRadians(double degrees)
{
	return (degrees * (M_PI / 180.0));
}



/**
 * Function converts redians decimal value into degrees decimal value.
 * @param radians - value in radians to be converted
 * @return converted degrees value
 */
double toDegrees(double radians)
{
	return (radians * (180.0 / M_PI));
}



/**
 * Function converts distance in nautical miles into kilometers.
 * @param nm - distance in nautical miles
 * @return distance in km
 */
double toKm(double nm)
{
	return (nm / 0.53996);
}


/**
 * Function converts distance in kilometers into nautical miles.
 * @param km - distance in kilometers
 * @return distance in nm
 */
double toNm(double km)
{
	return (km * 0.53996);
}




/**
 * Function calculates distance between two spherical coordinates using haversine formula.
 * @param first - first coordinates in tCoords structure
 * @param second - second coordinates in tCoords structure
 * @return distance in km between two provided coordinates (double)
 */
double getDistance(tCoords first, tCoords second)
{	
	double deltaLat = toRadians(first.lat) - toRadians(second.lat);
	double deltaLon = toRadians(first.lon) - toRadians(second.lon);
	
	double aHarv = pow(sin(deltaLat / 2.0), 2.0) + cos(toRadians(first.lat)) * cos(toRadians(second.lat)) * pow(sin(deltaLon / 2), 2);
	double cHarv = 2 * atan2(sqrt(aHarv), sqrt(1.0-aHarv));
	
	return EARTH_RADIUS * cHarv;
}

	

/**
 * Function calculates forward azimuth (initial bearing) from first provided coordinates to second.
 * @param first - initial coordinates in tCoords structure
 * @param second - final coordinates in tCoords structure
 * @return initial bearing from first to second position in decimal degrees
 */
double getBearing(tCoords first, tCoords second)
{
	double deltaLon = toRadians(second.lon) - toRadians(first.lon);
	
	double dPhi = log(tan(toRadians(second.lat) / 2.0 + M_PI / 4.0) / tan(toRadians(first.lat) / 2.0 + M_PI / 4.0));
	
	if (abs(deltaLon) > M_PI)
	{
		if (deltaLon > 0.0)
		{
			deltaLon = -(2.0 * M_PI - deltaLon);
		}
		else
		{
			deltaLon = (2.0 * M_PI - deltaLon);
		}
	}
	
	return (std::fmod((toDegrees(atan2(deltaLon, dPhi)) + 360.0), 360.0));
}




/**
 * Function splits string into vector of substrings originally separated by delimiter
 * @param str - string to be splitted
 * @param delimiter - character used as split delimiter
 * @return vector of substrings
 */
std::vector<std::string> split(std::string str, char delimiter)
{
	std::vector<std::string> internal;
	
	// Turn the string into a stream.
	std::stringstream ss(str); 
	std::string tok;
	
	while(getline(ss, tok, delimiter))
	{
		internal.push_back(tok);
	}
	
	return internal;
}



/**
 * Constructor.
 * Initialize object from external file
 * @param path - std::string containing path to initialization file.
 */
data::data(std::string path)
{
	std::ifstream f(path);
	if (!f)		// input stream creation failed
	{
		fprintf(stderr, "ERROR: Unable to open init file.\n");
		exit(1);
	}
	
	std::string line;
	
	uptime = std::time(nullptr);
	
	// Load timestamp (line 1)
	if (! std::getline(f, line))
	{
		formatError();
	}

	timestamp = std::stoi(line);
	
	
	// Load reference lat (line 2)
	if (! std::getline(f, line))
	{
		formatError();
	}
	
	ref.lat = std::stod(line);
	
	
	// Load reference lon (line 3)
	if (! std::getline(f, line))
	{
		formatError();
	}
	
	ref.lon = std::stod(line);
	
	
	// Load polar range plot values (lines 4-363)
	for (int i = 0; i < 360; i++)
	{
		if (! std::getline(f, line))
		{
			formatError();
		}
		std::vector<std::string> vec = split(line, '|');
		tCoords newPos;
		newPos.lat = std::stod (vec[0]);
		newPos.lon = std::stod (vec[1]);
		
		polarRange.push_back(newPos);
	}
	
	// Load delimiting blank line
	if (! std::getline(f, line))
	{
		formatError();
	}
	
	// Load altPlot values (500 values)
	for (int i = 0; i <= 500; i++)
	{
		if (! std::getline(f,line))
		{
			formatError();
		}
		altPlot.push_back(std::stoi(line));
	}
	
	// Load delimiting blank line
	if (! std::getline(f, line))
	{
		formatError();
	}
	
	
	// Load heatMap weighted points
	while (std::getline(f, line))
	{
		if (line == "")
		{
			break;
		}
		std::vector<std::string> vec = split(line, '|');
		int index = std::stoi (vec[0]);
		int weight = std::stoi (vec[1]);
		heatMap[index] = weight;
	}
	
	// Load companyPlot string-keyed map
	while (std::getline(f, line))
	{
		if (line == "")
		{
			break;
		}
		std::string key = line.substr(0,3);		// First 3 characters
		int val = std::stoi(line.substr(4));
		companyPlot[key] = val;
	}
	
	// Trailing $ check
	if (! std::getline(f, line))
	{
		formatError();
	}
	if (line != "$")
	{
		formatError();
	}
	else
	{
		// Init successful
		fprintf(stdout, "Loading successfull.\n");
		return;
	}
	
	
}



/**
 * Constructor.
 * If init file is not provided, generate object from scratch requiring reference position.
 * @param lat - decimal degree coordinate of reference position
 * @param lon - decimal degree coordinate of reference position
 */
data::data(double lat, double lon)
{
	// Reference position
	ref.lat = lat;
	ref.lon = lon;
	
	uptime = std::time(nullptr);
	
	// Fill 359 polarPlot values with reference position, since no other data is available yet
	for (int i = 0; i < 360; i++)
	{
		tCoords newPos;
		newPos.lat = ref.lat;
		newPos.lon = ref.lon;
		
		polarRange.push_back(newPos);
	}
	
	// Fill 500 altPlot values with zero.
	for (int i = 0; i <= 500; i++)
	{
		altPlot.push_back(0);
	}
	
	return;
}


/**
 * Empty constructor.
 */
data::data()
{
	return;
}



/**
 * Function returns uptime value from object instance.
 * @return uptime
 */
std::time_t data::getUptime()
{
	return uptime;
}




/**
 * Function clears from flightBuffer entries older than 30 minutes.
 * @return number of erased entries.
 */
int data::flushFBuffer()
{
	std::time_t now = std::time(nullptr);
	int counter = 0;
	
	for (int i = 0; i < flightBuffer.size(); i++)
	{
		tFStamp st = flightBuffer[i];
		if (now - st.timestamp > FBUFFER_TIMEOUT)
		{
			flightBuffer.erase(flightBuffer.begin() + i);
			counter++;
		}
	}
	
	return counter;
}
	

/**
 * Function writes file based on object data in internal representation of format.
 * If there is file on provided path, it will be overwritten!
 * @param path - path to output file
 * @return zero if success, nonzero otherwise
 */
int data::exportFile(std::string path)
{
	std::ofstream f;
	f.open(path);
	if (f.is_open())
	{
		timestamp = std::time(nullptr);
		f << timestamp << '\n';
		f << ref.lat << '\n';
		f << ref.lon << '\n';
		
		// Iterate over 359 polarPlot positions
		for (int i = 0; i < 360; i++)
		{
			tCoords p = polarRange[i];
			char buf[32];
			sprintf(buf, "%.4f|%.4f", polarRange[i].lat, polarRange[i].lon);
			std::string outLine = buf;
			f << outLine << '\n';
		}
		
		// Delimiting newline
		f << '\n';
		
		// Iterate over 500 altPlot altitude counts
		for (int i = 0; i <= 500; i++)
		{
			f << altPlot[i] << '\n';
		}
		
		// Delimiting newline
		f << '\n';
		
		// Iterate over heatMap active points
		std::map<int, int>::iterator heatMapIter;
		for (heatMapIter = heatMap.begin(); heatMapIter != heatMap.end(); ++heatMapIter)
		{
			char buf[32];
			sprintf(buf, "%d|%d", heatMapIter->first, heatMapIter->second);
			std::string outLine = buf;
			f << outLine << '\n';
		}
		
		// Delimiting newline
		f << '\n';
		
		// Iterate over company records
		std::map<std::string, int>::iterator companyIter;
		for (companyIter = companyPlot.begin(); companyIter != companyPlot.end(); ++companyIter)
		{
			char buf[32];
			sprintf(buf, "%s|%d", (companyIter->first).c_str(), companyIter->second);
			std::string outLine = buf;
			f << outLine << '\n';
		}
		
		// Delimiting newline
		f << '\n';
		
		// Trailing $ - valid file
		f << '$';
		
		// Close file
		f.close();
		
		return 0;
	}
	else
	{
		fprintf(stderr, "ERROR: Unable to open output file!\n");
		return 1;
	}
}


/**
 * Function checks whether a pair hex-callsign stored in stamp is currently in flightBuffer.
 * @param stamp - tFStamp containing pair of hex-callsign
 * @return true if pair is currently in buffer, false otherwise
 */
bool data::isInFBuffer(tFStamp stamp)
{
	for (tFStamp st : flightBuffer)
	{
		if ((st.hex == stamp.hex) && (st.callsign == stamp.callsign))
		{
			return true;
		}
	}
	
	return false;
}

 
 
 
/**
 * Function interprets incoming message, extracts relevant data based on message type
 * and fills apropriate object variables with this data.
 * 
 * Basestation SBS format message is basically single comma-separated-values (CSV) record.
 * Basestation produces four different SBS message types. This type is determined by first field of csv record.
 * Fields of which message consists are determined by this type.
 * 
 * ------------------------------
 * -----NEW AIRCRAFT MESSAGE-----
 * ------------------------------
 * This message is broadcasted when SBS-1 picks up a signal for an aircraft that isn't currently tracking,
 * i.e. it's when a new aircraft appers on the aircraft list. Structure of this type:
 *
 * 1  AIR
 * 2  [null]
 * 3  System-generated sessionID
 * 4  System-generated aircraftID
 * 5  ICAO24 hex ident
 * 6  System generated flightID
 * 7  Date message detected
 * 8  Time message detected
 * 9  Date message logged
 * 10 Time message logged
 * 
 * 
 * -------------------- 
 * -----ID MESSAGE-----
 * --------------------
 * This message is broadcasted when a callsign is first received, or is changed.
 * 
 * 1  ID
 * 2  [null]
 * 3  System-generated sessionID
 * 4  System-generated aircraftID
 * 5  ICAO24 hex ident
 * 6  System generated flightID
 * 7  Date message detected
 * 8  Time message detected
 * 9  Date message logged
 * 10 Time message logged
 * 11 Callsign
 * 
 * 
 * ----------------------------------
 * -----SELECTION CHANGE MESSAGE-----
 * ----------------------------------
 * This is rather internal Basestation system message with no new information value.
 * It is broadcasted when user changes the selection, or in special cases, it can be broadcasted,
 * when new aircraft has been added (due to implementation).
 * 
 * 1  SEL
 * 2  [null]
 * 3  System-generated sessionID
 * 4  System-generated aircraftID
 * 5  ICAO24 hex ident
 * 6  System generated flightID
 * 7  Date message detected
 * 8  Time message detected
 * 9  Date message logged
 * 10 Time message logged
 * 11 Callsign
 * 
 * 
 * ------------------------------
 * -----TRANSMISSION MESSAGE-----
 * ------------------------------
 * Finally, most important message. It is basically rebroadcasting of every ADS-B message
 * received from aircraft - in decoded format.
 * This format is produced by most ADS-B decoders as text based decoded format. It is convenient
 * to use Basestation format for further use, due to computational complexity of decoding raw bitwise
 * ADS-B messages.
 * 
 * 1  MSG
 * 2  Transmission Type
 * 3  System generated sessionID
 * 4  System generated aircraftID
 * 5  ICAO24 hex ident
 * 6  System generated flightID
 * 7  Date message detected
 * 8  Time message detected
 * 9  Date message logged
 * 10 Time message logged
 * 11 Callsign
 * 12 Altitude
 * 13 Ground Speed
 * 14 Track
 * 15 Lat
 * 16 Lon
 * 17 Vertical speed
 * 18 Squawk
 * 19 Alert
 * 20 Emergency
 * 21 SPI
 * 22 IsOnGround
 * 
 * Based on second field - Transmission Type, we can distinguish a few more transmission types,
 * of which each sets different fields:
 * 
 * Transmission Type:
 * 	1 = ID message ----------------------- (Callsign)
 * 	2 = Surface position message --------- (Altitude, GroundSpeed, Track, Lat, Lon)
 * 	3 = Airborne position message -------- (Altitude, Lat, Lon, Alert, Emergency, SPI)
 * 	4 = Airborne velocity message -------- (GroundSpeed, Rate, VerticalSpeed)
 * 	5 = Surveillance Altitude message ---- (Altitude, Alert, SPI)
 * 	6 = Surveillance ID (Squawk) message - (Altitude, Squawk, Alert, Emergency, SPI)
 * 	8 = Air-Call Reply / TCAS Acquisition Squitter (None)
 * 
 * 
 * Dump1090 emulates Basestation message broadcast. It outputs only MSG (Transmission) messages,
 * as it rebroadcasts decoded ADS-B messages and there is no need to emulate internal SBS messages.
 * This function takes single transmission message and processes containing info for statistical purposes.
 * 
 * [ Thanks to Mr Dave Reid for comprehensive information on this topic ]
 * 
 * @param message - incoming message converted to std::string
 * @return 1 or 3 based on type of processed message, zero for discarded message.
 */
int data::processMessage(std::string message)
{
	// Split message into individual csv fields
	std::vector<std::string> fields = split(message, ',');
	
	
	tFStamp stamp;
	// Switch based on message type
	switch (std::stoi(fields[1]))
	{
		case 1:
			// ID message (hex+callsign available)
			if ((fields[4] != "") && (fields[10] != ""))
			{
				stamp.hex = fields[4];
				stamp.callsign = fields[10];
				stamp.timestamp = std::time(nullptr);
				
				if (! isInFBuffer(stamp))
				{
					std::locale loc;
					std::string company = stamp.callsign.substr(0,3);
					if ((std::isalpha(company[0])) && (std::isalpha(company[1])) && (std::isalpha(company[2])) && (std::isdigit(stamp.callsign[3])))
					{
						if ( companyPlot.find(company) == companyPlot.end() )
						{
							companyPlot[company] = 1;
						}
						else
						{
							companyPlot[company]++;
						}
					}
					
					flightBuffer.push_back(stamp);
				}
			}
			return 1;
			break;
			
		case 3:
			// Airborne position message (Altitude+lat/lon available)
			if ((fields[14] != "") && (fields[15] != ""))
			{
				tCoords mPos;
				mPos.lat = std::stod(fields[14]);
				mPos.lon = std::stod(fields[15]);
			
				int bearing = (int) round(getBearing(ref, mPos));
				
				double distance = getDistance(ref, mPos);
				
				tCoords maxPos = polarRange[bearing];
				
				double maxDistance = getDistance(ref, maxPos);
				
				if (distance > maxDistance)
				{
					polarRange[bearing] = mPos;
				}
				
				char buf[32];
				sprintf(buf, "%d%d", (int) round(mPos.lat * 100), (int) round(mPos.lon * 100));
				std::string sIntPos = buf;
				int intPos = std::stoi(sIntPos);
				
				if (heatMap.find(intPos) == heatMap.end())
				{
					heatMap[intPos] = 1;
				}
				else
				{
					heatMap[intPos]++;
				}
			}
			if (fields[11] != "")
			{
				int fl = std::stoi(fields[11]) / 100;	// Convert altitude to FL
				if (fl <= 500)
				{
					altPlot[fl]++;
				}
			}
			return 3;
			break;
	}
	return 0;
}



/**
 * Function loads icao-iata database from external file into
 * internal representation.
 * Function fills instance data.
 * @param path - path to icao-iata.db
 * @return zero if success, nonzero otherwise
 */
int data::loadIcaoIata(std::string path)
{
	std::ifstream f(path);
	if (!f)		// input stream creation failed
	{
		return 1;
	}
	
	std::string line;
	
	while (std::getline(f, line))
	{
		std::vector<std::string> vec = split(line, '\t');
		std::vector<std::string> vecc;
		vecc.push_back(vec[2]);
		vecc.push_back(vec[4]);
		icaoIata[vec[1]] = vecc;
	}
	
	return 0;
}



/**
 * Function converts instance data and produces files with
 * Javascript code using GoogleMaps API to display collected data.
 * @param dir - directory to create JS files
 * @param launchDir - directory of executable
 * @param cThr - company treshold. If count of company is below or equal treshold, company will not appear in airline chart.
 * @return zero if success, nonzero otherwise
 */
int data::createJS(std::string dir, std::string launchDir, int cThr)
{
	// Create rangePlot
	std::ofstream f;
	std::string fpath = dir + "/polarPlot.js";
	f.open(fpath);
	if (f.is_open())
	{
		f << "function initializePolarPlot() {\n  var polarMapOptions = {\n    zoom: 7,\n    center: new google.maps.LatLng(";
		f << ref.lat << ", " << ref.lon << "),\n    mapTypeId: google.maps.MapTypeId.TERRAIN\n  };\n\n  var polarPlot;\n\n  var polarMap = new google.maps.Map(document.getElementById('polar-map-canvas'),\n      polarMapOptions);";
		f << "var triangleCoords = [\n";
		
		for (int i = 0; i < 360; i++)
		{
			tCoords p = polarRange[i];
			f << "    new google.maps.LatLng(" << p.lat << ", " << p.lon << ")";
			if (i != 359)
			{
				f << ",\n";
			}
			else
			{
				f << "\n";
			}
		}
		f << "  ];\n\n  polarPlot = new google.maps.Polygon({\n    paths: triangleCoords,\n    strokeColor: '#FF0000',\n    strokeOpacity: 0.8,\n    strokeWeight: 2,\n    fillColor: '#FF0000',\n    fillOpacity: 0.35\n  });\n\n";
		f << "  var image = new google.maps.MarkerImage('http://maps.google.com/mapfiles/kml/pal4/icon57.png', null, new google.maps.Point(0,0), new google.maps.Point(16,16));";
		f << "  var myLatLng = new google.maps.LatLng(" << ref.lat << ", " << ref.lon << ");";
		f << "  var beachMarker = new google.maps.Marker({\n      position: myLatLng,\n      map: polarMap,\n      icon: image\n  });\n\n";
		f << "  polarPlot.setMap(polarMap);\n}\n\ngoogle.maps.event.addDomListener(window, 'load', initializePolarPlot);";
		
		f.close();
		
	}
	else
	{
		fprintf(stderr, "ERROR: Unable to open output file: [%s]\n", fpath.c_str());
		return 1;
	}
	
	
	// Create heatMap
	fpath = dir + "/heatMap.js";
	f.open(fpath);
	if (f.is_open())
	{
		f << "var map, pointarray, heatmap;\n\nvar heatMapData = [\n";
		
		std::map<int, int>::iterator heatMapIter;
		for (heatMapIter = heatMap.begin(); heatMapIter != heatMap.end(); ++heatMapIter)
		{
			std::string pos = std::to_string(heatMapIter->first);
			int iLat = std::stoi(pos.substr(0,4));
			int iLon = std::stoi(pos.substr(4,4));
			int weight = heatMapIter->second;
			
			double hLat = iLat / 100.0;
			double hLon = iLon / 100.0;
			
			f << "  {location: new google.maps.LatLng(" << hLat << ", " << hLon << "), weight: " << weight << "}";
			
			if (++heatMapIter != heatMap.end())
			{
				f << ",\n";
			}
			else
			{
				f << "\n";
			}
			heatMapIter--;
		}
			
		f << "];\n\nfunction initialize() {\n  var mapOptions = {\n    zoom: 9,\n    center: new google.maps.LatLng(" << ref.lat << ", " << ref.lon << "),\n    mapTypeId: google.maps.MapTypeId.SATELLITE\n";
		f << "  };\n\n  map = new google.maps.Map(document.getElementById('map-canvas'),\n      mapOptions);\n\n  var pointArray = new google.maps.MVCArray(heatMapData);\n\n";
		f << "  heatmap = new google.maps.visualization.HeatmapLayer({\n    data: pointArray\n  });\n\n  var image = new google.maps.MarkerImage('http://maps.google.com/mapfiles/kml/pal4/icon57.png', null, new google.maps.Point(0,0), new google.maps.Point(16,16));";
		f << "  var myLatLng = new google.maps.LatLng(" << ref.lat << ", " << ref.lon << ");  var beachMarker = new google.maps.Marker({\n      position: myLatLng,\n      map: map,\n      icon: image\n";
		f << "  });heatmap.setMap(map);\n}\n\nfunction toggleHeatmap() {\n  heatmap.setMap(heatmap.getMap() ? null : map);\n}\n\n";
		f << "function changeGradient() {\n  var gradient = [\n    'rgba(0, 255, 255, 0)',\n    'rgba(0, 255, 255, 1)',\n    'rgba(0, 191, 255, 1)',\n    'rgba(0, 127, 255, 1)',\n";
		f << "    'rgba(0, 63, 255, 1)',\n    'rgba(0, 0, 255, 1)',\n    'rgba(0, 0, 223, 1)',\n    'rgba(0, 0, 191, 1)',\n    'rgba(0, 0, 159, 1)',\n    'rgba(0, 0, 127, 1)',\n";
		f << "    'rgba(63, 0, 91, 1)',\n    'rgba(127, 0, 63, 1)',\n    'rgba(191, 0, 31, 1)',\n    'rgba(255, 0, 0, 1)'\n  ]\n  heatmap.set('gradient', heatmap.get('gradient') ? null : gradient);\n}\n\n";
		f << "function changeRadius() {\n  heatmap.set('radius', heatmap.get('radius') ? null : 20);\n}\n\nfunction changeOpacity() {\n  heatmap.set('opacity', heatmap.get('opacity') ? null : 0.2);\n";
		f << "}\n\nfunction mtypeHybrid() {\n	map.setMapTypeId(google.maps.MapTypeId.HYBRID);\n}\n\nfunction mtypeSat() {\n	map.setMapTypeId(google.maps.MapTypeId.SATELLITE);\n}google.maps.event.addDomListener(window, 'load', initialize);";
		
		f.close();
	}
	else
	{
		fprintf(stderr, "ERROR: Unable to open output file: [%s]\n", fpath.c_str());
		return 1;
	}
	
	
	//Create csv file for highcharts airline chart
	fpath = dir + "/airline.csv";
	f.open(fpath);
	if (f.is_open())
	{
		f << "Airline,Share\n";
		
		if (loadIcaoIata(launchDir + "/data/iata-icao.db") != 0)
		{
			fprintf(stderr, "ERROR: Error while loading iata-icao database!\n");
			return 1;
		}
			
		int total = 0;
		std::map<std::string, int>::iterator airlineIter;
		for (airlineIter = companyPlot.begin(); airlineIter != companyPlot.end(); ++airlineIter)
		{
			total += airlineIter->second;
		}
		
		for (airlineIter = companyPlot.begin(); airlineIter != companyPlot.end(); ++airlineIter)
		{
			std::string name;
			if ( icaoIata.find(airlineIter->first) == icaoIata.end() )
			{
				continue;
			}
			else
			{
				std::vector<std::string> vec = icaoIata[airlineIter->first];
				name = vec[0];
			}
			
			if (airlineIter->second > cThr)
			{
				f << name << "," << (std::round((double(airlineIter->second) / double(total)) * 10000.0 ) / 10000.0) * 100;
			
				if (++airlineIter != companyPlot.end())
				{
					f << "\n";
				}
				airlineIter--;
			}
			else
			{
				continue;
			}
		}
		
		f.close();
	}
	else
	{
		fprintf(stderr, "ERROR: Unable to open output file: [%s]\n", fpath.c_str());
		return 1;
	}
	
	
	// Create csv file for highcharts altitude plot
	fpath = dir + "/altitude.csv";
	f.open(fpath);
	if (f.is_open())
	{
		f << "Altitude,Share\n";
		
		int total = 0;
		for (int i = 0; i <= 500; i++)
		{
			total += altPlot[i];
		}
		
		for (int i = 0; i <= 500; i++)
		{
			f << i*100 << "," << (std::round((double(altPlot[i]) / double(total)) * 10000.0 ) / 10000.0) * 100;
			if (i != 500)
			{
				f << '\n';
			}
		}
		
		f.close();
	}
	else
	{
		fprintf(stderr, "ERROR: Unable to open output file: [%s]\n", fpath.c_str());
		return 1;
	}
	
	return 0;
}
