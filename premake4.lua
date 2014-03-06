solution "NetworkCamera"
  configurations {"Debug", "Release"}
  location "build"
  targetdir "bin"

  project "ipTest"
    -- kind "ConsoleApp"
    kind "WindowedApp"
    language "C++"
    files { "main.cc" }
    -- links { "opencv" }
    buildoptions { "`pkg-config --cflags opencv`", "-std=c++11", '-g', '-stdlib=libc++' }
    linkoptions { "`pkg-config --libs opencv`", '-L/opt/local/lib', '-stdlib=libc++' }

    configuration "Debug"
      defines { "DEBUG" }
      flags {"Symbols" }

    configuration "Release"
      defines {"NDEBUG"}
      flags {"Optimize"}

  project "property_test"
    kind "ConsoleApp"
    language "C++"
    files { "property.cc" }
    -- links { "opencv" }
    -- buildoptions { "`pkg-config --cflags opencv`" }
    linkoptions { "-L/opt/local/lib" }
    links { "gtest" }
    buildoptions { "-std=c++11" }

    configuration "Debug"
      defines { "DEBUG" }
      flags {"Symbols" }

    configuration "Release"
      defines {"NDEBUG"}
      flags {"Optimize"}

  project "property_simplified_test"
    kind "ConsoleApp"
    language "C++"
    files { "property_simplified.cc" }
    -- links { "opencv" }
    -- buildoptions { "`pkg-config --cflags opencv`" }
    linkoptions { "-L/opt/local/lib" }
    links { "gtest" }
    buildoptions { "-std=c++1y" }

    configuration "Debug"
      defines { "DEBUG" }
      flags {"Symbols" }

    configuration "Release"
      defines {"NDEBUG"}
      flags {"Optimize"}


  project "motion_detect_other"
    kind "WindowedApp"
    language "C++"
    files { "motion_detect.cpp" }
    -- links { "opencv" }
    buildoptions { "`pkg-config --cflags opencv`" }
    linkoptions { "`pkg-config --libs opencv`", "-lconfig", "-L/opt/local/lib" }
    buildoptions { "-std=c++11" }

    configuration "Debug"
      defines { "DEBUG" }
      flags {"Symbols" }

    configuration "Release"
      defines {"NDEBUG"}
      flags {"Optimize"}

     
