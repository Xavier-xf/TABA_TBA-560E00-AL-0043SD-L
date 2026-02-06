#!/bin/sh
#udhcpc&
# if [ -f "/data/app.sh" ]
# then
# 	echo "/data/app.sh"
# 	/data/app.sh
# elif [ -f "/etc/config/app.sh" ]
# then
# 	echo "/etc/config/app.sh"
# 	/etc/config/app.sh
# fi
# 守护进程
while test "1" = "1"
do
    sleep 1
    value=$(ps aux | grep ANYKA37E.BIN | grep -v grep | wc -l)
    if [ $value -ne 1 ]
    then
        killall ANYKA37E.BIN
        /app/app/ANYKA37E.BIN leo &
    fi
done