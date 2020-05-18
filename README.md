# embedded-memory-game

1. Prepare an application in C language, which uses the buttons and LED
diodes. When reading the buttons, please take into consideration the
„contact bounce” effect. The application should use the new „libgpiod”
based handling of GPIOs
2. It is required that application implements thorough error checking (you
can’t assume that e.g. all hardware communication functions always
succeed).
3. The application must respond to changes of state of the buttons without
active waiting (should sleep waiting for the required change)
4. „Memory game” – application displays a sequence using the LEDs, the
user must repeat it with the buttons.
5. The application should be converted into the Buildroot package
6. Please try debugging your application with the gdb debugger
