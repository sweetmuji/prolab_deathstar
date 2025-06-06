#define CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <Windows.h>
#include <time.h>
#include <stdlib.h>
#include<mmsystem.h>
#pragma comment(lib, "winmm.lib")

// 오디오 파일 절대 경로 ( 다른 컴퓨터에서 빌드할 시 디버그 폴더만 로컬 절대 경로로 바꾸면 됨!!! )
#define SOUND_DIR "C:\\Users\\wonj1\\Desktop\\prolab_deathstar\\prolab\\x64\\Debug"

#define xwingsound SOUND_DIR "\\xwing.wav"
#define tiesound   SOUND_DIR "\\tie.wav"
#define r2sound  SOUND_DIR "\\r2d2.wav"
#define standbysound  SOUND_DIR "\\standby.wav"
#define explodesound  SOUND_DIR "\\explode.wav"
#define scream  SOUND_DIR "\\scream.wav"
#define saberonsound  SOUND_DIR "\\saberon.wav"
#define r2sound1  SOUND_DIR "\\r2d21.wav"

// 색상 정의
#define BLACK	0
#define BLUE1	1
#define GREEN1	2
#define CYAN1	3
#define RED1	4
#define MAGENTA1 5
#define YELLOW1	6
#define GRAY1	7
#define GRAY2	8
#define BLUE2	9
#define GREEN2	10
#define CYAN2	11
#define RED2	12
#define MAGENTA2 13
#define YELLOW2	14
#define WHITE	15

#define BLANK ' ' // ' ' 로하면 흔적이 지워진다 

#define ESC 0x1b //  ESC 누르면 종료

#define SPECIAL1 0xe0 // 특수키는 0xe0 + key 값으로 구성된다.
#define SPECIAL2 0x00 // keypad 경우 0x00 + key 로 구성된다.

#define UP		'w'
#define DOWN	's'
#define LEFT	'a'
#define RIGHT	'd'
#define STOP1 'c'
#define SHOOT1  'v'

#define UP2  0x48 // Up key는 0xe0 + 0x48 두개의 값이 들어온다.
#define DOWN2 0x50
#define LEFT2 0x4b
#define RIGHT2 0x4d
#define STOP2 'm'
#define SHOOT2 'n'

#define SPACE 32

#define WIDTH 160
#define HEIGHT 50

#define P1_x_min 0
#define P1_x_max 78
#define P2_x_min 81
#define P2_x_max 159
#define playArea_y 35

// 루크와 베이더의 좌표 정리
int luke_x;
int luke_y;
int vader_x;
int vader_y;

// 루크와 베이더의 체력
int luke_life;
int vader_life;
int is_luke_dead; // 사망 상태 
int is_vader_dead; 

// 승리 결과
int is_luke_win;
int is_vader_win;

// 플레이어들이 발사하는 탄환
#define MAX_BULLETS 30

typedef struct 
{
	int owner; // 0: 루크, 1: 베이더
	int x;
	int y;
	int active; // 0: 비활성화, 1: 활성화
} Bullet;

Bullet bullets[MAX_BULLETS];

// 적 타이파이터 구조체
char tie[3] = { '|', 'O', '|' };

#define MAX_TIES 30

typedef struct
{
	int x;
	int y;
	int active;
	int life;
} tieFighter;

tieFighter ties[MAX_TIES];

// 점수 
int score; // 점수 저장 while 반복 1 회 마다 1 증가

void removeCursor(void) { // 커서를 안보이게 한다

	CONSOLE_CURSOR_INFO curInfo;
	GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
	curInfo.bVisible = 0;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
}

void gotoxy(int x, int y) //내가 원하는 위치로 커서 이동
{
	COORD pos = { x, y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);// WIN32API 함수입니다. 이건 알필요 없어요
}

void putstar(int x, int y, char ch)
{
	gotoxy(x, y);
	putchar(ch);
}
void erasestar(int x, int y)
{
	gotoxy(x, y);
	putchar(BLANK);
}

void putPlayer(int x, int y, char* xwing)
{
	gotoxy(x - 1, y);

	for (int i = 0; i < 3; i++)
	{
		putchar(xwing[i]);
	}
}

void textcolor(int fg_color, int bg_color)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), fg_color | bg_color << 4);
}

void playEffect(const char* path)
{
	char command[256];
	static int count = 0;

	MCIERROR err;

	// open 명령
	sprintf(command, "open \"%s\" alias effect%d", path, count);
	err = mciSendString(command, NULL, 0, NULL);
	if (err != 0) {
		char errMsg[128];
		mciGetErrorString(err, errMsg, sizeof(errMsg));
		MessageBox(NULL, errMsg, "MCI Open Error", MB_OK);
		return;
	}

	// play 명령
	sprintf(command, "play effect%d from 0", count);
	err = mciSendString(command, NULL, 0, NULL);
	if (err != 0) {
		char errMsg[128];
		mciGetErrorString(err, errMsg, sizeof(errMsg));
		MessageBox(NULL, errMsg, "MCI Play Error", MB_OK);
		return;
	}

	count++;
}

void erasePlayer(int x, int y)
{
	gotoxy(x - 1, y);

	for (int i = 0; i < 3; i++)
	{
		putchar(BLANK);
	}
}

