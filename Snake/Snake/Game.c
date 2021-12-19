#include "Game.h"

//g_WallArray全局数组用来表示墙上的坐标点，(GAME_MAP_ROW + GAME_MAP_COL) * 2表示周长每个点，[2]代表X和Y坐标
char g_WallArray[(GAME_MAP_ROW + GAME_MAP_COL) * 2][2] = { 0 };

//g_nWallCount用来表示墙的个数【周长上面的各个点】
int g_nWallCount = 0;

//g_SnakeArray全局数组用来保存蛇
g_SnakeArray[(GAME_MAP_ROW + GAME_MAP_COL) * 2][2] = { 0 };

//蛇长度
int g_nSnakeCount = 0;

//食物
char g_FoodPos[2] = { 0 };

//初始化墙
void InitWall()
{
	int i = 0;
	int j = 0;
	g_nWallCount = 0;
	for (i = 0; i < GAME_MAP_ROW; i++)
	{
		for (j = 0; j < GAME_MAP_COL; j++)
		{
			//如果是第一行和最后一行则表示墙
			if (i == 0 || i == GAME_MAP_ROW - 1)
			{
				g_WallArray[g_nWallCount][POS_ROW] = i;//[0]表示横坐标
				g_WallArray[g_nWallCount][POS_COL] = j;//[1]表示纵坐标
				g_nWallCount++;
			}
			//如果是第一列和最后一列则表示墙
			else if (j == 0 || j == GAME_MAP_COL - 1)
			{
				g_WallArray[g_nWallCount][POS_ROW] = i;//[0]表示横坐标
				g_WallArray[g_nWallCount][POS_COL] = j;//[1]表示纵坐标
				g_nWallCount++;
			}
		}
	}
}

void InitSnake()
{

	int i = 0;

	//初始化蛇的长度为3
	g_nSnakeCount = 3;

	int nRowindex = 0;
	int nColindex = 0;

	//蛇需要初始化在一个安全范围（避免刚出生就撞墙）
	int nRowLow = 5;
	int nRowHigh = GAME_MAP_ROW - 5;

	int nColLow = 5;
	int nColHigh = GAME_MAP_COL - 5;

	//随机蛇头位置
	nRowindex = rand() % (nRowHigh - nRowLow) + nRowLow;
	nColindex = rand() % (nColHigh - nColLow) + nColLow;
	//初始化蛇头位置
	//假设：初始化时 蛇一直向上运动
	for (i = 0; i < g_nSnakeCount; i++)
	{
		g_SnakeArray[i][POS_ROW] = nRowindex + i;	//行每次增加1
		g_SnakeArray[i][POS_COL] = nColindex;		//列不变
	}


}

void InitFood()
{
	//食物不能跟蛇和墙重合
	int i = 0;
	//随机食物坐标
	int nRandomRowIndex = 0;
	int nRandomColIndex = 0;
	//食物坐标范围
	int nRowLow = 2;
	int nRowHIgh = GAME_MAP_ROW - 2;
	int nColLow = 2;
	int nColHigh = GAME_MAP_COL - 2;
	//循环次数标志
	int nFlagCount = 0;
	//判断是否跟蛇的位置重合
	do
	{
		//随机食物位置
		nRandomRowIndex = rand() % (nRowHIgh - nRowLow) + nRowLow;
		nRandomColIndex = rand() % (nColHigh - nColLow) + nColLow;

		nFlagCount = 0;
		//循环判断是否和蛇重合
		for (i = 0; i < g_nSnakeCount; i++)
		{
			int nSnakeRow = g_WallArray[i][POS_ROW];
			int nSnakeCol = g_WallArray[i][POS_COL];
			//判断是否和蛇重合
			if (nSnakeRow == nRandomRowIndex && nSnakeCol == nRandomColIndex)
			{
				//重合就重新随机食物的位置
				break;
			}
			else
			{
				nFlagCount++;
			}
			//如果循环次数标志等于蛇的长度，说明未和蛇重合
			if (nFlagCount == g_nSnakeCount)
			{
				g_FoodPos[POS_ROW] = nRandomRowIndex;
				g_FoodPos[POS_COL] = nRandomColIndex;
				return;
			}
		}
	} while (1);
}

