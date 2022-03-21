### Edit engine.cpp in order to add support for your game
### Currently supported games:
 - Fortnite
 - Rogue Company
 - Dead By Daylight
 - Witch It
 - Brickadia
 - POLYGON
 - SCUM
 - Scavengers
 - The Isle
 - Splitgate
 - Dauntless
 - Boundary
 - The Cycle: Frontier
### Supported engine versions: 
- UE 4.23-4.27
### Usage:
```
.\Dumper.exe [option] [option]
```
```
Options:
  '-h' - prints help message
  '-p' - dump only names and objects
  '-w' - wait for input
  '-f packageNameHere' - specifies package where we should look for pointers in paddings (can take a lot of time)
```
### Todo:
- Analyze functions to get offsets to referenced fields
