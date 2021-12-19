#pragma once

#define GAME_MAP_ROW (15)			//����
#define GAME_MAP_COL (25)			//����

#define POS_ROW (0)					//X����
#define POS_COL (1)					//Y����

#define ITEM_TYPE_NULL (0)			//�հ�λ��
#define ITEM_TYPE_WALL (1)			//ǽ��λ��
#define ITEM_TYPE_SNAKE_HEAD (2)	//��ͷ��λ��
#define ITEM_TYPE_SNAKE_BODY (3)	//�������λ��
#define ITEM_TYPE_FOOD (4)			//�������λ��

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
//��ʼ��ǽ
void InitWall();

//��ʼ����
void InitSnake();

//��ʼ��ʳ��
void InitFood();

//��ӡǽ
void ShowToScreen();

//�����ƶ�
void SnakeMoveUp();

//�����ƶ�
void SnakeMoveLeft();

//�����ƶ�
void SnakeMoveDown();

//�����ƶ�
void SnakeMoveRight();

//�Ƿ�����ƶ������ܺ��ߵڶ���λ���ظ���
int SnakeCanMove(int nRowAddValue, int nColAddValue);

//��ʳ��
void EatFood();

//�ж��Ƿ�ײǽ
int SnakeCrashWall();

//�ж��Ƿ�ײ�Լ�������
int SnakeCrashBody();

//�Զ��ƶ�
void SnakeAutoMove();

//��Ϸ����
void GameOver();


