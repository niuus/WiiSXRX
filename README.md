![WiiSXRX logo](./logo.jpg)

WiiSXRX was born as a fork of WiiSXR, itself a fork of the original WiiSX, a PSX emulator for the Gamecube / Wii.

The starting point for this code base was Mystro256's WiiSXR, a continuation of
daxtsu's libwupc mod of wiisx, which is in turn based off of Matguitarist's "USB mod5".

For all the details concerning the use of the emulator, DO READ THIS:
https://github.com/niuus/WiiSXRX/tree/WiiSXRX-Evo/Gamecube/release/apps

* Official thread for discussion:
https://gbatemp.net/threads/wiisx-rx-a-new-fork.570252/

* daxtsu's libwupc mod (obsolete):
http://www.gc-forever.com/forums/viewtopic.php?t=2524

* WiiSX is GNU GPL and the source can be found here:
https://code.google.com/archive/p/pcsxgc/downloads  
https://github.com/emukidid/pcsxgc

* WiiSXR is GNU GPL and the source can be found here:
https://github.com/Mystro256/wiisxr

* libwupc and libwiidrc are also GPL, which can be found here:
https://github.com/FIX94/libwupc
https://github.com/FIX94/libwiidrc

* With code from:
PCSX / PCSX-df / PCSX-reloaded / PCSX-Revolution / PCSX-ReARMed

## Downloads

All downloads can be found here:

https://github.com/niuus/wiisxrx/releases

## Reporting Bugs

Don't report bugs related to specific games. Only bugs related to the emulator. Please note that i am not affiliated with PCSX or any of its forks, so mentioning me or WiiSXRX is unnecessary and unadvised, to avoid confusion.

If you have any programming skill, feel free to collaborate and check the Goals section!

## Goals

- Fix gcc build warnings (see build.log for details). Not sure how much the punned pointers will affect optimization, but no warnings is always better than any at all IMHO.
- Improve plugins (perhaps replace them?). Maybe an OpenGL plugin can be ported to GX (with the help of something like gl2gx, WIP see gxrender branch).
- Xbox 360 and USB HID controller support.
- DualShock 3, DualShock 4 and DualShock 5 controller support.
- Ability to take screenshots like Snes9x RX.
- Possibility to select other BIOS with some basic buttons.
- 240p support.
- CHD, ECM, PBP compressed file support.

Any help is appreciated.

# DISCLAIMER: Again, do not report issues with specific games, as they may or not be fixed with updates to the code later in the future.

## Credits:
Original WiiSX team:
tehpola, sepp256, emu_kidid

Original WiiSXR coder:
Mystro256

Mods & updates:
Matguitarist, daxtsu, Mystro256, FIX94, NiuuS, xjsxjs197
