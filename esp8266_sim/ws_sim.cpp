#include "Arduino.h"

#include <WinSock2.h>

int main() {

    WSADATA wsaData;
    int ret = WSAStartup(0x0202, &wsaData);
    if (ret != 0)
    {
        printf("Error %d starting winsock\n", ret);
        return ret;
    }

    setup();

    while (true)
    {
        loop();
    }

    WSACleanup();

    return 0;
}
