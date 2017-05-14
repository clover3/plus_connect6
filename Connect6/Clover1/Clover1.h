#pragma once
#include <cstring>
#include <list>
using namespace std;

const int MAX_X = 19;
const int MAX_Y = 19;

// Stone range from 0-19
// 1 : Me
// 2 : Enemy

const int COLOR_ME = 1;
const int COLOR_ENEMY = 2;

void printf_debug(const char* format, ...);


struct Position{
	int x;
	int y;
	Position(int x_, int y_)
	{
		x = x_;
		y = y_;
	}
	Position()
	{
		int x = -1;
		int y = -1;
	}
};

struct Action
{
	Position stone1;
	Position stone2;
	int player;
	bool singleAction;
	Action(Position p1, Position p2, int color, bool single)
	{ 
		stone1 = p1;
		stone2 = p2;
		player = color;
		singleAction = single;
	}
	Action()
	{}
};

Action put_center();

class BestAnswer{
	int best_score;
	Action best_action;
public:
	BestAnswer(){ best_score = -100000; }
	void update_max(int score, Action& action)
	{
		if (score > best_score)
		{
			score = best_score;
			best_action = Action(action);
		}
	}
	int score(){ return best_score; }
	Action action(){ return best_action;  }
};

class Player{
	int player;
public:
	Player(int p){ player = p; }
	Player oppo(){
		return Player(2 - player);
	}
	static Player enemy(){
		return Player(2);
	}
	int color(){
		return player;
	}
};




class Plate
{
	// 0 : Empty
	// 1 : My
	// 2 : Enemy
	int board[MAX_X][MAX_Y];
public:
	Plate(){
		memset(board, MAX_X * MAX_Y, 0);
		void printf_debug(const char* format, ...);
	}
	Plate(Plate& other)
	{
		memcpy(board, other.board, MAX_X * MAX_Y);
	}

	bool is_block(int x, int y, int color){ return board[x][y] > 0 && board[x][y] != color; }
	bool has_stone(int x, int y){ return board[x][y] > 0; }
	bool has_stone(Position& p){ return board[p.x][p.y] > 0; }
	void put_stone(Position& p, int player);
	Plate do_action(Action& action);
	// Assuming enemy turn, will is lose?
	bool isFail_1();
	// Assuming my turn, will i win?
	bool isSuc_1();



	bool can_win_checkrow(int r, int c, int color);
	bool can_win_checkcol(int r, int c, int color);
	bool can_win_checkdiag1(int r, int c, int color);
	bool can_win_checkdiag2(int r, int c, int color);
	bool can_win(Player& player);


	bool always_lose(Player& player);
};


class Clover1
{
public:
	Action nextAction();
	void commit_action(Action& action);
	void read_board( int(*pf)(int, int) );
private:
	int max_depth;
	list<Action> generateCandidate(Plate& plate);
	Plate curPlate;
};