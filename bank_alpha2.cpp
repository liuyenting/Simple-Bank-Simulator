#define _DEBUG_ENABLED

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
#define INTERVAL    1

/* cash dependent constant */
#define RATE        1.1

using namespace std;

/* console menu operation */
void gotoXY(int x, int y);
void textAttribute(int color);
void clearScreen();
void cursorVisible(bool visibility);

/* bank function */
void checkBalance();
void liquidate();
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
private:
    time_t timer;   // absolute timestamp, from 1900
    bool isValid(tm date);
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

bool recordCompare(HISTORY a, HISTORY b)
{
    int A = a.date.elapseDays();
    int B = b.date.elapseDays();
    return A<B;
}

/* global constant */
HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
Date initialDate;
vector< HISTORY > history;
double balance, interest;
float rate;

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

/* simulation time span, fast forward under ratio defined by INTERVAL */
time_t Date::sim_time()
{
    return ( 86400/INTERVAL * (time(NULL) - startup_time) ) + base_time;
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
    timeinfo->tm_year = year - 1900;
    timeinfo->tm_mon = month - 1;
    timeinfo->tm_mday = day;

    base_time = mktime(timeinfo);
    timer = base_time;

    if(base_time != -1) return true;
    else return false;

    /*
    tm date;

    date.tm_year = year - 1900;
    date.tm_mon = month - 1;
    date.tm_mday = day;


    if(isValid(date))
    {
        base_time = mktime(&date);
        timer = base_time;
        return true;
    }
    else
        return false; */
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

/* day elapsed */
int Date::elapseDays()
{
    double interval = difftime(timer, base_time);
    return floor(interval / 86400);
}

/* check if the date is valid */
bool Date::isValid(tm date)
{
    bool result = true;
    int day = date.tm_mday;
    int month = date.tm_mon + 1;
    int year = date.tm_year + 1900;
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


void initialize()
{
    /* set display */
    textAttribute(NORMAL);

#ifndef _DEBUG_ENABLED
    /* configure initial date */
    vector< int > tmpDate(3);
    cout << " * Please enter initial date in the form of \"YYYY MM DD\"" <<endl;
    cout << "    -> ";
    while(1)
    {
        while(!(cin >> tmpDate[0] >> tmpDate[1] >> tmpDate[2]))
        {
            cout << " ! Invalid character entered." << endl << endl;
            cout << " Retry in \"YYYY MM DD\" form -> ";
            cin.clear();
            cin.ignore(100, '\n');
        }

        if(!initialDate.defineDate(tmpDate[0], tmpDate[1], tmpDate[2]))
        {
            cout << " ! An invalid date." << endl << endl;
            cout << " Retry correct date -> ";
        }
        else
        {
            cout << "Confirmed... ";
            initialDate.printDateFormat();
            //cout << "... Elapse... " << initialDate.elapseDays();
            Sleep(2000);
            break;
        }
    }
#else
    initialDate.syncDate(1996, 8, 16);

    tm* timeinfo;
    timeinfo = localtime( &base_time );

    char buffer[20];
    strftime (buffer, 20, "%Y-%m-%d", timeinfo);

    cout << buffer;
#endif // _DEBUG_ENABLED

    startup_time = time(NULL);
    Sleep(1000);

    /* initialize parameters */
    rate = RATE;
    balance = 0;
    interest = 0;

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

    /* fresh new screen */
    clearScreen();
}

/* main function */
int main()
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

    gotoXY(3, 7);
    textAttribute(NORMAL);
    cout << "Press [ENTER] again to exit..." << endl;
    while(getch() != 13);
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
        Menu balanceInquiryMenu("SIMPLE BANK SIMULATION",
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
            break;

        case 1:
            flag = false;
            break;
        }
    }
}

void liquidate()
{

}

