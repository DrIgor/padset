Apple's Magic Trackpad can be used on a PC without bootcamp by [extracting drivers from their update files](http://superuser.com/questions/170044/how-can-i-install-an-apple-magic-trackpad-on-a-pc-without-boot-camp).  This utility emulates the device configuration routine of the bootcamp control panel, allowing PC users to modify the behavior of their trackpad without installing additional Apple software.


---

### Background ###
Attaching a debugger to the Boot Camp control panel, one can see that the settings are written with an [IOCTL](http://www.osronline.com/ddkx/kmarch/irps_1l0n.htm) of the format `CTL_CODE(FILE_DEVICE_MOUSE, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)`.  The data written matches what is found in the Windows Registry at `KEY_CURRENT_USER\Software\Apple Inc.\Trackpad\Mode`.  The format of this entry is:

```
    0 1 1 0 1 1 1 1  0x6F
    | | | | | | | |
    | | | | | | | tap to click
    | | | | | | dragging
    | | | | | drag lock
    | | | | [reserved, always 1]
    | | | [reserved, always 0]
    | | secondary tap
    | secondary click
    bottom left (0 for bottom right)
```

This program will read this registry entry and write the settings to the `\\.\AppleWirelessTrackpad` device.  Someone with Windows programming experience could contribute to this project a nice UI and re-implement it as a service to detect and initialize when the device is re-connected.

For my personal setup I manually changed the registry setting (0x6b: defaults minus drag lock!) and run this program as a Windows Scheduled Task (at login).  On the rare occasion that I have to re-pair or change batteries, I just run the program again manually to refresh the settings.

### Notes ###
"Secondary click" refers to using one finger to press on one of the bottom corners of the trackpad.  You can safely disable this and still use two fingers together to click (anywhere on the pad) for a context menu.  My new preferred settings are 0xAB since I often end up clicking too close to the bottom edge.