// Samsung Go Tournament Form C (g++-4.8.3)

/*
[AI 코드 작성 방법]

1. char info[]의 배열 안에					"TeamName:자신의 팀명,Department:자신의 소속"					순서로 작성합니다.
( 주의 ) Teamname:과 Department:는 꼭 들어가야 합니다.
"자신의 팀명", "자신의 소속"을 수정해야 합니다.

2. 아래의 myturn() 함수 안에 자신만의 AI 코드를 작성합니다.

3. AI 파일을 테스트 하실 때는 "육목 알고리즘대회 툴"을 사용합니다.

4. 육목 알고리즘 대회 툴의 연습하기에서 바둑돌을 누른 후, 자신의 "팀명" 이 들어간 알고리즘을 추가하여 테스트 합니다.



[변수 및 함수]
myturn(int cnt) : 자신의 AI 코드를 작성하는 메인 함수 입니다.
int cnt (myturn()함수의 파라미터) : 돌을 몇 수 둬야하는지 정하는 변수, cnt가 1이면 육목 시작 시  한 번만  두는 상황(한 번), cnt가 2이면 그 이후 돌을 두는 상황(두 번)
int  x[0], y[0] : 자신이 둘 첫 번 째 돌의 x좌표 , y좌표가 저장되어야 합니다.
int  x[1], y[1] : 자신이 둘 두 번 째 돌의 x좌표 , y좌표가 저장되어야 합니다.
void domymove(int x[], int y[], cnt) : 둘 돌들의 좌표를 저장해서 출력


//int board[BOARD_SIZE][BOARD_SIZE]; 바둑판 현재상황 담고 있어 바로사용 가능함. 단, 원본데이터로 수정 절대금지
// 놓을수 없는 위치에 바둑돌을 놓으면 실격패 처리.

boolean ifFree(int x, int y) : 현재 [x,y]좌표에 바둑돌이 있는지 확인하는 함수 (없으면 true, 있으면 false)
int showBoard(int x, int y) : [x, y] 좌표에 무슨 돌이 존재하는지 보여주는 함수 (1 = 자신의 돌, 2 = 상대의 돌, 3 = 블럭킹)


<-------AI를 작성하실 때, 같은 이름의 함수 및 변수 사용을 권장하지 않습니다----->
*/

#include <stdio.h>
#include <Windows.h>
#include <time.h>
#include "Connect6Algo.h"


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
			best_score = score;
			best_action = Action(action);
		}
	}
	int score(){ return best_score; }
	Action action(){ return best_action; }
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
	void print_plate();
};


class Clover1
{
public:
	Action nextAction();
	void commit_action(Action& action);
	void read_board(int(*pf)(int, int));

