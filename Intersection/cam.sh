#!/bin/bash

if [ -h /dev/video0 ]; then 
	sudo rm /dev/video-
elif [ -e /dev/video0 ]; then
	sudo mv /dev/video0 /dev/video0.original
fi
if [ -e /dev/video1 ]; then
	sudo ln -s /dev/video1 /dev/video0
elif [ -e /dev/video0.original ]; then
	sudo ln -s /dev/video0.original /dev/video0
else
	ls -l /dev/video
fi
