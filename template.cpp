
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

    Menu depositMenu("SIMPLE BANK SIMULATION", depositMenu_options, depositMenu_explanations);
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
        cout << "-----------------------------------";
        gotoXY(6, 6);
        cout << setw(20) << right << "Available balance =";
        gotoXY(28, 6);
        cout << balance + depositCash;

        selection = depositMenu.acquireSelection();

        switch(selection)
        {
        case 0:
            textAttribute(NORMAL);
            gotoXY(7, 8);
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

            flag = false;
            break;

        case 1:
            deposit();
        case 2:
            flag = false;
            break;
        }
    }

#ifdef _DEBUG_ENABLED
    debug();
#endif // _DEBUG_ENABLED
}
