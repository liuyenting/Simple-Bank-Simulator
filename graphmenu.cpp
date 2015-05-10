#include <windows.h>
#include <conio.h>
#include <iostream>
#include <vector>
#include <string>

#define MaxNo_Menu 5

using namespace std;

HANDLE con=GetStdHandle(STD_OUTPUT_HANDLE);

void gotoxy(int x,int y)
{
    COORD Coord;
    Coord.X=x;
    Coord.Y=y;

    SetConsoleCursorPosition(con,Coord);
}

void textattr(int color)
{
    SetConsoleTextAttribute(con, color);
}

void SetColorAndBackground(int ForgC, int BackC)
{
    WORD wColor = ((BackC & 0x0F) << 4) + (ForgC & 0x0F);;
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), wColor);
    return;
}

void ClearScreen()
{
    HANDLE                     hStdOut;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD                      count;
    DWORD                      cellCount;
    COORD                      homeCoords = { 0, 0 };

    hStdOut = GetStdHandle( STD_OUTPUT_HANDLE );
    if (hStdOut == INVALID_HANDLE_VALUE) return;

    /* Get the number of cells in the current buffer */
    if (!GetConsoleScreenBufferInfo( hStdOut, &csbi )) return;
    cellCount = csbi.dwSize.X *csbi.dwSize.Y;

    /* Fill the entire buffer with spaces */
    if (!FillConsoleOutputCharacter(
                hStdOut,
                (TCHAR) ' ',
                cellCount,
                homeCoords,
                &count
            )) return;

    /* Fill the entire buffer with the current colors and attributes */
    if (!FillConsoleOutputAttribute(
                hStdOut,
                csbi.wAttributes,
                cellCount,
                homeCoords,
                &count
            )) return;

    /* Move the cursor home */
    SetConsoleCursorPosition( hStdOut, homeCoords );
}

int main()
{
// This piece of code hides the cursor !!
    HANDLE hConsoleOutput;
    CONSOLE_CURSOR_INFO structCursorInfo;
    hConsoleOutput = GetStdHandle( STD_OUTPUT_HANDLE );
    GetConsoleCursorInfo( hConsoleOutput, &structCursorInfo );
    structCursorInfo.bVisible = false;
    SetConsoleCursorInfo( hConsoleOutput, &structCursorInfo );


    vector<string> menu_list = { "Applications", "Games", "Prompt", "Calculator", "Quit" };
    int i,
        xpos = 5,
        ypos[MaxNo_Menu] = { 2, 3, 4, 5, 6 };

    // list the menu
    for (i=0; i< MaxNo_Menu; ++i)
    {
        gotoxy(xpos, ypos[i] );
        SetColorAndBackground(255, 0);
        cout << menu_list[i];
    }

    // make menu available to choose
    i=0;
    while(1)
    {


        gotoxy(xpos, ypos[i] );
        SetColorAndBackground(0, 8);
        cout << menu_list[i];

        /* note : 72 -> UP button
        	75 -> RIGHT button
            77 -> LEFT button
            80 -> DOWN button
        */

        switch( getch() )
        {
        case 72:
            if(i>0)
            {
                gotoxy(xpos, ypos[i] );
                SetColorAndBackground(255, 0);
                cout << menu_list[i];
                --i;
            }
            break;

        case 80:
            if(i< MaxNo_Menu-1 )
            {
                gotoxy(xpos, ypos[i] );
                SetColorAndBackground(255, 0);
                cout << menu_list[i];
                ++i;
            }
            break;

        case 13:
            ClearScreen();
            if(i==0)
            {
                gotoxy (10,1);
                cout << "you choose Applications";
            }
            if(i==1)
            {
                gotoxy (10,1);
                cout << "you choose Games  ";
            }
            if(i==2)
            {
                gotoxy (10,1);
                cout << "you choose Prompt";
            }
            if(i==3)
            {
                gotoxy (10,1);
                cout << "you choose Calculator  ";
            }
            if(i==4)
            {
                gotoxy (10,1);
                cout << "you choose Quit  ";
            }
            break;
        }
    }

    return 0;
}


