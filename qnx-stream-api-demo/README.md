
# SCREEN STREAM API DEMO
This project is only for test purpose. The code is very experimental. Please
keep in mind that when you reference this code.


## How to cross compile this project
To build this project, you need to have QNX SDP. If you have, follow the below
commands.

```
$ source ~/your/sdp/path/qnxsdp-env.sh
$ cd ~/project/path
$ mkdir ./build
$ cd ./build
$ cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/modules/qnx-sadk-toolchain.cmake
```

## How to run code.

After copying the binaries, run the below commands.

```
# ./cons &
# ./prod

```