void paintBG(int x, int y, int color)
{
	for (int i = 0; i <= x; i++)
	{
		for (int j = 0; j <= y; j++)
		{
			textcolor(color, color);
			putchar(BLANK);
		}
	}
}

void draw_ingame_UI()
{
	paintBG(WIDTH, playArea_y, GRAY1);

	for (int i = P1_x_max + 1; i < P2_x_min;i++)
	{
		for (int j = 0; j <= playArea_y + 1; j++)
		{
			textcolor(GRAY2, GRAY1);
			gotoxy(i, j);
			printf("■");
		}
	}

	for (int i = 0; i < 159; i++)
	{
		textcolor(GRAY2, GRAY1);
		gotoxy(i, playArea_y + 1);
		printf("■");
	}
}

erase_speech(int x, int y)
{
	textcolor(BLACK, BLACK);
	printf("                                                                         ");
	textcolor(CYAN1, BLACK);
	gotoxy(x, y);
}

void speech(int score) // 특정 점수 분기점 마다 대사 출력하기 
{
	int x = 0;
	int y = playArea_y + 5;
	gotoxy(x, y);
	textcolor(CYAN1, BLACK);

	switch (score)
	{
	case 50:
		playEffect(standbysound);
		printf("레드 전대장 가벤 드레이스 : 전 대원, S - foil 공격 상태로 고정하라.");
		break;
	case 80:
		erase_speech(x, y);
		printf("레드 텐 대기 중");
		break;
	case 110:
		erase_speech(x, y);
		printf("레드 세븐 대기 중");
		break;
	case 140:
		erase_speech(x, y);
		printf("레드 식스 대기 중");
		break;
	case 170:
		erase_speech(x, y);
		printf("레드 나인 대기 중");
		break;
	case 200:
		erase_speech(x, y);
		printf("레드 투 대기 중");
		break;
	case 220:
		erase_speech(x, y);
		printf("레드 일레븐 대기 중");
		break;
	case 250:
		erase_speech(x, y);
		printf("루크 스카이워커 : 레드 파이브 대기 중");
		break;
	case 270:
		erase_speech(x, y);
		playEffect(r2sound);
		printf("R2D2 : 삐비비빅");
		break;
	case 290:
		erase_speech(x, y);
		printf("웨지 안틸레스 : 정말 거대하군!");
		break;
	case 310:
		erase_speech(x, y);
		printf("루크 스카이워커 : 여기는 레드 파이브, 진입한다.");
		break;
	case 330:
		erase_speech(x, y);
		textcolor(RED1, BLACK);
		printf("은하 제국 장교 : 총독님, 반란군 기지가 7분 후 사정권에 들어옵니다.");
		break;
	case 360:
		erase_speech(x, y);
		textcolor(RED1, BLACK);
		printf("월허프 타킨 총독 : 준비가 되면 발사하게");
		break;
	}
}

void shootLaser(int input)
{
	// 탄환 배열을 모두 순회하면서 비활성화 된 남은 탄환이 있는지 검사!!
	for (int i = 0; i < MAX_BULLETS; i++)
	{
		if (!bullets[i].active) // 비활성화 된 탄환을 찾았다면
		{
			if (input == SHOOT1)
			{
				playEffect(xwingsound);
				bullets[i].owner = 0; // 루크
				bullets[i].active = 1;
				bullets[i].x = luke_x;  
				bullets[i].y = luke_y - 1; 
			}
			else if (input == SHOOT2)
			{
				playEffect(tiesound);
				bullets[i].owner = 1; // 베이더
				bullets[i].active = 1;
				bullets[i].x = vader_x; 
				bullets[i].y = vader_y - 1;
			}

			if (bullets[i].owner == 1 && bullets[i].x < P2_x_min)
			{
				bullets[i].x = P2_x_min + 2;
			}

			if (bullets[i].owner == 0 && bullets[i].x > P1_x_max)
			{
				bullets[i].x = P1_x_max - 2;
			}

			break;
		}
	}
}

void luke_damage(int damage) // 루크의 데미지, 사망 처리 
{
	luke_life -= damage;

	if (luke_life <= 0)
	{
		is_luke_dead = 1;
		is_vader_win = 1; // 베이더 승리

		for (int i = luke_x; i <= luke_x + 2; i++)
		{
			putchar(BLANK); // 엑스윙 지우기
		}
	}
}

void vader_damage(int damage) // 베이더의 데미지, 사망 처리
{
	vader_life -= damage;

	if (vader_life <= 0)
	{
		is_vader_dead = 1;
		is_luke_win = 1;

		for (int i = vader_x; i <= vader_x + 2; i++)
		{
			putchar(BLANK); // 베이더 지우기
		}
	}
}

void enemyShoot(int tieNum)
{
	int i;
	for (i = 0; i < MAX_BULLETS; i++)
	{
		if (!bullets[i].active)
		{
			playEffect(tiesound);
			bullets[i].x = ties[tieNum].x - 1; // 왼쪽 탄환
			bullets[i].y = ties[tieNum].y + 3;
			bullets[i].owner = 2; // 타이 파이터가 주인
			bullets[i].active = 1;

			break; // 다음 탄환 찾기
		}
	}

	for (int j = i + 1; j < MAX_BULLETS; j++)
	{
		if (!bullets[j].active)
		{
			bullets[j].x = ties[tieNum].x + 1; // 오른쪽 탄환
			bullets[j].y = ties[tieNum].y + 3;
			bullets[j].owner = 2; // 타이 파이터가 주인
			bullets[j].active = 1;

			break;
		}
	}
}

