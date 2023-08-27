# Proxy discovery library for the Mac platform.

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
- Xcode - 12.2 and higher (Minimum for Universal binaries)
- Xcode Command Line Tools - running `clang`, `gcc` or similar commands will trigger a prompt, or just run `xcode-select --install` to begin the install process.

# Build

To build ProxyDiscovery-Mac, perform following commands:

## XCode debug build:
~~~
mkdir build
cd build
cmake -G "Xcode" -DCMAKE_BUILD_TYPE=Debug ../
~~~
Open XCode and build

## XCode release build:
~~~
mkdir build
cd build
cmake -G "Xcode" -DCMAKE_BUILD_TYPE=RelWithDebInfo ../
~~~
Open XCode and build

## Regular debug build:
~~~
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ../
cmake --build .
~~~

## Regular release build:
~~~
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ../
cmake --build .
~~~