void deposit()
{
    vector<string> depositMenu_options =
    {
        "Immediate deposit",
        "Advance deposit",
        "Back",
    };
    vector<string> depositMenu_explanations =
    {
        "Immediately import cash into your account.",
        "Cash will be imported in a defined date.",
        "Return to main menu.",
    };

    double depositCash;
    Date depositDate;
    int selection;

    bool flag = true;
    while(flag)
    {
        Menu depositMenu("SIMPLE BANK SIMULATION", depositMenu_options, depositMenu_explanations);
        depositMenu.display();

        selection = depositMenu.acquireSelection();

        switch(selection)
        {
            /* inquire deposit cash */
        case 0 ... 1:
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
                    break;
                else
                {
                    textAttribute(NORMAL);
                    gotoXY(22, 4);
                    cout << "Value not acceptable.";
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                }
            }
            textAttribute(CLEAR);
            gotoXY(22, 4);
            cout << "                         ";
            cursorVisible(false);
            textAttribute(NORMAL);
            break;

        case 2:
            flag = false;
            break;
        }

        if(selection == 1)
        {
            gotoXY(6, 5);
            cout << "Deposit date: ";

            int year, month, day;

            cursorVisible(true);
            while(true)
            {
                textAttribute(HIGHLIGHT);
                gotoXY(22, 5);
                cout << "               ";
                gotoXY(23, 5);

                if(cin >> year >> month >> day)
                {
                    textAttribute(NORMAL);
                    gotoXY(23, 6);

                    if(depositDate.setDate(year, month, day))
                    {
                        if(depositDate.isFuture())
                        {
                            flag = false;
                            break;
                        }
                        else
                            cout << "Not a future date.";
                    }
                    else
                        cout << "Not a valid date.";
                }
                else
                {
                    textAttribute(NORMAL);
                    gotoXY(23, 6);
                    cout << "Value not acceptable.";
                }

                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
            textAttribute(CLEAR);
            gotoXY(22, 6);
            cout << "                         ";
            cursorVisible(false);
            textAttribute(NORMAL);
        }
        else
        {
            depositDate.syncDate();
            flag = false;
        }
    }



    if(selection != 2)
    {
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
    }

#ifdef _DEBUG_ENABLED
    debug();
#endif // _DEBUG_ENABLED
}

void withdraw()
{
    /*  1: deposit
        2: withdraw
        3: check balance
        4: modify interest */

    vector<string> withdrawMenu_options =
    {
        "Immediate withdraw",
        "Advance withdraw",
        "Back",
    };
    vector<string> withdrawMenu_explanations =
    {
        "Immediately transfer cash into your account.",
        "Cash will be withdrawn in a defined date.",
        "Return to main menu.",
    };

    double withdrawCash;
    Date withdrawDate;
    int selection;

    bool flag = true;
    while(flag)
    {
        Menu withdrawMenu("SIMPLE BANK SIMULATION", withdrawMenu_options, withdrawMenu_explanations);
        withdrawMenu.display();

        selection = withdrawMenu.acquireSelection();

        switch(selection)
        {
            /* inquire withdraw cash */
        case 0 ... 1:
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
                    break;
                else
                {
                    textAttribute(NORMAL);
                    gotoXY(22, 4);
                    cout << "Value not acceptable.";
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                }
            }
            textAttribute(CLEAR);
            gotoXY(23, 4);
            cout << "                         ";
            cursorVisible(false);
            textAttribute(NORMAL);
            break;

        case 2:
            flag = false;
            break;
        }

        if(selection == 1)
        {
            gotoXY(6, 5);
            cout << "Withdraw date: ";

            int year, month, day;

            cursorVisible(true);
            while(true)
            {
                textAttribute(HIGHLIGHT);
                gotoXY(23, 5);
                cout << "               ";
                gotoXY(24, 5);

                if(cin >> year >> month >> day)
                {
                    textAttribute(NORMAL);
                    gotoXY(24, 6);

                    if(withdrawDate.setDate(year, month, day))
                    {
                        if(withdrawDate.isFuture())
                        {
                            flag = false;
                            break;
                        }
                        else
                            cout << "Not a future date.";
                    }
                    else
                        cout << "Not a valid date.";
                }
                else
                {
                    textAttribute(NORMAL);
                    gotoXY(24, 6);
                    cout << "Value not acceptable.";
                }

                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
            textAttribute(CLEAR);
            gotoXY(23, 6);
            cout << "                         ";
            cursorVisible(false);
            textAttribute(NORMAL);
        }
        else
        {
            withdrawDate.syncDate();
            flag = false;
        }
    }

    if(selection != 2)
    {
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
    }

#ifdef _DEBUG_ENABLED
    debug();
#endif // _DEBUG_ENABLED
}

void configurations()
{
    /*  1: deposit
        2: withdraw
        3: check balance
        4: modify interest */
}

#ifdef _DEBUG_ENABLED
/* debug function */
void debug()
{
    cout << endl << endl;

    /* sort thte history by date */
    sort(history.begin(), history.end(), recordCompare);

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
