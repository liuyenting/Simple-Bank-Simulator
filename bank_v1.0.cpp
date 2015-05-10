// #define _DEBUG_ENABLED

#include <windows.h>
#include <conio.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <ctime>
#include <cmath>
#include <limits>
#include <algorithm>

/* color definition */
#define NORMAL      15
#define CLEAR       0
#define HIGHLIGHT   128 // 240

/* key code definition */
#define UP          72
#define RIGHT       75
#define LEFT        77
#define DOWN        80
#define ENTER       13
#define ESCAPE      27

/* time constant, unit: seconds */
#define DEFAULT_IT   5

/* cash dependent constant */
#define DEFAULT_RT   1.1

/* display dependent constant, default */
#define DEFAULT_HL   10

using namespace std;

/* console menu operation */
void gotoXY(int x, int y);
void textAttribute(int color);
void clearScreen();
void cursorVisible(bool visibility);

/* bank function */
void checkBalance();
void showHistory();
void deposit();
void withdraw();
void configurations();

/* forward declaration for debug function */
void debug();

/* forward declaration for Date */
class Date
{
public:
    Date();
    void printDateFormat();
    void syncDate();
    bool syncDate(int year, int month, int day);
    bool setDate(int year, int month, int day);
    bool isFuture();
    int elapseDays();
    int elapseDays(Date target);
private:
    time_t timer;   // absolute timestamp, from 1900
    bool isValid(tm *date);
    time_t sim_time();
};

/* history type */
typedef struct
{
    Date date;
    /*  1: deposit
        2: withdraw
        3: check balance
        4: modify interest */
    int action;
    double cash;
    double balance;
    double interest;
    float rate;
} HISTORY;

/* custom defined sorting algorithm for history sort */
bool recordCompare_ascending(HISTORY a, HISTORY b)
{
    int A = a.date.elapseDays();
    int B = b.date.elapseDays();
    return A<B;
}

bool recordCompare_descending(HISTORY a, HISTORY b)
{
    int A = a.date.elapseDays();
    int B = b.date.elapseDays();
    return A>B;
}

/* global constant */
HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
Date initialDate;
vector< HISTORY > history;
double balance, interest;
double rate;
int historyLength;              // records per page
int interval = DEFAULT_IT;      // simulation time ratio, CRITICAL in initialization
bool isAscending;               // true: asc, false: des, default: false

/* global time constant
    base_time: simulation date,
    startup_time: actual program starup time */
time_t base_time, startup_time;

/* class for menu */
class Menu
{
public:
    /* constructor for question+selection, type 1 */
    Menu(string title,
         vector< string > options)
    {
        menuType = 1;
        this -> title = title;
        this -> options = options;
    }

    /* constructor for question+selection+explanation, type 2 */
    Menu(string title,
         vector< string > options,
         vector< string > explanations)
    {
        menuType = 2;
        this -> title = title;
        this -> options = options;
        this -> explanations = explanations;
    }
    int acquireSelection()
    {
        while(1)
        {
            gotoXY(xPos_item, yPos_item+selection);
            textAttribute(HIGHLIGHT);
            cout << options[selection];

            if(menuType == 2)
            {
                gotoXY(xPos_explanation, yPos_explanation);
                textAttribute(NORMAL);
                cout << explanations[selection];
            }

            switch(getch())
            {
            case UP:
                if(selection > 0)
                {
                    if(menuType == 2)
                    {
                        gotoXY(xPos_explanation, yPos_explanation);
                        textAttribute(CLEAR);
                        cout << explanations[selection];
                    }

                    gotoXY(xPos_item, yPos_item+selection);
                    textAttribute(NORMAL);
                    cout << options[selection];
                    selection--;
                }
                break;

            case DOWN:
                if(selection < options.size()-1)
                {
                    if(menuType == 2)
                    {
                        gotoXY(xPos_explanation, yPos_explanation);
                        textAttribute(CLEAR);
                        cout << explanations[selection];
                    }

                    gotoXY(xPos_item, yPos_item+selection);
                    textAttribute(NORMAL);
                    cout << options[selection];
                    selection++;
                }
                break;

            case ENTER:
                return selection;
            }
        }
    }

