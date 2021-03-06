#!/bin/bash
BR_NAME=buildroot-2020.02
BR_FILE=${BR_NAME}.tar.bz2
BR_DL=../${BR_FILE}
set -e
if [ ! -f ${BR_DL} ] || ! ( bzip2 -q -t ${BR_DL}); then
  (  
     cd ..
     rm -f ${BR_FILE}
     wget https://buildroot.org/downloads/${BR_FILE}
  )
fi
tar -xjf ${BR_DL}
cp BR_config ${BR_NAME}/.config
rm -rf ${BR_NAME}/output/build/memory-1.0
cp Config.in ${BR_NAME}/package/Config.in
cp -r memory ${BR_NAME}/package
cd ${BR_NAME}
for i in ../patches/* ; do
    patch -p1 < $i
done
make


