Retro
=====

This is a port of [Retro, by Curious](http://progsoc.org/~curious/software/art/retro/) to the Android NDK.

Requirements
------------
 * Android NDK and SDK
 * Ant
 * A fontsheet in assets/sonic.png

The fontsheet should contain an 8-wide by 12-high array of characters, each character 8x8 pixels. The first character
should be a space, continuing up the ASCII table until character 127. So the first line will be ` ,!"#$%&`, the next
line `'()*+,-.`

Building
--------

	$ android update project --path .
	$ ndk-build
	$ ant debug

