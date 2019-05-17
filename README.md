# What is syslog

Syslog is a standard for message logging. It allows separation of the software that generates messages, the system that stores them, and the software that reports and analyzes them. Each message is labeled with a facility code, indicating the software type generating the message, and assigned a severity level. 

# logging-daemon for demo purpose

A simple program which creates a /dev/log socket and ensures that is readable/writeable for all users. Each log message is forwarded to the standard output and in case of the program has been executed with at least 1 additional parameter it also saves logs into these files.

It is basically the server from a client-server application which waits until a client is connected. Furthermore, each log message is stored in a linked list. A large number of different things happen when a SIGINT signal is invoked:
* The main loop is interrupted => no more logging messages are being accepted.
* Saves logs into files.
* Count of the most used message content.
* and finally deallocation of used resources.

## Compilation
* Make sure you have g++14 available on your computer. 
* ```make```
* ```unlink /dev/log```
* ```systemctl stop rsyslog```
* ```systemctl stop systemd-journald```

## Run
sudo ./logging-daemon /tmp/output1.log /tmp/output2.log /tmp/output3.log

Create log message by [logger](https://linux.die.net/man/1/logger) command.
