
Debian
====================
This directory contains files used to package vertcoind/vertcoin-qt
for Debian-based Linux systems. If you compile vertcoind/vertcoin-qt yourself, there are some useful files here.

## vertcoin: URI support ##


vertcoin-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install vertcoin-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your vertcoin-qt binary to `/usr/bin`
and the `../../share/pixmaps/vertcoin128.png` to `/usr/share/pixmaps`

vertcoin-qt.protocol (KDE)

