<h1>Pain_T</h1>
<h2>Simplistic Linux-based paint application</h2><br>


**HOW TO RUN**<br>
A configured makefile is attached to the main directory. Simply run **make** to compile the program.
There is quite a lot of files, so it is far easier this way.


**IMPLEMENTED FEATURES**<br>
I didn't go too overboard for this, as from this point on it would get really complicated.
Therefore as of now, following features have been fully implemented:
    - Brush/Eraser/Fill tools for basic drawing
    - Fully working snapshot-based history with Undo/Redo buttons and ctrl+z/ctrl+y implementations.
    - Fully implemented saving mechanism for the images (System-native file browser used, hope it'll work outside of Arch and Mint)
    - Working theme switching (Dark/Light themes)
    - Working image insertion and basic scaling with ctrl+v support (This one was a bit more complex, maybe it could use a bit more care in the future)

**TESTING**<br>
The program was tested on two machines, one running Arch Linux with custom wayland-based desktop environment and second running Linux Mint with X11 based desktop environment. 
Third party software was used to emulate the program in order to see any obvious memory leaks apart from visual check of the code.
No memory sticks were hurt during these tests and no memory leaks were discovered, so I hope it will be just fine.

**DISCLAIMER**<br>
I do in fact own exactly 0 computers with Windows (because I really don't want to), so in order to compile on Windows machine, it's up to you.
You may install something that will allow you to use make, but it's probably easier to compile it by hand at that point.
