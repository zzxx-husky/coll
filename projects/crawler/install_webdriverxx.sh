scriptdir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
install_dir=$1

if [ -z "${install_dir}" ]; then
  echo "Installation directory for webdriverxx is not specified"
  exit
fi

cd ${install_dir}
git clone https://github.com/GermanAizek/webdriverxx
cd webdriverxx

mkdir release
cd release
cmake ../src -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$(pwd)/../install

make install -j4

if [ -z "$(cat ~/.bashrc | grep WEBDRIVERXX_ROOT)" ]; then
  cd ../install
  echo "export WEBDRIVERXX_ROOT=$(pwd)/" >> ~/.bashrc
  echo "export CMAKE_PREFIX_PATH=\${WEBDRIVERXX_ROOT}:\${CMAKE_PREFIX_PATH}" >> ~/.bashrc
fi

cd ${install_dir}
git clone https://github.com/GermanAizek/picobase64
cp ./picobase64/picobase64.h ./webdriverxx/install/include/webdriverxx/webdriverxx/

cd ${scriptdir}
wget https://selenium-release.storage.googleapis.com/4.0-beta-4/selenium-server-4.0.0-beta-4.jar
