# Nano Video

Video player for NanoPi M3. The video player uses onboard coda960 VPU 
and MLC video layer of s5p6818 soc device to play video files.

## Installation

Debian installation package with nano-video program depends on
gstreamer1.0-plugins-good. This package contains video4linux plugins used by
the program.

To use VPU, also gstreamer1.0-plugins-bad is needed. From this package
h264parse plugin is used. Note that the program works also without VPU
(e.g. when nxp-vpu kernel module is not loaded or the _gstreamer1.0-plugins-bad_
package is not installed). In this case software decoding is made.

If pulseaudio package is installed in system, it is also recommended to
install gstreamer1.0-pulseaudio package.

The nano-video package may be simply installed using _dpkg -i_ command.
