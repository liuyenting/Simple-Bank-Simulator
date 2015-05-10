#include <string>
#include <iostream>
#include <conio.h>
#include <fstream>
#include <vector>

#define DEFAULT_RATE 1.1

using namespace std;

/* customize date class */
class Date
{
public:
    Date()
    {
        for(int i=0; i<3; i++)
            tmpDate.push_back(0);
        date = tmpDate;
    }
    void initialize()
    {
        cout << " * Please enter initial date in the form of \"YYYY MM DD\"" <<endl;
        cout << "    -> ";

        acquire();

        cout << endl;
    }
    string toString()
    {
        /* string result = "";
        stringstream ss;
        for(int i=0; i<3; i++)
        {
            ss << date[i];
            result += ss.str();
        }
        return result; */
    }
    float interval() /* target interval between initial date */
    {
        float inv;

        return inv;
    }
private:
    int millisecond; /* function for realtime synchronize */
    /* date in form of 0:year 1:month 2:day */
    vector< int > date;
    vector< int > tmpDate;
    void acquire()
    {
        while(!(cin >> tmpDate[0] >> tmpDate[1] >> tmpDate[2]))
        {
            cout << " * Invalid character entered." << endl << endl;
            cout << " Retry in \"YYYY MM DD\" form -> ";
            cin.clear();
            cin.ignore(100, '\n');
        }

        if(isValid()) date = tmpDate;
        else
        {
            cout << " * An invalid date." << endl << endl;
            cout << " Retry correct date -> ";
            acquire();
        }
    }
    bool isLeapYear(int year)
    {
        if((year % 4 == 0) && !(year % 100 == 0) || (year % 400 == 0))
            return true;
        else return false;
    }
    bool isValid()
    {
        bool result = true;
        if(tmpDate[0] < 0) result = false;
        else
        {
            if(tmpDate[1] > 12 || tmpDate[1] < 1 || tmpDate[2] < 0) result = false;
            else
            {
                switch(tmpDate[1])
                {
                case 1:
                case 3:
                case 5:
                case 7:
                case 8:
                case 10:
                case 12:
                    if(tmpDate[2] > 31) result = false;
                    break;
                case 2:
                    if(isLeapYear(tmpDate[0]))
                    {
                        if(tmpDate[2] > 29) result = false;
                    }
                    else
                    {
                        if(tmpDate[2] > 28) result = false;
                    }
                    break;
                default:
                    if(tmpDate[2] > 30) result = false;
                    break;
                }
            }
        }
        return result;
    }
};

/* show user interface */
class Menu
{
public:
    void display()
    {
        cout << " <<SIMPLE BANK SIMULATION>> " << endl;
        cout << "  1. Print balance" << endl;
        cout << "  2. Deposit" << endl;
        cout << "  3. Withdraw" << endl;
        cout << "  4. Modify interest rate" << endl;
        cout << "  5. Quit" << endl << endl;

        cout << " Choice(1-5) -> ";
    }
    int acquire()
    {
        while(!(cin >> sel))
        {
            cout << "  * Not an valid option." << endl << endl;
            cout << " Retry(1-5) -> ";
            cin.clear();
            cin.ignore(100, '\n');
        }

        if(validChoice(sel)) return sel;
        else
        {
            cout << "  * An out-of-range option." << endl << endl;
            cout << " Retry(1-5) -> ";
            return acquire();
        }
    }
private:
    int sel;
    bool validChoice(int input)
    {
        bool result = true;
        if(input > 5 || input < 1) result = false;
        return result;
    }
};

/* money history */
class Record
{
public:
    Record()
    {
        ofstream file(name);
        if(fileValid())
        {

        }
    }
    void deposit(Date date, float money);
    void withdraw(Date date, float money);
    ~Record();
private:
    string name = "temp.SBH";
    bool fileValid()
    {
        bool result = false;
        ifstream file(name);
        if(file) result = true;
        file.close();
        return result;
    }
};

/* menu subfunctions */
void initial();
void printBalance();
void deposit();
void withdraw();
void interestRate();

/* global parameters */
Menu selections;
Date initialDate;
float rate;
float balance;

/* define history form, dynamic */
typedef struct
{
    Date date;
    int action;
    double balance;
    double interest;
    float rate;
}HistoryForm;
vector<HistoryForm> dynamicHistory;

int main()
{
    initial();

    bool flag = true;
    while(flag)
    {
        selections.display();
        switch(selections.acquire())
        {
        case 1:
            printBalance();
            break;
        case 2:
            deposit();
            break;
        case 3:
            withdraw();
            break;
        case 4:
            interestRate();
            break;
        case 5:
            flag = false;
            break;
        }
    }

    cout << endl << " <<THANKS FOR USING>>" << endl;
    return 0;
}

void initial()
{
    initialDate.initialize();

    rate = DEFAULT_RATE;
    balance = 0;
}

void printBalance()
{
}

void deposit()
{
}

void withdraw()
{
}

void interestRate()
{
}
