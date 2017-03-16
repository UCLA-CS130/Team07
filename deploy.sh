#!/bin/bash
docker save httpserver | bzip2 | ssh -i CS130.pem ec2-user@35.166.184.153 'bunzip2 | docker load && docker stop $(docker ps -aqf "name=httpserver") && docker run --rm -t -p 8080:8080 httpserver'
