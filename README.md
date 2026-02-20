# rpy_proj_analyzer

App for displaying information and a visual representation of Ren'Py scripts / projects.

This is still very much a work in progress!

# Building

You will need CMake and a C++ compiler that supports C++23.
I suggest having the latest version of raylib installed, but if CMake can't
find it, it'll download a fresh copy.
```bash
cmake -S . -B build # or whatever you want to call your build dir
make -C ./build # I like to add `-j6` as an option for faster builds
```

# Running

You can run the app with the following:
```bash
./build/rpy_proj_analyzer # or, again, whatever you call your build dir
```

Running it as shown above will prompt you to drop your script into the window. You can also
point the app directly to a script:
```bash
./build/rpy_proj_analyzer path/to/your/script.rpy
```
You can supply flags as follows:
```bash
./build/rpy_proj_analyzer script.rpy <FLAGS>
# or
./build/rpy_proj_analyzer <FLAGS>
```

### Flags
- `-h`, `--help`
    - Show the help message.
- `-t [threads]`, `--threads [threads]`
    - Use the given number of threads for processing whole projects (currently unused).
- `-w [width]`, `--width [width]`
    - Use the given width for the app window.
- `-w [height]`, `--height [height]`
    - Use the given height for the app window.
- `-d`, `--dark-mode`
    - Use dark colors instead of the light defaults.
- `--no-gui`
    - Run the program as a CLI tool.
    Currently unused, but will display statistics about the given script when implemented.

# Usage
From anywhere, press Ctrl + Q to quit.

### Basic Keys
While viewing a script:
- Up / Down **OR** Alt + Scroll:
    - Increase / decrease the scroll speed of the camera.
- Plus **OR** Minus:
    - Zoom the camera in / out.
- R:
    - Reset the camera zoom.
- WASD **OR** Scroll:
    - Move the camera.
- Shift + Scroll:
    - Move the camera horizontally.
- Hover node w/ mouse (1 sec.):
    - Show line + column number in of this node in the script file.

### Shortcuts:
While viewing a script:
- Ctrl + D:
    - Toggle debug stats.

# To Be Implemented:
I have a few things I need to finish before this is more usable:

- [ ] Finish script.rpy parsing for images and `Character` declarations
- [ ] Crawl project directory and show a file tree on the (left hand side of the) screen
- [ ] Show basic statistics (word count, *effective* word count)
- [ ] Maximum variable increase / decrease per file (simple expressions ONLY)
- [ ] Show complete node details

And a few other things I want to do to be nice and polished:

- [ ] Preview image / audio files when hovering
- [ ] More robust variable tracking
- [ ] More comprehensive reporting for errors / warnings encountered during tokenizing / parsing

And a few goals which are fairly far off:
- [ ] "Simulate" script / project
- [ ] "Screen language" parsing
