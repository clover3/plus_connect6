#define BOARD_SIZE 19
// Can be a value from 1-3  1 being hardest, 3 being the easiest AI to play against.  
#define DIFFICULTY 3

#include <string>
using namespace std;
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <Windows.h>
#include "BadRobot.h"

void printf_debug(const char* format, ...)
{
	char buf[300];
	va_list arglist;
	va_start(arglist, format);
	vsprintf_s(buf, format, arglist);
	OutputDebugStringA(buf);
	va_end(arglist);
}

void initialize_game_board();
void print_game_board();
int human_place_piece(int moves);
int ai_move_offensively(int moves, int player);
int ai_move_defensively(int moves, int player, int difficulty);
int play_move(int x, int y, int player);
void print_game_board_weights();
int modify_game_board_weights(int x, int y, int player);
void modify_weight(int x, int y, int modifier);

int check_for_win(int *moves_left, int opponent);
int check_horizontal(int *moves_left, int x, int y, char opponent, int player);
int check_verticle(int *moves_left, int x, int y, char opponent, int player);
int check_down_left(int *moves_left, int x, int y, char opponent, int player);
int check_down_right(int *moves_left, int x, int y, char opponent, int player);
void set_up_offense();
int attack_step = -1;
int attack_X;
int attack_Y;
int attack_up;
int attack_over;
int attack_pattern[] = { 0, 0, 0, 1, 1, 2, 1, 3, 1, 0, 1, 1, 0, 2, 0, 3, 2, 0, 3, 0, 2, 1, 3, 1 };

char game_board[BOARD_SIZE][BOARD_SIZE];
int game_board_weights[BOARD_SIZE][BOARD_SIZE] =
{
	{ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 },
	{ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 },
	{ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 },
	{ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 },
	{ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 },
	{ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 },
	{ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 },
	{ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 },
	{ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 },
	{ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 },
	{ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 },
	{ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 },
	{ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 },
	{ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 },
	{ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 },
	{ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 },
	{ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 },
	{ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 },
	{ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 }
};

int g_move_cnt = 0;
int g_recent_move_x[4];
int g_recent_move_y[4];

void add_move(int x, int y)
{
	printf_debug("Add Move(%d,%d)", x, y);
	g_recent_move_x[g_move_cnt] = x;
	g_recent_move_y[g_move_cnt] = y;
	g_move_cnt++;
}

void get_move(int *x, int *y)
{
	x[0] = g_recent_move_x[0]-1;
	y[0] = g_recent_move_y[0]-1;
	x[1] = g_recent_move_x[1]-1;
	y[1] = g_recent_move_y[1]-1;
	g_move_cnt = 0;
}


int old_main(void)
{
	initialize_game_board();
	//human_place_piece(1);
	ai_move_defensively(2, 1, DIFFICULTY);
	while (1)
	{
//		if (human_place_piece(2) == 1)
		{
			printf_debug("Black Won\n");
			break;
		}

		if (ai_move_defensively(2, 1, DIFFICULTY) == 1)
		{
			printf_debug("White Won - You got your ass kicked by code\n");
			break;
		}
	}

	return(0);
}

// A function which allows a user to place a piece on the
// game board.


// A function that will capture the offensive (game winning)
// movemnets of the AI.

int check_attack(int x, int y, int up, int over, int player){
	int step;
	char enemy = 'W';
	if (player == 2)
		enemy = 'B';

	for (step = 0; step < 12; step++){
		if (game_board[y + (up * attack_pattern[(step * 2)]) - 1][x + (over * attack_pattern[(step * 2) + 1]) - 1] == enemy){
			//printf_debug("Checking Y= %d, X= %d\n", y + (up * attack_pattern[(step * 2)]),  x + (over * attack_pattern[(step * 2)+1]));
			//printf_debug("Busted!\n");
			return 0;
		}
		//printf_debug("Checking Y= %d, X= %d\n", y + (up * attack_pattern[(step * 2)]),  x + (over * attack_pattern[(step * 2)+1]));
	}
	return 1;

}