void spawnEnemy()
{
	for (int i = 0; i < MAX_TIES; i++)
	{
		if (!ties[i].active)
		{
			ties[i].active = 1;
			ties[i].life = 1;

			// 루크 화면에서 스폰 되는 타이 파이터 x, y 랜덤
			int ranX = rand() % 60 + 15;
			int ranY = rand() % 3;

			ties[i].x = ranX;
			ties[i].y = ranY;

			enemyShoot(i); // 스폰 되자 마자 탄환 발사

			break; // 적을 하나 스폰했으면 종료
		}
	}
}

void updateEnemy()
{
	for (int i = 0; i < MAX_TIES; i++)
	{
		if (ties[i].life <= 0)
		{
			ties[i].active = 0;
		}

		if (ties[i].active)
		{
			erasePlayer(ties[i].x, ties[i].y);
			ties[i].y++;

			// 루크의 화면 최하단으로 갔을 때 베이더 화면으로 넘어가게
			if (ties[i].x < 80 && ties[i].y >= playArea_y)
			{
				ties[i].x += 80;
				ties[i].y = 0;
			}

			if (ties[i].x > 80 && ties[i].y >= playArea_y)
			{
				ties[i].active = 0;
			}
			
			textcolor(BLACK, GRAY1);
			putPlayer(ties[i].x, ties[i].y, tie);
		}
		else
		{
			erasePlayer(ties[i].x, ties[i].y);

			ties[i].x = 157;
			ties[i].y = 0;
		}
	}
}

void updateBullets() // 탄환들을 모아둔 배열을 순회하여 상태를 갱신한다!!
{
	for (int i = 0; i < MAX_BULLETS; i++)
	{
		if (bullets[i].active)
		{
			gotoxy(bullets[i].x, bullets[i].y);
			putchar(BLANK);
			// 이전 위치 지우기

		// 만약 탄환이 플레이어 ( 루크, 베이더 ) 라면 위로 상승, 타이 파이터 라면 아래로 하강
			if (bullets[i].owner == 2)
			{
				bullets[i].y += 3;
			}
			else
			{
				bullets[i].y -= 2;
			}

			// 히트박스를 약간 여유롭게 하여 움직일 때 대비 ( 베이더와 타이의 공격 ) 
			if (bullets[i].owner != 0 && bullets[i].y >= (luke_y - 1) && bullets[i].y <= luke_y)
			{
				if (bullets[i].x >= (luke_x - 1) && bullets[i].x <= (luke_x + 1))
				{
					bullets[i].active = 0;
					luke_damage(1); // 루크가 받는 데미지
				}
			}

			// 루크 -> 타이 공격 
			for (int j = 0; j < MAX_TIES; j++)
			{
				if (!ties[j].active)
				{
					continue;
				}

				if (bullets[i].owner == 0 && bullets[i].y >= (ties[j].y - 2) && bullets[i].y <= ties[j].y)
				{
					if (bullets[i].x >= (ties[j].x - 2) && bullets[i].x <= (ties[j].x + 2)) // 타이 파이터 명중 이벤트
					{
						int ran = rand() % 4;

						if (ran == 1)
						{
							playEffect(scream);
						}
						else
						{
							playEffect(explodesound);
						}

						score += 50; // 점수 50점 증가
						bullets[i].active = 0;
						ties[j].life -= 1;
					}
				}

			}

			gotoxy(bullets[i].x, bullets[i].y);

			if (bullets[i].owner != 0)
			{
				textcolor(GREEN1, GRAY1);
			}
			else
			{
				textcolor(RED1, GRAY1);
			}

			putchar('|');

			if (bullets[i].owner == 1 && bullets[i].y <= 0 && bullets[i].x > 80) // 베이더의 탄환은 y 0에 도달하면 루크 쪽으로 옮기기 
			{
				gotoxy(bullets[i].x, 1);
				printf("  ");

				bullets[i].x = bullets[i].x - 81;
				bullets[i].y = playArea_y - 1;
			}

			if (bullets[i].owner == 2 && bullets[i].y >= playArea_y - 2 && bullets[i].x < 80) // 타이 파이터의 탄환은 y playAreaY - 25에 도달하면 베이더 쪽으로 옮기기
			{
				gotoxy(bullets[i].x, bullets[i].y);
				putchar(BLANK);

				bullets[i].x = bullets[i].x + 81;
				bullets[i].y = 0;
			}

			if ((bullets[i].x < WIDTH / 2 && bullets[i].y <= 1))
			{
				bullets[i].active = 0; // 화면 밖으로 나가면 비활성화
			}

			if (bullets[i].owner == 2 && bullets[i].x > WIDTH / 2 && bullets[i].y >= (playArea_y - 2))
			{
				bullets[i].active = 0;
			}

			if (!bullets[i].active)
			{
				gotoxy(bullets[i].x, bullets[i].y);
				putchar(BLANK);
			}
		}
		else
		{
			gotoxy(bullets[i].x, bullets[i].y);
			putchar(BLANK);
		}
	}
}

