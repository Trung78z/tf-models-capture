make stop

rm -r build/*

cd build

cmake ..

make -j4

cd ..

cp -r ./build/capture ./collects/


make restart
