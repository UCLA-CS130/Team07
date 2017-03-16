#!/bin/bash

rm -rf deploy/
mkdir deploy/
cp config_deploy deploy/
cp Dockerfile.run deploy/
cp -r files/ deploy/
cp -r files1/ deploy/
cp -r files2/ deploy/
cp -r ssl_certificates/ deploy/ 2>/dev/null

#build the binary
docker build -t httpserver.build .
docker run httpserver.build > binary.tar
tar -xvf binary.tar -C ./deploy

#build the new image
docker build -t httpserver -f deploy/Dockerfile.run ./deploy

rm -f binary.tar
