This example can be compiled using Mbed's own build system.
You can download the Mbed CLI here
<https://os.mbed.com/docs/mbed-os/v5.12/quick-start/offline-with-mbed-cli.html>

If running Windows:

- run these git commands before continuing (might need to run
  in admin terminal)

> git config core.symlinks true
> git reset --hard

Using the Mbed CLI:

1. Navigate to the current directory (> examples > mbed)
2. Download Mbed sources required for compiling:

   > mbed deploy

3. Create new mbed program/library:

   > mbed new

4. Set your target device, for example:

   > mbed target NUCLEO_F103RB

5. Compile the example using:

   > mbed compile -t GCC_ARM --profile debug.json
