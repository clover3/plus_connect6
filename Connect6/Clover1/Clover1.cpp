#include "Clover1.h"
#include <stdio.h>
#include <windows.h>
#include <stdarg.h>
#include <stdlib.h>
#include <vector>
#include <exception>
#include <string>


int heuristic_eval(Plate& node, Player& player);



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
	OutputDebugStringA(buf);
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
	Position center(9,9);
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

vector<Action> candi_gen_one_plus_one(Plate& node, Player player)
{
	int start = GetTickCount();
	auto candi_actions = generate_one_stone_candidate(node, Player::me());
	printf_debug("Num candidate : %d", candi_actions.size());

	function<TopK(vector<Action>, Plate)> get_top_k = [](vector<Action> actions, Plate node){
		TopK topK(5);
		for (auto c : actions)
		{
			auto next = node.do_action(c);
			int score = heuristic_eval(next, Player::me());
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
	printf_debug("candi_gen_one_plus_one: %d ms" , GetTickCount()- start);
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



int eval(Plate &node, int depth, Player player)
{
	printf_debug("eval(depth=%d, player=%d)", depth, player.color());
	if (node.always_lose(player))
		return -1000;
	if (node.can_win(player))
		return 1000;

	if (depth == 0 )
	{
		return heuristic_eval(node, player);
	}
	
	auto candidate = generate_simple(node, player);
	BestAnswer best_answer;
	for(auto c : candidate)
	{
		auto next = node.do_action(c);
		int score = -1 * eval(next, depth - 1, player.oppo());
		best_answer.update_max(score, c);
	}
	return best_answer.score();
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

Action Clover1::threat_defense(Plate& plate)
{
	list<Action> candidates = generate_simple(plate, 1);
	printf_debug("threat_defense - %d options...", candidates.size());
	for (auto c : candidates)
	{
		auto next = plate.do_action(c);
		if (!next.can_win(Player::enemy()))
			return c;
	}
	printf_debug("No... there is no way to depence");
	return candidates.front();
}

Action Clover1::nextAction()
{
	printf_debug("========Next Action========");
	int start_time = GetTickCount();
	curPlate.print_dbg();
	if (curPlate.can_win(Player::me()))
	{
		printf_debug("Kkkkya!! Lets Mak Ta!");
		auto c = curPlate.find_win(Player::me());
		return c;
	}
	pair<bool, Action> need = need_defense(curPlate);
	if (need.first)
	{
		printf_debug("Oooops. We must defense.");
		printf_debug("Defense : %d", GetTickCount() - start_time);
		return need.second;
	}
	else
	{
		// Generate Candidate

		// Select best

		auto candidates = candi_gen_one_plus_one(curPlate, Player::me());
		BestAnswer best_answer;
		int heuri_time = 0;
		for (auto c : candidates)
		{
			int score = heuristic_eval(curPlate.do_action(c), Player::me());
			best_answer.update_max(score, c);
		}

		printf_debug("I will do :");
		dbg_print_action(best_answer.action());
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
		else if (board[c][r+i] == color)
			ours++;
		else if (is_block(c, r+i, color))
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
		if (board[c+i][r] == color)
			ours++;
		else if (is_block(c+i,r, color))
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
		else if (board[c+i][r+i] == color )
			ours++;
		else if (is_block(c+i, r+i, color) )
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
		else if (board[c+i][r-i] == color)
			ours++;
		else if (is_block(c+i, r-i, color))
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
		for(int j = 0; j < 19; j++) 
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

void Plate::print_dbg()
{
	for (int y = 0; y < MAX_Y; y++)
	{
		string s;
		for (int x = 0; x < MAX_X; x++)
		{
			if (board[x][y] == 0)
				s += " ";
			else if (board[x][y] == 1)
				s += "��";
			else if (board[x][y] == 2)
				s += "��";

		}
		printf_debug(s.c_str());
	}
}



int get_state(int board[MAX_X][MAX_Y], int x, int y)
{
	if (x< 0 || x > MAX_X || y < 0 || y > MAX_Y)
		return 3;
	else
		return board[x][y];
}

int count_threat(int board[MAX_X][MAX_Y], int x_0, int y_0, int dx, int dy)
{
	int ours = 0;
	int unblocked = 0;
	for (int cursor = 0; cursor < 6; cursor++)
	{
		int nx = x_0 + cursor * dx;
		int ny = y_0 + cursor * dy;

		int state = get_state(board, nx, ny);
		if (state == 1)
			ours++;
		if (state == 0)
			unblocked++;
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
			return 1;
		}
	}
	return 0;
}

int count_live3(int board[MAX_X][MAX_Y], int x_0, int y_0, int dx, int dy)
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
		if (states[cursor] == 1)
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

int count_live2(int board[MAX_X][MAX_Y], int x_0, int y_0, int dx, int dy)
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
		if (states[cursor] == 1)
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

int count_dead3(int board[MAX_X][MAX_Y], int x_0, int y_0, int dx, int dy)
{
	int ours = 0;
	int unblocked = 0;
	for (int cursor = 0; cursor < 6; cursor++)
	{
		int nx = x_0 + cursor * dx;
		int ny = y_0 + cursor * dy;

		int state = get_state(board, nx, ny);
		if (state == 1)
			ours++;
		if (state == 0)
			unblocked++;
	}

	int pre_x = x_0 - 1 * dx;
	int pre_y = y_0 - 1 * dy;
	int next_x = x_0 + 6 * dx;
	int next_y = y_0 + 6 * dy;
	if (ours == 3 && unblocked == 3) {
		bool more_connection = (get_state(board, pre_x, pre_y) == 1) || (get_state(board, next_x, next_y) == 1);
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

int count_dead2(int board[MAX_X][MAX_Y], int x_0, int y_0, int dx, int dy)
{
	int ours = 0;
	int unblocked = 0;
	for (int cursor = 0; cursor < 6; cursor++)
	{
		int nx = x_0 + cursor * dx;
		int ny = y_0 + cursor * dy;

		int state = get_state(board, nx, ny);
		if (state == 1)
			ours++;
		if (state == 0)
			unblocked++;
	}

	int pre_x = x_0 - 1 * dx;
	int pre_y = y_0 - 1 * dy;
	int next_x = x_0 + 6 * dx;
	int next_y = y_0 + 6 * dy;
	if (ours == 2 && unblocked == 4) {
		bool more_connection = (get_state(board, pre_x, pre_y) == 1) || (get_state(board, next_x, next_y) == 1);
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

int calc_threat(int board[MAX_X][MAX_Y]) {
	int threats = 0;

	for (int x_0 = 0; x_0 < MAX_X; x_0++)
		for (int y_0 = 0; y_0 + 6 < 19; y_0++)
			threats += count_threat(board, x_0, y_0, 0, 1);


	for (int y_0 = 0; y_0 < 19; y_0++)
		for (int x_0 = 0; x_0 + 6 < 19; x_0++)
			threats += count_threat(board, x_0, y_0, 1, 0);


	for (int x_0 = -MAX_X + 5; x_0 < MAX_X - 5; x_0++)
		for (int y_0 = 0; y_0 + 6 < 19; y_0++)
			threats += count_threat(board, x_0, y_0, 1, 1);


	for (int x_0 = 5; x_0 < MAX_X * 2 - 5; x_0++)
		for (int y_0 = 0; y_0 + 6 < 19; y_0++)
			threats += count_threat(board, x_0, y_0, -1, 1);

	return threats;
}

int calc_live3(int board[MAX_X][MAX_Y]) {
	int lives = 0;

	for (int x_0 = 0; x_0 < MAX_X; x_0++)
		for (int y_0 = 0; y_0 + 6 < 19; y_0++)
			lives += count_live3(board, x_0, y_0, 0, 1);

	for (int y_0 = 0; y_0 < 19; y_0++)
		for (int x_0 = 0; x_0 + 6 < 19; x_0++)
			lives += count_live3(board, x_0, y_0, 1, 0);

	for (int x_0 = -MAX_X + 5; x_0 < MAX_X - 5; x_0++)
		for (int y_0 = 0; y_0 + 6 < 19; y_0++)
			lives += count_live3(board, x_0, y_0, 1, 1);

	for (int x_0 = 5; x_0 < MAX_X * 2 - 5; x_0++)
		for (int y_0 = 0; y_0 + 6 < 19; y_0++)
			lives += count_live3(board, x_0, y_0, -1, 1);

	return lives;
}

int calc_live2(int board[MAX_X][MAX_Y]) {
	int lives = 0;

	for (int x_0 = 0; x_0 < MAX_X; x_0++)
		for (int y_0 = 0; y_0 + 6 < 19; y_0++)
			lives += count_live2(board, x_0, y_0, 0, 1);

	for (int y_0 = 0; y_0 < 19; y_0++)
		for (int x_0 = 0; x_0 + 6 < 19; x_0++)
			lives += count_live2(board, x_0, y_0, 1, 0);

	for (int x_0 = -MAX_X + 5; x_0 < MAX_X - 5; x_0++)
		for (int y_0 = 0; y_0 + 6 < 19; y_0++)
			lives += count_live2(board, x_0, y_0, 1, 1);

	for (int x_0 = 5; x_0 < MAX_X * 2 - 5; x_0++)
		for (int y_0 = 0; y_0 + 6 < 19; y_0++)
			lives += count_live2(board, x_0, y_0, -1, 1);

	return lives;
}

int calc_dead3(int board[MAX_X][MAX_Y]) {
	int deads = 0;

	for (int x_0 = 0; x_0 < MAX_X; x_0++)
		for (int y_0 = 0; y_0 + 6 < 19; y_0++)
			deads += count_dead3(board, x_0, y_0, 0, 1);

	for (int y_0 = 0; y_0 < 19; y_0++)
		for (int x_0 = 0; x_0 + 6 < 19; x_0++)
			deads += count_dead3(board, x_0, y_0, 1, 0);

	for (int x_0 = -MAX_X + 5; x_0 < MAX_X - 5; x_0++)
		for (int y_0 = 0; y_0 + 6 < 19; y_0++)
			deads += count_dead3(board, x_0, y_0, 1, 1);

	for (int x_0 = 5; x_0 < MAX_X * 2 - 5; x_0++)
		for (int y_0 = 0; y_0 + 6 < 19; y_0++)
			deads += count_dead3(board, x_0, y_0, -1, 1);

	return deads;
}

int calc_dead2(int board[MAX_X][MAX_Y]) {
	int deads = 0;

	for (int x_0 = 0; x_0 < MAX_X; x_0++)
		for (int y_0 = 0; y_0 + 6 < 19; y_0++)
			deads += count_dead2(board, x_0, y_0, 0, 1);

	for (int y_0 = 0; y_0 < 19; y_0++)
		for (int x_0 = 0; x_0 + 6 < 19; x_0++)
			deads += count_dead2(board, x_0, y_0, 1, 0);

	for (int x_0 = -MAX_X + 5; x_0 < MAX_X - 5; x_0++)
		for (int y_0 = 0; y_0 + 6 < 19; y_0++)
			deads += count_dead2(board, x_0, y_0, 1, 1);

	for (int x_0 = 5; x_0 < MAX_X * 2 - 5; x_0++)
		for (int y_0 = 0; y_0 + 6 < 19; y_0++)
			deads += count_dead2(board, x_0, y_0, -1, 1);

	return deads;
}


int heuristic_eval(Plate& node, Player& player)
{
	int val = rand() % 10 - 5;
	int board[MAX_X][MAX_Y];

	node.copy_to(board);
	int n_threats = calc_threat(board);
	int n_live3 = calc_live3(board);
	int n_live2 = calc_live2(board);
	int n_dead3 = calc_dead3(board);
	int n_dead2 = calc_dead2(board);
	// TODO implement this
	int score = n_threats * 1000
		+ n_live3 * 500
		+ n_live2 * 100
		+ n_dead3 * 30
		+ n_dead2 * 10;
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
	if (real_plate.can_win(Player::enemy()))
	{
		return pair<bool, Action>(true, threat_defense(real_plate));
	}
	else
	{
		printf_debug("Check possible double threat");
		Plate inversePlate = real_plate.inverse();
		vector<Action> actions = generate_one_stone_candidate(inversePlate, Player::me());
		for (auto c : actions)
		{
			auto next = inversePlate.do_action(c);
			int board[MAX_X][MAX_Y];
			next.copy_to(board);
			if (calc_threat(board) >= 1 && calc_live3(board) >= 1)
			{
				auto candi_action2s = generate_one_stone_candidate(real_plate, Player::me());
				BestAnswer best;
				for (auto c2 : candi_action2s)
				{
					Plate n2 = real_plate.do_action(c2);
					int score = heuristic_eval(n2, Player::me());
					best.update_max(score, c2);
				}

				Position stone1 = c.stone1;
				Position stone2 = best.action().stone1;

				return pair<bool, Action>(true, Action(stone1, stone2, 1, false));
			}
		}
		return pair<bool, Action>(false, Action());
	}
}