#!/bin/bash

# integration test to redirect to ucla.edu and return a proper 200 response.

PROXY_CONFIG="port 12345;

default NotFoundHandler {}

path / SpaceteamProxyHandler {
	host www.ucla.edu;
	proxy_port 80;
}"

echo "creating test config"
echo $PROXY_CONFIG > test_302_config

echo "running server"
./webserver test_302_config & sleep 2

echo "sending request and saving response" &
output=$(curl -I http://localhost:12345/ | head -n 1| cut -d $' ' -f2)
RC=0

if [ $output -eq "200" ]; 
	then
		echo "302 redirect integration test SUCCESS"
		RC=0
	else
		echo "302 redirect integration test FAIL"
		RC=1
fi

echo "cleaning up"
rm test_302_config
kill %1

exit $RC