void ShowToScreen()
{
	int i = 0;
	int j = 0;
	int nRowIndex = 0;
	int nColIndex = 0;
	//输出字符缓冲区
	char szBuffer[GAME_MAP_ROW * GAME_MAP_COL * 4] = { ITEM_TYPE_NULL };//乘以4为了防止越界。

	//临时游戏地图
	char szGameMap[GAME_MAP_ROW][GAME_MAP_COL] = { ITEM_TYPE_NULL };
	//遍历墙的一周,取出墙每个元素的坐标
	for (i = 0; i < g_nWallCount; i++)
	{
		//墙每一个元素的坐标
		nRowIndex = g_WallArray[i][POS_ROW];
		nColIndex = g_WallArray[i][POS_COL];
		//标记墙的元素为1
		szGameMap[nRowIndex][nColIndex] = ITEM_TYPE_WALL;
	}
	//融合蛇的元素到临时的地图数组
	for (i = 0; i < g_nSnakeCount; i++)
	{
		//蛇每一个元素的坐标
		nRowIndex = g_SnakeArray[i][POS_ROW];
		nColIndex = g_SnakeArray[i][POS_COL];

		if (i == 0)
		{
			//标记蛇头
			szGameMap[nRowIndex][nColIndex] = ITEM_TYPE_SNAKE_HEAD;
		}
		else
		{
			//标记蛇的身体
			szGameMap[nRowIndex][nColIndex] = ITEM_TYPE_SNAKE_BODY;
		}
	}
	//融合食物的元素到临时的地图数组
	szGameMap[g_FoodPos[POS_ROW]][g_FoodPos[POS_COL]] = ITEM_TYPE_FOOD;

	//遍历临时游戏地图，把其中的元素刷新到输出缓冲区中
	for (i = 0; i < GAME_MAP_ROW; i++)
	{
		for (j = 0; j < GAME_MAP_COL; j++)
		{
			char cTemp = szGameMap[i][j];
			//printf("%d\r\n", cTemp);
			switch (cTemp)
			{
			case ITEM_TYPE_WALL:
				printf("■");
				break;
			case ITEM_TYPE_SNAKE_HEAD:
				printf("▲");
				break;
			case ITEM_TYPE_SNAKE_BODY:
				printf("■");
				break;
			case ITEM_TYPE_FOOD:
				printf("●");
				break;
			default:
				printf("□");
				break;
			}
		}
		printf("\r\n");
	}

}

void SnakeMoveUp()
{
	int i = 0;
	int nLastRow = 0;
	int nLastCol = 0;
	for (i = 0; i < g_nSnakeCount; i++)
	{
		int CurrentSnakeRow = g_SnakeArray[i][POS_ROW];//蛇X坐标
		int CurrentSnakeCol = g_SnakeArray[i][POS_COL];//蛇Y坐标

		if (i == 0)
		{
			//蛇头位置
			g_SnakeArray[i][POS_ROW] = CurrentSnakeRow - 1;
			g_SnakeArray[i][POS_COL] = CurrentSnakeCol;
			//保存当前位置信息，是下一次的位置
			nLastRow = CurrentSnakeRow;
			nLastCol = CurrentSnakeCol;
		}
		else
		{
			//蛇身体位置
			g_SnakeArray[i][POS_ROW] = nLastRow;
			g_SnakeArray[i][POS_COL] = nLastCol;

			//再次保存当前位置信息，是下一次的位置
			nLastRow = CurrentSnakeRow;
			nLastCol = CurrentSnakeCol;
		}
	}
}

void SnakeMoveLeft()
{
	int i = 0;
	int nLastRow = 0;
	int nLastCol = 0;
	for (i = 0; i < g_nSnakeCount; i++)
	{
		int CurrentSnakeRow = g_SnakeArray[i][POS_ROW];//蛇X坐标
		int CurrentSnakeCol = g_SnakeArray[i][POS_COL];//蛇Y坐标

		if (i == 0)
		{
			//蛇头位置
			g_SnakeArray[i][POS_ROW] = CurrentSnakeRow ;
			g_SnakeArray[i][POS_COL] = CurrentSnakeCol - 1;
			//保存当前位置信息，是下一次的位置
			nLastRow = CurrentSnakeRow;
			nLastCol = CurrentSnakeCol;
		}
		else
		{
			//蛇身体位置
			g_SnakeArray[i][POS_ROW] = nLastRow;
			g_SnakeArray[i][POS_COL] = nLastCol;

			//再次保存当前位置信息，是下一次的位置
			nLastRow = CurrentSnakeRow;
			nLastCol = CurrentSnakeCol;
		}
	}
}

void SnakeMoveDown()
{
	int i = 0;
	int nLastRow = 0;
	int nLastCol = 0;
	for (i = 0; i < g_nSnakeCount; i++)
	{
		int CurrentSnakeRow = g_SnakeArray[i][POS_ROW];//蛇X坐标
		int CurrentSnakeCol = g_SnakeArray[i][POS_COL];//蛇Y坐标

		if (i == 0)
		{
			//蛇头位置
			g_SnakeArray[i][POS_ROW] = CurrentSnakeRow + 1;
			g_SnakeArray[i][POS_COL] = CurrentSnakeCol;
			//保存当前位置信息，是下一次的位置
			nLastRow = CurrentSnakeRow;
			nLastCol = CurrentSnakeCol;
		}
		else
		{
			//蛇身体位置
			g_SnakeArray[i][POS_ROW] = nLastRow;
			g_SnakeArray[i][POS_COL] = nLastCol;

			//再次保存当前位置信息，是下一次的位置
			nLastRow = CurrentSnakeRow;
			nLastCol = CurrentSnakeCol;
		}
	}

}

