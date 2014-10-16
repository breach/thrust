#!/bin/sh

cd .. && ninja -C out/Release thrust_shell -t clean && cd dist/
cd .. && ninja -C out/Release thrust_shell -j 4 && cd dist/
mkdir -p out && cd out
rm -rf thrust-v0.7.0-linux-x64
mkdir -p thrust-v0.7.0-linux-x64 && cd thrust-v0.7.0-linux-x64
cp ../../../out/Release/thrust_shell .
cp ../../../out/Release/*.pak .
cp ../../../out/Release/*.so .
cp ../../../out/Release/*.dat .
cd ..
tar -pczf thrust-v0.7.0-linux-x64.tar.gz thrust-v0.7.0-linux-x64
cd ..


