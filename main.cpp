#include <iostream>
#include <cstdlib>
#include <Windows.h>
#include <fstream>
#include <string.h>
#include <ctime>
#include <queue>

#pragma warning(disable:4996)

#define CONSOLE_WIDTH 120
#define CONSOLE_HEIGHT 30

const int MAX_HEIGHT = 20, MAX_WIDTH = 75;
const int MAX_VECT_SIZE = MAX_HEIGHT * MAX_WIDTH;

int directionLine[4] = { -1,  0, 1, 0 };
int directionColumn[4] = { 0, -1, 0, 1 };

const int snakeMoveDelayHorizontal = 75;
const int snakeMoveDelayVertical = int(snakeMoveDelayHorizontal * 1.25);

class DoubleBufferedConsole
{
	HANDLE hStdout, hNewScreenBuffer, hNewScreenBuffer2;
	SMALL_RECT srctWriteRect;
	CHAR_INFO buffer[CONSOLE_WIDTH * CONSOLE_HEIGHT];
	COORD coordBufSize;
	COORD coordBufCoord;
	int cursorIndex = 0;

	WORD attribute = FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN;
	static int currentBuffer;
public:
	DoubleBufferedConsole()
	{
		hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

		coordBufSize.X = CONSOLE_WIDTH;
		coordBufSize.Y = CONSOLE_HEIGHT;

		coordBufCoord.X = 0;
		coordBufCoord.Y = 0;

		srctWriteRect.Left = srctWriteRect.Top = 0;
		srctWriteRect.Right = CONSOLE_WIDTH - 1;
		srctWriteRect.Bottom = CONSOLE_HEIGHT - 1;

		hNewScreenBuffer = CreateConsoleScreenBuffer(
			GENERIC_WRITE,
			0,
			NULL,                    // default security attributes 
			CONSOLE_TEXTMODE_BUFFER, // must be TEXTMODE 
			NULL);                   // reserved; must be NULL 
		hNewScreenBuffer2 = CreateConsoleScreenBuffer(
			GENERIC_WRITE,
			0,
			NULL,                    // default security attributes 
			CONSOLE_TEXTMODE_BUFFER, // must be TEXTMODE 
			NULL);                   // reserved; must be NULL 

		SetConsoleScreenBufferSize(hNewScreenBuffer, coordBufSize);
		SetConsoleWindowInfo(hNewScreenBuffer, TRUE, &srctWriteRect);
		SetConsoleScreenBufferSize(hNewScreenBuffer2, coordBufSize);
		SetConsoleWindowInfo(hNewScreenBuffer2, TRUE, &srctWriteRect);
	}
	void clear()
	{
		cursorIndex = 0;
		for (int i = 0; i < CONSOLE_HEIGHT * CONSOLE_WIDTH; ++i)
		{
			buffer[i].Char.AsciiChar = ' ';
			buffer[i].Attributes = attribute;
		}
	}

	void setColor(WORD color)
	{
		attribute = color;
	}

	void display()
	{
		HANDLE currentScreenBufferHandle = NULL;
		if (currentBuffer % 2)
		{
			currentScreenBufferHandle = hNewScreenBuffer;
		}
		else
		{
			currentScreenBufferHandle = hNewScreenBuffer2;
		}
		BOOL fSuccess = WriteConsoleOutput(
			currentScreenBufferHandle,
			buffer,  
			coordBufSize,  
			coordBufCoord,
			&srctWriteRect);

		if (!fSuccess)
			return;

		if (!SetConsoleActiveScreenBuffer(currentScreenBufferHandle)) 
		{
			printf("SetConsoleActiveScreenBuffer failed - (%d)\n", GetLastError());
			return;
		}
		++currentBuffer;
	}

