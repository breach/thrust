#!/bin/sh

#cd .. && ninja -C out/Release thrust_shell -t clean && cd dist/
#cd .. && ninja -C out/Release thrust_shell -j 4 && cd dist/
mkdir -p out && cd out
rm -rf thrust-v0.7.1-darwin-x64*
mkdir -p thrust-v0.7.1-darwin-x64 && cd thrust-v0.7.1-darwin-x64
cp -R ../../../out/Release/ThrustShell.app .
zip -r ../thrust-v0.7.1-darwin-x64.zip *
cd .. && cd ..


