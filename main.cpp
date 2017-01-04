#include <iostream>
#include <Windows.h>

const int MAX_HEIGHT = 20, MAX_WIDTH = 75;
const int MAX_VECT_SIZE = MAX_HEIGHT * MAX_WIDTH;

enum CellType
{
	EmptyCell,
	SnakeBody,
	PowerUp,
	Border,
	Food
};

enum Direction
{
	Up,
	Left,
	Down,
	Right
};

struct BoardCell
{
	int line, column;
	CellType cellType;
};

struct CellPosition
{
	int line, column;
};

int directionLine[4]   = {-1,  0, 1, 0};
int directionColumn[4] = { 0, -1, 0, 1};

struct GameBoard
{
	int height;
	int width;
	BoardCell boardCells[MAX_VECT_SIZE];

	int filledCellsSize;
	int& firstEmptyCellIndex = filledCellsSize;
	
	Direction direction;
	CellPosition snakeBody[MAX_VECT_SIZE];
	int snakeBodySize;
	int snakeTailIndex;
};

bool getCell(GameBoard& gameBoard,int index, BoardCell& boardCell)
{
	if (index < 0 || index > gameBoard.height * gameBoard.width)
		return false;
	boardCell = gameBoard.boardCells[index];
	return true;
}

bool getCell(GameBoard& gameBoard, int line, int column, BoardCell& boardCell)
{
	if (line < 0 || line > gameBoard.height)
		return false;
	if (column < 0 || column > gameBoard.width)
		return false;

	bool found = false;
	int foundIndex;
	for (int i = gameBoard.firstEmptyCellIndex; i < gameBoard.height *gameBoard.width; ++i)
	{
		if (gameBoard.boardCells[i].line == line && gameBoard.boardCells[i].column == column)
		{
			found = true;
			foundIndex = i;
			break;
		}
	}
	if (!found)
	{
		return false;
	}
	return getCell(gameBoard, foundIndex, boardCell);
}


bool fillCell(GameBoard& gameBoard, int index, CellType cellType) // transforma o celula din empty in filled
{
	if (index < 0 || index > gameBoard.height * gameBoard.width)
		return false;
	if (gameBoard.filledCellsSize == gameBoard.height * gameBoard.width)
		return false;
	if (index < gameBoard.firstEmptyCellIndex)
		return false;
	if(cellType == CellType::EmptyCell)
		return false;

	if (index == gameBoard.firstEmptyCellIndex)
	{
		// no swap required
		gameBoard.boardCells[gameBoard.firstEmptyCellIndex++].cellType = cellType;
	}
	else
	{
		gameBoard.boardCells[index].cellType = cellType;
		std::swap(gameBoard.boardCells[index], gameBoard.boardCells[gameBoard.firstEmptyCellIndex]);
		++gameBoard.firstEmptyCellIndex;
	}
	return true;
}

bool fillCell(GameBoard& gameBoard, int line, int column, CellType cellType) // transforma o celula din empty in filled stiind linia si coloana si ce vrem sa devina
{
	if (line < 0 || line > gameBoard.height)
		return false;
	if (column < 0 || column > gameBoard.width)
		return false;
	if (cellType == CellType::EmptyCell)
		return false;
	if (gameBoard.filledCellsSize == gameBoard.height * gameBoard.width)
		return false;

	bool found = false;
	int foundIndex;
	for (int i = gameBoard.firstEmptyCellIndex; i < gameBoard.height *gameBoard.width; ++i)
	{
		if (gameBoard.boardCells[i].line == line && gameBoard.boardCells[i].column == column)
		{
			found = true;
			foundIndex = i;
			break;
		}
	}
	if (!found)
	{
		return false;
	}
	return fillCell(gameBoard, foundIndex, cellType);
}

bool clearCell(GameBoard& gameBoard, int index) // transforma o celula din filled in empty
{
	if (index < 0 || index > gameBoard.height * gameBoard.width)
		return false;
	if (gameBoard.filledCellsSize == gameBoard.height * gameBoard.width)
		return false;
	if (index >= gameBoard.firstEmptyCellIndex)
		return false;
	if (gameBoard.boardCells[index].cellType == CellType::EmptyCell)
		return false;

	if (index == gameBoard.firstEmptyCellIndex - 1)
	{
		// no swap required
		gameBoard.boardCells[gameBoard.firstEmptyCellIndex--].cellType = EmptyCell;
	}
	else
	{
		gameBoard.boardCells[index].cellType = EmptyCell;
		std::swap(gameBoard.boardCells[index], gameBoard.boardCells[gameBoard.firstEmptyCellIndex - 1]);
		--gameBoard.filledCellsSize;
	}
	return true;
}

bool clearCell(GameBoard& gameBoard, int line, int column) // transforma o celula din filled in empty
{
	if (line < 0 || line > gameBoard.height)
		return false;
	if (column < 0 || column > gameBoard.width)
		return false;
	bool found = false;
	int foundIndex;
	for (int i = 0; i < gameBoard.filledCellsSize; ++i)
	{
		if (gameBoard.boardCells[i].line == line && gameBoard.boardCells[i].column == column)
		{
			found = true;
			foundIndex = i;
			break;
		}
	}
	if (!found)
	{
		return false;
	}
	return clearCell(gameBoard, foundIndex);
}

