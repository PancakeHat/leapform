# Leapform
This is the repository for Leapform's engine.
The code is not very clean because I didn't originally intend for anybody else to see it. You can try to create mods for it if you really want to dig through the code and figure out how it works, but I don't recommend it.

### Building
The steps shown will be starting from the directory where the executable is located.

**Requirements**
- C/C++ 20 or higher
- CMake
- MinGW *(see note below)*

**Note about Mac/Linux**
These build steps are done on Windows. If you want to try it on Mac or Linux, you will need to edit the ```CMakeLists.txt``` file to use ```Unix Makefiles``` instead of ```MinGW Makefiles``` in ```add_custom_target(release)```.

**Actually compiling**
First, enter the build directory. You can make a seperate folder if you want.
```
cd build
```

Next, CMake generation needs to occur. In this step I will be using MinGW Make, but you can use whatever you want.
```
cmake .. -G "MinGW Makefiles"
```

Finally, generation for the release build needs to happen. Keep in mind that unless you change ```CMakeLists.txt```, you need to have MinGW installed and set up for using Make.
```
make release
```

Make should do its job and create a folder called "release" where the executable and all of its dependencies should be stored. If there is a need to rebuild the game, CMake does not need to be run again.