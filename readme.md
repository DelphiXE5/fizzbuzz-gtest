# Fizzbuzz C++
## Solution
```cpp
#include "Fizzbuzz.h"
// #include <string>

string Fizzbuzz::fizzbuzz(int number){
    if (number % 5 == 0 && number % 3 == 0){
        return "FizzBuzz";
    }
    if (number % 3 == 0){
        return "Fizz";
    }
    if (number % 5 == 0){
        return "Buzz";
    }
    return to_string(number);
}
```

# Writing C++ Challenges
1. You need to copy the Makefile, the Dockerfile.* and create a empty src and test directory <br> (build and lib are filled by Makefile)


2. Copy the CMakeLists.txt to the `root` dir, the `scr` dir and the `test` dir. <br> Now you have a blank C++ Workspace


 3. To get the right xml file please copy the `catch_reporter_eh.cpp` and `catch_reporter_eh.hpp` to the `test` dir. (You don't need to config the Makefile for this)


4. If you need a custom addition to the `eh_reporter` you can add a custom Event-test-listener (https://github.com/catchorg/Catch2/blob/master/docs/event-listeners.md) <br>Test-Listeners need the definition
    ```cpp
    #define CATCH_CONFIG_EXTERNAL_INTERFACES
    ```


5. Now you are ready to write your own challenge. Please keep in mind, that (now) the only usable testframework is Catch2 (https://github.com/catchorg/Catch2/blob/master/docs/Readme.md)


6. Writing a test is pretty simple. 
You only need to write a new `Filename.cpp` and add a 
    ```cpp
    #define CATCH_CONFIG_MAIN
    ```
    to your testfile (only needed once, if you have multiple testfiles.)<br>Then you can follow the steps from the tutorial.


7. Create a editable File for use is also quiet simple: You add a `Filename.cpp` and a `Filename.hpp` to the project and now you can start. Please be aware, that you have to supress the printf and other direkt stdout writing methods, because Catch2 is not capable to read and delete the stream (not great, but it works).
    ```cpp 
    #define printf(fmt, ...) (0)
    ```
    This disables the printf function., if you put it in the header (`.hpp`) file.


8. Exceute tests and setup the full workspace:
To setup your workspace you go to the project root and run:
    ```shell
    make resources
    ```
    To execute the tests, run the following in the project dir:
    ```shell
    make tests
    ```


9. This manual is only for one stage challenges. Multi-stage needs some modifications in Makefile.  