void set_up_offense(int player){

	attack_up = 1;
	attack_over = 1;

	for (attack_Y = 3; attack_Y < BOARD_SIZE - 5; attack_Y++){
		for (attack_X = 3; attack_X < BOARD_SIZE - 5; attack_X++){
			if (check_attack(attack_X, attack_Y, attack_up, attack_over, player))
				return;
		}
	}

	attack_up = -1;

	for (attack_Y = 5; attack_Y < BOARD_SIZE - 3; attack_Y++){
		for (attack_X = 3; attack_X < BOARD_SIZE - 5; attack_X++){
			if (check_attack(attack_X, attack_Y, attack_up, attack_over, player))
				return;
		}
	}

	attack_over = -1;

	for (attack_Y = 5; attack_Y < BOARD_SIZE - 3; attack_Y++){
		for (attack_X = 5; attack_X < BOARD_SIZE - 3; attack_X++){
			if (check_attack(attack_X, attack_Y, attack_up, attack_over, player))
				return;
		}
	}

	attack_up = 1;

	for (attack_Y = 3; attack_Y < BOARD_SIZE - 5; attack_Y++){
		for (attack_X = 5; attack_X < BOARD_SIZE - 3; attack_X++){
			if (check_attack(attack_X, attack_Y, attack_up, attack_over, player))
				return;
		}
	}

}

int ai_move_offensively(int moves_remaining, int player)
{
	if (!check_attack(attack_X, attack_Y, attack_up, attack_over, 2)){
		attack_step = -1;
		printf_debug("Lameeee\n");
	}
	if (attack_step == -1){
		set_up_offense(2);
		attack_step++;
	}

	while (moves_remaining > 0){

		while (game_board[attack_Y + (attack_up * attack_pattern[(attack_step * 2)]) - 1][attack_X + (attack_over * attack_pattern[(attack_step * 2) + 1]) - 1] == 'W')
			attack_step++;

		play_move(attack_Y + (attack_up * attack_pattern[(attack_step * 2)]), attack_X + (attack_over * attack_pattern[(attack_step * 2) + 1]), 2);
		moves_remaining--;
		attack_step++;
	}

	return moves_remaining;

}

// a function that will check and see if we have an opportuninity to win

int check_for_win(int *moves_left, int player){
	char opponent = 'B';
	if (player == 1)
		opponent = 'W';
	int y, x;
	for (y = 0; y < BOARD_SIZE; y++){
		for (x = 0; x < BOARD_SIZE; x++){
			if (x < (BOARD_SIZE - 5) && *moves_left > 0)
				check_horizontal(moves_left, x, y, opponent, player);
			if (y < (BOARD_SIZE - 5) && *moves_left > 0)
				check_verticle(moves_left, x, y, opponent, player);
			if (x < (BOARD_SIZE - 5) && y < (BOARD_SIZE - 5) && *moves_left > 0)
				check_down_right(moves_left, x, y, opponent, player);
			if (x > 4 && y < (BOARD_SIZE - 5) && *moves_left > 0)
				check_down_left(moves_left, x, y, opponent, player);

			if (*moves_left == 0){
				return 1;
			}
		}
	}
	return 0;
}

int check_horizontal(int *moves_left, int x, int y, char opponent, int player){
	int empty = 0;
	int filled = 0;

	int remember[4];


	remember[0] = 0;
	remember[1] = 0;
	remember[2] = 0;
	remember[3] = 0;

	int i;
	for (i = 0; i < 6; i++){
		char temp = game_board[y][x + i];
		if (temp == opponent)
			return 0;
		if (temp == ' '){
			if (empty >= 2)
				return 0;
			else{
				remember[empty * 2] = x + i + 1;
				remember[(empty * 2) + 1] = y + 1;
				empty++;
			}
		}
		else
			filled++;
	}

	if (filled >= 4 && empty <= 2){
		play_move(remember[1], remember[0], player);
		play_move(remember[3], remember[2], player);
		*moves_left = 0;
	}
	return 1;
}

