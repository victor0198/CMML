# Instructions for compiling:
Make sure you have g++ and cmake installed on your computer.

Build restbed:
```
git clone --recursive https://github.com/corvusoft/restbed.git
mkdir restbed/build
cd restbed/build
cmake [-DBUILD_SSL=NO] [-DBUILD_TESTS=NO] ..
make install
make test
```

Command to compile the program:
```
g++ -o main.exe main.cpp -l restbed
```
