#!/bin/sh

MESOS_HOME_DIR="$HOME/src/mesos"

THIRD_PARTY="$MESOS_HOME_DIR/build/3rdparty/libprocess/3rdparty"

THIRD_PARTY_PATH="$MESOS_HOME/3rdparty/libprocess"

GTEST_INCLUDE_PATH="-I $MESOS_HOME_DIR/src -I $MESOS_HOME_DIR/include -I $MESOS_HOME_DIR/build/src -I $MESOS_HOME_DIR/build/include/mesos -I $MESOS_HOME_DIR/build/3rdparty/zookeeper-3.4.5/src/c/include  -I $MESOS_HOME_DIR/build/3rdparty/zookeeper-3.4.5/src/c/generated -I $THIRD_PARTY_PATH/include  -I $THIRD_PARTY/gmock-1.7.0/include -I $THIRD_PARTY/boost-1.53.0 -I$THIRD_PARTY/glog-0.3.3/src -I $THIRD_PARTY/stout/include -I $THIRD_PARTY/protobuf-2.5.0/src"

#echo $GTEST_INCLUDE_PATH

g++ uTest_FedAlloc.cpp $GTEST_INCLUDE_PATH -lmesos -std=c++11 -lgtest -lgtest_main -lpthread