    /* constructor for question+direct execution, type 3 */
    Menu(string title,
         vector< string > options,
         vector<void (*)()> function)
    {
        menuType = 3;
        this -> title = title;
        this -> options = options;
        this -> function = function;
    }
    void execution()
    {
        function[selection]();
    }

    /* display content */
    void display()
    {
        clearScreen();

        yPos_explanation = yPos_item + options.size() + 1;

        switch(menuType)
        {
        case 1 ... 2:
            gotoXY(xPos_title, yPos_title);
            textAttribute(NORMAL);
            cout << title;

            for(int i = 0; i < options.size(); i ++)
            {
                gotoXY(xPos_item, yPos_item+i);
                textAttribute(NORMAL);
                cout << options[i];
            }

            break;

        case 3:
            break;

        case 4:
            gotoXY(xPos_title, yPos_title);
            textAttribute(NORMAL);
            cout << title;
        }

        /* explanation display or not */
        switch(menuType)
        {
        case 2:
            gotoXY(xPos_explanation, yPos_explanation);
            textAttribute(NORMAL);
            cout << explanations[selection];
            break;
        }
    }

    /* display with line shift, specific usage */
    void display(int yShift)
    {
        yPos_item += yShift;
        display();
    }

    /* show title only, for specific situation */
    void reshowTitle()
    {
        clearScreen();
        gotoXY(xPos_title, yPos_title);
        textAttribute(NORMAL);
        cout << title;
    }
private:
    int selection = 0, menuType = 0;

    /* default cursor position */
    const int xPos_title = 3;
    const int xPos_item = 6;
    const int xPos_explanation = 3;
    const int yPos_title = 1;
    int yPos_item = 3;
    int yPos_explanation;

    /* display contents */
    string title;
    vector< string > options;
    vector< string > explanations;
    vector<void (*)()> function;
};

/* constructor, synchronize date with current time */
Date::Date()
{
    syncDate();
}

/* simulation time span, fast forward under ratio defined by interval*/
time_t Date::sim_time()
{
    return ( 86400/interval * (time(NULL) - startup_time) ) + base_time;
}

/* print date in format of YYYY-MM-DD */
void Date::printDateFormat()
{
    char buffer[20];
    strftime (buffer, 20, "%Y-%m-%d", localtime(&timer));

    cout << buffer;
}

/* synchronize timer variable */
void Date::syncDate()
{
    timer = sim_time();
}

/* define current absolute time using date as argument, specific usage */
bool Date::syncDate(int year, int month, int day)
{
    tm *timeinfo;

    time(&base_time);
    timeinfo = localtime( &base_time );
    timeinfo -> tm_year = year - 1900;
    timeinfo -> tm_mon = month - 1;
    timeinfo -> tm_mday = day;

    base_time = mktime(timeinfo);
    timer = base_time;

    /* date availability */
    if(isValid(timeinfo)) return true;
    else return false;
    /* if(base_time != -1) return true;
    else return false; */
}

/* define current absolute time using date as argument, normal usage */
bool Date::setDate(int year, int month, int day)
{
    tm *timeinfo;

    timer = sim_time();
    timeinfo = localtime( &timer );
    timeinfo->tm_year = year - 1900;
    timeinfo->tm_mon = month - 1;
    timeinfo->tm_mday = day;

    timer = mktime(timeinfo);

    if(timer != -1) return true;
    else return false;
}

/* check if current time is in future */
bool Date::isFuture()
{
    if(difftime(timer, sim_time()) > 0)
        return true;
    else
        return false;
}

/* day elapsed, with global time */
int Date::elapseDays()
{
    double interval = difftime(timer, base_time);
    return floor(interval / 86400);
}

/* day elapsed, between target time */
int Date::elapseDays(Date target)
{
    double interval = abs(elapseDays() - target.elapseDays());
    return floor(interval / 86400);
}

/* check if the date is valid */
bool Date::isValid(tm *date)
{
    bool result = true;
    int day = date->tm_mday;
    int month = date->tm_mon + 1;
    int year = date->tm_year + 1900;
    if(year < 0) result = false;
    else
    {
        if(month > 12 || month < 1 || day < 0)
            result = false;
        else
        {
            switch(month)
            {
            case 1:
            case 3:
            case 5:
            case 7:
            case 8:
            case 10:
            case 12:
                if(day > 31) result = false;
                break;

                /* check leap year exception */
            case 2:
                if((year % 4 == 0) &&
                        !(year % 100 == 0) ||
                        (year % 400 == 0))
                {
                    if(day > 29) result = false;
                }
                else
                {
                    if(day > 28) result = false;
                }
                break;

            default:
                if(day > 30) result = false;
                break;
            }
        }
    }

    return result;
}

