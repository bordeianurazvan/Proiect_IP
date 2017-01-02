#include <iostream>
#include <Windows.h>

const int MAX_HEIGHT = 25, MAX_WIDTH = 80;

enum CellType
{
	EmptyCell,
	SnakeBody,
	PowerUp,
	Border
};

struct GameBoard 
{
	int height;
	int width;
	CellType map[MAX_HEIGHT][MAX_WIDTH];
};

void initBoard(GameBoard& gameBoard)
{
	for (int i = 0; i < gameBoard.height; ++i)
	{
		for (int j = 0; j < gameBoard.width; ++j)
		{
			if (i == 0 || i == (gameBoard.height - 1) || j == 0 || j == (gameBoard.width - 1))
				gameBoard.map[i][j] = Border;
			else
				gameBoard.map[i][j] = EmptyCell;
		}
	}
}

void setColor(WORD color)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

int height = 10, width = 20;

void displayBoard(GameBoard& gameBoard)
{
	for (int i = 0; i < gameBoard.height; ++i)
	{
		for (int j = 0; j < gameBoard.width; ++j)
		{
			switch (gameBoard.map[i][j])
			{
				case EmptyCell:
				{
					setColor(BACKGROUND_INTENSITY | BACKGROUND_GREEN | BACKGROUND_RED);
					std::cout << ' ';
				}
				break;
				case SnakeBody:
				{
					setColor(BACKGROUND_INTENSITY | BACKGROUND_GREEN | BACKGROUND_RED);
					std::cout << (char)219;
				}
				break;
				case PowerUp:
				{

				}
				break;
				case Border:
				{
					setColor(BACKGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_INTENSITY);
					std::cout << (char)219;
				}
				break;
				default:
					break;
			}
		}
		setColor(FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN);
		if (gameBoard.width < 80)
		{
			std::cout << "\n";
		}
	}
}

void singlePlayer()
{
	GameBoard gameBoard = {};
	gameBoard.width = 10;
	gameBoard.height = 10;

	char playerName[256];
	std::cout << "Type your name:\n";
	std::cin >> playerName;
	std::cout << "The typed name is: " << playerName << "\nGood luck!";
	Sleep(3000);
	system("cls");

	initBoard(gameBoard);

	while (1)
	{
		system("cls");
		displayBoard(gameBoard);
		Sleep(50);
	}
}

void playerVsAi()
{
	std::cout << "playerVsAi()\n";
}

void displayHelp()
{
	std::cout << "displayHelp()\n";
}

void displayHighscore()
{
	std::cout << "displayHighscore()\n";
}


void menu()
{

	system("cls");
	std::cout << "Joc snake!\n"
		"1. SinglePlayer\n"
		"2. PlayerVsAi\n"
		"3. Help\n"
		"4. Highscores\n"
		"5. Exit\n";
	if (GetAsyncKeyState('1'))
	{
		singlePlayer();
	}
	else if (GetAsyncKeyState('2'))
	{
		playerVsAi();
	}
	else if (GetAsyncKeyState('3'))
	{
		displayHelp();
	}
	else if (GetAsyncKeyState('4'))
	{
		displayHighscore();
	}
	else if (GetAsyncKeyState('5'))
	{
		exit(0);
	}
}

int main()
{
	while (true)
	{
		menu();
		Sleep(50);
	}
}