void updateUI()
{
	textcolor(GREEN1, BLACK);

	gotoxy(0, playArea_y + 2);
	printf("루크 체력 : %2d", luke_life);
	gotoxy(0, playArea_y + 3);
	printf("베이더 체력 : %2d", vader_life);
	gotoxy(0, playArea_y + 4);
	printf("현재 점수 : %d ", score);
 }

void startgame()
{
	// 점수 카운터 초기화
	score = 0;

	// 대사 분기점
	int speechScore = 0;

	// 체력 초기화
	luke_life = 15;
	vader_life = 15;

	// 사망 상태 초기화
	is_luke_dead = 0;
	is_vader_dead = 0;

	// 승리 결과 초기화
	is_luke_win = 0;
	is_vader_win = 0;

	PlaySound(TEXT("yavin.wav"), NULL, SND_ASYNC | SND_LOOP);

	draw_ingame_UI();
	char xwing[3] = { '>','B', '<' };
	char vader[3] = { '<', 'O', '>' };

	int input;

	// 초기 위치

	int old_x1 = 39;
	int new_x1 = 39;
	int old_y1 = 30;
	int new_y1 = 30;

	int old_x2 = 120;
	int new_x2 = 120;
	int old_y2 = 30;
	int new_y2 = 30;

	// 초기 좌표 초기화
	luke_x = 39;
	luke_y = 30;
	vader_x = 120;
	vader_y = 30;

	// 그리기 
	textcolor(BLACK, GRAY1);
	putPlayer(new_x1, new_y1, xwing);
	textcolor(BLACK, GRAY1);
	putPlayer(new_x2, new_y2, vader);

	// 이전 키 입력을 저장
	int p1_before = STOP1;
	int p2_before = STOP2;

	while (1)
	{	
		if (is_luke_dead)
		{
			break;
		}

		if (is_vader_dead)
		{
			break;
		}

		if (_kbhit())
		{
			input = _getch();

			switch (input)
			{
			case UP:
				new_y1 = old_y1 - 1;
				p1_before = UP;
				break;
			case DOWN:
				new_y1 = old_y1 + 1;
				p1_before = DOWN;
				break;
			case RIGHT:
				new_x1 = old_x1 + 1;
				p1_before = RIGHT;
				break;
			case LEFT:
				new_x1 = old_x1 - 1;
				p1_before = LEFT;
				break;
			case STOP1:
				p1_before = STOP1;
				break;
			case SHOOT1:
				shootLaser(SHOOT1);
				break;
			case UP2:
				new_y2 = old_y2 - 1;
				p2_before = UP2;
				break;
			case DOWN2:
				new_y2 = old_y2 + 1;
				p2_before = DOWN2;
				break;
			case RIGHT2:
				new_x2 = old_x2 + 1;
				p2_before = RIGHT2;
				break;
			case LEFT2:
				new_x2 = old_x2 - 1;
				p2_before = LEFT2;
				break;
			case STOP2:
				p2_before = STOP2;
				break;
			case SHOOT2:
				shootLaser(SHOOT2);
				break;
			}
		}
		else
		{
			switch (p1_before)
			{
			case UP:
				new_y1 = old_y1 - 1;
				break;
			case DOWN:
				new_y1 = old_y1 + 1;
				break;
			case RIGHT:
				new_x1 = old_x1 + 1;
				break;
			case LEFT:
				new_x1 = old_x1 - 1;
				break;
			case STOP1:
				break;
			}

			switch (p2_before)
			{
			case UP2:
				new_y2 = old_y2 - 1;
				break;
			case DOWN2:
				new_y2 = old_y2 + 1;
				break;
			case RIGHT2:
				new_x2 = old_x2 + 1;
				break;
			case LEFT2:
				new_x2 = old_x2 - 1;
				break;
			case STOP2:
				break;
			}
		}

		// 루크
		if (new_x1 < P1_x_min + 1)
			new_x1 = old_x1;
		if (new_x1 > P1_x_max - 1) 
			new_x1 = old_x1;

		// y 축 위치 보정
		if (new_y1 < 0 || new_y1 > playArea_y - 2)
			new_y1 = old_y1;

		// 베이더
		if (new_x2 < P2_x_min + 3)
			new_x2 = old_x2;
		if (new_x2 > P2_x_max - 1) 
			new_x2 = old_x2;
		// y 축 위치 보정
		if (new_y2 < 0 || new_y2 > playArea_y - 2)
			new_y2 = old_y2;

		luke_x = new_x1;
		luke_y = new_y1;
		vader_x = new_x2;
		vader_y = new_y2;		

		// 루크 움직임
		textcolor(BLACK, GRAY1);
		erasePlayer(old_x1, old_y1);
		putPlayer(new_x1, new_y1, xwing);

		// 베이더 움직임
		textcolor(BLACK, GRAY1);
		erasePlayer(old_x2, old_y2);
		putPlayer(new_x2, new_y2, vader);

		// 탄환 움직임 갱신
		updateBullets();

		old_x1 = new_x1;
		old_y1 = new_y1;
		old_x2 = new_x2;
		old_y2 = new_y2;

		// 점수 누적
		score++;

		if (score%100 == 0)
		{
			spawnEnemy();
			spawnEnemy();
			spawnEnemy();
			spawnEnemy();
		}

		// 적 갱신
		updateEnemy();

		// UI 업데이트 
		updateUI();

		// 대사 업데이트
		speechScore++;
		speech(speechScore);

		Sleep(30);
	}
}