int check_verticle(int *moves_left, int x, int y, char opponent, int player){
	int empty = 0;
	int filled = 0;

	int remember[4];

	remember[0] = 0;
	remember[1] = 0;
	remember[2] = 0;
	remember[3] = 0;

	int i;
	for (i = 0; i < 6; i++){
		char temp = game_board[y + i][x];
		if (temp == opponent)
			return 0;
		if (temp == ' '){
			if (empty >= 2)
				return 0;
			else{
				remember[empty * 2] = x + 1;
				remember[(empty * 2) + 1] = y + i + 1;
				empty++;
			}
		}
		else
			filled++;
	}

	if (filled >= 4 && empty <= 2){
		play_move(remember[1], remember[0], player);
		play_move(remember[3], remember[2], player);
		*moves_left = 0;
	}
	return 1;
}

int check_down_right(int *moves_left, int x, int y, char opponent, int player){
	int empty = 0;
	int filled = 0;

	int remember[4];

	remember[0] = 0;
	remember[1] = 0;
	remember[2] = 0;
	remember[3] = 0;

	int i;
	for (i = 0; i < 6; i++){
		char temp = game_board[y + i][x + i];
		if (temp == opponent)
			return 0;
		if (temp == ' '){
			if (empty >= 2)
				return 0;
			else{
				remember[empty * 2] = x + i + 1;
				remember[(empty * 2) + 1] = y + i + 1;
				empty++;
			}
		}
		else
			filled++;
	}

	if (filled >= 4 && empty <= 2){
		play_move(remember[1], remember[0], player);
		play_move(remember[3], remember[2], player);
		*moves_left = 0;
	}
	return 1;
}

int check_down_left(int *moves_left, int x, int y, char opponent, int player){
	int empty = 0;
	int filled = 0;

	int remember[4];

	remember[0] = 0;
	remember[1] = 0;
	remember[2] = 0;
	remember[3] = 0;

	int i;
	for (i = 0; i < 6; i++){
		char temp = game_board[y + i][x - i];
		if (temp == opponent)
			return 0;
		if (temp == ' '){
			if (empty >= 2)
				return 0;
			else{
				remember[empty * 2] = x - i + 1;
				remember[(empty * 2) + 1] = y + i + 1;
				empty++;
			}
		}
		else
			filled++;
	}

	if (filled >= 4 && empty <= 2){
		play_move(remember[1], remember[0], player);
		play_move(remember[3], remember[2], player);
		*moves_left = 0;
	}
	return 1;
}

int ai_move_defensively(int moves_remaining, int player, int difficulty)
{
	int x1 = 0;
	int y1 = 0;
	int z1 = 0;
	int i = 0;
	int moves_left = moves_remaining;
	if (check_for_win(&moves_remaining, 2) == 1)
	{
		printf_debug("I win");
	}

	int move_idx = 0;
	int x[2], y[2];

	while (moves_left > 0)
	{
		for (i = 0; i < (19 * 19); i++)
		{
			if (game_board_weights[i / (19)][i % (19)] >= (difficulty + 2) && game_board_weights[i / (19)][i % (19)] > z1)
			{
				z1 = game_board_weights[i / (19)][i % (19)];
				x1 = i / (19) + 1;
				y1 = i % (19) + 1;
			}
		}

		// Need to fix this so that it only plays our highest priority play. Assuming that this means z1 for now...
		if (z1 >= (difficulty + 2))
		{
			if (play_move(x1, y1, 2) == 0)
			{
				z1 = 0;
				moves_left -= 1;
			}
		}
		else if (moves_left > 0)
		{
			moves_left = ai_move_offensively(moves_left, 1);
		}
	}
	return 0;
}

