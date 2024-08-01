# Statistics Plugin
This plugin calculates general statistics. The statistics can be exported as text files and read by OBS to be used in stream overlays.

_Plugin Window:_
![Plugin Window](https://imgur.com/FeiKfO1.jpg)

## Available Statistics
  - **Neutral Wins**: Number of times a player won neutral
  - **Neutral Losses**: Number of times a player lost neutral
  - **Non-Killing Neutral Wins**: Number of times a player won neutral and it didn't lead to a kill (opponent was able to reset neutral again)
  - **Stocks Taken**: Number of times a player won neutral and got a kill (This of course, will not count SDs as a kill)
  - **Stocks**: Number of times a your opponent has died. (Essentially the same as _Stocks Taken_, but includes SDs)
  - **Self Destructs**: Number of times a player dies, but was not killed.
  - **Neutral Win%**: Measure of how much more a player was able to win neutral vs their opponent
  - **Average Damage / Opening**: The average amount of damage a player does for every time they get an opening
  - **Stage Control**: Measure of how much time a player has controlled center stage over their opponent, in percent
  - **Average Death%**: The average damage a player has before dying
  - **Earliest Death**: Lowest damage a player had before dying
  - **Latest Death**: Highest damage a player had before dying
  - **Total Damage Dealt**: Total damage output
  - **Total Damage Received**: Total damage received from opponent
  - **Most Common Neutral Opening Move**: The move that was used the most to win neutral
  - **Most Common Kill Move**: The most common move that resulted in a KO
  - **Most Common Neutral Opener Into Kill Move**: The most common move that started a string/combo which lead into a KO

_Example from Tru4:_
![Example stream from Tru4](https://imgur.com/qvLK155.jpg)

_Example from Tournameta:_
![Example stream from Tournameta](https://imgur.com/52Sq7Sb.jpg)

_Example from CouchwarriorsSmash:_
![Example stream from CouchWarriorsSmash](https://imgur.com/wP2dgMt.jpg)

## Compiling

### On Windows

In short:
  1) Download and install Qt6.
  2) Build ReFramed and install it to a location that doesn't have spaces in the path (I'm sorry I don't know how CMAKE\_INSTALL\_PREFIX works)
  3) Build The plugin


Go to the [Qt Website](https://www.qt.io/download) and download the web installer. I recommend installed **version 5.15**. You will need MSVC 2019 for this (Visual Studio 2022), so install that first if you haven't already.

The Qt web installer will ask you to create an account. You can use one of the many temporary e-mail services to get around this.

In the Windows start menu, search for "VS" and you should see "x86\_x64 Cross Tools Command Prompt for VS 2022". Open that up.

Change the directory to a location where you would like to build ReFramed and its plugins. For me, this was ```C:\Users\username\Documents\programming```.

We have to tell CMake where it can find Qt6. Type ```set Qt5_DIR=C:\Qt\Qt5.15.11\msvc2019_64```. This path may be different on your installation, so change it if necessary. The important detail is that you tell it to use the 64-bit version (msvc2019\_64).

Now you can clone ReFramed. Type ```git clone https://github.com/reframedultimate/ReFramed```.

Create a build directory. 
```
cd ReFramed
mkdir build-debug
cd build-debug
```

Generate a solution. The video player plugin is broken so disable it. Make sure to specify an installation location. This is where the compiled version of ReFramed will be copied to. I chose ```C:\ReFramed-debug-master``` for this (since we're building a debug version and it's from the master branch), but you can choose whatever you want as long as the path doesn't contain spaces!
```
cmake -A x64 -DCMAKE_CXX_FLAGS="/MP" -DREFRAMED_plugin-video-player=OFF -DCMAKE_INSTALL_PREFIX=C:\ReFramed-debug-master ..\
```

Now you can build and install it. Type ```cmake --build . --config Debug --target install```.

Hopefully this all works successfully. If you run into issues you'll have to open the .sln file and start debuggin the issue.

Next, clone this plugin:
```
cd C:\Users\username\Documents\programming
git clone https://github.com/reframedultimate/ReFramed-plugin-stats
```

Create a build directory:
```
cd ReFramed-plugin-stats
mkdir build-debug
cd build-debug
```

We have to tell CMake where it can find the build version of ReFramed:
```
set ReFramed_DIR=C:\ReFramed-debug-master
```

Generate a solution
```
cmake -A x64 -DCMAKE_CXX_FLAGS="/MP" ..\
```

Now you can build the plugin:
```
cmake --build . --config Debug
```

The plugin is automatically copied to ```C:\ReFramed-debug-master``` so you should be able to double-click on ```ReFramed.exe``` and launch the plugin from there.

To set things up in Visual Studio, you can double-click on the solution file (.sln) in the build-debug folder for this plugin. Set the ```plugin-stats``` project as the active project by right-clicking on it and selecting "Set as active project". Then, right-click on the "plugin-stats" project again and go to properties, and under "Debugging" you need to select the command to launch ```C:\ReFramed-debug-master\ReFramed.exe``` and also set the working directory to ```C:\ReFramed-debug-master```. After that, you can simply press F5 to start debugging the plugin.
