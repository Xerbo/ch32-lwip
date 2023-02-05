#!/usr/bin/env bash
set -e

if [ -z $1 ]; then
    echo "Download the SDK for WCH's RISC-V line of microcontrollers"
    echo "Usage: $0 CH32Vxxx"
    exit 1
fi

# TODO: This is incomplete
NAME=${1^^}
case $NAME in
    CH561)                          FILENAME=CH561EVT.zip URL=https://www.wch.cn/downloads/file/235.html;;
    CH563)                          FILENAME=CH563EVT.zip URL=https://www.wch.cn/downloads/file/189.html;;
    CH569)                          FILENAME=CH569EVT.zip URL=https://www.wch.cn/downloads/file/330.html;;
    CH571 | CH573)                  FILENAME=CH573EVT.zip URL=https://www.wch.cn/downloads/file/337.html;;
    CH581 | CH582 | CH583)          FILENAME=CH583EVT.zip URL=https://www.wch.cn/downloads/file/363.html;;
    CH32V003)                       FILENAME=CH32V003EVT.zip URL=https://www.wch.cn/downloads/file/409.html;;
    CH32V103)                       FILENAME=CH32V103EVT.zip URL=https://www.wch.cn/downloads/file/326.html;;
    CH32V203 | CH32V205 | CH32V208) FILENAME=CH32V20xEVT.zip URL=https://www.wch.cn/downloads/file/385.html;;
    CH32V303 | CH32V305 | CH32V307) FILENAME=CH32V307EVT.zip URL=https://www.wch.cn/downloads/file/356.html;;
    *)
        echo "Unknown chip '$NAME'"
        exit 1
esac

# Download
if [ ! -f /tmp/$FILENAME ]; then
    wget -O /tmp/$FILENAME $URL
    unzip -d /tmp/$NAME /tmp/$FILENAME
fi

# Find correct startup file
STARTUP_FILE=$(grep -l -m 1 $NAME /tmp/$NAME/EVT/EXAM/SRC/Startup/* || true)
if [ -z $STARTUP_FILE ]; then
    STARTUP_FILE=$(grep -l -m 1 ${NAME::-1}x /tmp/$NAME/EVT/EXAM/SRC/Startup/* || true)
fi
if [ -z $STARTUP_FILE ]; then
    echo "Could not find startup file"
    exit 1
fi

# Copy SDK files
if [ -d $NAME-SDK/ ]; then
    rm -rf $NAME-SDK/
fi
cp -r /tmp/$NAME/EVT/EXAM/SRC/ $NAME-SDK/

# Startup file
rm -r $NAME-SDK/Startup
cp $STARTUP_FILE $NAME-SDK/startup.S
