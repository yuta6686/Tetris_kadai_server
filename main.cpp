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

  // �}�N����`
#define FIELD_H         20      // �t�B�[���h�̍���
#define FIELD_W         10      // �t�B�[���h�̕�
#define FIELD_X         5       // �t�B�[���h�\�����ʒu
#define FIELD_Y         2       // �t�B�[���h�\���s�ʒu
#define GAME_CLEAR_PONT 100     // �Q�[���N���A�ɂȂ链�_
#define EXPAND_POINT    4       // �A���폜���̓_���̊g�嗦
#define FALL_TIMING     300     // �u���b�N��������������^�C�~���O(m�b)

#define CURSOR_ERASE	printf("\033[?25l")		// �J�[�\������
#define CURSOR_DISPLAY	printf("\033[?25h")		// �J�[�\���\��
#define COLOR_RESET		printf("\033[0m")		// �F�ݒ�̃��Z�b�g
#define WINDOW_CLEAR	printf("\033[2J")		// ��ʃN���A

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
#define PIECE_DEL       BLACK           // �폜���̃s�[�X�̐F

// �u���b�N
int    Block_list[BLOCK_TYPE][BLOCK_SIZE][BLOCK_SIZE] = {
	// �^�C�v�O�u���b�N(I�^)
	{
		{  EMPTY, PIECE0,  EMPTY,  EMPTY },     //   ��
		{  EMPTY, PIECE0,  EMPTY,  EMPTY },     //   ��
		{  EMPTY, PIECE0,  EMPTY,  EMPTY },     //   ��
		{  EMPTY, PIECE0,  EMPTY,  EMPTY },     //   ��
	},
	// �^�C�v�P�u���b�N(T�^)
	{
		{  EMPTY,  EMPTY,  EMPTY,  EMPTY },     //   
		{ PIECE1, PIECE1, PIECE1,  EMPTY },     // ������
		{  EMPTY, PIECE1,  EMPTY,  EMPTY },     //   ��
		{  EMPTY,  EMPTY,  EMPTY,  EMPTY },     //
	},
	// �^�C�v�Q�u���b�N(���^)
	{
		{  EMPTY,  EMPTY,  EMPTY,  EMPTY },     //   
		{  EMPTY, PIECE2, PIECE2,  EMPTY },     //   ����
		{  EMPTY, PIECE2, PIECE2,  EMPTY },     //   ����
		{  EMPTY,  EMPTY,  EMPTY,  EMPTY },     //
	},
	// �^�C�v�R�u���b�N(L�^)
	{
		{  EMPTY, PIECE3,  EMPTY,  EMPTY },     //   ��
		{  EMPTY, PIECE3,  EMPTY,  EMPTY },     //   ��
		{  EMPTY, PIECE3, PIECE3,  EMPTY },     //   ����
		{  EMPTY,  EMPTY,  EMPTY,  EMPTY },     //   
	},
	// �^�C�v�S�u���b�N(�tL�^)
	{
		{  EMPTY,  EMPTY, PIECE4,  EMPTY },     //   �@��
		{  EMPTY,  EMPTY, PIECE4,  EMPTY },     //   �@��
		{  EMPTY, PIECE4, PIECE4,  EMPTY },     // �@����
		{  EMPTY,  EMPTY,  EMPTY,  EMPTY },     //   
	},
	// �^�C�v�T�u���b�N(N�^)
	{
		{  EMPTY, PIECE5,  EMPTY,  EMPTY },     //   ��  
		{  EMPTY, PIECE5, PIECE5,  EMPTY },     //   ����
		{  EMPTY,  EMPTY, PIECE5,  EMPTY },     //     ��
		{  EMPTY,  EMPTY,  EMPTY,  EMPTY },     //   
	},
	// �^�C�v�U�u���b�N(�tN�^)
	{
		{  EMPTY,  EMPTY, PIECE6,  EMPTY },     //     ��
		{  EMPTY, PIECE6, PIECE6,  EMPTY },     //   ����
		{  EMPTY, PIECE6,  EMPTY,  EMPTY },     //   ��
		{  EMPTY,  EMPTY,  EMPTY,  EMPTY },     //   
	},
	// �^�C�v7�u���b�N(1�^)
	{
		{  EMPTY,  EMPTY, EMPTY,  EMPTY },			//     
		{  EMPTY, EMPTY, EMPTY,  EMPTY },			//   
		{  PIECE7, EMPTY,  PIECE7,  EMPTY },			//   ��
		{  PIECE7,  PIECE7,  PIECE7,  EMPTY },		//   
	},
	// �^�C�v8�u���b�N(�\���^)
	{
		{  EMPTY,  EMPTY, EMPTY,  EMPTY },			//     
		{  EMPTY, PIECE7, EMPTY,  EMPTY },			//   
		{  PIECE7, PIECE7,  PIECE7,  EMPTY },			//   ��
		{  EMPTY,  PIECE7,  EMPTY,  EMPTY },		//   
	},
};

