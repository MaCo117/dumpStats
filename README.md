# dumpStats
DumpStats is a simple tool for collecting and processing statistical data from ADS-B decoder.
Main features include:
* Works with any ADS-B receiver capable of producing Basestation SBS formatted feed
* Can run alongside receiver, or on completely different machine - data acquired via TCP sockets
* Currently producing polar range plot, position heatmap with resolution ~0,75km and company (airline) diagram.
* Produces blocks of Javascript code using GoogleMaps API

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

## Usage
DumpStats can be used in two modes - collect and convert.
In collect mode, program connects to TCP feed from receiver and processes data until interrupted.
In convert mode program converts its internal representation of data into blocks of Javascript code.

Collect mode examples: (load from file, running on localhost, SBS on 30003, no display):
```
dumpStats -f myStats.out 127.0.0.1 30003
```

(start from scratch, running on remote machine, SBS on 30003, display messages):
```
dumpStats -d -p 48.9939 -m 18.0979 -f myStats.out 192.168.1.50 30003
```

Convert mode example: (load data from file, save JS files into subdir):
```
dumpStats -c ./JavaScript myStats.out
```

## Credits
dumpStats was written by Marcel Kebisek (marcel.kebisek@gmail.com) and is released under GNU GPL License v3.
