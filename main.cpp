// udpserver.cpp

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <winsock.h>
#include <signal.h>
#include <fcntl.h>
#include <conio.h>
#include <time.h>
#pragma comment (lib, "Ws2_32.lib")

#define PORT_NUMBER 55555

  // マクロ定義
#define FIELD_H         20      // フィールドの高さ
#define FIELD_W         10      // フィールドの幅
#define FIELD_X         5       // フィールド表示桁位置
#define FIELD_Y         2       // フィールド表示行位置
#define GAME_CLEAR_PONT 100     // ゲームクリアになる得点
#define EXPAND_POINT    4       // 連続削除時の点数の拡大率
#define FALL_TIMING     300     // ブロックが自動落下するタイミング(m秒)

#define CURSOR_ERASE	printf("\033[?25l")		// カーソル消去
#define CURSOR_DISPLAY	printf("\033[?25h")		// カーソル表示
#define COLOR_RESET		printf("\033[0m")		// 色設定のリセット
#define WINDOW_CLEAR	printf("\033[2J")		// 画面クリア

#define BLACK			0
#define DARK_RED		1
#define DARK_GREEN		2
#define DARK_YELLOW		3
#define DARK_BLUE		4
#define DARK_MAGENTA	5
#define DARK_CYAN		6
#define LIGHT_GRAY		7
#define DARK_GRAY		8
#define LIGHT_RED		9
#define LIGHT_GREEN		10
#define LIGHT_YELLOW	11
#define LIGHT_BLUE		12
#define LIGHT_MAGENTA	13
#define LIGHT_CYAN		14
#define WHITE			15

#define BLOCK_SIZE		4
#define BLOCK_TYPE		9
#define EMPTY           -1
#define PIECE0          DARK_RED
#define PIECE1          DARK_GREEN
#define PIECE2          DARK_YELLOW
#define PIECE3          DARK_BLUE
#define PIECE4          DARK_MAGENTA
#define PIECE5          DARK_CYAN
#define PIECE6          LIGHT_GRAY
#define PIECE7          LIGHT_CYAN
#define PIECE_DEL       BLACK           // 削除中のピースの色

// ブロック
int    Block_list[BLOCK_TYPE][BLOCK_SIZE][BLOCK_SIZE] = {
	// タイプ０ブロック(I型)
	{
		{  EMPTY, PIECE0,  EMPTY,  EMPTY },     //   ■
		{  EMPTY, PIECE0,  EMPTY,  EMPTY },     //   ■
		{  EMPTY, PIECE0,  EMPTY,  EMPTY },     //   ■
		{  EMPTY, PIECE0,  EMPTY,  EMPTY },     //   ■
	},
	// タイプ１ブロック(T型)
	{
		{  EMPTY,  EMPTY,  EMPTY,  EMPTY },     //   
		{ PIECE1, PIECE1, PIECE1,  EMPTY },     // ■■■
		{  EMPTY, PIECE1,  EMPTY,  EMPTY },     //   ■
		{  EMPTY,  EMPTY,  EMPTY,  EMPTY },     //
	},
	// タイプ２ブロック(口型)
	{
		{  EMPTY,  EMPTY,  EMPTY,  EMPTY },     //   
		{  EMPTY, PIECE2, PIECE2,  EMPTY },     //   ■■
		{  EMPTY, PIECE2, PIECE2,  EMPTY },     //   ■■
		{  EMPTY,  EMPTY,  EMPTY,  EMPTY },     //
	},
	// タイプ３ブロック(L型)
	{
		{  EMPTY, PIECE3,  EMPTY,  EMPTY },     //   ■
		{  EMPTY, PIECE3,  EMPTY,  EMPTY },     //   ■
		{  EMPTY, PIECE3, PIECE3,  EMPTY },     //   ■■
		{  EMPTY,  EMPTY,  EMPTY,  EMPTY },     //   
	},
	// タイプ４ブロック(逆L型)
	{
		{  EMPTY,  EMPTY, PIECE4,  EMPTY },     //   　■
		{  EMPTY,  EMPTY, PIECE4,  EMPTY },     //   　■
		{  EMPTY, PIECE4, PIECE4,  EMPTY },     // 　■■
		{  EMPTY,  EMPTY,  EMPTY,  EMPTY },     //   
	},
	// タイプ５ブロック(N型)
	{
		{  EMPTY, PIECE5,  EMPTY,  EMPTY },     //   ■  
		{  EMPTY, PIECE5, PIECE5,  EMPTY },     //   ■■
		{  EMPTY,  EMPTY, PIECE5,  EMPTY },     //     ■
		{  EMPTY,  EMPTY,  EMPTY,  EMPTY },     //   
	},
	// タイプ６ブロック(逆N型)
	{
		{  EMPTY,  EMPTY, PIECE6,  EMPTY },     //     ■
		{  EMPTY, PIECE6, PIECE6,  EMPTY },     //   ■■
		{  EMPTY, PIECE6,  EMPTY,  EMPTY },     //   ■
		{  EMPTY,  EMPTY,  EMPTY,  EMPTY },     //   
	},
	// タイプ7ブロック(1型)
	{
		{  EMPTY,  EMPTY, EMPTY,  EMPTY },			//     
		{  EMPTY, EMPTY, EMPTY,  EMPTY },			//   
		{  PIECE7, EMPTY,  PIECE7,  EMPTY },			//   ■
		{  PIECE7,  PIECE7,  PIECE7,  EMPTY },		//   
	},
	// タイプ8ブロック(十字型)
	{
		{  EMPTY,  EMPTY, EMPTY,  EMPTY },			//     
		{  EMPTY, PIECE7, EMPTY,  EMPTY },			//   
		{  PIECE7, PIECE7,  PIECE7,  EMPTY },			//   ■
		{  EMPTY,  PIECE7,  EMPTY,  EMPTY },		//   
	},
};