int     Field[FIELD_H][FIELD_W];        // �t�B�[���h
int     Block[BLOCK_SIZE][BLOCK_SIZE];  // �\������u���b�N
int     Block_X, Block_Y;   // �u���b�N�ʒu(�t�B�[���h�����Έʒu)
int     Point = 0;          // ���_

enum { GAME_DOING, GAME_CLEAR, GAME_OVER };
int     Status = GAME_DOING;

// �֐��v���g�^�C�v�錾
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

//	��U�S���O���[�o���ɂ��Ă����B
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

	system("cls");                      // �G�X�P�[�v�V�[�P���X���g�p�\�ɂ���
	CURSOR_ERASE;                       // �J�[�\����\��

	srand((unsigned int)time(NULL));    // �����̎�̏�����
	memset(Field, EMPTY, sizeof(Field));// �t�B�[���h�̃��Z�b�g
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

	//	�o�C���h
	nRtn = bind(s, (LPSOCKADDR)&addrin, (int)sizeof(addrin));
	if (nRtn == SOCKET_ERROR) {
		perror(" bind Error\n");
		closesocket(s);
		WSACleanup();
		return -3;
	}

	//	add
	//	�m���u���b�L���O����
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
			{	//	�󂯎�����炱��
				printf("%s>%s\n", inet_ntoa(from.sin_addr), szBuf);
				
				//	��������ꏊ
				{
					if (!operate_block()) {
						break;
					}
				}

				print_field();      // �t�B�[���h�̕\��
				print_block();      // �A�N�e�B�u�u���b�N�̕\��

				

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
						printf("�I�����܂�\n");
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

	print_end();            // �Q�[���X�e�[�^�X�ɉ������I�����b�Z�[�W��\��

	closesocket(s);
	WSACleanup();
	printf(" Termination\n");

	return 0;
}
//=============================================
// ���s�L�[�҂�
//=============================================
void wait_enter(void)
{
	while (_getch() != 0x0d);
}
//=============================================
// �O�i�F�ݒ�(0�ȏ�15�ȉ��̐F�ԍ����w��A�͈͊O�͕␳)
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
// �w�i�F�ݒ�(0�ȏ�15�ȉ��̐F�ԍ����w��A�͈͊O�͕␳)
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
// �J�[�\���ʒu�ݒ�
//=============================================
void set_cursor_pos(int x, int y)
{
	printf("\033[%d;%dH", y, x);
}
//==============================================================
// Block���E��90�x��]������
//==============================================================
void rotate_block(void)
{
	int    tmp[BLOCK_SIZE][BLOCK_SIZE];

	// Block�z���tmp�z��ɃR�s�[����
	memcpy(tmp, Block, sizeof(tmp));

	// tmp�z����E��90�x��]���������̂�Block�z��ɓ����
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
// Field��\������
//==============================================================
void print_field(void)
{
	int         x = FIELD_X, y = FIELD_Y;

	// �t�B�[���h�̕\��(�Œ艻�����u���b�N�܂�)
	set_back_color(DARK_GRAY);
	for (int i = 0; i < FIELD_H; i++) {
		set_cursor_pos(x, y++);
		for (int j = 0; j < FIELD_W; j++) {
			if (Field[i][j] != EMPTY) {
				set_text_color(Field[i][j]);
				printf("��");
			}
			else {
				printf("  ");
			}
		}
	}
	COLOR_RESET;
	printf("\n\n���_�F%4d", Point);
}
//==============================================================
// �A�N�e�B�u�u���b�N(�Œ艻����Ă��Ȃ��u���b�N)��\������
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
				printf("��");
			}
		}
	}
	COLOR_RESET;
}
//==============================================================
// �V�K�u���b�N�̍쐬
//==============================================================
void create_block(void)
{
	int     type;   // �u���b�N�̃^�C�v�ԍ�
	int     r_num;  // �p�x�p�^�[��

	type = rand() % BLOCK_TYPE;
	r_num = rand() % 4;    // �p�x�p�^�[����4��(0��,90��,180��,270��)

	// �\������ʒu���ŏ�s�̒����ɂ���
	Block_X = (FIELD_W - BLOCK_SIZE) / 2;
	Block_Y = 0;

	// Block�z��ɋ��߂�type�̃u���b�N������
	memcpy(Block, Block_list[type], sizeof(Block));

	// �p�x�p�^�[���񐔕��A�E��90�x����]������
	for (int i = 0; i < r_num; i++) {
		rotate_block();
	}
}
//==============================================================
// �w��ʒu�Ƀu���b�N��\���ł��邩���ׂ�
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
// ���ړ��ł��Ȃ��Ȃ������̏���(�u���b�N�Œ艻�A�V�K�u���b�N�̍쐬)
// �V�K�u���b�N���ړ��ł��ȏꍇ�̓Q�[���I�[�o�[�ɁA
// ���_���Q�[���N���A���_�ɂȂ����ꍇ�̓Q�[���N���A�ɂ���
//==============================================================
bool fix_check_game_end(void)
{
	Sleep(100);
	fixed_block();
	if (!can_move_block(Block_X, Block_Y)) {
		print_field();          // �Ō�ɌŒ艻�����u���b�N��\�����邽��
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
// �L�[����Ńu���b�N�𓮂���
//==============================================================
bool operate_block(void)
{
	// ���͑��i���b�Z�[�W�\���ƃL�[����
	set_cursor_pos(1, FIELD_Y + FIELD_H + 2);
	printf("a(��),s(��),d(��),x(��]),�X�y�[�X(��������),*(�I��) ");
	

	switch (szBuf[0]) {
	case '*':   // �I��
		return false;

	case 'a':   // ���ړ�
		if (can_move_block(Block_X - 1, Block_Y)) {
			Block_X--;
		}
		break;
	case 'd':   // �E�ړ�
		if (can_move_block(Block_X + 1, Block_Y)) {
			Block_X++;
		}
		break;
	case 's':   // ���ړ�
		if (can_move_block(Block_X, Block_Y + 1)) {
			Block_Y++;
		}
		else {
			return fix_check_game_end();    // �u���b�N���Œ艻���Q�[���I�������ׂ�
		}
		break;
	case ' ':   // ��������
		while (can_move_block(Block_X, Block_Y + 1)) {
			Block_Y++;
			print_field();      // �t�B�[���h�̕\��
			print_block();      // �A�N�e�B�u�u���b�N�̕\��
			Sleep(20);
		}
		return fix_check_game_end();    // �u���b�N���Œ艻���Q�[���I�������ׂ�

	case 'x':   // ��]	
	case 'm':
		rotate_block();
		break;
	default:
		break;
	}
	return true;
}
//==============================================================
// Block�̃t�B�[���h�Œ艻
//==============================================================
void fixed_block(void)
{
	// �u���b�N�s�[�X�̂���ʒu���Œ艻�s�[�X�ɂ���
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
// Field�ɉ����s�[�X�����񂾂�폜����i���_�A�b�v�j
// �i�����ɂȂ����s�[�X�́ABLOCK_DEL��\�����Ă���폜����j
//==============================================================
void delete_lines(void)
{
	int     add_point = 1;      // �ǉ����_

	for (int i = 0; i < FIELD_H; i++) {
		bool    complete = true;

		for (int j = 0; j < FIELD_W; j++) {
			if (Field[i][j] == EMPTY) {
				complete = false;   // ���̍s�͉�������ł��Ȃ�
				break;
			}
		}
		// EMPTY���P���Ȃ������ꍇ�͉�����PIECE_DEL���Z�b�g����
		if (complete) {
			memset(Field[i], PIECE_DEL, sizeof(Field[i]));

			print_field();
			Sleep(200);

			// �㕔�̃f�[�^�ŏ㏑������i�폜���铮���ɂȂ�j
			for (int j = i; j > 0; j--) {
				memcpy(Field[j], Field[j - 1], sizeof(Field[j]));
			}
			memset(Field[0], EMPTY, sizeof(Field[0]));  // �ŏ�s�͋�ɂ���

			Point += add_point;
			add_point *= EXPAND_POINT;  // �A���폜�̏ꍇ�͒ǉ����_������

			print_field();
			Sleep(200);
		}
	}
}
//==============================================================
// �Q�[���I�����̃��b�Z�[�W�\��
//==============================================================
void print_end(void)
{
	set_cursor_pos(1, 1);
	switch (Status) {
	case GAME_CLEAR:
		printf("�Q�[���N���A�@�S(o�L��`o)Ɂ@");
		break;
	case GAME_OVER:
		printf("�Q�[���I�[�o�[  (-_-;)������");
		break;
	default:
		printf("�Q�[���I��");
		break;
	}
}
//==============================================================
// �_�C���N�g�L�[����
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
			key = 's';  // ���L�[�Ɠ��������ɂ���
			break;
		}
	}
	switch (key) {
	case 0xe0:
		switch (_getch()) {
		case 0x4b:  // ���L�[
			key = 'a';
			break;
		case 0x4d:  // ���L�[
			key = 'd';
			break;
		case 0x50:  // ���L�[
			key = 's';
			break;
		default:
			key = 0;
		}
		break;
	case 0x1b:  // ESC�L�[
		key = '*';
		break;
	case '*':   // �I��
	case 'a':   // ���ړ�
	case 'd':   // �E�ړ�
	case 's':   // ���ړ�
	case ' ':   // ��������
	case 'x':   // ��]
		break;
	default:
		key = 0;
		break;
	}
	return key;
}