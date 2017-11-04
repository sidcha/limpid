# Limpid

Limpid is library that allows data transfer to/from your embedded Linux application. It is has no external dependencies and provides a a easy interface thus having minimal impact on existing application.

### Features

  * Includes a miniature readline implementation.
  * No external dependency.
  * Little to no impact on existing software.

### Usage

Have a look at `examples/cli/` directory for creating a console server and client and `examples/api` for creating a JSON AIP server.

### Build & Cross Compiling

Clone the repository and execute `make` from the top level of the repository. Link with -llimpid in your application.

Limpid can be cross compiling by exporting your compiler prefix. If you want to compile with `arm-linux-gnueabihf-gcc`, you would have to do like so, 

``` shell
export CROSS_COMPILE=arm-linux-gnueabihf-
```

Of course, the path to your compiler should be exported in your shell's `PATH` environment.

### Bugs

Report bugs to Siddharth Chandrasekaran <siddharth@embedjournal.com>
