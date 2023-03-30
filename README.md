# gstreamer-sdk
gstreamer sdk，有demo，可以根据demo修改

1.安装依赖
sudo apt-get install libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libgstreamer-plugins-bad1.0-dev gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav gstreamer1.0-doc gstreamer1.0-tools gstreamer1.0-x gstreamer1.0-alsa gstreamer1.0-gl gstreamer1.0-gtk3 gstreamer1.0-qt5 gstreamer1.0-pulseaudio 

2.编译
cd gstreamer-sdk
mkdir build
cd build
cmake ..
make -j

3.编译生成demo的软件在x86_64中
cd x86_64/bin

4.如果只需要使用动态库
将x86_64中的include和lib拷走使用，使用方法参考Demo

tips:可以根据需要解开CMakeLists.txt最后两行注释
add_subdirectory(Demo/TestCamera)
add_subdirectory(Demo/TestEncode)
