#include <iostream>
#include <string>
#include <random>
#include <windows.h>
using namespace std;
char game_board[3][3];
bool check[9];
HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
void SetColor(int color)
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}
void Show_Board(int player_select, int select, int handout)
{
    if (player_select != 0)
    {
        int row = (player_select - 1) / 3;
        int col = (player_select - 1) % 3;
        if (select == 1)
        {
            if (handout == 1)
            {
                game_board[row][col] = 'X';
            }
            if (handout == 2)
            {
                game_board[row][col] = 'O';
            }
        }
        if (select == 2)
        {
            if (handout == 1)
            {
                game_board[row][col] = 'O';
            }
            if (handout == 2)
            {
                game_board[row][col] = 'X';
            }
        }
    }
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (game_board[i][j] == 'X')
            {
                SetColor(9);
            }
            else if (game_board[i][j] == 'O')
            {
                SetColor(12);
            }
            else
            {
                SetColor(7);
            }
            cout << game_board[i][j];
            if (j < 2)
            {
                SetColor(2);
                cout << " | ";
            }
        }
        cout << endl;
        if (i < 2)
        {
            SetColor(10);
            cout << "---------" << endl;
        }
    }
    SetColor(7);
}
char Check_WD()
{
    for (int i = 0; i < 3; i++)
    {
        if (game_board[i][0] != ' ' &&
            game_board[i][0] == game_board[i][1] &&
            game_board[i][1] == game_board[i][2])
        {
            return game_board[i][0];
        }
    }
    for (int j = 0; j < 3; j++)
    {
        if (game_board[0][j] != ' ' &&
            game_board[0][j] == game_board[1][j] &&
            game_board[1][j] == game_board[2][j])
        {
            return game_board[0][j];
        }
    }
    if (game_board[0][0] != ' ' &&
        game_board[0][0] == game_board[1][1] &&
        game_board[1][1] == game_board[2][2])
    {
        return game_board[0][0];
    }
    if (game_board[0][2] != ' ' &&
        game_board[0][2] == game_board[1][1] &&
        game_board[1][1] == game_board[2][0])
    {
        return game_board[0][2];
    }
    bool full = true;
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (game_board[i][j] == ' ')
            {
                full = false;
                break;
            }
        }
    }
    if (full)
    {
        return 'D';
    }
    return 'N';
}
void Reset_Game(char board[3][3], bool check[9], int &player_select, int &handout)
{
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            board[i][j] = ' ';
        }
    }
    for (int i = 0; i < 9; i++)
    {
        check[i] = false;
    }
    player_select = 0;
    handout = 1;
}
int main()
{
    SetColor(11);
    cout << "Welcome to Tic Tac Toe game, player !!!" << endl;
    int key;
    int i;
    int player_select = 0;
    for (i = 0; i < 9; i++)
    {
        check[i] = false;
    }
    for (i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            game_board[i][j] = ' ';
        }
    }
    string player_name;
    string another_player_name;
    int select;
    int handout = 1;
    cout << "Please enter your name before starting the game : ";
    getline(cin, player_name);
    cout << endl;
    cout << "I'm so glad you chose my game, sir/madam " << player_name << endl;
    cout << "To start the game, please pay attention to the game instructions and start the game. " << endl;
    while (true)
    {
        SetColor(10);
        cout << "========== Guidance ==========" << endl;
        cout << "Player vs Player (1)" << endl;
        cout << "Player vs Computer (2)" << endl;
        cout << "Exit (0)" << endl;
        cout << "==============================" << endl;
        cout << "Please enter the desired number : ";
        cin >> key;
        cout << endl;
        switch (key)
        {
        case 0:
            SetColor(3);
            cout << "========== Exit ==========" << endl;
            cout << "Thank you for playing Tic Tac Toe!" << endl;
            if (player_name.empty())
            {
                cout << "We hope to see you again!" << endl;
            }
            else
            {
                cout << "We hope to see you again, dear " << player_name << "!" << endl;
            }
            cout << "Goodbye and have a wonderful day!" << endl;
            cout << "==========================" << endl;
            return 0;

        case 1:
            SetColor(14);
            cout << "========== Player vs Player ==========" << endl;
            cout << "Please introduce yourself to the next player : ";
            cin.ignore();
            getline(cin, another_player_name);
            cout << "I'm so glad you chose my game, sir/madam " << another_player_name << endl;
            cout << "I ask the first player to choose between X and O " << endl;
            cout << "1. X" << endl;
            cout << "2. O" << endl;
            cout << "Select (Please do not enter outside the options) : ";
            cin >> select;
            cout << endl;
            if (select == 1)
            {
                cout << "Thus, " << player_name << " plays the role of X and " << another_player_name << " plays the role of O" << endl;
            }
            else if (select == 2)
            {
                cout << "Thus, " << player_name << " plays the role of O and " << another_player_name << " plays the role of X" << endl;
            }
            cout << "We start the game. The rules of the game are that you must fill three squares vertically, horizontally, or diagonally with a symbol. The winner of the game is the first person to fill three squares with their symbol. Good luck." << endl;
            while (true)
            {
                SetColor(14);
                if (player_select == 0)
                {
                    Show_Board(player_select, select, handout);
                }
                char result = Check_WD();
                if (result == 'X' || result == 'O')
                {
                    if (select == 1 && result == 'X')
                    {
                        cout << "Congratulations, player number 1 " << player_name << " you won." << endl;
                    }
                    else if (select == 1 && result == 'O')
                    {
                        cout << "Congratulations, player number 2 " << another_player_name << " you won." << endl;
                    }
                    if (select == 2 && result == 'X')
                    {
                        cout << "Congratulations, player number 2 " << another_player_name << " you won." << endl;
                    }
                    else if (select == 2 && result == 'O')
                    {
                        cout << "Congratulations, player number 1 " << player_name << " you won." << endl;
                    }
                    break;
                }
                else if (result == 'D')
                {
                    cout << "The game is a draw!" << endl;
                    break;
                }
                if (handout == 1)
                {
                    cout << "It's your turn " << player_name << " ." << endl;
                    cout << "Please select a number from box 1 to box 9 : ";
                    cin >> player_select;
                    cout << endl;
                    while (check[player_select - 1])
                    {
                        SetColor(14);
                        cerr << "This house is played. Please choose another house : ";
                        cin >> player_select;
                        cout << endl;
                    }
                    check[player_select - 1] = true;
                    Show_Board(player_select, select, handout);
                    handout = 3 - handout;
                }
                else if (handout == 2)
                {
                    cout << "It's your turn " << another_player_name << " ." << endl;
                    cout << "Please select a number from box 1 to box 9 : ";
                    cin >> player_select;
                    cout << endl;
                    while (check[player_select - 1])
                    {
                        SetColor(14);
                        cerr << "This house is played. Please choose another house : ";
                        cin >> player_select;
                        cout << endl;
                    }
                    check[player_select - 1] = true;
                    Show_Board(player_select, select, handout);
                    handout = 3 - handout;
                }
            }
            Reset_Game(game_board, check, player_select, handout);
            SetColor(14);
            cout << "======================================" << endl;
            break;

        case 2:
            SetColor(13);
            cout << "========== Player vs Computer ==========" << endl;
            cout << "I ask the real player to choose between X and O " << endl;
            cout << "1. X" << endl;
            cout << "2. O" << endl;
            cout << "Select (Please do not enter outside the options) : ";
            cin >> select;
            cout << endl;
            if (select == 1)
            {
                cout << "Thus, " << player_name << " plays the role of X and the computer plays the role of O" << endl;
            }
            else if (select == 2)
            {
                cout << "Thus, " << player_name << " plays the role of O and the computer plays the role of X" << endl;
            }
            cout << "We start the game. The rules of the game are that you must fill three squares vertically, horizontally, or diagonally with a symbol. The winner of the game is the first person to fill three squares with their symbol. Good luck." << endl;
            while (true)
            {
                SetColor(13);
                if (player_select == 0)
                {
                    Show_Board(player_select, select, handout);
                }
                char result = Check_WD();
                if (result == 'X' || result == 'O')
                {
                    if (select == 1 && result == 'X')
                    {
                        cout << "Congratulations, player number 1 " << player_name << " you won." << endl;
                    }
                    else if (select == 1 && result == 'O')
                    {
                        cout << "Congratulations,the computer you won." << endl;
                    }
                    if (select == 2 && result == 'X')
                    {
                        cout << "Congratulations, the computer you won." << endl;
                    }
                    else if (select == 2 && result == 'O')
                    {
                        cout << "Congratulations, player number 1 " << player_name << " you won." << endl;
                    }
                    break;
                }
                else if (result == 'D')
                {
                    cout << "The game is a draw!" << endl;
                    break;
                }
                if (handout == 1)
                {
                    SetColor(13);
                    cout << "It's your turn " << player_name << " ." << endl;
                    cout << "Please select a number from box 1 to box 9 : ";
                    cin >> player_select;
                    cout << endl;
                    while (check[player_select - 1])
                    {
                        SetColor(13);
                        cerr << "This house is played. Please choose another house : ";
                        cin >> player_select;
                        cout << endl;
                    }
                    check[player_select - 1] = true;
                    Show_Board(player_select, select, handout);
                    handout = 3 - handout;
                }
                else if (handout == 2)
                {
                    cout << "It's the computer turn ." << endl;
                    random_device rd;
                    mt19937 gen(rd());
                    uniform_int_distribution<> dist(1, 9);
                    player_select = dist(gen);
                    while (check[player_select - 1])
                    {
                        SetColor(13);
                        player_select = dist(gen);
                    }
                    cout << "The computer selected box number " << player_select << "." << endl;
                    check[player_select - 1] = true;
                    Show_Board(player_select, select, handout);
                    handout = 3 - handout;
                }
            }
            Reset_Game(game_board, check, player_select, handout);
            SetColor(13);
            cout << "========================================" << endl;
            break;

        default:
        SetColor(15);
            cout << "You chose the wrong number. Please note that the number you choose must be between 0 and 2." << endl;
            break;
        }
    }
}