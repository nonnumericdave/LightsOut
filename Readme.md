## LightsOut
My solution to the Google Foobar "Lights Out" challenge using a mixture of C++, Objective-C, OpenCV and Form.

### Description
The "Lights Out" game is a variation of the older Tiger Electronics "Lights Out" game.  The premise of "Lights Out" is simple:  you are given a N by N grid of lighted buttons, with each light in an initial "On" or "Off" state, chosen at random.  Pressing a button toggles the state of that button's respective light.  However, it also toggles the state of each light in the same row and column.  The goal of the game is to determine the sequence of buttons to press in order for every light on the grid to be in the "Off" state simultanteously.

### Requirements
In order to build this Xcode project, you will need to install the OpenCV and Form frameworks.  Directions on how to do this can be found in the "Frameworks" directory in the "readme.txt" file.

### Usage
The goal of this application is to automagically locate a "Lights Out" game board, along with its state, using the camera on the device.  After the game board has been detected, the application will then give the user an opportunity to "solve" the game.

### Todo
Detecting the "Lights Out" game board state is done -- at least, detecting it from a drawing, as I don't actually know if a physical version exists.  Also, the backend for the solver is complete.  All that is left is to complete the Form solver plugin which will solve the game in an animated manner.  I might also craft up a version of the game to run on an Ableton Push whevever I have some time.
