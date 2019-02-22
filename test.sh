#!/bin/sh
echo 'inserting module'
insmod crtc_driver.ko
echo 'showing module info'
modinfo crtc_driver.ko
echo crtc: `cat /dev/crtc`
echo rtc `hwclock`
sleep 3s
echo crtc: `cat /dev/crtc`
echo rtc `hwclock`
echo 'fast mode'
echo 1 > /proc/crtc
cat /proc/crtc
echo "set rtc's valus to crtc"
timetoset=`hwclock`
echo ${timetoset::26} > /dev/crtc
echo crtc: `cat /dev/crtc`
echo rtc `hwclock`
echo 'sleep 5s'
sleep 5s
echo crtc: `cat /dev/crtc`
echo rtc `hwclock`
echo 'slow mode'
echo 0 > /proc/crtc
cat /proc/crtc
echo 'set time to crtc and sleep 20s'
echo "1997-01-31 00:05:00.000901" > /dev/crtc
echo crtc: `cat /dev/crtc`
sleep 20s
echo crtc: `cat /dev/crtc`
echo 'unistall module'
rmmod crtc_driver.ko