[[/Images/banner.png|Banner]]
This project is an effort to develop a jukebox expansion board for vintage Macintosh computers. 
It is meant to plug into the standard modem port on all compacts (and later desktops), and provide
USB storage, audio output via a headphone jack, and a control interface for the computers. 

It will support MP3 files at most common bitrates and sample frequencies. The Macintosh, being
unable to actually decode modern music files or output high qualithy sound, will act as the command
and control interface for this external board. Playlists can be made, viewing artists/albums/songs, 
controlling mixer levels and volume, and the external board will handle all the decoding of the file,
outputting it to the audio jack.

This repo is organised in to two parts, the firmware for the board itself under "Board", and the Macintosh
software side under "MacApp". 