// Used to print out the game board for visual display to the
// user.
void print_game_board()
{
	int x, y = 0;
	for (x = 0; x < (BOARD_SIZE * 2) + 1; x++)
	{
		string s;
		if (x % 2 == 0)
			s += "+";
		if (x % 2 != 0)
			s += "|";
		for (y = 0; y < BOARD_SIZE; y++)
		{
			if (x % 2 == 0)
				s += "---+";
			if (x % 2 != 0)
			{
				char c = game_board[y][x / 2];
				s += " ";
				s += c;
				s += " |";
			}
		}
		printf_debug(s.c_str());
	}
}

void initialize_game_board()
{
	int x;

	for (x = 0; x < BOARD_SIZE * BOARD_SIZE; x++)
	{
		game_board[x / BOARD_SIZE][x % BOARD_SIZE] = ' ';
		game_board_weights[x / BOARD_SIZE][x % BOARD_SIZE] = 6;
	}
}

int play_move(int x, int y, int player)
{
	int status = 1; // 1 = fail 0 = pass

	if (x >= 1 && x <= BOARD_SIZE && y >= 1 && y <= BOARD_SIZE && game_board[x - 1][y - 1] == ' ')
	{
		printf_debug("Player %d, Played Move: ( %d, %d )\n", player, x, y);
		if ( player == 2)
			add_move(x, y);
		if (player == 1)
		{
			game_board[x - 1][y - 1] = 'B';
		}
		else
		{
			game_board[x - 1][y - 1] = 'W';
		}
		modify_game_board_weights(x - 1, y - 1, player);
		//print_game_board_weights();
		print_game_board();
		status = 0;
	}

	return status;
}

