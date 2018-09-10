#include "types.h"
#include "stat.h"
#include "user.h"


#define Head_File

#define ROW 10
#define LINE 10
#define MINE 20

static unsigned long int next = 1;


void Strncpy( char* _dst, const char* _src, int _n )
{
   int i = 0;
   while(i++ != _n && (*_dst++ = *_src++));
}

int rand(void) // RAND_MAX assumed to be 32767
{
    next = next * 1103515245 + 12345;
    return (unsigned int)(next/65536) % 32768;
}

void srand(unsigned int seed)
{
    next = seed;
}



void display_arr(char arr[ROW][LINE],int row,int line);
void display_mine(char arr[ROW+2][LINE+2],int row,int line);
void set_mine(char arr[ROW+2][LINE+2],int lei);
void player(char arr[ROW+2][LINE+2],char copy[ROW][LINE],int *count,int row,int line);

void display_mine(char arr[ROW+2][LINE+2],int row,int line)
{
    int i=0,j=0;
    for(i=0;i<=row;i++)
    {
        printf(2, " %d  ",i);
    }
    printf(2, "\n--------------------------------------------\n");
    for(i=1;i<=row;i++)
    {
        printf(2, "%2d |",i);
        for(j=1;j<=line;j++)
        {
            printf(2, " %c |",arr[i][j]);
        }
        printf(2, "\n--------------------------------------------\n");
    }
}
void display_arr(char arr[ROW][LINE],int row,int line)
{
    int i=0,j=0;
    for(i=0;i<=row;i++)
    {
        printf(2, " %d  ",i);
    }
    printf(2, "\n--------------------------------------------\n");
    for(i=0;i<row;i++)
    {
        printf(2, "%2d |",i+1);
        for(j=0;j<line;j++)
        {
            printf(2, " %c |",arr[i][j]);
        }
        printf(2, "\n--------------------------------------------\n");
    }
}
void set_mine(char arr[ROW+2][LINE+2],int lei)
{
    int x=0,y=0;
    while(lei)
    {
        x=rand()%ROW+1;
        y=rand()%LINE+1;
        if(arr[x][y]=='0')
        {
            arr[x][y]='1';
            lei--;
        }
    }
}
void player(char arr[ROW+2][LINE+2],char copy[ROW][LINE],int *count,int row,int line)
{
    int x=0,y=0;
    while(1)
    {
        int i=0,j=0;
        printf(2, "please input the coord：\n");
        // char buf[512];
        char data[512];
        // int n = 0;
        // int length = 0;
        if(read(0, data, sizeof(data)) > 0) {
            x = data[0] - '0';
            y = data[2] - '0';
        }
        if(x>=0&&x<=row&&y<=line&&y>=0)
        {
            if(arr[x][y]=='0')
            {
                for(i=x-1;i<=x+1;i++)
                {
                    for(j=y-1;j<=y+1;j++)
                    {
                        if(arr[i][j]=='0'&&(i-1>=0)&&(i-1<=row-1)&&(j-1>=0)&&(j-1<=line-1)&&copy[i-1][j-1]=='*')
                        {
                            copy[i-1][j-1]=(arr[i-1][j-1]-'0'+arr[i][j-1]-'0'+arr[i+1][j-1]-'0'+arr[i-1][j]-'0'+arr[i+1][j]-'0'+arr[i-1][j+1]-'0'+arr[i][j+1]-'0'+arr[i+1][j+1]-'0')+'0';
                            (*count)++;
                        }
                        else
                            continue;
                    }
                }
                break;
            }
            else
            {
                *count=row*line;
                break;
            }
        }
        else
        {
            printf(2, "Invalid input！");
        }
    }
}


void menu()
{
    printf(2, "*************************************\n");
    printf(2, "************    Saolei   ************\n");
    printf(2, "*************  PLAY--1  *************\n");
    printf(2, "*************  EXIT--0  *************\n");
    printf(2, "*************************************\n");
}
enum menu
{
    EXIT,PLAY
};
void Game()
{
    int count=0;
    char mine[ROW+2][LINE+2]={0};
    char arr[ROW][LINE]={0};
    memset(mine,'0',sizeof(char)*(ROW+2)*(LINE+2));//初始化数组mine为0
    memset(arr,'*',sizeof(char)*(ROW)*(LINE));
    set_mine(mine,MINE);//埋雷操作
    while(count<(ROW*LINE-MINE))
    {
        display_arr(arr,ROW,LINE);//打印雷盘
        player(mine,arr,&count,ROW,LINE);//玩家操作
    }
    if(count<=ROW*LINE-MINE)
    {
        display_arr(arr,ROW,LINE);//打印雷盘
        printf(2, "You win！\n");
    }
    else
    {
        printf(2, "You failed！\n");
        display_mine(mine,ROW,LINE);//失败显示雷区
    }
}



int main()
{
    int input;
    int fd = 0;
    // char buf[512];
    char data[512];
    // srand((unsigned int)time(NULL));
    do
    {
        menu();
        printf(2, "Please select");
        // int n;
        // int length = 0;

        read(fd, data, sizeof(data));

        //scanf("%d",&input);
        input = rand() % 2;
        switch(data[0])
        {
        case '1':
            Game();
            break;
        case '0':
            break;
        default:
            printf(2, "Invalid input, please input again：\n");
            break;
        }
    }while(input);
    // system("pause");
    return 0;
}
