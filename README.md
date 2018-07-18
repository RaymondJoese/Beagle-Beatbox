# Beagle Beatbox

## Description
Beagle Beatbox is an C language application built on BeagleBone Green and Zen cape. User can use 192.168.7.2:8080 or joystick on Zen to switch the playing mode (total 3 modes so far), change the volume, change tempo. Another awesome feature is it plays sounds if user air-shake the BeagleBone, which is playing concurrently with the playing mode.

## Environment
Linux ubuntu

## Get Started
Create shared folder through NFS: [demo](http://www.cs.sfu.ca/CourseCentral/433/bfraser/other/NFSGuide.pdf
```
# Download or clone repo
git clone https://github.com/HardysJin/Beagle-Beatbox.git

# Go to the directionary 
cd Beagle-Beatbox/

# make the project; create a copy of the executable at public folder
make

# login beagle-bone
ssh root@192.168.7.2

# go to shared folder
cd /mnt/remote/myApps

# run app and try joysticks -- up&down for volume adjustment;
# left&right for tempo adjustment; push-in for changing mode
./beatbox

### start web server to allow web control
# open another terminal
ctr+shift+T

# login beable-bone
ssh root@192.168.7.2

# go sharded server folder
cd /mnt/remote/myApps/beatbox-server-copy

# run server
node server.js

# open browser and type in
192.168.7.2:8080
```
