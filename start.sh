#!/bin/sh

DIR_LIB=/home/oven2/mini2440tools

export LD_LIBRARY_PATH=$DIR_LIB/lib
export QTDIR=$DIR_LIB                 
export QWS_MOUSE_PROTO=tslib:/dev/input/event0
export TSLIB_CALIBFILE=/etc/pointercal        
export TSLIB_CONFFILE=$DIR_LIB/etc/ts.conf  
export TSLIB_CONSOLEDEVICE=none               
export TSLIB_FBDEVICE=/dev/fb0              
export TSLIB_PLUGINDIR=$DIR_LIB/lib/ts
export TSLIB_TSDEVICE=$DIR_LIB/lib/ts 
export TSLIB_TSEVENTTYPE=INPUT                
export QWS_DISPLAY=LinuxFB:mmWidth=105:mmHeight=140

/home/oven2/build-ComOven2-Target-Release/src/ComOven2 -qws -d