int     Field[FIELD_H][FIELD_W];        // フィールド
int     Block[BLOCK_SIZE][BLOCK_SIZE];  // 表示するブロック
int     Block_X, Block_Y;   // ブロック位置(フィールド内相対位置)
int     Point = 0;          // 得点

enum { GAME_DOING, GAME_CLEAR, GAME_OVER };
int     Status = GAME_DOING;

// 関数プロトタイプ宣言
void wait_enter(void);
void set_text_color(int color);
void set_back_color(int color);
void set_cursor_pos(int x, int y);

void rotate_block(void);
void print_field(void);
void print_block(void);
void create_block(void);
bool can_move_block(int x, int y);
bool operate_block(void);
void fixed_block(void);
void delete_lines(void);
bool fix_check_game_end(void);
void print_end(void);
int input_key(void);

//	一旦全部グローバルにしておく。
WSADATA wsaData;
HOSTENT* lpHost;
IN_ADDR inaddr;

SOCKET s;
SOCKADDR_IN addrin;
SOCKADDR_IN from;
int fromlen;
int nRtn;
u_short port;
char szBuf[256], szIP[256];

int main()
{
	
	

	{
		/*printf(" Enter Any Press: ");
		gets_s(szBuf);*/

		//	delete
		/*gets_s(szBuf);

		port = atoi(szBuf);*/
	}

	system("cls");                      // エスケープシーケンスを使用可能にする
	CURSOR_ERASE;                       // カーソル非表示

	srand((unsigned int)time(NULL));    // 乱数の種の初期化
	memset(Field, EMPTY, sizeof(Field));// フィールドのリセット
	create_block();

	//	add
	port = PORT_NUMBER;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		perror(" WSAStartup Error\n");
		return -1;
	}
	//	printf
	{
		printf(" wVersion: %d.%d\n",
			LOBYTE(wsaData.wVersion),
			HIBYTE(wsaData.wVersion));
		printf(" wHighVersion: %d.%d\n",
			LOBYTE(wsaData.wHighVersion),
			HIBYTE(wsaData.wHighVersion));
		printf(" szDescription: %s\n", wsaData.szDescription);
		printf(" szSystemStatus: %s\n", wsaData.szSystemStatus);
		printf(" iMaxSockets: %d\n", wsaData.iMaxSockets);
		printf(" iMaxUdpDg: %d\n", wsaData.iMaxUdpDg);
		printf(" WSAStartup Succeeded\n");
		printf(" PortNumber: %d\n", PORT_NUMBER);	//	add
		gethostname(szBuf, (int)sizeof(szBuf));
		printf("\n Host Name: %s\n", szBuf);
		lpHost = gethostbyname(szBuf);
		memcpy(&inaddr, lpHost->h_addr_list[0], 4);
		strcpy_s(szIP, inet_ntoa(inaddr));
		printf(" IP Address: %s\n", szIP);
	}
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror(" socket Failure\n");
		WSACleanup();
		return -2;
	}

	memset(&addrin, 0, sizeof(addrin));
	addrin.sin_port = htons(port);
	addrin.sin_family = AF_INET;
	addrin.sin_addr.s_addr = htonl(INADDR_ANY);

	//	バインド
	nRtn = bind(s, (LPSOCKADDR)&addrin, (int)sizeof(addrin));
	if (nRtn == SOCKET_ERROR) {
		perror(" bind Error\n");
		closesocket(s);
		WSACleanup();
		return -3;
	}

	//	add
	//	ノンブロッキング処理
	u_long val = 1;
	ioctlsocket(s, FIONBIO, &val);

	while (1) {
		//	recvfrom
		{
			fromlen = (int)sizeof(from);
			nRtn = recvfrom(s,
				szBuf,
				(int)sizeof(szBuf) - 1,
				0,
				(SOCKADDR*)&from,
				&fromlen);

			//	add
			if (nRtn < 1) {
				if (errno == EAGAIN) {
					printf("MADA KONAI\n");
				}
			}
			else 
			{	//	受け取ったらここ
				printf("%s>%s\n", inet_ntoa(from.sin_addr), szBuf);
				
				//	処理する場所
				{
					if (!operate_block()) {
						break;
					}
				}

				print_field();      // フィールドの表示
				print_block();      // アクティブブロックの表示

				

				// send
				{
					nRtn = sendto(s, szBuf, (int)strlen(szBuf) + 1, 0,
						(LPSOCKADDR)&from, sizeof(from));

					if (nRtn != (int)strlen(szBuf) + 1) {
						perror("Send Srror\n");
						closesocket(s);
						WSACleanup();
						return -4;
					}
					if (strcmp(szBuf, "end") == 0) {
						printf("終了します\n");
						_getch();
						break;
					}
				}

				szBuf[nRtn] = '\0';
			}

			if (strcmp(szBuf, "end") == 0) {
				printf(" Terminate Server\n");
				break;
			}
		}

		

		//	delete
		{
			/*if (nRtn == SOCKET_ERROR) {
				perror(" recvform Error\n");
				closesocket(s);
				WSACleanup();
				return -4;
			}*/

			//	add
			//printf("%s>%s\n", inet_ntoa(from.sin_addr), szBuf);
		}		
	}

	print_end();            // ゲームステータスに応じた終了メッセージを表示

	closesocket(s);
	WSACleanup();
	printf(" Termination\n");

	return 0;
}
//=============================================
// 改行キー待ち
//=============================================
void wait_enter(void)
{
	while (_getch() != 0x0d);
}
//=============================================
// 前景色設定(0以上15以下の色番号を指定、範囲外は補正)
//=============================================
void set_text_color(int color)
{
	if (color < 0) {
		color = 30;
	}
	else if (color < 8) {
		color += 30;
	}
	else if (color < 16) {
		color = (color - 8) + 90;
	}
	else {
		color = 97;
	}
	printf("\033[%dm", color);
}
//=============================================
// 背景色設定(0以上15以下の色番号を指定、範囲外は補正)
//=============================================
void set_back_color(int color)
{
	if (color < 0) {
		color = 40;
	}
	else if (color < 8) {
		color += 40;
	}
	else if (color < 16) {
		color = (color - 8) + 100;
	}
	else {
		color = 107;
	}
	printf("\033[%dm", color);
}
//=============================================
// カーソル位置設定
//=============================================
void set_cursor_pos(int x, int y)
{
	printf("\033[%d;%dH", y, x);
}
//==============================================================
// Blockを右に90度回転させる
//==============================================================
void rotate_block(void)
{
	int    tmp[BLOCK_SIZE][BLOCK_SIZE];

	// Block配列をtmp配列にコピーする
	memcpy(tmp, Block, sizeof(tmp));

	// tmp配列を右に90度回転させたものをBlock配列に入れる
	for (int i = 0; i < BLOCK_SIZE; i++) {
		for (int j = 0; j < BLOCK_SIZE; j++) {
			Block[j][BLOCK_SIZE - i - 1] = tmp[i][j];
		}
	}
	if (!can_move_block(Block_X, Block_Y)) {
		memcpy(Block, tmp, sizeof(Block));
	}
}
//==============================================================
// Fieldを表示する
//==============================================================
void print_field(void)
{
	int         x = FIELD_X, y = FIELD_Y;

	// フィールドの表示(固定化したブロック含む)
	set_back_color(DARK_GRAY);
	for (int i = 0; i < FIELD_H; i++) {
		set_cursor_pos(x, y++);
		for (int j = 0; j < FIELD_W; j++) {
			if (Field[i][j] != EMPTY) {
				set_text_color(Field[i][j]);
				printf("■");
			}
			else {
				printf("  ");
			}
		}
	}
	COLOR_RESET;
	printf("\n\n得点：%4d", Point);
}
//==============================================================
// アクティブブロック(固定化されていないブロック)を表示する
//==============================================================
void print_block(void)
{
	int         x = FIELD_X, y = FIELD_Y;

	x += Block_X * 2;
	y += Block_Y;
	for (int i = 0; i < BLOCK_SIZE; i++) {
		for (int j = 0; j < BLOCK_SIZE; j++) {
			set_cursor_pos(x + (j * 2), y + i);
			if (Block[i][j] != EMPTY) {
				set_text_color(Block[i][j]);
				printf("■");
			}
		}
	}
	COLOR_RESET;
}
//==============================================================
// 新規ブロックの作成
//==============================================================
void create_block(void)
{
	int     type;   // ブロックのタイプ番号
	int     r_num;  // 角度パターン

	type = rand() % BLOCK_TYPE;
	r_num = rand() % 4;    // 角度パターンは4種(0°,90°,180°,270°)

	// 表示する位置を最上行の中央にする
	Block_X = (FIELD_W - BLOCK_SIZE) / 2;
	Block_Y = 0;

	// Block配列に求めたtypeのブロックを入れる
	memcpy(Block, Block_list[type], sizeof(Block));

	// 角度パターン回数分、右に90度ずつ回転させる
	for (int i = 0; i < r_num; i++) {
		rotate_block();
	}
}
//==============================================================
// 指定位置にブロックを表示できるか調べる
//==============================================================
bool can_move_block(int x, int y)
{
	for (int i = 0; i < BLOCK_SIZE; i++) {
		for (int j = 0; j < BLOCK_SIZE; j++) {
			if (Block[i][j] != EMPTY) {
				if (y + i < 0 || y + i >= FIELD_H ||
					x + j < 0 || x + j >= FIELD_W) {
					return false;
				}
				if (Field[y + i][x + j] != EMPTY) {
					return false;
				}
			}
		}
	}
	return true;
}
//==============================================================
// 下移動できなくなった時の処理(ブロック固定化、新規ブロックの作成)
// 新規ブロックが移動できな場合はゲームオーバーに、
// 得点がゲームクリア得点になった場合はゲームクリアにする
//==============================================================
bool fix_check_game_end(void)
{
	Sleep(100);
	fixed_block();
	if (!can_move_block(Block_X, Block_Y)) {
		print_field();          // 最後に固定化したブロックを表示するため
		Status = GAME_OVER;
		return false;
	}
	if (Point >= GAME_CLEAR_PONT) {
		Point = GAME_CLEAR_PONT;
		Status = GAME_CLEAR;
		return false;
	}
	return true;
}
//==============================================================
// キー操作でブロックを動かす
//==============================================================
bool operate_block(void)
{
	// 入力促進メッセージ表示とキー入力
	set_cursor_pos(1, FIELD_Y + FIELD_H + 2);
	printf("a(←),s(↓),d(→),x(回転),スペース(高速落下),*(終了) ");
	

	switch (szBuf[0]) {
	case '*':   // 終了
		return false;

	case 'a':   // 左移動
		if (can_move_block(Block_X - 1, Block_Y)) {
			Block_X--;
		}
		break;
	case 'd':   // 右移動
		if (can_move_block(Block_X + 1, Block_Y)) {
			Block_X++;
		}
		break;
	case 's':   // 下移動
		if (can_move_block(Block_X, Block_Y + 1)) {
			Block_Y++;
		}
		else {
			return fix_check_game_end();    // ブロックを固定化しゲーム終了か調べる
		}
		break;
	case ' ':   // 高速落下
		while (can_move_block(Block_X, Block_Y + 1)) {
			Block_Y++;
			print_field();      // フィールドの表示
			print_block();      // アクティブブロックの表示
			Sleep(20);
		}
		return fix_check_game_end();    // ブロックを固定化しゲーム終了か調べる

	case 'x':   // 回転	
	case 'm':
		rotate_block();
		break;
	default:
		break;
	}
	return true;
}
//==============================================================
// Blockのフィールド固定化
//==============================================================
void fixed_block(void)
{
	// ブロックピースのある位置を固定化ピースにする
	for (int i = 0; i < BLOCK_SIZE; i++) {
		for (int j = 0; j < BLOCK_SIZE; j++) {
			if (Block[i][j] != EMPTY) {
				Field[i + Block_Y][j + Block_X] = Block[i][j];
			}
		}
	}
	delete_lines();
	create_block();
}
//==============================================================
// Fieldに横一列ピースが並んだら削除する（得点アップ）
// （横一列になったピースは、BLOCK_DELを表示してから削除する）
//==============================================================
void delete_lines(void)
{
	int     add_point = 1;      // 追加得点

	for (int i = 0; i < FIELD_H; i++) {
		bool    complete = true;

		for (int j = 0; j < FIELD_W; j++) {
			if (Field[i][j] == EMPTY) {
				complete = false;   // この行は横一列並んでいない
				break;
			}
		}
		// EMPTYが１個もなかった場合は横一列にPIECE_DELをセットする
		if (complete) {
			memset(Field[i], PIECE_DEL, sizeof(Field[i]));

			print_field();
			Sleep(200);

			// 上部のデータで上書きする（削除する動きになる）
			for (int j = i; j > 0; j--) {
				memcpy(Field[j], Field[j - 1], sizeof(Field[j]));
			}
			memset(Field[0], EMPTY, sizeof(Field[0]));  // 最上行は空にする

			Point += add_point;
			add_point *= EXPAND_POINT;  // 連続削除の場合は追加得点が増加

			print_field();
			Sleep(200);
		}
	}
}
//==============================================================
// ゲーム終了時のメッセージ表示
//==============================================================
void print_end(void)
{
	set_cursor_pos(1, 1);
	switch (Status) {
	case GAME_CLEAR:
		printf("ゲームクリア　ヾ(o´∀`o)ﾉ　");
		break;
	case GAME_OVER:
		printf("ゲームオーバー  (-_-;)°°°");
		break;
	default:
		printf("ゲーム終了");
		break;
	}
}
//==============================================================
// ダイレクトキー入力
//==============================================================
int input_key(void)
{
	int     key;
	int     now_time, pre_time;

	now_time = pre_time = clock();
	while (true) {
		if (_kbhit()) {
			key = _getch();
			break;
		}
		now_time = clock();
		if (now_time - pre_time > FALL_TIMING) {
			key = 's';  // ↓キーと同じ動きにする
			break;
		}
	}
	switch (key) {
	case 0xe0:
		switch (_getch()) {
		case 0x4b:  // ←キー
			key = 'a';
			break;
		case 0x4d:  // →キー
			key = 'd';
			break;
		case 0x50:  // ↓キー
			key = 's';
			break;
		default:
			key = 0;
		}
		break;
	case 0x1b:  // ESCキー
		key = '*';
		break;
	case '*':   // 終了
	case 'a':   // 左移動
	case 'd':   // 右移動
	case 's':   // 下移動
	case ' ':   // 高速落下
	case 'x':   // 回転
		break;
	default:
		key = 0;
		break;
	}
	return key;
}