![WiiSXRX logo](./logo.jpg)

Fork of WiiSXR (a port of PCSX-R), a PSX emulator for the Gamecube / Wii / Wii U.

The starting point for this code base will be Mystro256's WiiSXR, a continuation of
daxtsu's libwupc mod of wiisx, which is in turn based off of Matguitarist's "USB mod5".

* Please see the following link for details:
http://www.gc-forever.com/forums/viewtopic.php?t=2524

* WiiSX is GNU GPL and the source can be found here:
https://code.google.com/archive/p/pcsxgc/downloads

* WiiSXR is GNU GPL and the source can be found here:
https://github.com/Mystro256/wiisxr

* libwupc and libwiidrc are also GPL, which can be found here:
https://github.com/FIX94/libwupc
https://github.com/FIX94/libwiidrc


## Downloads

All downloads can be found here:

https://github.com/niuus/wiisxrx/releases

## Reporting Bugs

Feel free to report bugs, but if you can, please test pcsxr first, to eliminate redundant bugs. If it's not a bug in pcsxr, but a bug here, report it here. I would hope this can be as aligned with pcsxr as possible, so any bugs in pcsxr will be inherited unfortunately. If it is a bug in pcsxr, feel free to report bugs with pcsxr... Please note that i am not affiliated with them, so mentioning me or WiiSXRX is unnecessary and unadvised to avoid confusion.

As well, i can't guarantee this project will be a success, unless i can get some help! So if you have any programming skill, feel free to collaborate and check the Goals section!

## Goals
(some taken from Mystro256's original readme)

- Fix gcc build warnings (see build.log for details). Not sure how much the punned pointers will affect optimization, but no warnings is always better than any at all IMHO.
- Update with any code from pcsxr (take as much as possible from pcsxr development (http://pcsxr.codeplex.com).
- Improve plugins (perhaps replace them?)... e.g. cdrmooby28 has some optimization and possible memory issues. As well, maybe an opengl plugin can be ported to gx (with the help of something like gl2gx, WIP see gxrender branch), and a sound plugin with the help of a SDL layer (or ported?).
------------------------------------------
- Xbox 360 and USB HID controller support.
- DualShock 3, DualShock 4 and DualShock 5 controller support.
- Ability to take screenshots like Snes9x RX.
- Possibility to select other BIOS with some basic buttons.
- 240p support.
- CD-DA support.
- CHD, ECM, PBP compressed file support.
- PS1 multitap support.

Any help is appreciated.

# DISCLAIMER: Please do not report issues with specific games, as they may or not be fixed with updates to the code later in the future.

## Credits:
Original WiiSX team:
tehpola, sepp256, emu_kidid

Original WiiSXR coder:
Mystro256

Mods & updates:
Matguitarist, daxtsu, Mystro256, FIX94, NiuuS, xjsxjs197