	friend DoubleBufferedConsole& operator << (DoubleBufferedConsole& doubleBufferedConsole, char chr)
	{
		if (chr == '\n')
		{
			while ((doubleBufferedConsole.cursorIndex % CONSOLE_WIDTH) != 0)
			{
				doubleBufferedConsole.buffer[doubleBufferedConsole.cursorIndex].Char.AsciiChar = ' ';
				doubleBufferedConsole.buffer[doubleBufferedConsole.cursorIndex].Attributes = doubleBufferedConsole.attribute;
				doubleBufferedConsole.cursorIndex++;
			}
			return doubleBufferedConsole;
		}

		if (doubleBufferedConsole.cursorIndex + 1 >= CONSOLE_WIDTH * CONSOLE_HEIGHT)
		{
			return doubleBufferedConsole;
		}
		doubleBufferedConsole.buffer[doubleBufferedConsole.cursorIndex].Char.AsciiChar = chr;
		doubleBufferedConsole.buffer[doubleBufferedConsole.cursorIndex].Attributes = doubleBufferedConsole.attribute;
		doubleBufferedConsole.cursorIndex++;
		return doubleBufferedConsole;
	}
	friend DoubleBufferedConsole& operator << (DoubleBufferedConsole& doubleBufferedConsole, const char* str)
	{
		size_t strLen = strlen(str);
		for (size_t i = 0; i < strLen; ++i)
		{
			if (doubleBufferedConsole.cursorIndex + 1 >= CONSOLE_WIDTH * CONSOLE_HEIGHT)
			{
				return doubleBufferedConsole;
			}
			if (str[i] == '\n')
			{
				while ((doubleBufferedConsole.cursorIndex % CONSOLE_WIDTH) != 0)
				{
					doubleBufferedConsole.buffer[doubleBufferedConsole.cursorIndex].Char.AsciiChar = ' ';
					doubleBufferedConsole.buffer[doubleBufferedConsole.cursorIndex].Attributes = doubleBufferedConsole.attribute;
					doubleBufferedConsole.cursorIndex++;
				}
			}
			else
			{
				doubleBufferedConsole.buffer[doubleBufferedConsole.cursorIndex].Char.AsciiChar = str[i];
				doubleBufferedConsole.buffer[doubleBufferedConsole.cursorIndex].Attributes = doubleBufferedConsole.attribute;
				doubleBufferedConsole.cursorIndex++;
			}
		}
		return doubleBufferedConsole;
	}

	void switchToSingleBuffer()
	{
		SetConsoleActiveScreenBuffer(GetStdHandle(STD_OUTPUT_HANDLE));
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN);
	}
};

int DoubleBufferedConsole::currentBuffer = 0;

DoubleBufferedConsole dbc;



bool isKeyPressed(int key)
{
	bool result = !!(GetAsyncKeyState(key) & 0x8000); // !! suppress compiler warning C4800
	FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
	return result; 
}


struct Timer
{
	time_t lastReadyTime = 0;
	size_t readyInterval = 0;
};

void changeReadyTime(Timer& timer, size_t readyInterval)
{
	timer.readyInterval = readyInterval;
}

void setTimer(Timer& timer, size_t readyInterval)
{
	timer.readyInterval = readyInterval;
	timer.lastReadyTime = clock();
	std::ofstream fout("debug.txt", std::ios::app);
	fout << "Timer set to " << timer.lastReadyTime << " and at interval " << readyInterval << "\n";
}


bool isTimerReady(Timer& timer)
{
	std::ofstream fout("debug.txt", std::ios::app);
	fout << "Check for timer read: clock() " << clock()  << "lastReadyTime:" << timer.lastReadyTime << " for interval :" << timer.readyInterval << "\n";
	if (clock() >= timer.lastReadyTime + timer.readyInterval)
	{
		timer.lastReadyTime = clock();
		fout << "ready";
		return true;
	}
	fout << "not ready";
	return false;
}

enum CellType
{
	EmptyCell,
	SnakeBody,
	SnakeBodyAI,
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



struct GameContext
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

	bool spawnNewFood;
	CellPosition currentFoodPositon;
	Timer timer;
	bool moveMadeForCycle = false;
	
	bool playerWon;

	Direction directionAI;
	CellPosition snakeBodyAI[MAX_VECT_SIZE];
	int snakeBodySizeAI;
	int snakeTailIndexAI;
};

bool getCell(GameContext& gameContext, int index, BoardCell& boardCell)
{
	if (index < 0 || index > gameContext.height * gameContext.width)
		return false;
	boardCell = gameContext.boardCells[index];
	return true;
}