/* timer for interest calculation */
DWORD WINAPI interestTimer()
{
    Date currentDate, interestAccrualDate;

    double temporaryInterest = 0;
    int countTime = 0;
    interestAccrualDate = initialDate;

    while(1)
    {
        currentDate.syncDate();

        /* acquire daily interest */
        if(abs(currentDate.elapseDays() - interestAccrualDate.elapseDays()) == 1)
        {
            temporaryInterest += balance * (rate * 0.01 /30);
            interestAccrualDate = currentDate;
            countTime++;
        }

        /* check if it's monthly interest liquidate time */
        if(countTime == 30)
        {
            /* move temporary interest to actual result */
            interest += temporaryInterest;

            /* create new record */
            HISTORY record;
            record.date = currentDate;
            record.action = 1;
            record.cash = temporaryInterest;
            record.balance = balance;
            record.interest = interest;
            record.rate = rate;

            /* push new record into history */
            history.push_back(record);

            /* reset variables */
            temporaryInterest = 0;
            countTime = 0;
        }
    }

    return 0;
}

void initialize()
{
    /* set display */
    textAttribute(NORMAL);

#ifndef _DEBUG_ENABLED
    /* configure initial date */
    vector< int > tmpDate(3);
    cout << " Initial date(YYYY MM DD) -> ";

    while(1)
    {
        while(!(cin >> tmpDate[0] >> tmpDate[1] >> tmpDate[2]))
        {
            cout << "  * Invalid character entered." << endl << endl;
            cout << " Retry in \"YYYY MM DD\" form -> ";
            cin.clear();
            cin.ignore(100, '\n');
        }

        if(!initialDate.syncDate(tmpDate[0], tmpDate[1], tmpDate[2]))
        {
            cout << "  * An invalid date." << endl << endl;
            cout << " Retry correct date -> ";
        }
        else
        {
            cout << "  * Confirmed... ";
            initialDate.printDateFormat();
            Sleep(500);
            break;
        }
    }
#else
    /* hard coded date definition for debug */
    initialDate.syncDate(1996, 8, 16);

    tm* timeinfo;
    timeinfo = localtime( &base_time );

    char buffer[20];
    strftime (buffer, 20, "%Y-%m-%d", timeinfo);

    cout << buffer;
#endif // _DEBUG_ENABLED

    /* configure startup time, real world time scale */
    startup_time = time(NULL);

    /* start new thread for interest calculation */
    CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE)interestTimer, NULL, 0, NULL);

    /* for visualize initial settings, removable */
    Sleep(1000);

    /* initialize parameters */
    rate = DEFAULT_RT;
    balance = 0;
    interest = 0;

    historyLength = DEFAULT_HL;
    isAscending = false;

    /* start record by initialize 0 */
    HISTORY record;
    record.date = initialDate;
    record.action = 1;
    record.cash = 0;
    record.balance = balance;
    record.interest = interest;
    record.rate = rate;

    history.push_back(record);

    /* disable cursor */
    cursorVisible(false);

    /* show overall status */
    clearScreen();

    cout << endl;
    cout << " > Startup date...  ";
    initialDate.printDateFormat();
    cout << endl << " > Interest rate...  " << DEFAULT_RT << "%";
    cout << endl << " > History length...  " << DEFAULT_HL << " lines";
    cout << endl << " > Time scale...  " << DEFAULT_IT << " seconds / 1 real-world day";
    cout << endl << " > Sorting...  " << ((isAscending)?"Ascending":"Descending");

    gotoXY(3, 7);
    textAttribute(NORMAL);
    cout << "Press [ENTER] to continue..." << endl;
    while(getch() != ENTER);

    /* fresh new screen */
    clearScreen();
}