void SnakeMoveRight()
{
	int i = 0;
	int nLastRow = 0;
	int nLastCol = 0;
	for (i = 0; i < g_nSnakeCount; i++)
	{
		int CurrentSnakeRow = g_SnakeArray[i][POS_ROW];//蛇X坐标
		int CurrentSnakeCol = g_SnakeArray[i][POS_COL];//蛇Y坐标

		if (i == 0)
		{
			//蛇头位置
			g_SnakeArray[i][POS_ROW] = CurrentSnakeRow;
			g_SnakeArray[i][POS_COL] = CurrentSnakeCol + 1;
			//保存当前位置信息，是下一次的位置
			nLastRow = CurrentSnakeRow;
			nLastCol = CurrentSnakeCol;
		}
		else
		{
			//蛇身体位置
			g_SnakeArray[i][POS_ROW] = nLastRow;
			g_SnakeArray[i][POS_COL] = nLastCol;

			//再次保存当前位置信息，是下一次的位置
			nLastRow = CurrentSnakeRow;
			nLastCol = CurrentSnakeCol;
		}
	}
}

int SnakeCanMove(int nRowAddValue,int nColAddValue)
{
	//取蛇头坐标
	int nHeadRow = g_SnakeArray[0][POS_ROW];
	int nHeadCol = g_SnakeArray[0][POS_COL];
	//取蛇第二节身体坐标
	int nSecRow = g_SnakeArray[1][POS_ROW];
	int nSecCol = g_SnakeArray[1][POS_COL];
	//判断蛇头位置是否与第二个位置重合
	if (nHeadRow + nRowAddValue == nSecRow && nHeadCol + nColAddValue == nSecCol)
	{
		//如果重合
		return 0;
	}
	else
	{
		return 1;
	}

	return 0;
}

void EatFood()
{
	//取蛇头坐标
	int nHeadRow = g_SnakeArray[0][POS_ROW];
	int nHeadCol = g_SnakeArray[0][POS_COL];

	//取蛇尾坐标
	int nTailRow = g_SnakeArray[g_nSnakeCount - 1][POS_ROW];
	int nTailCol = g_SnakeArray[g_nSnakeCount - 1][POS_COL];

	//如果蛇头和食物重合则表示吃到食物
	if (nHeadRow == g_FoodPos[POS_ROW] && nHeadCol == g_FoodPos[POS_COL])
	{
		//可以吃食物
		g_SnakeArray[g_nSnakeCount][POS_ROW] = nTailRow;
		g_SnakeArray[g_nSnakeCount][POS_ROW] = nTailCol;
		g_nSnakeCount++;

		//重新刷新一个食物
		InitFood();
	}
}

int SnakeCrashWall()
{
	//蛇头坐标不能和墙重合
	//取蛇头坐标
	int nHeadRow = g_SnakeArray[0][POS_ROW];
	int nHeadCol = g_SnakeArray[0][POS_COL];
	if (nHeadRow == 0 || nHeadRow == GAME_MAP_ROW - 1 || nHeadCol == 0 || nHeadCol == GAME_MAP_COL - 1)
	{
		return 1;
	}
	return 0;
}

int SnakeCrashBody()
{
	int i = 0;
	//取蛇头坐标
	int nHeadRow = g_SnakeArray[0][POS_ROW];
	int nHeadCol = g_SnakeArray[0][POS_COL];

	for (i = 1; i < g_nSnakeCount; i++)
	{
		//临时的某一节身体坐标
		int nTempRow= g_SnakeArray[i][POS_ROW];
		int nTempCol= g_SnakeArray[i][POS_COL];

		if (nHeadRow == nTempRow && nHeadCol == nTempCol)
		{
			return 1;
		}
	}
	return 0;
}

void SnakeAutoMove()
{
	//取蛇头坐标
	int nHeadRow = g_SnakeArray[0][POS_ROW];
	int nHeadCol = g_SnakeArray[0][POS_COL];
	//取蛇第二节身体坐标
	int nSecRow = g_SnakeArray[1][POS_ROW];
	int nSecCol = g_SnakeArray[1][POS_COL];
	//判断蛇头位置与第二个位置比较
	if (nHeadRow > nSecRow)
	{
		SnakeMoveDown();
	}

	if (nHeadRow < nSecRow)
	{
		SnakeMoveUp();
	}

	if (nHeadCol < nSecCol)
	{
		SnakeMoveLeft();
	}
	if (nHeadCol > nSecCol)
	{
		SnakeMoveRight();
	}
	
}

void GameOver()
{
	printf("\r\nGAME OVER\r\n");
}
