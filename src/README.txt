Linux GCC Compiler & Linker.


g++ -std=c++0x -I/usr/include -include/usr/include/math.h -include/usr/include/stdlib.h -O3 -Wall -c -fmessage-length=0 -std=c++11 -fpermissive -MMD -MP -MF"src/infinitygame.d" -MT"src/infinitygame.o" -o "src/infinitygame.o" "../src/infinitygame.cpp"


g++ -L/usr/lib -o "ConsoleGame"  ./src/InfinityGameLogic.o ./src/infinityassets.o ./src/infinitygame.o ./src/olcPixelGameEngine.o   -lGL -lX11 -lpthread -lpng