void result() // 게임 결과 화면
{
	system("cls");

	if (is_vader_win) // 베이더 승리 ( 루크 사망 ) 
	{
		gotoxy(0, 0);
		printf("베이더 승리");
	}
	else if (is_luke_win) // 루크 승리 ( 베이더 사망, 루크 완주 )
	{
		gotoxy(0, 0);
		printf("루크 승리");
	}

	gotoxy(100, 30);
}

void playOpeningCrawl() {
	char* longtime = "오래 전 멀고 먼 은하계에서는...";

	char* crawl[] = {
		"                             STAR WARS: The Death Star Mission",
		" ",
		" ",
		"                                  스타워즈: 데스스타 미션",
		" ",
		" ",
		" ",
		"                         사악한 시스의 군주 다스 시디어스의 계략으로",
		" ",
		"                       제다이 기사 '아나킨 스카이워커' 가 어둠에 물들고",
		" ",
		"                           제다이 기사단과 은하 공화국이 몰락한 이후",
		" ",
		"                  평온했던 은하계는 다시 억압과 전쟁의 소용돌이에 휘말리게 되었다.",
		" ",
		"                  그 암흑의 시대 속에서 은하 제국의 지배에 맞서 싸우는 반란 연합은",
		" ",
		"                         '카시안 안도르' 와 '진 어소' 를 주축으로 구성된",
		" ",
		"                          '로그원' 이라 불린 용감한 첩보원들의 희생으로",
		" ",
		"            제국의 기밀 정보가 저장된 스카리프 행성에서 은하 제국을 상대로 첫 승리를 거두고",
		" ",
		"                   은하 제국이 비밀리에 개발 중인 행성 파괴 병기, '데스스타' 의",
		" ",
		"                            설계도를 목숨을 걸고 탈취하는 데 성공했다.",
		" ",
		" ",
		"               이 설계도에는 '진 어소' 의 아버지이자 데스스타를 설계한 과학자 '갤런 어소'가",
		" ",
		"                  제국의 감시 속에서도 남몰래 심어놓은 치명적인 약점이 담겨 있었고",
		" ",
		"                        반란 연합의 일원이자 얼데란 행성의 의원 '레아 공주' 는",
		" ",
		"                         '데스스타' 의 치명적 약점을 담은 이 귀중한 설계도를",
		" ",
		"                          반란 연합의 기지인 야빈 4 행성으로 운반하기 위해",
		" ",
		"                               밀수꾼 '한 솔로' 와 '츄바카', 그리고 ",
		" ",
		"                      클론 전쟁의 영웅, 제다이 기사 '오비완 케노비' 의 도움으로",
		" ",
		"                         은하 제국의 추격을 피해 목숨을 건 탈출을 감행한다.",
		" ",
		" ",
		"          한편, '다스 베이더' 가 '데스 스타'와 함께 군대를 이끌고 야빈 행성계를 향해 다가오는 가운데",
		" ",
		"                 광활한 은하계의 변방, 타투인 행성의 평범한 소년 '루크 스카이워커' 는",
		" ",
		"                  '오비완 케노비' 의 도움으로 포스라는 신비로운 힘에 눈을 뜨게 되고",
		" ",
		"                        자신도 모르게 운명처럼 이 싸움에 발을 들이게 된다.",
		" ",
		"               그리고 그와 은하계의 운명을 바꿀 거대한 여정이 그의 앞에 펼쳐지게 되는데..."
	};

	char* logo[] =
	{
"@@@@@@@@@@@@@@@@@.=#@@@@@@@@@@@@@@@@@@@@@@@@@@@ @@@.;@@@@@@@ =@@@@@@@--@@@@@@@@@@@@@@@@@@@@@@@@@@@@@",
"@@@@@@@@@@@@@@@@.;@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ @@= @@@@@@@@@.!@@@@@@--@@@@@@@@@@@@@@@:~@@@@@@@@@@@@",
"@@@@@@@@@@@@@@@= @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ @@= @@@@-@@@@.;@@@@@@--@@@@@@@@@@@@@@@@ !@@@@@@@@@@@",
"@@@@@@@@@@@@@@@= @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ @@ #@@@@.=@@@@ @@@@@@--@@@@@:~~~::=@@@@;.@@@@@@@@@@@",
"@@@@@@@@@@@@@@@= @@@@@@ .,,,,,,,..@@@@# ,,,,,,,,@#.@@@@= ;@@@@.$@@@@@--@@@@@.@@@#,:@@@@;.@@@@@@@@@@@",
"@@@@@@@@@@@@@@@= @@@@@@ =@@@@@@@;.@@@@# @@@@@@@@@--@@@@= .@@@@;.@@@@@--@@@@@.@@@@@ @@@@;.@@@@@@@@@@@",
"@@@@@@@@@@@@@@@# ;@@@@@# =@@@@@@;.@@@@# @@@@@@@@@.$@@@@ # @@@@= #@@@@--@@@@@.     *@@@@;.@@@@@@@@@@@",
"@@@@@@@@@@@@@@@@@ @@@@@@~-@@@@@@;.@@@@# @@@@@@@@# @@@@@ #;.@@@@ =@@@@--@@@@@@@@@@@@@@@@ @@@@@@@@@@@@",
"@@@@@@@@@@@@@@@@@$ $@@@@@:~@@@@@;.@@@@# @@@@@@@@,*@@@@--@;.@@@@!-@@@@--@@@@@@@@@@@@@@$ ~@@@@@@@@@@@@",
"@@@@@@@@@@@@@@@@@@- $@@@@$ ;@@@@;.@@@@# @@@@@@@# $@@@@,    =@@@# @@@@--@@@@@@@@@@@@@= -@@@@@@@@@@@@@",
"@@@@$==============,-@@@@@# @@@@;.@@@@# @@@@@@@;.@@@@@#####@@@@@-:@@@--@@@@@,@@@@@@@- ..,;#@@@@@@@@@",
"@@@@-.;;;;;;;;;;;;;;@@@@@@@ @@@@;.@@@@# @@@@@@@-!@@@@@@@@@@@@@@@~-@@@--@@@@@.-@@@@@@@;;;;;;;;;.-@@@@",
"@@@@--@@@@@@@@@@@@@@@@@@@@@ @@@@;.@@@@# @@@@@@@ @@@@@@@@@@@@@@@@@ #@@--@@@@@. .@@@@@@@@@@@@@@@--@@@@",
"@@@@--@@@@@@@@@@@@@@@@@@@@@ @@@@;.@@@@# @@@@@@~~@@@@@@@@@@@@@@@@@,#@@--@@@@@.: ,@@@@@@@@@@@@@@--@@@@",
"@@@@--@@@@@@@@@@@@@@@@@@@# ;@@@@;.@@@@# @@@@@@.;@@@@~       ,@@@@= @@--@@@@@.!@.,@@@@@@@@@@@@@--@@@@",
"@@@@--@@@@@@@@@@@@@@@@@@@  @@@@@;.@@@@# @@@@@= @@@@@ @@@@@@@ #@@@# #@--@@@@@.;@@. @@@@@@@@@@@@--@@@@",
"@@@@-.:::::::::::::::::~ ,@@@@@@; ::::~ @@@@@= :::::-@@@@@@@:~:::: ;@-.::::: ;@@@* :::::::::::.-@@@@",
"@@@@:,,,,,,,,,,,,,,,,,~@@@@@@@@@!,,,,,,,@@@@@,,,,,,,!@@@@@@@=,,,,,,~@:,,,,,,,!@@@@@*,,,,,,,,,,,:@@@@",
"@@@@;,,,,,,,@@@-,,,,,,!@@=,,,,,,,@@@-,,,,,,,,,#@@@@@@@$============#@@@@@@@@@@@@@@~,,,,,,,,,,,,:@@@@",
"@@@@@ ~:::: $@@ ~:::: ;@@!~:::: ;@@# :::::::: #@@@@@@@-.:::::::::::~ .=@@@@@@@@$: ::::::::::::.-@@@@",
"@@@@@,#@@@@.;@--@@@@@@ @@ #@@@@.;@@= @@@@@@@@= @@@@@@@--@@@@@@@@@@@@@~ ;@@@@@@@,~@@@@@@@@@@@@@--@@@@",
"@@@@@= @@@@=,@--@@@@@@ @*,@@@@= @@@-!@@@@@@@@= @@@@@@@--@@@@@@@@@@@@@@# *@@@@@-;@@@@@@@@@@@@@@--@@@@",
"@@@@@# #@@@@ # @@@@@@@;.--@@@@;-@@@ #@@@@=@@@@.;@@@@@@--@@@@@@@@@@@@@@@@ @@@@@ =@@@@@@@@@@@@@@--@@@@",
"@@@@@@.;@@@@:. @@@@@@@!  @@@@@ #@@~-@@@@= @@@@,;@@@@@@--@@@@=......@@@@@:,@@@@ =@@@@@@@@@@@@@@--@@@@",
"@@@@@@$,@@@@! =@@@@@@@@  @@@@*.@@@--@@@@~ =@@@@ @@@@@@--@@@@= ;;;;;~@@@@;.@@@@ =@@@@@=         -@@@@",
"@@@@@@@ @@@@@ =@@@@@@@@*=@@@@--@@# @@@@@  ;@@@@-#@@@@@--@@@@= #####,@@@@;.@@@@ *@@@@@= @@@@@@@@@@@@@",
"@@@@@@@:-@@@@*@@@@@@@@@#=@@@@.$@@# @@@@;,=.@@@@;.@@@@@--@@@@=  .   #@@@@.=@@@@= @@@@@@..@@@@@@@@@@@@",
"@@@@@@@*.@@@@@@@@@ =@@@@@@@@# @@@ =@@@@-~@ @@@@@ $@@@@--@@@@@@@@@@@@@@@*,@@@@@@;,@@@@@@,*@@@@@@@@@@@",
"@@@@@@@@ =@@@@@@@@ =@@@@@@@@ *@@@ =@@@# @@;.@@@@ =@@@@--@@@@@@@@@@@@@@= ;@@@@@@@, @@@@@@ =@@@@@@@@@@",
"@@@@@@@@;;@@@@@@@;  @@@@@@@@ =@@;.@@@@!    .@@@@#.@@@@--@@@@@@@@@@@@@:,!@@@@@@@@# ~@@@@@! @@@@@@@@@@",
"@@@@@@@@# @@@@@@@:-.#@@@@@@;.@@@:~@@@@@@@@@@@@@@# #@@@--@@@@==@@@@@@@:  ,-~::::~-  @@@@@@--@@@@@@@@@",
"@@@@@@@@@-!@@@@@@ @--@@@@@@:!@@@ @@@@@@@@@@@@@@@@--@@@--@@@@= ;@@@@@@@$$$$$$$$$$$$#@@@@@@--@@@@@@@@@",
"@@@@@@@@@--@@@@@~:@$.@@@@@@ @@@!~@@@@@@@@@@@@@@@@!,@@@--@@@@= ,-#@@@@@@@@@@@@@@@@@@@@@@@@--@@@@@@@@@",
"@@@@@@@@@@ #@@@@.;@@ #@@@@!:@@@.;@@@@@@@@@@@@@@@@@ #@@--@@@@= @  @@@@@@@@@@@@@@@@@@@@@@@@-~@@@@@@@@@",
"@@@@@@@@@@ #@@@= @@@*,@@@@.;@@$ @@@@@.,..    -@@@@;-@@--@@@@= @@,.@@@@@@@@@@@@@@@@@@@@@@! @@@@@@@@@@",
"@@@@@@@@@@= @@@;-@@@$ #@@$ @@@!-@@@@.;@@@@@@@.@@@@= @@--@@@@= @@@, =@@@@@@@@@@@@@@@@@@@: $@@@@@@@@@@",
"@@@@@@@@@@= ... #@@@@. ..  @@@ ..... ;@@@@@@@ ....  !@- ....  @@@@# ................... #@@@@@@@@@@@",
"@@@@@@@@@@@!;;;;@@@@@!;;;;@@@@@@#$**!@@@@@@@@=;;;;;;=@*;;;;;;;@@@@@#;;;;;;;;;;;;;;;;;;@@@@@@@@@@@@@@"
	};


	removeCursor();

	gotoxy(67, 24);
	textcolor(CYAN1, BLACK);
	printf("%s", longtime);
	gotoxy(120, 48);
	printf("inspired by \"STAR WARS\" by George Lucas");
	Sleep(5000);

	system("cls");
	textcolor(YELLOW1, BLACK);
	gotoxy(28, 4);

	PlaySound(TEXT("maintheme.wav"), NULL, SND_ASYNC | SND_LOOP);

	for (int i = 0; i < WIDTH - 1; i++)
	{
		for (int j = 0; j < HEIGHT - 1; j++)
		{
			int rannum = rand() % 100;

			if (rannum == 1)
			{
				gotoxy(i, j);
				printf("*");
			}
		}
	}

	for (int i = 0; i < 38; i++)
	{
		gotoxy(28, 4 + i);
		printf("%s\n", logo[i]);
	}

	// 로고가 그려지는 좌표들!!
	int left = 28;
	int top = 4;
	int right = left + 102;
	int bottom = top + 37;

	Sleep(3000);

	while (left <= right && top <= bottom) {
		for (int y = top; y <= bottom; y++) {
			gotoxy(left, y);
			putchar(' ');
			Sleep(0.9);
		}
		left++;
		
		for (int x = left; x <= right; x++) {
			gotoxy(x, bottom);
			putchar(' ');
			Sleep(0.9);
		}
		bottom--;

		for (int y = bottom; y >= top; y--) {
			gotoxy(right, y);
			putchar(' ');
			Sleep(0.9);
		}
		right--;

		for (int x = right; x >= left; x--) {
			gotoxy(x, top);
			putchar(' ');
			Sleep(0.9);
		}
		top++;
	}

	system("cls");

	// 크롤 텍스트 스크롤
	int i;
	int crawlLine = sizeof(crawl) / sizeof(crawl[0]); // 크롤 줄 수
	int totalScroll = HEIGHT + crawlLine;

	for (int offset = 0; offset < totalScroll; offset++) {
		system("cls");

		for (int i = 0; i < WIDTH - 1; i++)
		{
			for (int j = 0; j < HEIGHT - 1; j++)
			{
				int rannum = rand() % 100;

				if (rannum == 1)
				{
					gotoxy(i, j);
					printf("*");
				}
			}
		}

		textcolor(YELLOW1, BLACK);

		for (i = 0; i < crawlLine; i++) {
			int y = HEIGHT - offset + i;
			if (y >= 0 && y < HEIGHT) {
				gotoxy(36, y);  
				printf("%s", crawl[i]);
			}
		}

		if (offset <= 70)
		{
			Sleep(630);
		}
		else
		{
			Sleep(100);
		}

		if (_kbhit() == 1)
		{
			break; // 아무 키나 눌러 스킵
		}
	}

	system("cls");
}