	pair<bool, Action> need_defense(Plate plate);
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

#include <stdio.h>
#include <windows.h>
#include <stdarg.h>
#include <stdlib.h>
#include <vector>
#include <exception>
#include <string>


int heuristic_eval(Plate& node, int player);



bool is_in_board(int x, int y)
{
	return x >= 0 && x < MAX_X && y >= 0 && y < MAX_Y;
}

void printf_debug(const char* format, ...)
{
	char buf[300];
	va_list arglist;
	va_start(arglist, format);
	vsprintf_s(buf, format, arglist);
	//OutputDebugStringA(buf);
	va_end(arglist);
}

void dbg_print_action(Action& action)
{
	printf_debug("Action(%d,%d / %d,%d)",
		action.stone1.x, action.stone1.y,
		action.stone2.x, action.stone2.y);
}


Action put_center()
{
	Position center(9, 9);
	Action action(center, center, COLOR_ME, true);
	return action;
}



vector<Position> generate_candidate_points(Plate& node, Player player)
{
	bool near_area[MAX_X][MAX_Y] = { 0 };
	/*
	const int neighbor_n = 24;
	int dx[neighbor_n] = {-3, 0, 3, -2, 0, 2, -1, 0, 1,
	-3,-2,-1, 1,2,3,
	-1,0,1, -2,0,2, -3,0,3};
	int dy[neighbor_n] = {-3,-3,-3, -2,-2,-2, -1,-1,-1,
	0,0,0, 0,0,0,
	1,1,1,2,2,2,3,3,3};
	*/
	const int neighbor_n = 16;
	int dx[neighbor_n] = { -2, 0, 2, -1, 0, 1,
		-2, -1, 1, 2,
		-1, 0, 1, -2, 0, 2, };
	int dy[neighbor_n] = { -2, -2, -2, -1, -1, -1,
		0, 0, 0, 0,
		+1, +1, +1, +2, +2, +2 };
	for (int x = 0; x < MAX_X; x++)
	{
		for (int y = 0; y < MAX_Y; y++)
		{
			if (node.has_stone(x, y))
			{
				for (int i = 0; i < neighbor_n; i++)
				{
					int nx = x + dx[i];
					int ny = y + dy[i];
					if (is_in_board(nx, ny))
					{
						near_area[nx][ny] = true;
					}
				}
			}
		}
	}

	vector<Position> candidate_point;
	candidate_point.reserve(MAX_X * MAX_Y);
	for (int x = 0; x < MAX_X; x++)
	{
		for (int y = 0; y < MAX_Y; y++)
		{
			if (near_area[x][y] && !node.has_stone(x, y))
			{
				candidate_point.push_back(Position(x, y));
			}
		}
	}
	return candidate_point;
}

vector<Action> generate_one_stone_candidate(Plate& node, Player player)
{
	auto v1 = generate_candidate_points(node, player);
	std::function<Action(Position)> toAction = [](Position p){
		return Action(p, 1);
	};

	return mapf(v1, toAction);
}

vector<Action> candi_gen_one_plus_one(Plate& node, Player player, bool reverse)
{
	int start = GetTickCount();
	auto candi_actions = generate_one_stone_candidate(node, Player::me());
	printf_debug("Num candidate : %d", candi_actions.size());

	int color = player.color();
	function<TopK(vector<Action>, Plate)> get_top_k = [color, reverse](vector<Action> actions, Plate node){
		TopK topK(5);
		for (auto c : actions)
		{
			auto next = node.do_action(c);

			int score = heuristic_eval(next, color);
			if (reverse)
				score = -score;
			topK.push(c, score);
		}
		return topK;
	};

	TopK topK = get_top_k(candi_actions, node);

	vector<Action> final_candidate;
	for (auto c : topK.get()){
		Plate mid_state = node.do_action(c);
		auto candi_action2s = generate_one_stone_candidate(mid_state, Player::me());
		TopK topK2 = get_top_k(candi_action2s, mid_state);
		for (auto c2 : topK2.get())
		{
			final_candidate.push_back(Action(c.stone1, c2.stone1, 1, false));
		}
	}
	printf_debug("final_candidate : %d", candi_actions.size());
	printf_debug("candi_gen_one_plus_one: %d ms", GetTickCount() - start);
	return final_candidate;
}

list<Action> generate_simple(Plate& node, Player player)
{
	// TODO///////////////
	// all possible two stone action
	auto candidate_point = generate_candidate_points(node, player);
	list<Action> candidate;
	for (int i = 0; i < (int)candidate_point.size(); i++)
	{
		for (int j = i + 1; j < (int)candidate_point.size(); j++)
		{
			Action action(candidate_point[i], candidate_point[j], player.color(), false);
			candidate.push_back(action);
		}
	}
	return candidate;
}


list<Action> generate_threaten(Plate& node, Player player)
{
	auto candidates = generate_simple(node, player.color());
	list<Action> threat_actions;
	for (auto c : candidates)
	{
		auto next = node.do_action(c);
		if (next.can_win(Player::me()))
			threat_actions.push_back(c);
	}
	printf_debug("%d threat candidate generated", threat_actions.size());
	return threat_actions;
}


void Clover1::read_board(int(*showBoard)(int, int))
{
	int board[MAX_X][MAX_Y] = { 0 };
	for (int x = 0; x < MAX_X; x++)
	{
		for (int y = 0; y < MAX_Y; y++)
		{
			board[x][y] = showBoard(x, y);
		}
	}
	curPlate.set(board);
}


Action Clover1::nextAction()
{
	printf_debug("========Next Action========");
	int start_time = GetTickCount();
	//curPlate.print_plate();
	if (curPlate.can_win(Player::me()))
	{
		printf_debug("Kkkkya!! Lets Mak Ta!");
		auto c = curPlate.find_win(Player::me());
		return c;
	}
	pair<bool, Action> need = need_defense(curPlate);
	if (need.first)
	{
		printf_debug("We must do this defense option.");
		printf_debug("Defense : %d", GetTickCount() - start_time);
		return need.second;
	}
	else
	{
		// Generate Candidate

		// Select best

		auto candidates = candi_gen_one_plus_one(curPlate, COLOR_ME, false);
		BestAnswer best_answer;
		int heuri_time = 0;
		for (auto c : candidates)
		{
			int score = heuristic_eval(curPlate.do_action(c), COLOR_ME);
			best_answer.update_max(score, c);
		}

		printf_debug("I will do :");
		dbg_print_action(best_answer.action());
		printf_debug("My score will be %d", best_answer.score());
		int end_time = GetTickCount();
		printf_debug("Total Elapsed : %d", end_time - start_time);
		return best_answer.action();
	}
}

void Clover1::commit_action(Action& action)
{
	curPlate = curPlate.do_action(action);
}


list<Action> Clover1::generateCandidate(Plate& plate)
{
	return generate_simple(plate, 1);
}

Plate Plate::do_action(Action& action)
{
	Plate result = *this;
	result.put_stone(action.stone1, action.player);
	if (!action.singleAction)
		result.put_stone(action.stone2, action.player);
	return result;
}

void Plate::put_stone(Position& p, int player)
{
	if (has_stone(p))
		throw new exception();
	board[p.x][p.y] = player;
}

bool Plate::can_win_checkrow(int r, int c, int color)
{
	int ours = 0;
	int blocked = 0;
	for (int i = 0; i < 6; i++)
	{
		if (r + i > 18)
			blocked++;
		else if (board[c][r + i] == color)
			ours++;
		else if (is_block(c, r + i, color))
			blocked++;
	}
	return (ours > 3) && (blocked == 0);
}

bool Plate::can_win_checkcol(int r, int c, int color) {
	int ours = 0;
	int blocked = 0;
	for (int i = 0; i < 6; i++) {
		if (c + i > 18)
			blocked++;
		if (board[c + i][r] == color)
			ours++;
		else if (is_block(c + i, r, color))
			blocked++;
	}
	return (ours > 3) && (blocked == 0);
}

bool Plate::can_win_checkdiag1(int r, int c, int color) {
	int ours = 0;
	int blocked = 0;
	for (int i = 0; i < 6; i++) {
		if (c + i > 18 || r + i > 18)
			blocked++;
		else if (board[c + i][r + i] == color)
			ours++;
		else if (is_block(c + i, r + i, color))
			blocked++;
	}
	return (ours > 3) && (blocked == 0);
}

bool Plate::can_win_checkdiag2(int r, int c, int color) {
	int ours = 0;
	int blocked = 0;
	for (int i = 0; i < 6; i++) {
		if (c + i > 18 || r - i > 18)
			blocked++;
		else if (board[c + i][r - i] == color)
			ours++;
		else if (is_block(c + i, r - i, color))
			blocked++;
	}
	return (ours > 3) && (blocked == 0);
}


// Assuming players turn, can player win?
bool Plate::can_win(Player& player)
{
	// TODO
	int color = player.color();
	// Type 1 : OOOO (4 in row)
	// Type 2 : XOOOOO ( 5 in row)
	// Type 3 : OOOOOX ( 5 in row)
	// Type 3 : OO__OO ( 4 in line)
	// Horizontal
	// Vertical
	// Diagonal( left top - right bottom )
	// Diagonal( / shape
	bool iswin = false;
	for (int i = 0; i < 19; i++)
	{
		for (int j = 0; j < 19; j++)
		{
			iswin = iswin || (can_win_checkrow(i, j, color)
				|| can_win_checkcol(i, j, color)
				|| can_win_checkdiag1(i, j, color)
				|| can_win_checkdiag2(i, j, color));
			if (iswin)
				return iswin;
		}
	}

	return iswin;
}

bool Plate::always_lose(Player& player)
{
	return can_win(player.oppo());
}


void Plate::print_stdout()
{
	for (int y = 0; y < MAX_Y; y++)
	{
		for (int x = 0; x < MAX_X; x++)
		{
			printf("%d", board[x][y]);
		}
		printf("\n");
	}
	printf("\n");
}

void Plate::print_plate()
{
	for (int y = 0; y < MAX_Y * 2 + 1; y++)
	{
		string s;

		if (y % 2 == 0)
			s += "+";
		if (y % 2 != 0)
			s += "|";
		for (int x = 0; x < MAX_X; x++)
		{
			if (y % 2 == 0)
				s += "---+";
			else
			{
				int state = board[x][y / 2];
				if (state == 0)
					s += "   |";
				else if (state == 1)
					s += " ● |";
				else if (state == 2)
					s += " ○ |";
				else
					s += " ? |";
			}

		}
		printf_debug(s.c_str());
	}
}



int get_state(int board[MAX_X][MAX_Y], int x, int y)
{
	if (x < 0 || x >= MAX_X || y < 0 || y >= MAX_Y)
		return 3;
	else
		return board[x][y];
}

int count_threat(int board[MAX_X][MAX_Y], int x_0, int y_0, int dx, int dy, int color)
{
	int ours = 0;
	int unblocked = 0;
	for (int cursor = 0; cursor < 6; cursor++)
	{
		int nx = x_0 + cursor * dx;
		int ny = y_0 + cursor * dy;

		int state = get_state(board, nx, ny);
		if (state == color)
			ours++;
		if (state == 0)
			unblocked++;
	}

	int pre_x = x_0 - 1 * dx;
	int pre_y = y_0 - 1 * dy;
	int next_x = x_0 + 6 * dx;
	int next_y = y_0 + 6 * dy;
	if ((ours == 4 && unblocked == 2)
		|| (ours == 5 && unblocked == 1)) {
		bool more_connection = (get_state(board, pre_x, pre_y) == color)
			|| (get_state(board, next_x, next_y) == color);
		if (!more_connection)
		{
			for (int cursor = 0; cursor < 6; cursor++) {
				int nx = x_0 + cursor * dx;
				int ny = y_0 + cursor * dy;
				if (get_state(board, nx, ny) == 0)
					board[nx][ny] = 4;
			}
			return 1;
		}
	}
	return 0;
}

int count_live3(int board[MAX_X][MAX_Y], int x_0, int y_0, int dx, int dy, int color)
{
	int ours = 0;
	int unblocked = 0;

	bool live3_ok = true;

	int states[6];
	for (int cursor = 0; cursor < 6; cursor++) {
		int nx = x_0 + cursor * dx;
		int ny = y_0 + cursor * dy;

		states[cursor] = get_state(board, nx, ny);
	}
	for (int cursor = 1; cursor < 5; cursor++) {
		if (states[cursor] == color)
			ours++;
		if (states[cursor] == 0)
			unblocked++;
	}

	if (ours != 3 || unblocked != 1)
		live3_ok = false;
	else if (states[0] != 0 || states[5] != 0)
		live3_ok = false;

	int pre_x = x_0 - 1 * dx;
	int pre_y = y_0 - 1 * dy;
	int next_x = x_0 + 6 * dx;
	int next_y = y_0 + 6 * dy;
	if (live3_ok) {
		live3_ok = (get_state(board, pre_x, pre_y) == 0) && (get_state(board, next_x, next_y) == 0);
		if (live3_ok)
		{
			for (int cursor = 0; cursor < 6; cursor++) {
				int nx = x_0 + cursor * dx;
				int ny = y_0 + cursor * dy;
				if (get_state(board, nx, ny) == 0)
					board[nx][ny] = 4;
			}
			board[pre_x][pre_y] = 4;
			board[next_x][next_y] = 4;
			return 1;
		}
	}
	return 0;
}

int count_live2(int board[MAX_X][MAX_Y], int x_0, int y_0, int dx, int dy, int color)
{
	int ours = 0;
	int unblocked = 0;

	bool live2_ok = true;

	int states[6];
	for (int cursor = 0; cursor < 6; cursor++) {
		int nx = x_0 + cursor * dx;
		int ny = y_0 + cursor * dy;

		states[cursor] = get_state(board, nx, ny);
	}
	for (int cursor = 1; cursor < 5; cursor++) {
		if (states[cursor] == color)
			ours++;
		if (states[cursor] == 0)
			unblocked++;
	}

	if (ours != 2 || unblocked != 2)
		live2_ok = false;
	else if (states[0] != 0 || states[5] != 0)
		live2_ok = false;

	int pre_x = x_0 - 1 * dx;
	int pre_y = y_0 - 1 * dy;
	int next_x = x_0 + 6 * dx;
	int next_y = y_0 + 6 * dy;
	if (live2_ok) {
		live2_ok = (get_state(board, pre_x, pre_y) == 0) && (get_state(board, next_x, next_y) == 0);
		if (live2_ok)
		{
			for (int cursor = 0; cursor < 6; cursor++) {
				int nx = x_0 + cursor * dx;
				int ny = y_0 + cursor * dy;
				if (get_state(board, nx, ny) == 0)
					board[nx][ny] = 4;
			}
			board[pre_x][pre_y] = 4;
			board[next_x][next_y] = 4;
			return 1;
		}
	}
	return 0;
}

int count_dead3(int board[MAX_X][MAX_Y], int x_0, int y_0, int dx, int dy, int color)
{
	int ours = 0;
	int unblocked = 0;
	for (int cursor = 0; cursor < 6; cursor++)
	{
		int nx = x_0 + cursor * dx;
		int ny = y_0 + cursor * dy;

		int state = get_state(board, nx, ny);
		if (state == color)
			ours++;
		if (state == 0)
			unblocked++;
	}

	int pre_x = x_0 - 1 * dx;
	int pre_y = y_0 - 1 * dy;
	int next_x = x_0 + 6 * dx;
	int next_y = y_0 + 6 * dy;
	if (ours == 3 && unblocked == 3) {
		bool more_connection = (get_state(board, pre_x, pre_y) == color)
			|| (get_state(board, next_x, next_y) == color);
		if (!more_connection)
		{
			for (int cursor = 0; cursor < 6; cursor++) {
				int nx = x_0 + cursor * dx;
				int ny = y_0 + cursor * dy;
				if (get_state(board, nx, ny) == 0)
					board[nx][ny] = 4;
			}
			return 1;
		}
	}
	return 0;
}

int count_dead2(int board[MAX_X][MAX_Y], int x_0, int y_0, int dx, int dy, int color)
{
	int ours = 0;
	int unblocked = 0;
	for (int cursor = 0; cursor < 6; cursor++)
	{
		int nx = x_0 + cursor * dx;
		int ny = y_0 + cursor * dy;

		int state = get_state(board, nx, ny);
		if (state == color)
			ours++;
		if (state == 0)
			unblocked++;
	}

	int pre_x = x_0 - 1 * dx;
	int pre_y = y_0 - 1 * dy;
	int next_x = x_0 + 6 * dx;
	int next_y = y_0 + 6 * dy;
	if (ours == 2 && unblocked == 4) {
		bool more_connection = (get_state(board, pre_x, pre_y) == color)
			|| (get_state(board, next_x, next_y) == color);
		if (!more_connection)
		{
			for (int cursor = 0; cursor < 6; cursor++) {
				int nx = x_0 + cursor * dx;
				int ny = y_0 + cursor * dy;
				if (get_state(board, nx, ny) == 0)
					board[nx][ny] = 4;
			}
			return 1;
		}
	}
	return 0;
}

int calc_threat(int board[MAX_X][MAX_Y], int color) {
	int threats = 0;

	for (int x_0 = 0; x_0 < MAX_X; x_0++)
		for (int y_0 = 0; y_0 + 6 < 19; y_0++)
			threats += count_threat(board, x_0, y_0, 0, 1, color);


	for (int y_0 = 0; y_0 < 19; y_0++)
		for (int x_0 = 0; x_0 + 6 < 19; x_0++)
			threats += count_threat(board, x_0, y_0, 1, 0, color);


	for (int x_0 = -MAX_X + 5; x_0 < MAX_X - 5; x_0++)
		for (int y_0 = 0; y_0 + 6 < 19; y_0++)
			threats += count_threat(board, x_0, y_0, 1, 1, color);


	for (int x_0 = 5; x_0 < MAX_X * 2 - 5; x_0++)
		for (int y_0 = 0; y_0 + 6 < 19; y_0++)
			threats += count_threat(board, x_0, y_0, -1, 1, color);

	return threats;
}

int calc_live3(int board[MAX_X][MAX_Y], int color) {
	int lives = 0;

	for (int x_0 = 0; x_0 < MAX_X; x_0++)
		for (int y_0 = 0; y_0 + 6 < 19; y_0++)
			lives += count_live3(board, x_0, y_0, 0, 1, color);

	for (int y_0 = 0; y_0 < 19; y_0++)
		for (int x_0 = 0; x_0 + 6 < 19; x_0++)
			lives += count_live3(board, x_0, y_0, 1, 0, color);

	for (int x_0 = -MAX_X + 5; x_0 < MAX_X - 5; x_0++)
		for (int y_0 = 0; y_0 + 6 < 19; y_0++)
			lives += count_live3(board, x_0, y_0, 1, 1, color);

	for (int x_0 = 5; x_0 < MAX_X * 2 - 5; x_0++)
		for (int y_0 = 0; y_0 + 6 < 19; y_0++)
			lives += count_live3(board, x_0, y_0, -1, 1, color);

	return lives;
}

int calc_live2(int board[MAX_X][MAX_Y], int color) {
	int lives = 0;

	for (int x_0 = 0; x_0 < MAX_X; x_0++)
		for (int y_0 = 0; y_0 + 6 < 19; y_0++)
			lives += count_live2(board, x_0, y_0, 0, 1, color);

	for (int y_0 = 0; y_0 < 19; y_0++)
		for (int x_0 = 0; x_0 + 6 < 19; x_0++)
			lives += count_live2(board, x_0, y_0, 1, 0, color);

	for (int x_0 = -MAX_X + 5; x_0 < MAX_X - 5; x_0++)
		for (int y_0 = 0; y_0 + 6 < 19; y_0++)
			lives += count_live2(board, x_0, y_0, 1, 1, color);

	for (int x_0 = 5; x_0 < MAX_X * 2 - 5; x_0++)
		for (int y_0 = 0; y_0 + 6 < 19; y_0++)
			lives += count_live2(board, x_0, y_0, -1, 1, color);

	return lives;
}

int calc_dead3(int board[MAX_X][MAX_Y], int color) {
	int deads = 0;

	for (int x_0 = 0; x_0 < MAX_X; x_0++)
		for (int y_0 = 0; y_0 + 6 < 19; y_0++)
			deads += count_dead3(board, x_0, y_0, 0, 1, color);

	for (int y_0 = 0; y_0 < 19; y_0++)
		for (int x_0 = 0; x_0 + 6 < 19; x_0++)
			deads += count_dead3(board, x_0, y_0, 1, 0, color);

	for (int x_0 = -MAX_X + 5; x_0 < MAX_X - 5; x_0++)
		for (int y_0 = 0; y_0 + 6 < 19; y_0++)
			deads += count_dead3(board, x_0, y_0, 1, 1, color);

	for (int x_0 = 5; x_0 < MAX_X * 2 - 5; x_0++)
		for (int y_0 = 0; y_0 + 6 < 19; y_0++)
			deads += count_dead3(board, x_0, y_0, -1, 1, color);

	return deads;
}

int calc_dead2(int board[MAX_X][MAX_Y], int color) {
	int deads = 0;

	for (int x_0 = 0; x_0 < MAX_X; x_0++)
		for (int y_0 = 0; y_0 + 6 < 19; y_0++)
			deads += count_dead2(board, x_0, y_0, 0, 1, color);

	for (int y_0 = 0; y_0 < 19; y_0++)
		for (int x_0 = 0; x_0 + 6 < 19; x_0++)
			deads += count_dead2(board, x_0, y_0, 1, 0, color);

	for (int x_0 = -MAX_X + 5; x_0 < MAX_X - 5; x_0++)
		for (int y_0 = 0; y_0 + 6 < 19; y_0++)
			deads += count_dead2(board, x_0, y_0, 1, 1, color);

	for (int x_0 = 5; x_0 < MAX_X * 2 - 5; x_0++)
		for (int y_0 = 0; y_0 + 6 < 19; y_0++)
			deads += count_dead2(board, x_0, y_0, -1, 1, color);

	return deads;
}


int heuristic_eval(Plate& node, int player)
{
	int board[MAX_X][MAX_Y];

	node.copy_to(board);
	int color = player;
	int n_threats = calc_threat(board, color);
	int n_live3 = calc_live3(board, color);
	int n_live2 = calc_live2(board, color);
	int n_dead3 = calc_dead3(board, color);
	int n_dead2 = calc_dead2(board, color);
	//printf_debug("TLL count : %d %d %d %d %d", n_threats, n_live3, n_live2, n_dead3, n_dead2);
	// TODO implement this
	int score = n_threats * 1000
		+ n_live3 * 700
		+ n_live2 * 200
		+ n_dead3 * 500
		+ n_dead2 * 50;
	if (n_threats >= 3)
		score = 1000 * 1000;
	return score;
}


pair<bool, Action> find_win_sub(int board[MAX_X][MAX_Y], int x_0, int y_0, int dx, int dy)
{
	int ours = 0;
	int unblocked = 0;
	Position stone[6];
	for (int cursor = 0; cursor < 6; cursor++)
	{
		int nx = x_0 + cursor * dx;
		int ny = y_0 + cursor * dy;

		int state = get_state(board, nx, ny);
		if (state == 1)
			ours++;
		if (state == 0)
		{
			stone[unblocked++] = Position(nx, ny);
		}
	}

	int pre_x = x_0 - 1 * dx;
	int pre_y = y_0 - 1 * dy;
	int next_x = x_0 + 6 * dx;
	int next_y = y_0 + 6 * dy;
	if (ours == 4 && unblocked == 2) {
		bool more_connection = (get_state(board, pre_x, pre_y) == 1) || (get_state(board, next_x, next_y) == 1);
		if (!more_connection)
		{
			for (int cursor = 0; cursor < 6; cursor++) {
				int nx = x_0 + cursor * dx;
				int ny = y_0 + cursor * dy;
				if (get_state(board, nx, ny) == 0)
					board[nx][ny] = 4;
			}
			return pair<bool, Action>(true, Action(stone[0], stone[1], 1, false));
		}
	}
	return pair<bool, Action>(false, Action());
}

Action Plate::find_win(Player& player)
{
	int board[MAX_X][MAX_Y];
	copy_to(board);

	for (int x_0 = 0; x_0 < MAX_X; x_0++){
		for (int y_0 = 0; y_0 + 6 < 19; y_0++){
			auto r = find_win_sub(board, x_0, y_0, 0, 1);
			if (r.first)
				return r.second;
		}
	}

	for (int y_0 = 0; y_0 < 19; y_0++){
		for (int x_0 = 0; x_0 + 6 < 19; x_0++){
			auto r = find_win_sub(board, x_0, y_0, 1, 0);
			if (r.first)
				return r.second;
		}
	}


	for (int x_0 = -MAX_X + 5; x_0 < MAX_X - 5; x_0++){
		for (int y_0 = 0; y_0 + 6 < 19; y_0++){
			auto r = find_win_sub(board, x_0, y_0, 1, 1);
			if (r.first)
				return r.second;
		}
	}

	for (int x_0 = 5; x_0 < MAX_X * 2 - 5; x_0++){
		for (int y_0 = 0; y_0 + 6 < 19; y_0++){
			auto r = find_win_sub(board, x_0, y_0, -1, 1);
			if (r.first)
				return r.second;
		}
	}
	///TODO remove at submit
	throw new exception("Should have found");
}

Plate Plate::inverse()
{
	Plate r;
	for (int x = 0; x < MAX_X; x++) {
		for (int y = 0; y < MAX_Y; y++) {
			if (board[x][y] = 0)
				r.board[x][y] = 0;
			else if (board[x][y] = 1)
				r.board[x][y] = 2;
			else if (board[x][y] = 2)
				r.board[x][y] = 1;
			else if (board[x][y] = 3)
				r.board[x][y] = 3;
		}
	}
	return r;
}

pair<bool, Action> Clover1::need_defense(Plate real_plate)
{
	int score = -heuristic_eval(real_plate, Player::enemy().color());
	printf_debug("Enemy theat : %d", score);
	if (score <= -1000)
	{
		printf_debug("We need defense!");
		auto actions = candi_gen_one_plus_one(real_plate, Player::enemy(), true);
		BestAnswer best_answer;
		for (auto c : actions)
		{
			int enemy_score = -heuristic_eval(real_plate.do_action(c), COLOR_ENEMY);
			int score = heuristic_eval(real_plate.do_action(c), COLOR_ME);
			if (enemy_score < -900)
				score = enemy_score;
			//printf_debug("Searching best defense : %d", score);
			best_answer.update_max(score, c);
		}
		printf_debug("Enemy theat decreased : %d->%d", score, best_answer.score());
		return pair<bool, Action>(true, best_answer.action());
	}
	return pair<bool, Action>(false, Action());
}



// Global State 
Clover1 cloverAI;

/////////////////


// "샘플코드[C]"  -> 자신의 팀명 (수정)
// "AI부서[C]"  -> 자신의 소속 (수정)
// 제출시 실행파일은 반드시 팀명으로 제출!
char info[] = { "TeamName:Clover-Bus,Department:POSTECH-PLUS" };

void myturn(int cnt) {
	// 이 부분에서 알고리즘 프로그램(AI)을 작성하십시오. 기본 제공된 코드를 수정 또는 삭제하고 본인이 코드를 사용하시면 됩니다.
	// 현재 Sample code의 AI는 Random으로 돌을 놓는 Algorithm이 작성되어 있습니다.

	// TODO Update Enemy action 

	cloverAI.read_board(showBoard);
	int x[2], y[2];
	int player = 1;
	if (cnt == 1)
	{
		printf_debug("Put at center");
		// TODO fix me
		x[0] = 9;
		y[0] = 9;
		cloverAI.commit_action(put_center());
	}
	else{
		printf_debug("Current Map :");
		cloverAI.now().print_plate();
		Action action = cloverAI.nextAction();
		cloverAI.commit_action(action);
		x[0] = action.stone1.x;
		y[0] = action.stone1.y;
		x[1] = action.stone2.x;
		y[1] = action.stone2.y;
	}

	// 이 부분에서 자신이 놓을 돌을 출력하십시오.
	// 필수 함수 : domymove(x배열,y배열,배열크기)
	// 여기서 배열크기(cnt)는 myturn()의 파라미터 cnt를 그대로 넣어야합니다.
	domymove(x, y, cnt);
	printf_debug("myturn EXIT");
}