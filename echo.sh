#!/bin/sh


echo "Configuring wifi credentials"

uci set wireless.@wifi-iface[0].ssid="$1"
uci set wireless.@wifi-iface[0].encryption="$2"
uci set wireless.@wifi-iface[0].key="$3"
uci commit wireless
wifi

# echo "$1" $1 $2 $3

