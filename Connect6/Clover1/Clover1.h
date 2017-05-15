#pragma once
#include <cstring>
#include <list>
#include <queue> 
#include <future>
#include <functional>
using namespace std;

const int MAX_X = 19;
const int MAX_Y = 19;

// Stone range from 0-19
// 1 : Me
// 2 : Enemy

const int COLOR_ME = 1;
const int COLOR_ENEMY = 2;

void printf_debug(const char* format, ...);


template <typename T, typename U>
U foldLeft(const std::vector<T>& data,
	const U& initialValue,
	const std::function<U(U, T)>& foldFn) {
	typedef typename std::vector<T>::const_iterator Iterator;
	U accumulator = initialValue;
	Iterator end = data.cend();
	for (Iterator it = data.cbegin(); it != end; ++it) {
		accumulator = foldFn(accumulator, *it);
	}
	return accumulator;
}


template <typename T, typename U>
std::vector<U> mapf(const std::vector<T>& data, const std::function<U(T)> mapper) {
	std::vector<U> result;
	foldLeft<T, std::vector<U>&>(data, result, [mapper](std::vector<U>& res, T value)  -> std::vector<U>& {
		res.push_back(mapper(value));
		return res;
	});
	return result;
}



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
	Action(Position p1, int color)
	{
		stone1 = p1;
		player = color;
		singleAction = true;
	}
	Action()
	{}
};

Action put_center();
struct MyCmp {

	bool operator()(pair<int, Action>& node1, pair<int, Action>& node2) const
	{
		return node1.first > node2.first;
	}
};

class TopK
{
	int k;
	vector<int> scores;
	vector<Action> items;
	priority_queue<int, std::vector<pair<int, Action>>, MyCmp> queue;
public:
	void push(Action action, int score)
	{
		queue.push(pair<int, Action>(score, action));
	}
	TopK(int k_) {
		k = k_;
	}
	vector<Action> get(){
		int cnt = k;
		vector<Action> v;
		while (cnt && !queue.empty())
		{
			pair<int, Action> a = queue.top();
			v.push_back(a.second);
			queue.pop();
		}
		return v;
	}
};

class BestAnswer{
	int best_score;
	Action best_action;
public:
	BestAnswer(){ best_score = -90000000; }
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
		return Player(3 - player);
	}

	static Player me(){
		return Player(1);
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
		memset(board, 0, MAX_X * MAX_Y * sizeof(int));
		void printf_debug(const char* format, ...);
	}
	Plate(Plate& other)
	{
		memcpy(board, other.board, MAX_X * MAX_Y * sizeof(int));
	}
	void set(int board_[MAX_X][MAX_Y]){
		memcpy(board, board_, MAX_X * MAX_Y * sizeof(int));
	}
	void copy_to(int board_[MAX_X][MAX_Y]){
		memcpy(board_, board, MAX_X * MAX_Y * sizeof(int));
	}

	bool is_block(int x, int y, int color)
	{ 
		return board[x][y] > 0 && board[x][y] != color; 
	}
	bool has_stone(int x, int y){ return board[x][y] > 0; }
	bool has_stone(Position& p){ return board[p.x][p.y] > 0; }
	void put_stone(Position& p, int player);
	Plate do_action(Action& action);
	Plate inverse();


	Action find_win(Player& player);

	bool can_win_checkrow(int r, int c, int color);
	bool can_win_checkcol(int r, int c, int color);
	bool can_win_checkdiag1(int r, int c, int color);
	bool can_win_checkdiag2(int r, int c, int color);
	bool can_win(Player& player);

	bool always_lose(Player& player);

	//
	void print_stdout();
	void print_dbg();
};


class Clover1
{
public:
	Action nextAction();
	void commit_action(Action& action);
	void read_board( int(*pf)(int, int) );

	pair<bool,Action> need_defense(Plate plate);
	Plate now() {
		return curPlate;
	}
	Clover1()
	{
		max_depth = 2;
	}
private:
	int max_depth;
	list<Action> generateCandidate(Plate& plate);
	Plate curPlate;
};