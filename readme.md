# AmpySynth!

![](docs/ampy-synth-pinout.png)

## Specifying version

For AmpySynth Rev1 (no buttons on left side, labeled v1.0), omit the hardware revision flag.
For AmpySynth Rev2 (buttons on side, labeled v1.1), ensure that `build_flags` includes `-DAMPYSYNTH_V1_1` in your platformio.ini

## Programming using USB
We use the CH340 USB to serial IC. Mac comes with drivers, while Windows may need you to install some.
Due to timing issues, you usually have to manually pull `IO0` to `GND` while programming. In Rev2 the top left button can be used for this.
In Rev 1, both the `IO0` and `IO2` (RGB LED) pins must be pulled to `GND` when uploading. 
This can be a little fiddly, one trick is to connect one end of the crocodile clip to `GND` (e.g. with rotary encoder shaft), and use the open jaws of the other end to short both `IO0` and `IO2` at the same time.

## Upload over network from PlatformIO

Uncomment the line `upload_port = 192.168.1.100` and change it to the IP address of your AmpySynth to upload over the network. You can then use the Upload button in PlatformIO to update your AmpySynth.

## Uploading to filesystem

Once your upload port is set to an IP address, you can upload files to the SPIFFS filesystem using the command `platformio run --target uploadfs --environment esp32dev` or click "Upload Filesystem Image" in PlatformIO.
_Note_ Don't use the "Upload Filesystem Image OTA" option, as this sends the wrong settings.

## Displays

### 240x240 IPS LCD ST7789

To use this LCD module add the flag `-DLCD_240_240` to `build_flags`
