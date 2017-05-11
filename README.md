
device middleware for Venga 
=======

All middleware code is put in the _lib_ directory

main.c is provided as an application example of the middleware


## Build on Linux 

simply run 

make 

And find the output executable in folder bin

## Build on other platform

make P=PLATFORM_NAME

where PLATFORM_NAME is the name of the platform you want to build
the middleware for. The directory lib/platform/PLATFORM_NAME must 
be provided to hold platform related code.

