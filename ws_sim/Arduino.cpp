#include <Windows.h>

extern "C" void yield()
{
    Sleep(10);
}

extern "C" unsigned long millis()
{
    return GetTickCount();
}

