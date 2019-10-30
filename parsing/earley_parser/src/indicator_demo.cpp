////////////////////////////////////////////////////////////////////////
// Demonstration of the progress and busy indicator classes
// Matthias Bisping
////////////////////////////////////////////////////////////////////////

#include "../incl/busy.hpp"
#include "../incl/load.hpp"

#include <iostream>
#include <chrono>
#ifdef _WIN32
#include <windows.h>
#endif

void secondsleep(float usec)
{
    #ifdef _WIN32
    HANDLE timer;
    LARGE_INTEGER ft;

    ft.QuadPart = -(10*usec);

    timer = CreateWaitableTimer(NULL, TRUE, NULL);
    SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
    WaitForSingleObject(timer, INFINITE);
    CloseHandle(timer);
    #else
    usleep(usec*1000);
    #endif
}

using namespace std::chrono;

int main()
{

    BUSY::Variant1 busybar1;
    std::cout << "Demonstrating the busy indicator (V1):\n";
    for (long i = 0; i < 16000; ++i)
    {
        secondsleep(0.1);
        busybar1.run();
    }
    busybar1.cancel();


    BUSY::Variant2 busybar2;
    std::cout << "Demonstrating the busy indicator (V2):\n";
    for (long i = 0; i < 20000; ++i)
    {
        secondsleep(0.1);
        busybar2.run();
    }
    busybar2.cancel();


    LOAD::Progressbar progressbar(500);
    std::cout << "Demonstrating the load indicator:\n";
    for (long i = 0; i < 500; ++i)
    {
        secondsleep(1.2);
        progressbar.run(i);
    }

}
