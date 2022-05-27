This project is only tested on Ubuntu 22.04, hack it with your own imagination.

Download source code
```shell
$ git clone https://github.com/derekdai/ioasync.git
$ cd ioasync
```

Prepare build directory
```shell
$ meson build .
```

Build library and tests
```shell
$ meson compile -C build
```
