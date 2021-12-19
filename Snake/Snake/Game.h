#pragma once

#define GAME_MAP_ROW (15)			//行数
#define GAME_MAP_COL (25)			//列数

#define POS_ROW (0)					//X坐标
#define POS_COL (1)					//Y坐标

#define ITEM_TYPE_NULL (0)			//空白位置
#define ITEM_TYPE_WALL (1)			//墙的位置
#define ITEM_TYPE_SNAKE_HEAD (2)	//蛇头的位置
#define ITEM_TYPE_SNAKE_BODY (3)	//蛇身体的位置
#define ITEM_TYPE_FOOD (4)			//蛇身体的位置

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
//初始化墙
void InitWall();

//初始化蛇
void InitSnake();

//初始化食物
void InitFood();

//打印墙
void ShowToScreen();

//向上移动
void SnakeMoveUp();

//向左移动
void SnakeMoveLeft();

//向下移动
void SnakeMoveDown();

//向右移动
void SnakeMoveRight();

//是否可以移动【不能和蛇第二个位置重复】
int SnakeCanMove(int nRowAddValue, int nColAddValue);

//吃食物
void EatFood();

//判断是否撞墙
int SnakeCrashWall();

//判断是否撞自己的身体
int SnakeCrashBody();

//自动移动
void SnakeAutoMove();

//游戏结束
void GameOver();