/* main function */
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPSTR lpszCmdLine, int iCmdShow)
{
    initialize();

    vector<string> rootLayer_options =
    {
        "Check balance",
        "Deposit",
        "Withdraw",
        "Configurations",
        "Quit"
    };
    vector<string> rootLayer_explanations =
    {
        "Check your overall balance, interest income and history.",
        "Deposit money into your account.",
        "Withdraw money into your account.",
        "Modify interest rate, refresh interval... etc.",
        "Exit this simulation program."
    };

    bool flag = true;
    while(flag)
    {
        Menu rootLayer("SIMPLE BANK SIMULATION", rootLayer_options, rootLayer_explanations);
        rootLayer.display();

        switch(rootLayer.acquireSelection())
        {
        case 0:
            checkBalance();
            break;

        case 1:
            deposit();
            break;

        case 2:
            withdraw();
            break;

        case 3:
            configurations();
            break;

        case 4:
            flag = false;
            break;

        }
    }

    /* end message */
    clearScreen();

    textAttribute(HIGHLIGHT);
    gotoXY(3, 2);
    cout << "                               ";
    gotoXY(3, 3);
    cout << "   THANKS FOR YOUR TRIAL~      ";
    gotoXY(3, 4);
    cout << "           By Andy Liu, 2013   ";
    gotoXY(3, 5);
    cout << "                               ";
    gotoXY(3, 6);
    cout << "              Version: RC3     ";
    gotoXY(3, 7);
    cout << "                               ";

    gotoXY(3, 9);
    textAttribute(NORMAL);
    cout << "Press [ENTER] again to exit..." << endl;
    while(getch() != ENTER);
    system("exit");

    return 0;
}

/* set cursor position */
void gotoXY(int x, int y)
{
    COORD coordinate;
    coordinate.X = x;
    coordinate.Y = y;
    SetConsoleCursorPosition(console, coordinate);
}

