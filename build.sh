#!/bin/bash

# This script expects mesos installation directory as mesos home directory
# For no argument and invalid path the script will end

MESOS_HOME_DIR="$1"
if [ "$MESOS_HOME_DIR" == "" ]; then
  echo -e "\nPlease provide the Mesos Directory Path as argument\n"
  exit 1
elif [ ! -d "$MESOS_HOME_DIR" ]; then
  echo -e "\nThe mesos Home Location '$MESOS_HOME_DIR' is Not VALID\n"
  exit 1
fi

echo -e "\nThe mesos Home Location is '$MESOS_HOME_DIR'"
	
COMPILER="g++"
THIRD_PARTY="$MESOS_HOME_DIR/build/3rdparty/libprocess/3rdparty"

HEADER_FILES=" -I $MESOS_HOME_DIR/include -I $MESOS_HOME_DIR/src -I $MESOS_HOME_DIR/build/src -I $MESOS_HOME_DIR/3rdparty/libprocess/include -I $THIRD_PARTY/boost-1.53.0 -I$THIRD_PARTY/glog-0.3.3/src -I $THIRD_PARTY/stout/include -I $THIRD_PARTY/protobuf-2.5.0/src"

$COMPILER $HEADER_FILES -lmesos -std=c++11 -fPIC -shared ./FedAllocator/src/FedAllocator.cpp ./FedComm/src/FedComm.cpp -o libFedModules.so

if [ $? -eq 0 ]; then
  echo -e "Code Compiled and created shared library (libFedModules.so) for you \n"
else
  echo -e "Code Compilation End with ERROR\n"
fi
