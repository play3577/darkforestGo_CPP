// test2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "board.h"

void str2play(const char *str, Coord *m, Stone *player) {
	if (str[0] == 'W') *player = S_WHITE;
	else *player = S_BLACK;

	if (! strcmp(str + 2, "PASS")) {
		*m = M_PASS;
		return;
	}
	if (! strcmp(str + 2, "RESIGN")) {
		*m = M_RESIGN;
		return;
	}

	int x = str[2] - 'A';
	if (x >= 8) x --;
	int y;
	sscanf(str + 3, "%d", &y);
	y --;
	*m = OFFSETXY(x, y);
}

void simple_play(Board *b, const char *move_str) {
	GroupId4 ids;
	Coord m;
	Stone player;
	str2play(move_str, &m, &player);

	if (TryPlay(b, X(m), Y(m), player, &ids)) {
		Play(b, &ids);
		 AllMoves black;
		 AllMoves white;
		 GetAllStones(b, &black, &white);

		 int nb = black.num_moves;
		 for (int i = 0; i < nb; i++)
		 {
			 Coord cd = black.moves[i];
			 char buf[30];
			 get_move_str(cd, player, buf);

			/* int x = X(cd);
			 char CX = x + 'A';
			 if (CX >= 'I')
				 CX++;
			 int y = Y(cd);
			 y++;
			 */

		 }
		 int nw = white.num_moves;
		 for (int i = 0; i < nw; i++)
		 {
			 Coord cd = black.moves[i];
			 char buf[30];
			 get_move_str(cd, player, buf);
		 }
	}
}


typedef enum { NORMAL = 0, KO_FIGHT, OPPONENT_IN_DANGER, OUR_ATARI, NAKADE, PATTERN, NO_MOVE, NUM_MOVE_TYPE } MoveType;
typedef struct {
	BOOL switches[NUM_MOVE_TYPE];
	// Try to save our group in atari if its size is >= thres_save_atari.
	int thres_save_atari;
	// Allow self-atari move if the group size is smaller than thres_allow_atari_stone (before the new move is put).
	int thres_allow_atari_stone;
	// Reduce opponent liberties if its liberties <= thres_opponent_libs and #stones >= thres_opponent_stones.
	int thres_opponent_libs;
	int thres_opponent_stones;
} DefPolicyParams;

/*
typedef struct {
	// Pattern matcher.
	void *p;

	// Parameters
	DefPolicyParams params;
} Handle;
*/
typedef struct {
	// Ownermap
	int total_ownermap_count;
	// Histogram. S_EMPTY (S_UNKNOWN), S_BLACK, S_WHITE, S_OFF_BOARD (S_DAME)
	int ownermap[MACRO_BOARD_SIZE][MACRO_BOARD_SIZE][4];
} Handle;


Stone OwnermapJudgeOne(Handle *h, int i, int j, float ratio) {
	int empty = h->ownermap[i][j][S_EMPTY];
	int black = h->ownermap[i][j][S_BLACK];
	int white = h->ownermap[i][j][S_WHITE];
	int n = h->total_ownermap_count;

	int thres = (int)(n * ratio);

	if (empty >= thres) return S_DAME;
	if (empty + black >= thres) return S_BLACK;
	if (empty + white >= thres) return S_WHITE;
	return S_UNKNOWN;
}

void GetDeadStones(void *hh, const Board *board, float ratio, Stone *livedead, Stone *group_stats) {
	// Threshold the ownermap and determine.
	Handle *h = (Handle *)hh;

	Stone *internal_group_stats = NULL;

	if (group_stats == NULL) {
		internal_group_stats = (Stone *)malloc(board->_num_groups * sizeof(Stone));
		group_stats = internal_group_stats;
	}

	memset(group_stats, S_EMPTY, board->_num_groups * sizeof(Stone));

	for (int i = 0; i < BOARD_SIZE; ++i) {
		for (int j = 0; j < BOARD_SIZE; ++j) {
			Coord c = OFFSETXY(i, j);
			Stone s = board->_infos[c].color;
			Stone owner = OwnermapJudgeOne(h, i, j, ratio);

			// printf("owner at (%d, %d) = %d\n", i, j, owner);
			short id = board->_infos[c].id;
			if (owner == S_UNKNOWN) {
				group_stats[id] = s | S_UNKNOWN;
			} else if (! (group_stats[id] & S_UNKNOWN)) {
				// The group has deterministic state or empty.
				Stone stat = s;
				if (owner == s) stat |= S_ALIVE;
				else if (owner == OPPONENT(s)) stat |= S_DEAD;
				else stat |= S_UNKNOWN;

				if (group_stats[id] == S_EMPTY) group_stats[id] = stat;
				else if (group_stats[id] != stat) group_stats[id] = s | S_UNKNOWN;
			}
		}
	}
	// Once we get the group stats, we thus can fill the ownermap.
	if (livedead != NULL) {
		// Zero out everything else.
		memset(livedead, S_EMPTY, BOARD_SIZE * BOARD_SIZE * sizeof(Stone));
		for (int i = 1; i < board->_num_groups; ++i) {
			TRAVERSE(board, i, c) {
				livedead[EXPORT_OFFSET(c)] = group_stats[i];
			} ENDTRAVERSE
		}
	}

	if (internal_group_stats != NULL) free(internal_group_stats);
}



// Get Trompy-Taylor score directly.
// If livedead != NULL, then livedead is a BOARD_SIZE * BOARD_SIZE array. Otherwise this output is ignored.
// If territory != NULL, then it is also a BOARD_SIZE * BOARD_SIZE array. Otherwise this output is ignored.

float GetTTScoreOwnermap(void *hh, const Board *board, Stone *livedead, Stone *territory) {
	Stone group_stats[MAX_GROUP];
	GetDeadStones(hh, board, 0.5, livedead, group_stats);
	return GetTrompTaylorScore(board, group_stats, territory);
}

int _tmain(int argc, _TCHAR* argv[])
{
	Board b;
	ClearBoard(&b);
	simple_play(&b, "B A2");
	simple_play(&b, "W A1");
	simple_play(&b, "B B1");
	//simple_play(&b, "B D4");
	//simple_play(&b, "B D16");
	//simple_play(&b, "W F3");

	//  float score = GetFastScore(&be->board, scoring->rule) - scoring->komi;
	//GetFastScore(board, s->common_params->rule);

	return 0;
}

