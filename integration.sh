#!/bin/bash

# Compiling the webserver
echo "Building webserver."
make

# starting webserver
{	
	echo "Starting webserver."
	timeout 15 ./webserver config
	echo "Stopping webserver."
}&
sleep 3

# testing webserver echo
echo "Running command curl -D headers.txt http://localhost:8007/echo > response.txt"
curl -D headers.txt http://localhost:8007/echo > response.txt

# checking reponse headers
echo "Checking response Headers."
if grep -q "HTTP/1.* 200 OK" headers.txt;then
	if grep -q "Content-Type: text/plain" headers.txt;then
		#Checking if content-length is correct
		count=`wc -c response.txt | awk '{print $1}'`
		if grep -q "Content-Length: $count" headers.txt;then
			echo "All response headers are correct."
		else
			echo "HTTP response does not contain correct Content-Length."
			exit 12
		fi
	else
		echo "HTTP response does not contain Content-Type or is not set to text/plain."
		exit 11
	fi
else
	echo "HTTP response does not contain 200 OK status code."
	exit 10
fi

rm headers.txt
rm response.txt

# testing webserver default mapping
echo "Running command curl -D headers.txt http://localhost:8007/static1/web_sample1.html > response.txt"
curl -D headers.txt http://localhost:8007/static1/web_sample1.html > response.txt

# checking reponse headers
echo "Checking response Headers."
if grep -q "HTTP/1.* 200 OK" headers.txt;then
	if grep -q "Content-Type: text/html" headers.txt;then
		#Checking if content-length is correct
		count=`wc -c response.txt | awk '{print $1}'`
		if grep -q "Content-Length: $count" headers.txt;then
			echo "All response headers are correct."
		else
			echo "HTTP response does not contain correct Content-Length."
			exit 12
		fi
	else
		echo "HTTP response does not contain Content-Type or is not set to text/plain."
		exit 11
	fi
else
	echo "HTTP response does not contain 200 OK status code."
	exit 10
fi

if grep -q "<link rel='canonical' href='http://www.sheldonbrown.com/web_sample1.html'/>" response.txt;then
	echo "File Succesfully transferred"
else
	echo "File contents not Succesfully obtained";
	exit 14
fi

rm headers.txt
rm response.txt

# testing webserver file
echo "Running command curl -D headers.txt http://localhost:8007/text.html > response.txt"
curl -D headers.txt http://localhost:8007/text.html > response.txt

# checking reponse headers
echo "Checking response Headers."
if grep -q "HTTP/1.* 404 Not Found" headers.txt;then
	if grep -q "Content-Type: text/html" headers.txt;then
		#Checking if content-length is correct
		count=`wc -c response.txt | awk '{print $1}'`
		if grep -q "Content-Length: $count" headers.txt;then
			echo "All response headers are correct."
		else
			echo "HTTP response does not contain correct Content-Length."
			exit 12
		fi
	else
		echo "HTTP response does not contain Content-Type or is not set to text/plain."
		exit 11
	fi
else
	echo "HTTP response does not contain 404 Not Found status code."
	exit 10
fi

rm headers.txt
rm response.txt

# testing webserver status
echo "Running command curl -D headers.txt http://localhost:8007/status > response.txt"
curl -D headers.txt http://localhost:8007/status > response.txt

# checking status
if grep -q "Number of Requests: 3" response.txt;then
	echo "Status Checked"
else
	echo "Status is wrong"
	cat response.txt
	exit 14
fi

# removing temp files

rm headers.txt
rm response.txt

#checking two concurrent connections to the server. First one initially partial.
echo "Opening telnet connection for 3 seconds in the background"
##exec 3<>/dev/tcp/localhost/8007
#timeout 1 echo -e "GE" >&3
#cat <&3
# starting webserver
timeout 3 telnet localhost 8007&


#(echo -n "GET /eo HTTP/1.1\r\n") | timeout 1 nc localhost 8007 &
#echo -n"tion: close\r\n\r\n" | timeout 1 nc localhost 8007


# testing concurrent webserver 404
echo "Running command curl -D headers.txt http://localhost:12345/non_existent_uri > response.txt"
curl -s -D headers.txt http://localhost:8007/non_existent_uri.txt > response.txt
#(echo "GET /echo HTTP/1.1\r\nhost: localhost\r\nConnection: close\r\n\r\n") | nc -p 12344 localhost 12345

sleep 1
# checking reponse headers
if grep -q "HTTP/1.* 404 Not Found" headers.txt;then
	if grep -q "Content-Type: text/html" headers.txt;then
		#Checking if content-length is correct
		count=`wc -c response.txt | awk '{print $1}'`
		if grep -q "Content-Length: $count" headers.txt;then
			echo "All response headers are correct."
		else
			echo "HTTP response does not contain correct Content-Length."
			exit 12
		fi
	else
		echo "HTTP response does not contain Content-Type or is not set to text/plain."
		exit 11
	fi
else
	echo "HTTP response does not contain 404 Not Found status code."
	exit 10
fi
sleep 1

# removing temporary headers file
echo "Removing temporary test files."
rm headers.txt
rm response.txt
sleep 1


# exiting with success message
echo "All HTTP tests passing. Well done!"

sleep 10

# starting webserver
{	
	echo "Starting webserver."
	timeout 10 ./webserver httpsconfig
	echo "Stopping webserver."
}&
sleep 3

# testing webserver echo
echo "Running command curl -k -D headers.txt https://localhost:8007/echo > response.txt"
curl -k -D headers.txt https://localhost:8007/echo > response.txt

# checking reponse headers
echo "Checking response Headers."
if grep -q "HTTP/1.* 200 OK" headers.txt;then
	if grep -q "Content-Type: text/plain" headers.txt;then
		#Checking if content-length is correct
		count=`wc -c response.txt | awk '{print $1}'`
		if grep -q "Content-Length: $count" headers.txt;then
			echo "All response headers are correct."
		else
			echo "HTTP response does not contain correct Content-Length."
			exit 12
		fi
	else
		echo "HTTP response does not contain Content-Type or is not set to text/plain."
		exit 11
	fi
else
	echo "HTTP response does not contain 200 OK status code."
	exit 10
fi

rm headers.txt
rm response.txt

# cleaning up
echo "Removing webserver executable and cleaning up."
make clean
sleep 1


echo "HTTPS tests passing also. Well done!"
exit 0
