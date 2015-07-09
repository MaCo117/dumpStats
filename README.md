# dumpStats
DumpStats is a simple tool for collecting and processing statistical data from ADS-B decoder dump1090 feed (SBS format). DumpStats connects to TCP output port of dump1090 and extracts statistical relevant data from this feed. It produces simple text file representation of this data, which can be later used i.e. in Javascript (GoogleMaps API).

## Requirements
Compilation requires g++ compiler capable of compiling c++11 (version 4.7 and higher).
You can check your g++ version like this
```
g++ --version
```

To install required version of compiler, use this
```
sudo apt-get install g++-4.7
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-4.7 50
```

## Installation
Type
```
make
```