void howTo()
{
	system("cls");

	int padding = 20;
	while (1)
	{
		textcolor(YELLOW1, BLACK);

		system("cls");

		for (int i = 0; i < WIDTH - 1; i++)
		{
			for (int j = 0; j < HEIGHT - 1; j++)
			{
				int rannum = rand() % 100;

				if (rannum == 1)
				{
					gotoxy(i, j);
					printf("*");
				}
			}
		}

		textcolor(YELLOW1, BLACK);
		gotoxy(padding, 3);
		printf("스타워즈 : 데스스타 미션");
		textcolor(CYAN1, BLACK);
		gotoxy(padding, 5);
		printf("> 두 명의 플레이어가 서로 경쟁하는 슈팅 게임입니다.");
		gotoxy(padding, 7);
		textcolor(GRAY1, BLACK);
		printf(">B<");
		textcolor(RED1, BLACK);
		printf(" - - - - - -");
		gotoxy(padding, 9);
		textcolor(CYAN1, BLACK);
		printf("> 플레이어 1 ( 루크 스카이워커 ) : WASD 로 이동합니다. V 키를 이용하여 공격, C 키를 이용하여 제자리에 멈춥니다.");
		gotoxy(padding, 11);
		textcolor(GRAY1, BLACK);
		printf("<O>");
		textcolor(GREEN1, BLACK);
		printf(" - - - - - -");
		gotoxy(padding, 13);
		textcolor(CYAN1, BLACK);
		printf("> 플레이어 2 ( 다스 베이더 ) : 방향키로 이동합니다. N 키를 이용하여 공격, M 키를 이용하여 제자리에 멈춥니다.");
		gotoxy(padding, 15);
		textcolor(GRAY1, BLACK);
		printf("|O|");
		textcolor(GREEN1, BLACK);
		printf(" - - - - - -");
		gotoxy(padding, 17);
		textcolor(CYAN1, BLACK);
		printf("> 적 ( 타이 파이터 ) : 일정 확률로 루크 스카이워커 앞에 나타납니다. 루크 스카이워커에게 공격을 발사합니다.");
		gotoxy(padding, 19);
		textcolor(GRAY1, BLACK);
		printf("게임 종료 조건");
		textcolor(CYAN1, BLACK);
		gotoxy(padding, 21);
		printf("> 플레이어 1 게임 승리 조건 : 루크 스카이워커는 최대한 오래 살아남아 점수에 도달한 경우 승리로 간주됩니다.");
		gotoxy(padding, 23);
		printf("> 플레이어 2 게임 승리 조건 : 다스 베이더는 공격을 발사하여 제한 시간 내에 루크 스카이워커를 처치한 경우 승리로 간주됩니다.");

		gotoxy( WIDTH - 50, HEIGHT - 5);
		printf("아무 키나 눌러 메뉴로 돌아갑니다.");

		if (_kbhit())
		{
			playEffect(r2sound1);
			break; // 아무 키나 눌러서 나가기
		}

		Sleep(150);
	}
}