// Used to modify the game_board_weights table as a player makes a move. 
// goal is to lessen the memory space required to track all game info.
//
// int x - x coord that was played. 
// 
// int y - y coord that was played.
//
// int player - who is playing 1 = player 1, 2 = player 2
//
/*
0 will represent an all ready played area or a place that we do not
want to play in at all.
*/
int modify_game_board_weights(int x, int y, int player)
{
	// 1 = fail; 0 = pass
	int z = 0;
	int status = 1;
	int modifier1 = 0;
	int modifier2 = 0;
	int modifier3 = 0;
	int modifier4 = 0;
	int modifier5 = 0;
	int modifier6 = 0;
	int modifier7 = 0;
	int modifier8 = 0;
	int base = 2;

	game_board_weights[x][y] = 0;

	if (player == 1)
	{
		for (z = 1; z < 6; z++)
		{
			if (game_board[x][y + z] == 'B')
			{
				modifier1++;
			}
			else if (game_board[x][y + z] == 'W')
			{
				break;
			}
		}
		for (z = 1; z < 6; z++)
		{
			if (game_board[x][y - z] == 'B')
			{
				modifier2++;
			}
			else if (game_board[x][y - z] == 'W')
			{
				break;
			}
		}
		for (z = 1; z < 6; z++)
		{
			if (game_board[x + z][y] == 'B')
			{
				modifier3++;
			}
			else if (game_board[x + z][y] == 'W')
			{
				break;
			}
		}
		for (z = 1; z < 6; z++)
		{
			if (game_board[x - z][y] == 'B')
			{
				modifier4++;
			}
			else if (game_board[x - z][y] == 'W')
			{
				break;
			}
		}
		for (z = 1; z < 6; z++)
		{
			if (game_board[x - z][y + z] == 'B')
			{
				modifier5++;
			}
			else if (game_board[x - z][y + z] == 'W')
			{
				break;
			}
		}
		for (z = 1; z < 6; z++)
		{
			if (game_board[x - z][y - z] == 'B')
			{
				modifier6++;
			}
			else if (game_board[x - z][y - z] == 'W')
			{
				break;
			}
		}
		for (z = 1; z < 6; z++)
		{
			if (game_board[x + z][y + z] == 'B')
			{
				modifier7++;
			}
			else if (game_board[x + z][y + z] == 'W')
			{
				break;
			}
		}
		for (z = 1; z < 6; z++)
		{
			if (game_board[x + z][y - z] == 'B')
			{
				modifier8++;
			}
			else if (game_board[x + z][y - z] == 'W')
			{
				break;
			}
		}

		if (game_board_weights[x][y - 1] != 0 && modifier1 > 0)
		{
			if (game_board_weights[x][y - 1] <= base + modifier1 + 1)
			{
				modify_weight(x, y - 1, -(game_board_weights[x][y - 1] - base));
			}
			modify_weight(x, y - 1, modifier1 + 1);
		}
		if (game_board_weights[x][y + 1] != 0 && modifier2 > 0)
		{
			if (game_board_weights[x][y + 1] <= base + modifier2 + 1)
			{
				modify_weight(x, y + 1, -(game_board_weights[x][y + 1] - base));
			}
			modify_weight(x, y + 1, modifier2 + 1);
		}
		if (game_board_weights[x - 1][y] != 0 && modifier3 > 0)
		{
			if (game_board_weights[x - 1][y] <= base + modifier3 + 1)
			{
				modify_weight(x - 1, y, -(game_board_weights[x - 1][y] - base));
			}
			modify_weight(x - 1, y, modifier3 + 1);
		}
		if (game_board_weights[x + 1][y] != 0 && modifier4 > 0)
		{
			if (game_board_weights[x + 1][y] <= base + modifier4 + 1)
			{
				modify_weight(x + 1, y, -(game_board_weights[x + 1][y] - base));
			}
			modify_weight(x + 1, y, modifier4 + 1);
		}
		if (game_board_weights[x + 1][y - 1] != 0 && modifier5 > 0)
		{
			if (game_board_weights[x + 1][y - 1] <= base + modifier5 + 1)
			{
				modify_weight(x + 1, y - 1, -(game_board_weights[x + 1][y - 1] - base));
			}
			modify_weight(x + 1, y - 1, modifier5 + 1);
		}
		if (game_board_weights[x + 1][y + 1] != 0 && modifier6 > 0)
		{
			if (game_board_weights[x + 1][y + 1] <= base + modifier6 + 1)
			{
				modify_weight(x + 1, y + 1, -(game_board_weights[x + 1][y + 1] - base));
			}
			modify_weight(x + 1, y + 1, modifier6 + 1);
		}
		if (game_board_weights[x - 1][y - 1] != 0 && modifier7 > 0)
		{
			if (game_board_weights[x - 1][y - 1] <= base + modifier7 + 1)
			{
				modify_weight(x - 1, y - 1, -(game_board_weights[x - 1][y - 1] - base));
			}
			modify_weight(x - 1, y - 1, modifier7 + 1);
		}
		if (game_board_weights[x - 1][y + 1] != 0 && modifier8 > 0)
		{
			if (game_board_weights[x - 1][y + 1] <= base + modifier8 + 1)
			{
				modify_weight(x - 1, y + 1, -(game_board_weights[x - 1][y + 1] - base));
			}
			modify_weight(x - 1, y + 1, modifier8 + 1);
		}

		for (z = 1; z < 6; z++)
		{
			if (game_board[x][y + z] == ' ' && modifier1 > 0)
			{
				if (game_board_weights[x][y + z] <= base + modifier1 + 1)
				{
					modify_weight(x, y + z, -(game_board_weights[x][y + z] - base));
				}
				modify_weight(x, y + z, modifier1 + 1);
				break;
			}
			else if (game_board[x][y + z] == 'W')
			{
				break;
			}
		}
		for (z = 1; z < 6; z++)
		{
			if (game_board[x][y - z] == ' ' && modifier2 > 0)
			{
				if (game_board_weights[x][y - z] <= base + modifier2 + 1)
				{
					modify_weight(x, y - z, -(game_board_weights[x][y - z] - base));
				}
				modify_weight(x, y - z, modifier2 + 1);
				break;
			}
			else if (game_board[x][y - z] == 'W')
			{
				break;
			}
		}
		for (z = 1; z < 6; z++)
		{
			if (game_board[x + z][y] == ' ' && modifier3 > 0)
			{
				if (game_board_weights[x + z][y] <= base + modifier3 + 1)
				{
					modify_weight(x + z, y, -(game_board_weights[x + z][y] - base));
				}
				modify_weight(x + z, y, modifier3 + 1);
				break;
			}
			else if (game_board[x + z][y] == 'W')
			{
				break;
			}
		}
		for (z = 1; z < 6; z++)
		{
			if (game_board[x - z][y] == ' ' && modifier4 > 0)
			{
				if (game_board_weights[x - z][y] <= base + modifier4 + 1)
				{
					modify_weight(x - z, y, -(game_board_weights[x - z][y] - base));
				}
				modify_weight(x - z, y, modifier4 + 1);
				break;
			}
			else if (game_board[x - z][y] == 'W')
			{
				break;
			}
		}
		for (z = 1; z < 6; z++)
		{
			if (game_board[x - z][y + z] == ' ' && modifier5 > 0)
			{
				if (game_board_weights[x - z][y + z] <= base + modifier5 + 1)
				{
					modify_weight(x - z, y + z, -(game_board_weights[x - z][y + z] - base));
				}
				modify_weight(x - z, y + z, modifier5 + 1);
				break;
			}
			else if (game_board[x - z][y + z] == 'W')
			{
				break;
			}
		}
		for (z = 1; z < 6; z++)
		{
			if (game_board[x - z][y - z] == ' ' && modifier6 > 0)
			{
				if (game_board_weights[x - z][y - z] <= base + modifier6 + 1)
				{
					modify_weight(x - z, y - z, -(game_board_weights[x - z][y - z] - base));
				}
				modify_weight(x - z, y - z, modifier6 + 1);
				break;
			}
			else if (game_board[x - z][y - z] == 'W')
			{
				break;
			}
		}
		for (z = 1; z < 6; z++)
		{
			if (game_board[x + z][y + z] == ' ' && modifier7 > 0)
			{
				if (game_board_weights[x + z][y + z] <= base + modifier7 + 1)
				{
					modify_weight(x + z, y + z, -(game_board_weights[x + z][y + z] - base));
				}
				modify_weight(x + z, y + z, modifier7 + 1);
				break;
			}
			else if (game_board[x + z][y + z] == 'W')
			{
				break;
			}
		}
		for (z = 1; z < 6; z++)
		{
			if (game_board[x + z][y - z] == ' ' && modifier8 > 0)
			{
				if (game_board_weights[x + z][y - z] <= base + modifier8 + 1)
				{
					modify_weight(x + z, y - z, -(game_board_weights[x + z][y - z] - base));
				}
				modify_weight(x + z, y - z, modifier8 + 1);
				break;
			}
			else if (game_board[x + z][y - z] == 'W')
			{
				break;
			}
		}
	}
	status = 0;

	return status;
}


// Used to modify the game board weights and to perform boundry checking
// and to ensure that all move that we want to make are legal. 
void modify_weight(int x, int y, int modifier)
{
	if (game_board_weights[x][y] != 0 && x >= 0 && x <= 18 && y >= 0 && y <= 18)
	{
		if (game_board_weights[x][y] + modifier > 0)
		{
			game_board_weights[x][y] += modifier;
		}
		else
		{
			game_board_weights[x][y] = 1;
		}
	}
}

// Print out the current weights of the game board (DEBUGGING ONLY).
void print_game_board_weights()
{
	int x, y = 0;

	for (x = 0; x < (BOARD_SIZE * 2) + 1; x++)
	{
		if (x % 2 == 0)
			printf_debug("+");
		if (x % 2 != 0)
			printf_debug("|");
		for (y = 0; y < BOARD_SIZE; y++)
		{
			if (x % 2 == 0)
				printf_debug("---+");
			if (x % 2 != 0)
			{
				printf_debug(" %d |", game_board_weights[y][x / 2]);
			}
		}
		printf_debug("\n");
	}
}
