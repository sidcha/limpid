# Limpid

Limpid is library that allows data transfer to/from your embedded Linux application. It is has no external dependencies and provides a a easy interface thus having minimal impact on existing application.

### Features

  * Includes a miniature readline implementation.
  * No external dependency.
  * Little to no impact on existing software.

### Usage

Have a look at `examples/cli/` directory for creating a console server and client and `examples/api` for creating a JSON AIP server.

### Build and Install

Limpid can be cross compiling by exporting your compiler prefix. If you want to compile with `arm-linux-gnueabihf-gcc`, you would have to `export CROSS_COMPILE=arm-linux-gnueabihf-`. Of course, the path to your compiler should be exported in your shell's `PATH` environment variable. You can skip this step if you are building host.

Additionally, you could `export PREFIX=/custom/install_dir` if you are installing to a non standard install path. 

To build and install limpid,

```shell
make && make install
```

### Bugs

Report bugs to Siddharth Chandrasekaran <siddharth@embedjournal.com>
