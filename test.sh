#!/bin/sh
echo 'inserting module'
insmod crtc.ko 
echo 'showing module info'
modinfo crtc.ko
echo crtc: `hwclock -f /dev/rtc1`
echo rtc `hwclock`
echo 'sleep 10s'
sleep 10s
echo crtc: `hwclock -f /dev/rtc1`
echo rtc `hwclock`
echo 'fast mode'
echo 1 > /proc/crtc
cat /proc/crtc
echo "set valus to crtc"
`hwclock -f /dev/rtc1 --set --date "2000/01/01 00:00:00.0"`
echo crtc: `hwclock -f /dev/rtc1`
echo rtc `hwclock`
echo 'sleep 10s'
sleep 10s
echo crtc: `hwclock -f /dev/rtc1`
echo rtc `hwclock`
echo 'slow mode'
echo 0 > /proc/crtc
cat /proc/crtc
echo "set valus to crtc"
`hwclock -f /dev/rtc1 --set --date "2000/01/01 00:00:00.0"`
sleep 1
echo crtc: `hwclock -f /dev/rtc1`
echo rtc `hwclock`
echo 'sleep 10s'
sleep 10s
echo crtc: `hwclock -f /dev/rtc1`
echo rtc `hwclock`
echo 'unistall module'
rmmod crtc.ko