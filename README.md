# ChowLoader Nintendo Switch Binary

This repo contain the source of the code in the patch of the console version and the transformer that convert the Mach-O output of the build (build made on Apple Silicon Mac) to ipswitch patch compatible file.

The build command is

```console
clang -O0 -g ./**/*.c -mcpu=cortex-a57 -o chowloader.o
```