bool getCell(GameContext& gameContext, int line, int column, BoardCell& boardCell)
{
	if (line < 0 || line > gameContext.height)
		return false;
	if (column < 0 || column > gameContext.width)
		return false;

	bool found = false;
	int foundIndex;
	for (int i = 0; i < gameContext.height *gameContext.width; ++i)
	{
		if (gameContext.boardCells[i].line == line && gameContext.boardCells[i].column == column)
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
	return getCell(gameContext, foundIndex, boardCell);
}


bool fillCell(GameContext& gameContext, int index, CellType cellType) // transforma o celula din empty in filled
{
	if (index < 0 || index > gameContext.height * gameContext.width)
		return false;
	if (gameContext.filledCellsSize == gameContext.height * gameContext.width)
		return false;
	if (index < gameContext.firstEmptyCellIndex)
		return false;
	if(cellType == CellType::EmptyCell)
		return false;

	if (index == gameContext.firstEmptyCellIndex)
	{
		// no swap required
		gameContext.boardCells[gameContext.firstEmptyCellIndex++].cellType = cellType;
	}
	else
	{
		gameContext.boardCells[index].cellType = cellType;
		std::swap(gameContext.boardCells[index], gameContext.boardCells[gameContext.firstEmptyCellIndex]);
		++gameContext.firstEmptyCellIndex;
	}
	return true;
}

bool fillCell(GameContext& gameContext, int line, int column, CellType cellType) // transforma o celula din empty in filled stiind linia si coloana si ce vrem sa devina
{
	if (line < 0 || line > gameContext.height)
		return false;
	if (column < 0 || column > gameContext.width)
		return false;
	if (cellType == CellType::EmptyCell)
		return false;
	if (gameContext.filledCellsSize == gameContext.height * gameContext.width)
		return false;

	bool found = false;
	int foundIndex;
	for (int i = gameContext.firstEmptyCellIndex; i < gameContext.height *gameContext.width; ++i)
	{
		if (gameContext.boardCells[i].line == line && gameContext.boardCells[i].column == column)
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
	return fillCell(gameContext, foundIndex, cellType);
}

bool clearCell(GameContext& gameContext, int index) // transforma o celula din filled in empty
{
	if (index < 0 || index > gameContext.height * gameContext.width)
		return false;
	if (gameContext.filledCellsSize == gameContext.height * gameContext.width)
		return false;
	if (index >= gameContext.firstEmptyCellIndex)
		return false;
	if (gameContext.boardCells[index].cellType == CellType::EmptyCell)
		return false;

	if (index == gameContext.firstEmptyCellIndex - 1)
	{
		// no swap required
		gameContext.boardCells[gameContext.firstEmptyCellIndex--].cellType = EmptyCell;
	}
	else
	{
		gameContext.boardCells[index].cellType = EmptyCell;
		std::swap(gameContext.boardCells[index], gameContext.boardCells[gameContext.firstEmptyCellIndex - 1]);
		--gameContext.filledCellsSize;
	}
	return true;
}

bool updateCell(GameContext& gameContext, int line, int column, CellType cellType)
{
	if (line < 0 || line > gameContext.height)
		return false;
	if (column < 0 || column > gameContext.width)
		return false;
	if (cellType == EmptyCell)
		return false;
	bool found = false;
	int foundIndex;
	for (int i = 0; i < gameContext.firstEmptyCellIndex; ++i)
	{
		if (gameContext.boardCells[i].line == line && gameContext.boardCells[i].column == column)
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
	if (foundIndex >= gameContext.firstEmptyCellIndex)
		return false;
	gameContext.boardCells[foundIndex].cellType = cellType;
	return true;
}

bool clearCell(GameContext& gameContext, int line, int column) // transforma o celula din filled in empty
{
	if (line < 0 || line > gameContext.height)
		return false;
	if (column < 0 || column > gameContext.width)
		return false;
	bool found = false;
	int foundIndex;
	for (int i = 0; i < gameContext.filledCellsSize; ++i)
	{
		if (gameContext.boardCells[i].line == line && gameContext.boardCells[i].column == column)
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
	return clearCell(gameContext, foundIndex);
}

int getSnakeHeadIndex(GameContext& gameContext)
{
	return (gameContext.snakeTailIndex + gameContext.snakeBodySize - 1) % MAX_VECT_SIZE;
}

int getSnakeHeadIndexAI(GameContext& gameContext)
{
	return (gameContext.snakeTailIndexAI + gameContext.snakeBodySizeAI - 1) % MAX_VECT_SIZE;
}

bool moveSnake(GameContext& gameContext, int&score)
{
	int snakeHead = getSnakeHeadIndex(gameContext);
	int newPosLine = gameContext.snakeBody[snakeHead].line + directionLine[gameContext.direction];
	int newPosColumn = gameContext.snakeBody[snakeHead].column + directionColumn[gameContext.direction];

	gameContext.snakeBody[(snakeHead + 1) % MAX_VECT_SIZE].line = newPosLine;
	gameContext.snakeBody[(snakeHead + 1) % MAX_VECT_SIZE].column = newPosColumn;

	BoardCell boardCell;
	if (!getCell(gameContext, newPosLine, newPosColumn, boardCell))
	{
		return false;
	}

	if (boardCell.cellType == CellType::Border || boardCell.cellType == CellType::SnakeBody || boardCell.cellType == CellType::SnakeBodyAI)
	{
		return false;
	}

	if (boardCell.cellType == CellType::Food)
	{
		CellPosition& tailCell = gameContext.snakeBody[gameContext.snakeTailIndex];
	//	gameContext.snakeTailIndex = (gameContext.snakeTailIndex + 1) % MAX_VECT_SIZE;
		gameContext.snakeBodySize++;
		score++;
		gameContext.snakeBody[getSnakeHeadIndex(gameContext)] = { boardCell.line, boardCell.column};
		if (!updateCell(gameContext, newPosLine, newPosColumn, SnakeBody))
		{
			return false;
		}
		gameContext.spawnNewFood = true;
		
	}
	else
	{
		CellPosition& tailCell = gameContext.snakeBody[gameContext.snakeTailIndex];
		clearCell(gameContext, tailCell.line, tailCell.column);
		gameContext.snakeTailIndex = (gameContext.snakeTailIndex + 1) % MAX_VECT_SIZE;
		fillCell (gameContext, newPosLine, newPosColumn, SnakeBody);
	}
	return true;
}

#include <iomanip>

void chooseDirectionForAI(GameContext& gameContext)
{
	std::queue<CellPosition> leeQueue;
	int markedCells[MAX_HEIGHT][MAX_WIDTH] = {};
	int index = getSnakeHeadIndexAI(gameContext);

	int headPosLine = gameContext.snakeBodyAI[index].line;
	int headPosColumn = gameContext.snakeBodyAI[index].column;
	
	markedCells[headPosLine][headPosColumn] = 1;

	leeQueue.push(gameContext.snakeBodyAI[index]);


	while (!leeQueue.empty())
	{
		CellPosition cell = leeQueue.front();
		leeQueue.pop();
		bool finish = false;
		for (int i = 0; i < 4; i++)
		{
			if ((cell.line + directionLine[i] < 0) || (cell.line + directionLine[i] >= MAX_HEIGHT))
				continue;
			if ((cell.column + directionColumn[i] < 0) || (cell.column + directionColumn[i] >= MAX_WIDTH))
				continue;
			if (markedCells[cell.line + directionLine[i]][cell.column + directionColumn[i]] != 0)
				continue;
			if (cell.line	+ directionLine[i]		== gameContext.currentFoodPositon.line && 
				cell.column	+ directionColumn[i]	== gameContext.currentFoodPositon.column)
			{
				markedCells[gameContext.currentFoodPositon.line][gameContext.currentFoodPositon.column] 
					= markedCells[cell.line][cell.column] + 1;
				finish = true;
				break;
			}
			BoardCell newCell;
			getCell(gameContext, cell.line + directionLine[i], cell.column + directionColumn[i], newCell);
			if (newCell.cellType == CellType::EmptyCell)
			{
				leeQueue.push(CellPosition{ newCell.line, newCell.column });
				markedCells[newCell.line][newCell.column] = markedCells[cell.line][cell.column] + 1;
			}
		}
		if (finish)
		{
			break;
		}
	}
	std::ofstream fout("dbg.txt");
	for (int i = 0; i < MAX_HEIGHT; i++)
	{
		for (int j = 0; j < MAX_WIDTH; j++)
		{
			fout << std::setw(2) << markedCells[i][j] << " ";
		}

		fout << "\n";
	}

	fout.close();

	if (markedCells[gameContext.currentFoodPositon.line][gameContext.currentFoodPositon.column])
	{
		int i = gameContext.currentFoodPositon.line;
		int j = gameContext.currentFoodPositon.column;
		Direction lastDirection = gameContext.directionAI;

		bool found = false;

		while (i != headPosLine || j != headPosColumn)
		{
			int d;
			for (d = 0; d < 4; ++d)
			{
				if ((i + directionLine[d] < 0) || (i + directionLine[d] >= MAX_HEIGHT))
					continue;
				if ((j + directionColumn[d] < 0) || (j + directionColumn[d] >= MAX_WIDTH))
					continue;
				if (markedCells[i + directionLine[d]][j + directionColumn[d]] == markedCells[i][j] - 1)
				{
					i += directionLine[d];
					j += directionColumn[d];
					break;
				}
			}
			switch (d)
			{
			case 0:
				lastDirection = Direction::Up;
				break;
			case 1:
				lastDirection = Direction::Left;
				break;
			case 2:
				lastDirection = Direction::Down;
				break;
			case 3:
				lastDirection = Direction::Right;
				break;
			default:
				break;
			}
			if (markedCells[i][j] == 1)
			{
				found = true;
			}

		}
		if(lastDirection == Direction::Left)
			gameContext.directionAI = Direction::Right;
		else if (lastDirection == Direction::Right)
			gameContext.directionAI = Direction::Left;
		else if (lastDirection == Direction::Up)
			gameContext.directionAI = Direction::Down;
		else if (lastDirection == Direction::Down)
			gameContext.directionAI = Direction::Up;
	}
}


bool moveSnakeAI(GameContext& gameContext)
{
	chooseDirectionForAI(gameContext);

	int snakeHead = getSnakeHeadIndexAI(gameContext);
	int newPosLine = gameContext.snakeBodyAI[snakeHead].line + directionLine[gameContext.directionAI];
	int newPosColumn = gameContext.snakeBodyAI[snakeHead].column + directionColumn[gameContext.directionAI];
	

	gameContext.snakeBodyAI[(snakeHead + 1) % MAX_VECT_SIZE].line = newPosLine;
	gameContext.snakeBodyAI[(snakeHead + 1) % MAX_VECT_SIZE].column = newPosColumn;

	BoardCell boardCell;
	if (!getCell(gameContext, newPosLine, newPosColumn, boardCell))
	{
		return false;
	}

	if (boardCell.cellType == CellType::Border || boardCell.cellType == CellType::SnakeBodyAI || boardCell.cellType == CellType::SnakeBody)
	{
		return false;
	}

	if (boardCell.cellType == CellType::Food)
	{
		CellPosition& tailCell = gameContext.snakeBodyAI[gameContext.snakeTailIndexAI];
		//	gameContext.snakeTailIndex = (gameContext.snakeTailIndex + 1) % MAX_VECT_SIZE;
		gameContext.snakeBodySizeAI++;
		gameContext.snakeBodyAI[getSnakeHeadIndexAI(gameContext)] = { boardCell.line, boardCell.column };
		if (!updateCell(gameContext, newPosLine, newPosColumn, SnakeBodyAI))
		{
			return false;
		}
		gameContext.spawnNewFood = true;
	}
	else
	{
		CellPosition& tailCell = gameContext.snakeBodyAI[gameContext.snakeTailIndexAI];
		clearCell(gameContext, tailCell.line, tailCell.column);
		gameContext.snakeTailIndexAI = (gameContext.snakeTailIndexAI + 1) % MAX_VECT_SIZE;
		fillCell(gameContext, newPosLine, newPosColumn, SnakeBodyAI);
	}
	return true;
}

bool spawnNewFood(GameContext& gameContext)
{
	if (gameContext.filledCellsSize == MAX_VECT_SIZE)
	{
		return false;
	}
	int intervalSize = MAX_VECT_SIZE - gameContext.firstEmptyCellIndex;
	int randomEmptyCellIndex = rand() % intervalSize;
	gameContext.spawnNewFood = false;
	
	BoardCell foodCell;
	getCell(gameContext, gameContext.firstEmptyCellIndex + randomEmptyCellIndex, foodCell);
	gameContext.currentFoodPositon.line = foodCell.line;
	gameContext.currentFoodPositon.column = foodCell.column;
	return fillCell(gameContext, gameContext.firstEmptyCellIndex + randomEmptyCellIndex, CellType::Food);
}

void initBoard(GameContext& gameContext)
{
	int size = 0;
	for (int i = 0; i < gameContext.height; ++i)
	{
		for (int j = 0; j < gameContext.width; ++j)
		{
			gameContext.boardCells[size].line = i;
			gameContext.boardCells[size].column = j;
			gameContext.boardCells[size].cellType = EmptyCell;
			size++;	
		}
	}
	for (int i = 0; i < gameContext.height; ++i)
	{
		for (int j = 0; j < gameContext.width; ++j)
		{
			if (i == 0 || i == (gameContext.height - 1) || j == 0 || j == (gameContext.width - 1))
			{
				fillCell(gameContext, i, j, CellType::Border);
			}
		}
	}
	gameContext.direction = Right;
	fillCell(gameContext, 9, 10, CellType::SnakeBody);
	fillCell(gameContext, 9, 11, CellType::SnakeBody);
	fillCell(gameContext, 9, 12, CellType::SnakeBody);

	gameContext.snakeBody[0].line = 9;
	gameContext.snakeBody[1].line = 9;
	gameContext.snakeBody[2].line = 9;

	gameContext.snakeBody[0].column = 10;
	gameContext.snakeBody[1].column = 11;
	gameContext.snakeBody[2].column = 12;

	gameContext.snakeBodySize = 3;
	gameContext.snakeTailIndex = 0;

	spawnNewFood(gameContext);
	gameContext.spawnNewFood = false;

	if (gameContext.direction == Right || gameContext.direction == Left)
		setTimer(gameContext.timer, snakeMoveDelayHorizontal);
	else
	if (gameContext.direction == Up || gameContext.direction == Down)
		setTimer(gameContext.timer, snakeMoveDelayVertical);
}

void initBoardAI(GameContext& gameContext)
{
	int size = 0;
	for (int i = 0; i < gameContext.height; ++i)
	{
		for (int j = 0; j < gameContext.width; ++j)
		{
			gameContext.boardCells[size].line = i;
			gameContext.boardCells[size].column = j;
			gameContext.boardCells[size].cellType = EmptyCell;
			size++;
		}
	}
	for (int i = 0; i < gameContext.height; ++i)
	{
		for (int j = 0; j < gameContext.width; ++j)
		{
			if (i == 0 || i == (gameContext.height - 1) || j == 0 || j == (gameContext.width - 1))
			{
				fillCell(gameContext, i, j, CellType::Border);
			}
		}
	}
	gameContext.direction = Right;
	fillCell(gameContext, 9, 10, CellType::SnakeBody);
	fillCell(gameContext, 9, 11, CellType::SnakeBody);
	fillCell(gameContext, 9, 12, CellType::SnakeBody);

	gameContext.snakeBody[0].line = 9;
	gameContext.snakeBody[1].line = 9;
	gameContext.snakeBody[2].line = 9;

	gameContext.snakeBody[0].column = 10;
	gameContext.snakeBody[1].column = 11;
	gameContext.snakeBody[2].column = 12;

	gameContext.snakeBodySize = 3;
	gameContext.snakeTailIndex = 0;


	gameContext.directionAI = Right;
	fillCell(gameContext, 9, 30, CellType::SnakeBodyAI);
	fillCell(gameContext, 9, 31, CellType::SnakeBodyAI);
	fillCell(gameContext, 9, 32, CellType::SnakeBodyAI);

	gameContext.snakeBodyAI[0].line = 9;
	gameContext.snakeBodyAI[1].line = 9;
	gameContext.snakeBodyAI[2].line = 9;

	gameContext.snakeBodyAI[0].column = 30;
	gameContext.snakeBodyAI[1].column = 31;
	gameContext.snakeBodyAI[2].column = 32;

	gameContext.snakeBodySizeAI = 3;
	gameContext.snakeTailIndexAI = 0;

	spawnNewFood(gameContext);
	gameContext.spawnNewFood = false;

	if (gameContext.direction == Right || gameContext.direction == Left)
		setTimer(gameContext.timer, snakeMoveDelayHorizontal);
	else
		if (gameContext.direction == Up || gameContext.direction == Down)
			setTimer(gameContext.timer, snakeMoveDelayVertical);
}


void displayBoard(GameContext& gameContext)
{
	dbc.clear();
	CellType map[MAX_HEIGHT][MAX_WIDTH] = {};
	for (int i = 0; i < gameContext.filledCellsSize; ++i)
	{
		BoardCell& currentCell = gameContext.boardCells[i];
		map[currentCell.line][currentCell.column] = currentCell.cellType;
	}

	for (int i = 0; i < gameContext.height; ++i)
	{
		for (int j = 0; j < gameContext.width; ++j)
		{
			switch (map[i][j])
			{
			case EmptyCell:
			{
				dbc.setColor(BACKGROUND_INTENSITY | BACKGROUND_GREEN | BACKGROUND_RED);
				dbc <<' ';
			}
			break;
			case SnakeBody:
			{
				dbc.setColor(BACKGROUND_INTENSITY | BACKGROUND_GREEN | BACKGROUND_RED);
				dbc << (char)219;
			}
			break;
			case SnakeBodyAI:
			{
				dbc.setColor(BACKGROUND_INTENSITY | BACKGROUND_BLUE | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
				dbc << (char)219;
			}
			break;
			case PowerUp:
			{
				dbc.setColor(BACKGROUND_INTENSITY | BACKGROUND_GREEN | BACKGROUND_RED | FOREGROUND_GREEN);
				dbc << (char)63;
			}
			break;
			case Border:
			{
				dbc.setColor(BACKGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_INTENSITY);
				dbc << (char)219;
			}
			break;
			case Food:
			{
				dbc.setColor(BACKGROUND_INTENSITY | BACKGROUND_GREEN | BACKGROUND_RED | FOREGROUND_GREEN);
				dbc << (char)43;
				
			}
			break;
			default:
			
				break;
			}
		}
		dbc.setColor(FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN);
		if (gameContext.width < 80)
		{
			dbc << "\n";
		}
	}
	
	dbc.display();
}

void handleUserInput(GameContext& gameContext)
{
	if (gameContext.moveMadeForCycle)
	{
		return;
	}
	if (isKeyPressed(VK_UP))
	{
		if (gameContext.direction != Down)
		{
			gameContext.moveMadeForCycle = true;
			gameContext.direction = Up;		
		}	
	}
	else if (isKeyPressed(VK_LEFT))
	{
		if (gameContext.direction != Right)
		{
			gameContext.moveMadeForCycle = true;
			gameContext.direction = Left;
		}
	}
	else if (isKeyPressed(VK_DOWN))
	{
		if (gameContext.direction != Up)
		{
			gameContext.moveMadeForCycle = true;
			gameContext.direction = Down;
		}
	}
	else if (isKeyPressed(VK_RIGHT))
	{
		if (gameContext.direction != Left)
		{
			gameContext.moveMadeForCycle = true;
			gameContext.direction = Right;
		}
	}
	if (gameContext.direction == Right || gameContext.direction == Left)
		changeReadyTime(gameContext.timer, snakeMoveDelayHorizontal);
	else
		if (gameContext.direction == Up || gameContext.direction == Down)
		changeReadyTime(gameContext.timer, snakeMoveDelayVertical);
}

void GetHighScore(int& highScore)
{
	std::ifstream fin("score.txt");
	char nameFromFile[256];
	int scoreFromFile;
	if (fin >> nameFromFile >> scoreFromFile)
	{
		highScore = scoreFromFile;
	}
}


void updateHighScore(char* name,int score)
{
	const int maxTopPlayers = 10;
	char nameFromFile[maxTopPlayers][256] = {};
	int scoreFromFile[maxTopPlayers] = {};
	int modified = 0;

	int scoreEntryCount = 0;
	std::ifstream fin("score.txt");
	while (scoreEntryCount < maxTopPlayers &&
		fin >> nameFromFile[scoreEntryCount] >> scoreFromFile[scoreEntryCount] )
	{
		scoreEntryCount++;
	}
	fin.close();

	if (scoreEntryCount == 0)
	{
		std::ofstream fout("score.txt");
		fout << name << " " << score << "\n";
		return;
	}

	if (scoreEntryCount < maxTopPlayers)
	{
		int i = 0;
		while (score <= scoreFromFile[i] && i < scoreEntryCount)
		{
			i++;
		}

		for (int j = scoreEntryCount - 1; j >= i; j--)
		{
			strcpy(nameFromFile[j + 1], nameFromFile[j]);
			scoreFromFile[j + 1] = scoreFromFile[j];
		}
		++scoreEntryCount;

		strcpy(nameFromFile[i], name);
		scoreFromFile[i] = score;

		std::ofstream fout("score.txt");
		for (int i = 0; i < scoreEntryCount; ++i)
		{
			fout << nameFromFile[i] << " " << scoreFromFile[i] << "\n";
		}
	}
	else
	{
		int i = 0;
		while (score <= scoreFromFile[i] && i < scoreEntryCount)
		{
			i++;
		}
		if (i < scoreEntryCount)
		{
			for (int j = scoreEntryCount - 2; j >= i; j--)
			{
				strcpy(nameFromFile[j + 1], nameFromFile[j]);
				scoreFromFile[j + 1] = scoreFromFile[j];
			}

			strcpy(nameFromFile[i], name);
			scoreFromFile[i] = score;

			std::ofstream fout("score.txt");
			for (int i = 0; i < scoreEntryCount; ++i)
			{
				fout << nameFromFile[i] << " " << scoreFromFile[i] << "\n";
			}
		}
	}
}
void endgame(int score, int highscore);
void singlePlayer()
{
	GameContext gameContext = {};
	gameContext.width = 75;
	gameContext.height = 20;
	
	char playerName[256];
	std::cout << "Type your name:\n";
	std::cin >> playerName;

	std::cout << "The typed name is: " << playerName << "\nGood luck!";
	Sleep(1000);
	system("cls");
	int score = 0, highscore = 0;
	initBoard(gameContext);
	while (1)
	{
		displayBoard(gameContext);
		handleUserInput(gameContext);
		if (isTimerReady(gameContext.timer))
		{
			gameContext.moveMadeForCycle = false;
			if (!moveSnake(gameContext,score))
			{
				dbc.switchToSingleBuffer();
				system("cls");

				updateHighScore(playerName, score);
				GetHighScore(highscore);

				endgame(score, highscore);
				gameContext.playerWon = false;
				Sleep(4000);
				return;
			}
		}

		if (gameContext.spawnNewFood)
		{
			
			if (!spawnNewFood(gameContext))
			{
				gameContext.playerWon = true;
				dbc.switchToSingleBuffer();
				return;
			}
		}
		Sleep(50);
	}
}

void playerVsAi()
{
	GameContext gameContext = {};
	gameContext.width = 75;
	gameContext.height = 20;

	char playerName[256];
	std::cout << "Type your name:\n";
	std::cin >> playerName;

	std::cout << "The typed name is: " << playerName << "\nGood luck!";
	Sleep(1000);
	system("cls");
	int score = 0, highscore = 0;
	initBoardAI(gameContext);
	while (1)
	{
		displayBoard(gameContext);
		handleUserInput(gameContext);
		if (isTimerReady(gameContext.timer))
		{
			gameContext.moveMadeForCycle = false;
			
			moveSnakeAI(gameContext);

			if (!moveSnake(gameContext, score))
			{
				dbc.switchToSingleBuffer();
				system("cls");

				updateHighScore(playerName, score);
				GetHighScore(highscore);

				endgame(score, highscore);
				gameContext.playerWon = false;
				Sleep(4000);
				return;
			}

		}

		if (gameContext.spawnNewFood)
		{
			
			if (!spawnNewFood(gameContext))
			{
				gameContext.playerWon = true;
				dbc.switchToSingleBuffer();
				return;
			}
		}
		Sleep(50);
	}
}

void displayHelp()
{
	system("cls");
	std::cout << "\t\t" << " __    __   _______  __      .______  " << "\n";
	std::cout << "\t\t" << "|  |  |  | |   ____||  |     |   _  \\ " << "\n";
	std::cout << "\t\t" << "|  |__|  | |  |__   |  |     |  |_)  | " << "\n";
	std::cout << "\t\t" << "|   __   | |   __|  |  |     |   ___/ " << "\n";
	std::cout << "\t\t" << "|  |  |  | |  |____ |  `----.|  |     " << "\n";
	std::cout << "\t\t" << "|__|  |__| |_______||_______|| _|     " << "\n";
	std::cout << "\t\t" << "" << "\n" << "\n";
	std::cout << "\t" << "Snake este unul dintre cele mai populare jocuri video create vreodata.Acesta a fost lansat in anul 1976" << "\n";
	std::cout << "\t" << "dar si-a cunoscut succesul abia in anul 1998 cand celebrul fabricant de telefoane NOKIA" << "\n";
	std::cout << "\t" << "l-a preluat si introdus in telefoanele sale.In momentul actual jocul are sute de variatii." << "\n\n";
	std::cout << "\t" << "Scopul jocului este acela de a aduna cat mai multe puncte iar, jocul se termina fie in momentul in care" << "\n";
	std::cout << "\t" << "jucatorul a acoperit in intregime tabla de joc fie cand acesta se loveste de un perete sau de corpul propriu." << "\n";
	std::cout << "\t" << "Jucatorul controleaza o linie(un sarpe) care, creste in lungime, adunand pe parcursul jocului, diferitele" << "\n";
	std::cout << "\t" << "obiecte care ii apar in cale." << "\n\n";
	std::cout << "\t" << "Jucatorul se misca folosind tastele, Sageata sus(UpArrow) pentru a misca 'rama' in sus, Sageata jos(DownArrow)" << "\n";
	std::cout << "\t" << "pentru a se misca in jos, Sageata stanga(LeftArrow) respectiv Sageata dreapta(RigthArrow)" << "\n";
	std::cout << "\t" << "" << "\n" << "\n";
	system("pause");
}

void displayHighscore()
{
	system("cls");
	std::ifstream fin("score.txt");
	char linie[256];
	char * p;
	const char sep[3] = " \n";
	int i = 1;
	std::cout <<"\t"<< "Top 10 scoruri :" << "\n\n";
	while (!fin.eof() && i <= 10)
	{
		fin.getline(linie, 256);
		p = strtok(linie, sep);
		std::cout << "\t"<< i <<"."<<  p << " ";
		p = strtok(NULL, sep);
		std::cout << "Scor:" << p << "\n\n";
		i++;
	}
	system("pause");
}
void ShowTitle()
{
	std::cout << "\n\n";
	std::cout << "         ad88888ba  888b      88        db        88      a8P  88888888888 " << "\n";
	std::cout << "	d8       8b 8888b     88       d88b       88    ,88'   88" << "\n";
	std::cout << "	Y8,         88 `8b    88      d8'`8b      88  ,88      88 " << "\n";
	std::cout << "          8aaaaa,   88  `8b   88     d8'  `8b     88,d88'      88aaaaa " << "\n";
	std::cout << "	        8b, 88   `8b  88    d8YaaaaY8b    8888 88,     88      " << "\n";
	std::cout << "		`8b 88    `8b 88   d8        8b   88P   Y8b    88  " << "\n";
	std::cout << "	Y8a     a8P 88     `8888  d8'        `8b  88      88,  88 " << "\n";
	std::cout << "	  Y88888P   88      `888 d8'          `8b 88       Y8b 88888888888  " << "\n";
	
}
void ShowSnake()
{

	std::cout << "                  				  ____" << "\n";
	std::cout << "                         ________________________/ O  \\___/" << "\n";
	std::cout << "                        <_____________________________/   \\" << "\n";



}
void ShowMenu()
{
	std::cout << "\n"
		"				  1. SinglePlayer\n"
		"				  2. PlayerVsAi\n"
		"				  3. Help\n"
		"				  4. Highscores\n"
		"				  5. Exit\n";
}

void endgame(int score, int highscore) 
{
	std::cout << "" << "\n" << "\n";
	std::cout << " ------------------------------------------------------------------------- " << "\n";
	std::cout << "|    *****      *     *       * ******       ****  *       ****** ****    |" << "\n";
	std::cout << "|   *          * *    * *   * * *           *    *  *     * *     *   *   |" << "\n";
	std::cout << "|   *  ****   *   *   *  * *  * *****       *    *   *   *  ****  ****    |" << "\n";
	std::cout << "|   *  *  *  *******  *   *   * *           *    *    * *   *     * *     |" << "\n";
	std::cout << "|    *****  *       * *       * ******       ****      *    ***** *   *   |" << "\n";
	std::cout << " ------------------------------------------------------------------------- " << "\n";
	std::cout << "" << "\n" << "\n";
	std::cout << "                        Y O U R   S C O R E : " << score << "\n" << "\n";
	std::cout << "                        H I G H   S C O R E : " << highscore << "\n";
	std::cout << "" << "\n" << "\n";

	
}



void menu()
{

	system("cls");
	std::cout << "\n";
	ShowTitle();
	ShowMenu();
	ShowSnake();
	if (isKeyPressed('1'))
	{
		singlePlayer();
	}
	else if (isKeyPressed('2'))
	{
		playerVsAi();
	}
	else if (isKeyPressed('3'))
	{
		displayHelp();
	}
	else if (isKeyPressed('4'))
	{
		displayHighscore();
	}
	else if (isKeyPressed('5'))
	{
		exit(0);
	}
}

int main()
{
	srand((unsigned)time(0));
	SetConsoleTitle("Snake!");
	while (true)
	{
		menu();
		Sleep(50);
	}
}