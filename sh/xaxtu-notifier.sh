#!/bin/sh
#
if [ -x /usr/share/axtu/gui/axtu-notifier-gui ]; then
    pidcmd=`/sbin/pidof axtu-notifier-gui`
    if [ "$pidcmd" = "" ]; then
        /usr/share/axtu/gui/axtu-notifier-gui 2> /dev/null &
    fi
fi

