#!/bin/bash
echo "system reboot time" >> /home/output.txt
date >> /home/output.txt
var_name="test.sh"
echo "test $var_name"
sleep 10
echo "sleep 10 senconds"
cd /home/Makefile/net/udp_socket
c=`./main`









