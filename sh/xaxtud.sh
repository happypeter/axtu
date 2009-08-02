#!/bin/sh
#
if [ -x /usr/share/axtu/gui/axtu-notifier-gui ]; then
    killall axtu-notifier-gui 2> /dev/null    
fi

if [ -x /sbin/axtud ]; then
   /sbin/axtud &
fi

