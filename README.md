# Proxy discovery library for Mac and Linux platforms.

# Getting started
Fork the `UnifiedConnector/cm-ProxyDiscovery-Mac` repository:

```
git clone git@code.engine.sourcefire.com:<your username>/ProxyDiscovery-Mac.git
```

Next, add an upstream remote for fetching from the shared UnifiedConnector repository:

```
git remote add upstream git@code.engine.sourcefire.com:UnifiedConnector/ProxyDiscovery-Mac.git
```

# Making Changes

To make changes to files:

```
git checkout -b <branch name>
* Make changes *
git add ...
git commit ...
# Using -u will automatically set the branch to track origin
git push -u origin <branch name>
* Create pull request in GitHub *
```

# Environment

To be able to build ProxyDiscovery-Mac, the following software packages are required:

- CMake - which can be acquired from [here](https://cmake.org/download/)
- Xcode - 12.2 and higher (Minimum for Universal binaries)  (Mac only)
- Xcode Command Line Tools - running `clang`, `gcc` or similar commands will trigger a prompt, or just run `xcode-select --install` to begin the install process. (Mac only)
- gtest
- curl - (Linux only) 

# Mac Build

The CMAKE_CXX_STANDARD value in below commands could be 11, 14, 17.
To build ProxyDiscovery-Mac, perform following commands:

## XCode debug build:
~~~
mkdir build
cd build
cmake -G "Xcode" -DCMAKE_C_STANDARD=99 -DCMAKE_CXX_STANDARD=17 -DCMAKE_CXX_EXTENSIONS=ON -DCMAKE_BUILD_TYPE=Debug -Dgtest_INCLUDE_DIRS=<absolute path to the directory containing the google test header files> -Dgtest_LIBRARY=<absolute path to the directory containing the google test library binaries> ../
~~~
Open XCode and build

## XCode release build:
~~~
mkdir build
cd build
cmake -G "Xcode" -DCMAKE_C_STANDARD=99 -DCMAKE_CXX_STANDARD=17 -DCMAKE_CXX_EXTENSIONS=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo -Dgtest_INCLUDE_DIRS=<absolute path to the directory containing the google test header files> -Dgtest_LIBRARY=<absolute path to the directory containing the google test library binaries> ../
~~~
Open XCode and build

## Regular debug build:
~~~
mkdir build
cd build
cmake -DCMAKE_C_STANDARD=99 -DCMAKE_CXX_STANDARD=17 -DCMAKE_CXX_EXTENSIONS=ON -DCMAKE_BUILD_TYPE=Debug -Dgtest_INCLUDE_DIRS=<absolute path to the directory containing the google test header files> -Dgtest_LIBRARY=<absolute path to the directory containing the google test library binaries> ../
cmake --build .
~~~

## Regular release build:
~~~
mkdir build
cd build
cmake -DCMAKE_C_STANDARD=99 -DCMAKE_CXX_STANDARD=17 -DCMAKE_CXX_EXTENSIONS=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo -Dgtest_INCLUDE_DIRS=<absolute path to the directory containing the google test header files> -Dgtest_LIBRARY=<absolute path to the directory containing the google test library binaries> ../
cmake --build .
~~~

### If you want to use debug versions of google test libraries (gtestd, gmockd) for the release build you could do the following:
~~~
mkdir build
cd build
cmake -DCMAKE_C_STANDARD=99 -DCMAKE_CXX_STANDARD=17 -DCMAKE_CXX_EXTENSIONS=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo -DUSE_DEBUG_TEST_LIBRARIES=TRUE -Dgtest_INCLUDE_DIRS=<absolute path to the directory containing the google test header files> -Dgtest_LIBRARY=<absolute path to the directory containing the google test library binaries> ../
cmake --build .
~~~


# Linux Build

## Regular debug build:
~~~
mkdir build
cd build
cmake -DCMAKE_C_STANDARD=99 -DCMAKE_CXX_STANDARD=17 -DCMAKE_CXX_EXTENSIONS=ON -DCMAKE_BUILD_TYPE=Debug -Dgtest_INCLUDE_DIRS=<absolute path to the directory containing the google test header files> -Dgtest_LIBRARY=<absolute path to the directory containing the google test library binaries> -DCURL_LIBRARY_DIR=<absolute path to the directory containing the curl library> -DCURL_INCLUDE_DIR=<absolute path to directory containing curl header files ../
cmake --build .
~~~

## Regular release build:
~~~
mkdir build
cd build
cmake -DCMAKE_C_STANDARD=99 -DCMAKE_CXX_STANDARD=17 -DCMAKE_CXX_EXTENSIONS=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo -Dgtest_INCLUDE_DIRS=<absolute path to the directory containing the google test header files> -Dgtest_LIBRARY=<absolute path to the directory containing the google test library binaries> -DCURL_LIBRARY_DIR=<absolute path to the directory containing the curl library> -DCURL_INCLUDE_DIR=<absolute path to directory containing curl header files> ../
cmake --build .
~~~

### If you want to use debug versions of google test libraries (gtestd, gmockd) for the release build you could do the following:
~~~
mkdir build
cd build
cmake -DCMAKE_C_STANDARD=99 -DCMAKE_CXX_STANDARD=17 -DCMAKE_CXX_EXTENSIONS=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo -DUSE_DEBUG_TEST_LIBRARIES=TRUE -Dgtest_INCLUDE_DIRS=<absolute path to the directory containing the google test header files> -Dgtest_LIBRARY=<absolute path to the directory containing the google test library binaries> -DCURL_LIBRARY_DIR=<absolute path to the directory containing the curl library> -DCURL_INCLUDE_DIR=<absolute path to directory containing curl header files> ../
cmake --build .
~~~

# Note
- Repository of the google test library which cisco is using is:
git@code.engine.sourcefire.com:cloud/fireamp-win-google-test.git
