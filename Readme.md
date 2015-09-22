## LightsOut
My solution to the Google Foobar "Lights Out" challenge using a mixture of C++, Objective-C, OpenCV and Form.

### Description
The "Lights Out" game is a variation of the older Tiger Electronics "Lights Out" game.  The premise of "Lights Out" is simple:  you are given a N by N grid of lighted buttons, with each light in an initial "On" or "Off" state, chosen at random.  Pressing a button toggles the state of that button's respective light.  However, it also toggles the state of each light in the same row and column.  The goal of the game is to determine the sequence of buttons to press in order for every light on the grid to be in the "Off" state simultanteously.

### Requirements
In order to build the Form/PerformPlugins, as well as the LightsOutFormViewer, you will need to install the OpenCV and Form frameworks.  If you want to build the non-Form version of the viewer, LightOutViewer, you will only need to install the OpenCV framework.  Directions on how to do both can be found in the "Frameworks" directory in the "readme.txt" file.

### Usage
The goal of this application is to automagically locate a "Lights Out" game board, along with its state, using the camera on the device.  After the game board has been detected, the application will then give the user an opportunity to "solve" the game.

### YouTube Demonstration
[![IMAGE ALT TEXT](http://img.youtube.com/vi/v02fEpykwyk/maxresdefault.jpg)](http://www.youtube.com/watch?v=v02fEpykwyk "GitHub LightsOut")

### Todo
Crafting up a version of the game to run on an Ableton Push whevever I have some time.
