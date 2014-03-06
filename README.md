motion-detection
================

Playing around with motion detection using various methods and algorithms in C++ and Python with support for recording from network and prebuffering the image using OpenCV.

##[Demo Sample](http://gfycat.com/BrokenGroundedAstarte):
This is a snippet from a recording that was taken using the motion detector. This is my dumbass roommate playing with it when I told him about it.

![](http://giant.gfycat.com/BrokenGroundedAstarte.gif)

##Features:
- Let's you open a network stream or local camera.
- Implements recording of the stream when motion is detected:
  - Maintains a prebuffer of 5 seconds to capture the moments before the person triggers motion.
  - Maintains a postbuffer to allow a person to reenter the stream without needing to retrigger.
  - Records in MKV format.
  - Maintains a log file
- Multiple Motion Adapters which implement different methods for detecting motion because I felt like it.
- Partially written in both Python and C++.

##Compilation:
###Requires:
- `premake4` for building.
- I can't remmember anything else right now. TODO.