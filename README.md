# IoT Communications Library
- HW & FW supporting a range of devices 4G, 3G, 2G, Wifi
- MQTT messaging protocol

### Toolchain 
- meson
- ninja

### Unit Tests
- CppUnit

### Code Guidelines & Linter
- https://github.com/isocpp/CppCoreGuidelines
- clang-format linter

### Dependancies
`git submodule init`
`git submodule update`

meson
ninja
python
clang-format
docker?

### Build
- should the build be separate for each example project?
- one main build in the root for unit tests

`meson build`
`meson build --cross-file stm32.cross.build`

`cd build`
`ninja`

### Documentation
- doxygen

### Directory Structure
- root
    - examples
        - platform
            - os
    - dep
        - library_name
            - src
    - src
        - Config
        - CommmDevices
        - Modules
        - Platform     <-- i don't think this belongs here
    - test
        - should match the src directory


# Notes
- consider splitting out dependancy folder into src & examples. if i was implementing this library i want to be able to import the src dir, have access to all required depedancies but exclude any that are platform specific.
- move platform folder our of src and into examples, see note above. I don't think any platform specific code should be in the src dir.