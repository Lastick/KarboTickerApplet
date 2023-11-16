# KarboTickerApplet

apt-get install libcurl4-openssl-dev
apt-get install libpanel-applet2-dev

sudo cp KarboTickerApplet.server /usr/lib/bonobo/servers/
sudo cp KarboTickerApplet /usr/lib/KarboTickerApplet/KarboTickerApplet

mkdir build
cd build/
cmake -D CMAKE_INSTALL_PREFIX=/usr -D CMAKE_BUILD_TYPE=Release ..
make
sudo make install

