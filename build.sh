make stop

rm -r build/*

cd build


make clean

cmake ..

make -j4

cd ..

cp -r ./build/capture ./collects/


make restart
