#include "Clover1.h"
#include <stdio.h>
#include <windows.h>
#include <stdarg.h>
#include <stdlib.h>
#include <vector>
#include <exception>
#include <string>


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



list<Action> generate_simple(Plate& node, Player player)
{
	// TODO///////////////
	// all possible two stone action
	
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
		-1, 0, 1, -2, 0, 2,};
	int dy[neighbor_n] = { -2, -2, -2, -1, -1, -1,
		0, 0, 0, 0,
		+1, +1, +1, +2, +2, +2 };
	for (int x = 0; x < MAX_X; x++)
	{
		for (int y = 0; y < MAX_Y; y++)
		{
			if (node.has_stone(x, y))
			{
				for (int i = 0; i < neighbor_n;i++)
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
	
	list<Action> candidate;
	for (int i = 0; i < candidate_point.size(); i++)
	{
		for (int j = i + 1; j < candidate_point.size(); j++)
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
	return candidates;
}


int heuristic_eval(Plate& node, Player& player)
{
	int val = rand() % 10 - 5;

	// TODO implement this
	return val;
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

Action Clover1::best_depence(Plate& plate)
{
	list<Action> candidates = generate_simple(plate, 1);
	printf_debug("best_depence - %d options...", candidates.size());
	for (auto c : candidates)
	{
		dbg_print_action(c);
		auto next = plate.do_action(c);
		if (!next.can_win(Player::enemy()))
			return c;
	}
	printf_debug("No... there is no way to depence");
	return candidates.front();
}

Action Clover1::nextAction()
{
	printf_debug("next Action");
	curPlate.print_dbg();
	if (curPlate.can_win(Player::enemy()))
	{
		printf_debug("Oooops. We must defense.");
		return best_depence(curPlate);
	}
	// Generate Candidate
	
	list<Action> candidates = generate_threaten(curPlate, Player::me());

	printf_debug("Num candidate : %d", candidates.size());
	// Select best
	BestAnswer best_answer;
	for (auto c : candidates)
	{
		auto next = curPlate.do_action(c);
		int score = heuristic_eval(next, Player::me());
		best_answer.update_max(score, c);
		if (score > -100)
		{
			dbg_print_action(c);
			printf_debug("score : %d", score);
		}
	}
	printf_debug("I will do :");
	dbg_print_action(best_answer.action());
	return best_answer.action();
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
			s += to_string(board[x][y]);
		}
		printf_debug(s.c_str());
	}
}