void mainMenu()
{
	int isSelected = 0; // 메뉴 선택 ( 0이면 게임 방법, 1 이면 게임 시작 )
	int padding = 30;

	while (1)
	{
		system("cls");

		textcolor(YELLOW1, BLACK);
		for (int i = 0; i < WIDTH - 1; i++)
		{
			for (int j = 0; j < HEIGHT - 1; j++)
			{
				int rannum = rand() % 100;

				if (rannum == 1)
				{
					gotoxy(i, j);
					printf("*");
				}
			}
		}

		textcolor(CYAN1, BLACK);

		gotoxy(140, 47);
		printf("컴공 2491111 원진우");
		gotoxy(120, 48);
		printf("inspired by \"STAR WARS\" by George Lucas");

		gotoxy(padding, 10);
		printf("스타워즈 : 데스스타 미션");

		gotoxy(padding, 15);
		printf("게임 방법");
		gotoxy(padding, 20);
		printf("게임 시작");
		gotoxy(padding, 40);
		printf("위/아래 방향 키로 이동, 스페이스 바로 선택");

		if (isSelected)
		{
			gotoxy(padding - 2, 15);
			putchar(BLANK);
			gotoxy(padding - 2, 20);
			printf(">");
		}
		else
		{
			gotoxy(padding - 2, 15);
			printf(">");
			gotoxy(padding - 2, 20);
			putchar(BLANK);
		}

		int input = 0;
		if (_kbhit())
		{
			input = _getch();
		}

		switch (input)
		{
		case UP2:
			if (isSelected)
			{
				playEffect(r2sound);
				isSelected = 0;
			}

			break;

		case DOWN2:
			if (!isSelected)
			{
				playEffect(r2sound);
				isSelected = 1;
			}
			
			break;
		default:
			break;
		}

		if (input == SPACE)
		{
			if (isSelected)
			{
				playEffect(saberonsound);
				system("cls");
				break;
			}
			else
			{
				playEffect(r2sound1);
				howTo();
			}
		}

		Sleep(300);
	}
}

void main()
{
	srand((unsigned)time(NULL));

	removeCursor();
	playOpeningCrawl();
	mainMenu();
	startgame();
	result();
}
