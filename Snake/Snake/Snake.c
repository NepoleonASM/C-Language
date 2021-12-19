// Snake.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//AUTHOR：NEPOLOEN 
//DATE：2021-12-19
//

#include "Game.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
int main()
{
    //char cTest = 1;
    //char cTest = '1';
    //printf("%d", cTest);

    //初始化随机种子
    srand((unsigned)time(NULL));

    //初始化墙
    InitWall();

    //初始化蛇
    InitSnake();
    
    //初始化食物
    InitFood();

    //打印元素
    ShowToScreen();

    //
    while (1)
    {
        system("cls");
        //system("cls");
        //手动移动
        //1.获取按键
        //2.判断按键信息
        //3.移动
        //4.刷新
        //5.判断吃食物
        //  5.1吃到食物蛇身体增加1倍
        //6.判断撞墙
        //7.判断首尾相连

        //如果有按任何键
        if (_kbhit())
        {
            //控制台有按键
            char cTemoInput = _getch();//_getch()获取按键
            switch (cTemoInput)
            {
            case 'W':
            case 'w':
            {
                if (SnakeCanMove(-1, 0)==1)
                {
                    SnakeMoveUp();
                    //打印元素
                    ShowToScreen();
                    EatFood();                
                }

            }
                break;
            case 'A':
            case 'a':
            {
                if (SnakeCanMove(0, -1) == 1)
                {
                    SnakeMoveLeft();
                    //打印元素
                    ShowToScreen();
                    EatFood();
                    
                }

            }
                break;
            case 'S':
            case 's':
            {
                if (SnakeCanMove(1, 0) == 1)
                {
                    SnakeMoveDown();
                    //打印元素
                    ShowToScreen();
                    EatFood();
                    
                }
            }
                break;
            case 'D':
            case 'd':
            {
                if (SnakeCanMove(0, 1) == 1)
                {
                    SnakeMoveRight();
                    //打印元素
                    ShowToScreen();
                    EatFood();                                       
                }
            }
                break;
            }
        }
        //如果没有按键
        //自动移动
        else
        {
            SnakeAutoMove();
            ShowToScreen();
            EatFood();
            Sleep(1000);
        }    

        if (SnakeCrashWall() == 1)
        {
            //撞墙  游戏结束
            GameOver();
            return;
        }  
        if (SnakeCrashBody() == 1)
        {
            //蛇头与身体重合  游戏结束
            GameOver();
            return;
        }
    }
    
    return 0;
}