int getSnakeHeadIndex(GameBoard& gameBoard)
{
	return (gameBoard.snakeTailIndex + gameBoard.snakeBodySize - 1) % MAX_VECT_SIZE;
}

bool moveSnake(GameBoard& gameBoard)
{
	int snakeHead = getSnakeHeadIndex(gameBoard);
	int newPosLine = gameBoard.snakeBody[snakeHead].line + directionLine[gameBoard.direction];
	int newPosColumn = gameBoard.snakeBody[snakeHead].column + directionColumn[gameBoard.direction];

	gameBoard.snakeBody[(snakeHead + 1) % MAX_VECT_SIZE].line = newPosLine;
	gameBoard.snakeBody[(snakeHead + 1) % MAX_VECT_SIZE].column = newPosColumn;

	BoardCell boardCell;
	if (!getCell(gameBoard, newPosLine, newPosColumn, boardCell))
	{
		return false;
	}

	if (boardCell.cellType == CellType::Food)
	{
		CellPosition& tailCell = gameBoard.snakeBody[gameBoard.snakeTailIndex];
		++gameBoard.snakeTailIndex;
		++gameBoard.snakeBodySize;
		fillCell(gameBoard, newPosLine, newPosColumn, SnakeBody);
	}
	else
	{
		CellPosition& tailCell = gameBoard.snakeBody[gameBoard.snakeTailIndex];
		clearCell(gameBoard, tailCell.line, tailCell.column);
		++gameBoard.snakeTailIndex;
		fillCell (gameBoard, newPosLine, newPosColumn, SnakeBody);
	}
	return true;
}

void initBoard(GameBoard& gameBoard)
{
	int size = 0;
	for (int i = 0; i < gameBoard.height; ++i)
	{
		for (int j = 0; j < gameBoard.width; ++j)
		{
			gameBoard.boardCells[size].line = i;
			gameBoard.boardCells[size].column = j;
			gameBoard.boardCells[size].cellType = EmptyCell;
			size++;	
		}
	}
	for (int i = 0; i < gameBoard.height; ++i)
	{
		for (int j = 0; j < gameBoard.width; ++j)
		{
			if (i == 0 || i == (gameBoard.height - 1) || j == 0 || j == (gameBoard.width - 1))
			{
				fillCell(gameBoard, i, j, CellType::Border);
			}
		}
	}
	gameBoard.direction = Up;
	fillCell(gameBoard, 9, 10, CellType::SnakeBody);
	fillCell(gameBoard, 10, 10, CellType::SnakeBody);
	fillCell(gameBoard, 11, 10, CellType::SnakeBody);

	gameBoard.snakeBody[0].line = 11;
	gameBoard.snakeBody[1].line = 10;
	gameBoard.snakeBody[2].line = 9;

	gameBoard.snakeBody[0].column = 10;
	gameBoard.snakeBody[1].column = 10;
	gameBoard.snakeBody[2].column = 10;

	gameBoard.snakeBodySize = 3;
	gameBoard.snakeTailIndex = 0;
}

void setColor(WORD color)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}


void displayBoard(GameBoard& gameBoard)
{
	CellType map[MAX_HEIGHT][MAX_WIDTH] = {};
	for (int i = 0; i < gameBoard.filledCellsSize; ++i)
	{
		BoardCell& currentCell = gameBoard.boardCells[i];
		map[currentCell.line][currentCell.column] = currentCell.cellType;
	}

	for (int i = 0; i < gameBoard.height; ++i)
	{
		for (int j = 0; j < gameBoard.width; ++j)
		{
			switch (map[i][j])
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
				std::cout << (char)178;
			}
			break;
			case PowerUp:
			{
				setColor(BACKGROUND_INTENSITY | BACKGROUND_GREEN | BACKGROUND_RED | FOREGROUND_GREEN);
				std::cout << (char)63;
			}
			break;
			case Border:
			{
				setColor(BACKGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_INTENSITY);
				std::cout << (char)219;
			}
			break;
			case Food:
			{
				setColor(BACKGROUND_INTENSITY | BACKGROUND_GREEN | BACKGROUND_RED | FOREGROUND_GREEN);
				std::cout << (char)43;
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

void handleUserInput(GameBoard& gameBoard)
{
	if (GetAsyncKeyState(VK_UP))
	{
		gameBoard.direction = Up;
	}
	else if (GetAsyncKeyState(VK_LEFT))
	{
		gameBoard.direction = Left;
	}
	else if (GetAsyncKeyState(VK_DOWN))
	{
		gameBoard.direction = Down;
	}
	else if (GetAsyncKeyState(VK_RIGHT))
	{
		gameBoard.direction = Right;
	}
}

void singlePlayer()
{
	GameBoard gameBoard = {};
	gameBoard.width = 75;
	gameBoard.height = 20;

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
		handleUserInput(gameBoard);
		if (!moveSnake(gameBoard))
		{
			system("cls");
			std::cout << "Game over!";
			Sleep(3000);
			return;
		}
		Sleep(100);
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