#include <windows.h>
#include <iostream>

DWORD WINAPI threadProc()
{
    for (int i = 0; ; ++i)
    {
        std::cout << i << '\n';
        Sleep (1000);
    }

    return 0;
}

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPSTR lpszCmdLine, int iCmdShow)
{
    CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE)threadProc, NULL, 0, NULL);

    int i;
    std::cin >> i;

    return 0;
}