/* set text appearance */
void textAttribute(int color)
{
    /* set background */
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

/* implement clear screen, without system call */
void clearScreen()
{
    HANDLE                     hStdOut;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD                      count;
    DWORD                      cellCount;
    COORD                      homeCoords = { 0, 0 };

    hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if(hStdOut == INVALID_HANDLE_VALUE) return;

    /* get the number of cells in the current buffer */
    if(!GetConsoleScreenBufferInfo(hStdOut, &csbi)) return;
    cellCount = csbi.dwSize.X * csbi.dwSize.Y;

    /* fill the entire buffer with spaces */
    if(!FillConsoleOutputCharacter(hStdOut,
                                   (TCHAR) ' ',
                                   cellCount,
                                   homeCoords,
                                   &count)) return;

    /* fill the entire buffer with the current colors and attributes */
    if(!FillConsoleOutputAttribute(hStdOut,
                                   csbi.wAttributes,
                                   cellCount,
                                   homeCoords,
                                   &count)) return;

    /* move the cursor home */
    SetConsoleCursorPosition(hStdOut, homeCoords);
}

/* hide or show the cursor */
void cursorVisible(bool visibility)
{
    HANDLE hConsoleOutput;
    CONSOLE_CURSOR_INFO structCursorInfo;
    hConsoleOutput = GetStdHandle( STD_OUTPUT_HANDLE );
    GetConsoleCursorInfo( hConsoleOutput, &structCursorInfo );
    structCursorInfo.bVisible = visibility;
    SetConsoleCursorInfo( hConsoleOutput, &structCursorInfo );
}

void checkBalance()
{
    /*  1: deposit
        2: withdraw
        3: check balance
        4: modify interest */

    vector<string> balanceInquiry_options =
    {
        "Show history",
        "Back"
    };
    vector<string> balanceInquiry_explanations =
    {
        "Print out transaction history.",
        "Return to main menu."
    };

    bool flag = true;
    while(flag)
    {
        Menu balanceInquiryMenu("SBS - BALANCE INQUARY",
                                balanceInquiry_options,
                                balanceInquiry_explanations);
        balanceInquiryMenu.display(7);

        /* show current account status */
        textAttribute(NORMAL);
        gotoXY(6, 3);
        cout << setw(19) << right << "Balance =";
        gotoXY(27, 3);
        cout << left << fixed << setprecision(2) << balance;
        gotoXY(6, 4);
        cout << setw(19) << right << "Interest income =";
        gotoXY(27, 4);
        cout << left << fixed << setprecision(2) << interest;
        gotoXY(6, 5);
        cout << "-----------------------------------";
        gotoXY(6, 6);
        cout << setw(19) << right << "Summation =";
        gotoXY(27, 6);
        cout << left << fixed << setprecision(2) << (balance + interest);

        /* show current date */
        Date currentDate;
        gotoXY(6, 8);
        cout << "Review date: ";
        currentDate.syncDate();
        currentDate.printDateFormat();

        switch(balanceInquiryMenu.acquireSelection())
        {
        case 0:
            showHistory();
        case 1:
            flag = false;
            break;
        }
    }
}

void showHistory()
{
    /* options with: previous, next, return */
    vector<string> historyOptions_pnr =
    {
        "Previous page",
        "Next page",
        "Return main menu"
    };

    /* options with: previous, return */
    vector<string> historyOptions_pr =
    {
        "Previous page",
        "Return main menu"
    };

    /* options with: next, return */
    vector<string> historyOptions_nr =
    {
        "Next page",
        "Return main menu"
    };

    /* options with: next, return */
    vector<string> historyOptions_r =
    {
        "Return main menu"
    };

    /* pointers parameter */
    int recordSize = history.size();
    int pageIndex = 0, maxPage;
    int shiftLines = 0;
    vector<string> *menu;

    /* sort history by descending */
    if(isAscending)
        sort(history.begin(), history.end(), recordCompare_ascending);
    else
        sort(history.begin(), history.end(), recordCompare_descending);

    /* force to at least 1 page */
    if(recordSize == 0) maxPage = 1;
    else
        maxPage = ceil((float)(recordSize) / historyLength);

    bool flag = true;
    while(flag)
    {
        textAttribute(NORMAL);

        if(maxPage == 1)
            /* single record page */
            menu = &historyOptions_r;
        else
        {
            /* multiple record page */
            if(pageIndex == 0)
                menu = &historyOptions_nr;
            else if(pageIndex == maxPage-1)
                menu = &historyOptions_pr;
            else
                menu = &historyOptions_pnr;
        }

        Menu showHistory("SBS - ACCOUNT HISTORY", *menu);
        showHistory.display(historyLength + 5);

        textAttribute(NORMAL);
        gotoXY(6, 3);
        cout << "-----------------------------------------------------";
        gotoXY(6, 4);
        cout << setw(13) << left << "|  Date"
             << setw(13) << left << "|  Deposit"
             << setw(13) << left << "|  Withdraw"
             << setw(13) << left << "|  Balance"
             << "|";
        gotoXY(6, 5);
        cout << "-----------------------------------------------------";

        for(shiftLines=0; shiftLines<historyLength; shiftLines++)
        {
            /* move index for history usage */
            int index = pageIndex*historyLength + shiftLines;

            /* out of range verification */
            if(index > history.size() - 1)
                break;

            /* move cursor to record coordinate */
            gotoXY(6, 6+shiftLines);

            /* print date */
            cout << "| " << setw(10) << left;
            history[index].date.printDateFormat();

            /* print deposit or withdraw cash amount*/
            if(history[index].action == 1)
            {
                cout << " |" << setw(11) << right << history[index].cash
                     << " |" << setw(11) << right << " ";
            }
            else
            {
                cout << " |" << setw(11) << right << " "
                     << " |" << setw(11) << right << history[index].cash;
            }

            cout << " |" << setw(11) << right << history[index].balance + history[index].interest
                 << " |";
        }

        /* fill the blank */
        for(; shiftLines < historyLength; shiftLines ++)
        {
            gotoXY(6, 6+shiftLines);
            cout << "| " << setw(10) << " ";
            cout << " |" << setw(11) << " "
                 << " |" << setw(11) << " ";
            cout << " |" << setw(11) << " "
                 << " |";
        }
        gotoXY(6, 6+shiftLines);
        cout << "-----------------------------------------------------";

        /*
        gotoXY(6, (shiftLines+1) + 5);
        cout << "-----------------------------------------------------";
        */

        /* show page number */
        gotoXY(40, historyLength + 8);
        cout << "Page " << pageIndex+1
             << " of " << maxPage;

        switch(showHistory.acquireSelection())
        {
        case 0:
            if(maxPage == 1)
                flag = false;
            else
            {
                if(pageIndex == 0)
                    pageIndex ++;
                else
                    pageIndex --;
            }
            break;

        case 1:
            if(pageIndex == 0 || pageIndex == maxPage-1)
                flag = false;
            else
                pageIndex ++;
            break;

        case 2:
            flag = false;
            break;
        }
    }

    textAttribute(NORMAL);
}

void deposit()
{
    vector<string> depositMenu_options =
    {
        "Confirm",
        "Modify",
        "Back"
    };
    vector<string> depositMenu_explanations =
    {
        "Confirm and proceed the transaction.",
        "Re-enter amount of money for this session.",
        "Return to main menu."
    };

    double depositCash;
    Date depositDate;

DEPOSIT_ENTER:

    Menu depositMenu("SBS - DEPOSIT", depositMenu_options, depositMenu_explanations);
    depositMenu.reshowTitle();

    textAttribute(NORMAL);
    gotoXY(6, 3);
    cout << "Deposit cash: ";

    cursorVisible(true);
    while(true)
    {
        textAttribute(HIGHLIGHT);
        gotoXY(22, 3);
        cout << "               ";
        gotoXY(23, 3);

        if(cin >> depositCash)
        {
            textAttribute(NORMAL);
            gotoXY(22, 4);
            if(depositCash < 0)
                cout << "Cash amount should be positive.";
            else
                break;
        }
        else
        {
            textAttribute(NORMAL);
            gotoXY(22, 4);
            cout << "Value not acceptable.";
        }
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    textAttribute(CLEAR);
    gotoXY(22, 4);
    cout << "                         ";
    cursorVisible(false);
    textAttribute(NORMAL);

    depositDate.syncDate();

    int selection;
    bool flag = true;
    while(flag)
    {
        depositMenu.display(5);

        textAttribute(NORMAL);
        gotoXY(6, 3);
        cout << setw(20) << right << "Deposit cash =";
        gotoXY(28, 3);
        cout << depositCash;
        gotoXY(6, 4);
        cout << setw(20) << right << "Deposit date =";
        gotoXY(28, 4);
        depositDate.printDateFormat();
        gotoXY(6, 5);
        cout << "---------------------------------------";
        gotoXY(6, 6);
        cout << setw(20) << right << "Available balance =";
        gotoXY(28, 6);
        cout << (balance + interest) + depositCash;

        selection = depositMenu.acquireSelection();

        switch(selection)
        {
        case 0:
        {
            textAttribute(NORMAL);
            gotoXY(15, 8);
            cout << "Proceed... ";

            /* save into temp result */
            balance += depositCash;

            /* create new record */
            HISTORY record;
            record.date = depositDate;
            record.action = 1;
            record.cash = depositCash;
            record.balance = balance;
            record.interest = interest;
            record.rate = rate;

            /* push new record into history */
            history.push_back(record);

            cout << "Complete!";
            Sleep(1000);
        }
        flag = false;
        break;

        case 1:
            goto DEPOSIT_ENTER;
            break;

        case 2:
            flag = false;
            break;
        }
    }

#ifdef _DEBUG_ENABLED
    gotoXY(3, 12);
    debug();
#endif // _DEBUG_ENABLED
}

void withdraw()
{
    vector<string> withdrawMenu_options =
    {
        "Confirm",
        "Modify",
        "Back"
    };
    vector<string> withdrawMenu_explanations =
    {
        "Confirm and proceed the transaction.",
        "Re-enter amount of money for this session.",
        "Return to main menu."
    };
    vector<string> withdrawForbidMenu_options =
    {
        "Back"
    };

    if(balance <= -1000)
    {
        Menu withdrawMenu("SBS - WITHDRAW", withdrawForbidMenu_options);
        withdrawMenu.display(3);

        textAttribute(NORMAL);
        gotoXY(6, 3);
        cout << "This account is currently in debt...";
        gotoXY(6, 4);
        cout << "Withdraw action is FORBIDDEN!!";

        while(withdrawMenu.acquireSelection());

        textAttribute(NORMAL);

        return;
    }

    double withdrawCash;
    Date withdrawDate;

WITHDRAW_ENTER:

    Menu withdrawMenu("SBS - WITHDRAW", withdrawMenu_options, withdrawMenu_explanations);
    withdrawMenu.reshowTitle();

    textAttribute(NORMAL);
    gotoXY(6, 3);
    cout << "Withdraw cash: ";

    cursorVisible(true);
    while(true)
    {
        textAttribute(HIGHLIGHT);
        gotoXY(23, 3);
        cout << "               ";
        gotoXY(24, 3);

        if(cin >> withdrawCash)
        {
            textAttribute(NORMAL);
            gotoXY(22, 4);
            if(withdrawCash < 0)
                cout << "Cash amount should be positive.";
            else
                break;
        }
        else
        {
            textAttribute(NORMAL);
            gotoXY(22, 4);
            cout << "Value not acceptable.";
        }
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    textAttribute(CLEAR);
    gotoXY(23, 4);
    cout << "                         ";
    cursorVisible(false);
    textAttribute(NORMAL);

    withdrawDate.syncDate();

    int selection;
    bool flag = true;
    while(flag)
    {
        withdrawMenu.display(5);

        textAttribute(NORMAL);
        gotoXY(6, 3);
        cout << setw(21) << right << "Withdraw cash =";
        gotoXY(29, 3);
        cout << withdrawCash;
        gotoXY(6, 4);
        cout << setw(21) << right << "Withdraw date =";
        gotoXY(29, 4);
        withdrawDate.printDateFormat();
        gotoXY(6, 5);
        cout << "---------------------------------------";
        gotoXY(6, 6);
        cout << setw(21) << right << "Available balance =";
        gotoXY(29, 6);
        cout << (balance + interest) - withdrawCash;

        /* in debt warning */
        if((balance + interest) - withdrawCash < 0)
            cout << " [liability]";

        selection = withdrawMenu.acquireSelection();

        switch(selection)
        {
        case 0:
        {
            textAttribute(NORMAL);
            gotoXY(15, 8);
            cout << "Proceed... ";

            /* save into temp result */
            balance -= withdrawCash;

            /* create new record */
            HISTORY record;
            record.date = withdrawDate;
            record.action = 2;
            record.cash = withdrawCash;
            record.balance = balance;
            record.interest = interest;
            record.rate = rate;

            /* push new record into history */
            history.push_back(record);

            cout << "Complete!";
            Sleep(1000);
        }
        flag = false;
        break;

        case 1:
            goto WITHDRAW_ENTER;
            break;

        case 2:
            flag = false;
            break;
        }
    }

#ifdef _DEBUG_ENABLED
    gotoXY(3, 12);
    debug();
#endif // _DEBUG_ENABLED
}

void configurations()
{
    /*  1: deposit
        2: withdraw
        3: check balance
        4: modify interest */

    vector<string> config_options =
    {
        "Interest rate",
        "Simulation time scale",
        "List length",
        "Sorting of history",
        "Back"
    };

    vector<string> config_explanations =
    {
        "Modify the monthly interest rate.",
        "Change the simulation-real world time scale ratio.",
        "Configure how many records displayed per page.",
        "Ascending or descending sort of the records.",
        "Return to main menu."
    };

    bool flag = true;
    while(flag)
    {
        Menu configMenu("SBS - CONFIGURATIONS", config_options, config_explanations);
        configMenu.display();
        switch(configMenu.acquireSelection())
        {
            /* interest rate */
        case 0: // TODO: identify if quite action perform
        {
            double newRate;

            configMenu.reshowTitle();
            gotoXY(6, 3);
            cout << "New interest rate:";

            cursorVisible(true);
            while(true)
            {
                textAttribute(HIGHLIGHT);
                gotoXY(25, 3);
                cout << "     ";
                textAttribute(NORMAL);
                gotoXY(31, 3);
                cout << "%";
                textAttribute(HIGHLIGHT);
                gotoXY(26, 3);

                if(cin >> newRate)
                {
                    textAttribute(CLEAR);
                    gotoXY(25, 4);
                    cout << "                                       ";

                    textAttribute(NORMAL);
                    gotoXY(25, 4);

                    /* proper value verification */
                    if(newRate < 0)
                        cout << "Interest rate should be positive.";
                    else if(newRate > 100)
                        cout << "Interest rate shouldn't exceed 100%.";
                    else
                        break;
                }
                else
                {
                    textAttribute(NORMAL);
                    gotoXY(25, 4);
                    cout << "Value not acceptable.";
                }
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }

            cursorVisible(false);
            textAttribute(NORMAL);

            rate = newRate;
            gotoXY(25, 4);
            cout << "Confirm... " << fixed << setprecision(2) << rate << "%";
            Sleep(1000);
        }
        flag = false;
        break;

        /* time scale */
        case 1:
        {
            vector<string> timeScale_options =
            {
                "Extremely fast",
                "Fast",
                "Default",
                "Slow",
                "Extremly Slow"
                "Back"
            };
            vector<string> timeScale_explanations =
            {
                "1 seconds / 1 real-world day",
                "5 seconds / 1 real-world day",
                "10 seconds / 1 real-world day",
                "15 seconds / 1 real-world day",
                "30 seconds / 1 real-world day",
                "Return to main menu."
            };

            bool flag = true;
            int selection;
            while(flag)
            {
                Menu timeScaleMenu("SBS - CONFIGURATIONS", timeScale_options,
                                   timeScale_explanations);
                timeScaleMenu.display();

                selection = timeScaleMenu.acquireSelection();

                switch(selection)
                {
                case 0:
                    interval = 1;
                    flag = false;
                    break;

                case 1:
                    interval = 5;
                    flag = false;
                    break;
                case 2:
                    interval = 10;
                    flag = false;
                    break;

                case 3:
                    interval = 15;
                    flag = false;
                    break;

                case 4:
                    interval = 30;
                    flag = false;
                    break;

                default:
                    flag = false;
                    break;
                }
            }

            textAttribute(NORMAL);
            gotoXY(26, selection + 3);
            cout << "...Applied!";
            Sleep(1000);
        }
        return;

        /* history length */
        case 2:
        {
            vector<string> listLength_options =
            {
                "Minium",
                "Default",
                "Maximum",
                "Back"
            };
            vector<string> listLength_explanations =
            {
                "5 records per page.",
                "8 records per page.",
                "11 records per page.",
                "Return to main menu."
            };

            bool flag = true;
            int selection;
            while(flag)
            {
                Menu listLengthMenu("SBS - CONFIGURATIONS", listLength_options,
                                    listLength_explanations);
                listLengthMenu.display();

                selection = listLengthMenu.acquireSelection();

                switch(selection)
                {
                case 0 ... 3:
                    historyLength = 5 + selection * 3;
                    flag = false;
                    break;
                default:
                    flag = false;
                    break;
                }
            }

            textAttribute(NORMAL);
            gotoXY(15, selection + 3);
            cout << "...Applied!";
            Sleep(1000);
        }
        return;

        /* sorting method */
        case 3:
        {
            vector<string> sortingMethod_options =
            {
                "Ascending",
                "Descending",
                "Back"
            };
            vector<string> sortingMethod_explanations =
            {
                "Oldest record at front.",
                "Newest record at front.",
                "Return to main menu."
            };

            bool flag = true;
            while(flag)
            {
                Menu sortingMethodMenu("SBS - CONFIGURATIONS", sortingMethod_options,
                                       sortingMethod_explanations);
                sortingMethodMenu.display();

                switch(sortingMethodMenu.acquireSelection())
                {
                case 0:
                    isAscending = true;
                    textAttribute(NORMAL);
                    gotoXY(17, 3);
                    cout << "...Applied!";
                    flag = false;
                    break;

                case 1:
                    isAscending = false;
                    textAttribute(NORMAL);
                    gotoXY(17, 4);
                    cout << "...Applied!";
                    flag = false;
                    break;

                default:
                    flag = false;
                    break;
                }

                /* wait for the message for shown */
                Sleep(1000);
            }
        }
        return;

        case 4:
            flag = false;
            break;
        }
    }
}

#ifdef _DEBUG_ENABLED
/* debug function */
void debug()
{
    cout << endl << endl;

    /* sort thte history by date */
    if(isAscending)
        sort(history.begin(), history.end(), recordCompare_ascending);
    else
        sort(history.begin(), history.end(), recordCompare_descending);

    /* check if history is blank */
    if(history.size() == 0)
        cout << "NO HISTORY" << endl;
    else
    {
        for(int i=0; i<history.size(); i++)
        {
            cout << "   " << "D:";
            history[i].date.printDateFormat();
            cout << "   " << "A:" << history[i].action;
            cout << "   " << "C:" << history[i].cash;
            cout << "   " << "B:" << history[i].balance;
            cout << "   " << "I:" << history[i].interest;
            cout << "   " << "R:" << history[i].rate;
            cout << endl;
        }
    }

    cout << endl;
    system("PAUSE");
}
#endif // _DEBUG_ENABLED
