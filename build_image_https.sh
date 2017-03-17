#!/bin/bash

rm -rf deploy_https/
mkdir deploy_https/
cp config_deploy_https deploy_https/
cp Dockerfile_https.run deploy_https/
cp -r files/ deploy_https/
cp -r files1/ deploy_https/
cp -r files2/ deploy_https/
cp -r ssl_certificates/ deploy_https/ 2>/dev/null

#build the binary
docker build -t httpsserver.build .
docker run httpsserver.build > binary_https.tar
tar -xvf binary_https.tar -C ./deploy_https

#build the new image
docker build -t httpsserver -f deploy_https/Dockerfile_https.run ./deploy_https

rm -f binary_https.tar
