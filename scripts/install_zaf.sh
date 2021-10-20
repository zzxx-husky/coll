#!/bin/bash

scriptdir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

ZAF_VERSION=master

if [ ! -z "$1" ]; then
  cd $1
else
  cd ${scriptdir}
fi

if [ ! -d ./zaf/install ]; then
  if [ ! -d ./zaf ]; then
    git clone --depth 1 http://github.com/zzxx-husky/zaf --branch ${ZAF_VERSION}
  fi

  ./zaf/scripts/install_libzmq.sh $(pwd)
  ./zaf/scripts/install_gtest.sh $(pwd)
  ./zaf/scripts/install_glog.sh $(pwd)
  ./zaf/scripts/install_phmap.sh $(pwd)

  # souring ~/.bashrc does not take effect here ... therefore, cmake below will fail to find coll's dependencies.
  # a workaround is to run this script first, source ~/.bashrc outside and then run the script for the second time
  # . ~/.bashrc

  cd zaf
  mkdir build
  cd build
  cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$(pwd)/../install && make -j4 install
  cd ../..
fi

if [ -z "$(cat ~/.bashrc | grep ZAF_ROOT)" ]; then
  echo "export ZAF_ROOT=$(pwd)/zaf/install" >> ~/.bashrc
  echo "export LD_LIBRARY_PATH=\${ZAF_ROOT}/lib:\${LD_LIBRARY_PATH}" >> ~/.bashrc
  echo "export CMAKE_PREFIX_PATH=\${ZAF_ROOT}:\${CMAKE_PREFIX_PATH}" >> ~/.bashrc
fi
