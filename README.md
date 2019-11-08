# Wind Waker Co-op

This program is intended to sync Wind Waker inventories between multiple players. It does this by hooking a running Dolphin emulator process.

This project is still in its early stages, but it currently supports syncing almost all major quest items, mail, and charts.

Things it will (hopefully) eventually sync:
- **Trading quest items**
- **Dungeon keys**
- **Pieces of Heart** *(as an option)*
- **World states** *(Tower of the Gods risen, Hyrule portal, etc)*
- **Chest flags** *(to avoid duplicates, particularly with mail bag items)*

## Running it
First, you'll want to launch Dolphin 5.0 and load your .iso. Neither are provided in this repository. 

This is a command line program. Usage is as follows:
>Start a server: WW-Co-Op -s \<port\> 

>Start a client: WW-Co-Op -c \<ipaddress\> \<port\>  

Alternatively there are two batch files provided in the Build folder. Edit those with the appropriate addresses, and begin your server. Clients can then join using the client batch script.

## Building
There are no releases at this time, so to run this you'll need to build it yourself. Currently only Windows 10 is being supported/targeted.

You'll need to install Visual Studio 2019 and C++ MFC for latest v142 build tools (x86 & x64). Earlier versions of VS may work but this is what I'm using.
