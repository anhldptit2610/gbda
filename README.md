# gbda

Simple **G**ame**B**oy emulator made by **D**uc **A**nh.

GameBoy is a very old game console, yet the engineering of it is very insane; making its emulator help me realize that. It's a fun and lots of frustration journey.

## Feature
 * Supports no MBC/MBC1s ROM type.
 * Has sound!(Although it's buggy).
 * Original GameBoy palette
## Screenshots
![Legends of Zelda](/images/Legend_of_Zelda.png)
![Battletoads](/images/battletoads.png)
![Megaman](/images/megaman.png)
![Metroid II](/images/metroid.png)
![Castlevania](/images/castlevania.png)
![Kirby's Dream Land 2](/images/kirby2.png)
## How to build
gbda requires [CMake](https://cmake.org) and [SDL2](https://www.libsdl.org/).

    $ git clone https://github.com/anhldptit2610/gbda.git
    $ cd gbda
    $ cmake -S . -B build
    $ cd build && make

## TODO
- [ ] Synchronize the sound with the system.
- [ ] Support more MBCs.
- [ ] GameBoy Color!!!
