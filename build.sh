#!/bin/bash

# This script expects mesos installation directory as mesos home directory
# For no argument and invalid path the script will end

MESOS_HOME="$1"
# Will convert relative path to absolute path
MESOS_HOME_DIR="$(dirname $(readlink -e $MESOS_HOME))/$(basename $MESOS_HOME)"

if [ "$MESOS_HOME_DIR" == "" ]; then
  echo -e "\nPlease provide the Mesos Directory Path as argument\n"
  exit 1
elif [ ! -d "$MESOS_HOME_DIR" ]; then
  echo -e "\nThe mesos Home Location '$MESOS_HOME_DIR' is Not VALID\n"
  exit 1
fi

echo -e "\nThe mesos Home Location is '$MESOS_HOME_DIR' \n"
	
COMPILER="g++"
B_THIRD_PARTY="$MESOS_HOME_DIR/build/3rdparty/libprocess/3rdparty"
UB_LIBPROCESS_PATH="$MESOS_HOME_DIR/3rdparty/libprocess"

MESOS_LIB_PATH="-L $MESOS_HOME_DIR/build/src/.libs"

HEADER_FILES=" -I $MESOS_HOME_DIR/include -I $MESOS_HOME_DIR/src -I $MESOS_HOME_DIR/build/include -I $MESOS_HOME_DIR/build/src -I $UB_LIBPROCESS_PATH/include  -I $UB_LIBPROCESS_PATH/3rdparty/stout/include -I $UB_LIBPROCESS_PATH/3rdparty/stout/include -I $B_THIRD_PARTY/picojson-1.3.0 -I $B_THIRD_PARTY/boost-1.53.0 -I$B_THIRD_PARTY/glog-0.3.3/src -I $B_THIRD_PARTY/stout/include -I $B_THIRD_PARTY/protobuf-2.5.8/src"

$COMPILER $HEADER_FILES $MESOS_LIB_PATH -lmesos -std=c++11 -fPIC -shared ./FedAllocator/src/FedAllocator.cpp ./FedComm/src/FedComm.cpp -o libFedModules.so

if [ $? -eq 0 ]; then
  echo -e "\nCode Compiled SUCCESSFULLY and created shared library (libFedModules.so) for you \n"
else
  echo -e "\nCode Compilation End with ERROR\n"
fi
