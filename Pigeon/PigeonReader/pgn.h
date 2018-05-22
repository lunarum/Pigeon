#ifndef PGN_H
#define PGN_H

#include <inttypes.h>
#include "cstring.h"

#define PGN_MAX_LINE 255
#define PGN_STD_LINE 80

#define PGN_TAG_STRING '"'
#define PGN_TAG_STRING_ESCAPE '\\'
#define PGN_TAG_COMMENT ';'
#define PGN_TAG_ESCAPE '%'
#define PGN_TAG_PERIOD '.'
#define PGN_TAG_ASTERIX '*'
#define PGN_TAG_NAG '$'
#define PGN_TAG_TAG_START '['
#define PGN_TAG_TAG_END ']'
#define PGN_TAG_COMMENT_START '{'
#define PGN_TAG_COMMENT_END '}'
#define PGN_TAG_LIST_START '('
#define PGN_TAG_LIST_END ')'
#define PGN_TAG_FUTURE_START '<'
#define PGN_TAG_FUTURE_END '>'

#define PGN_TAG_EOL '\012'
#define PGN_TAG_CAPTURE 'x'
#define PGN_TAG_CHECK '+'
#define PGN_TAG_CHECKMATE '#'
#define PGN_TAG_PROMOTION '='
#define PGN_TAG_CASTLING_KING "O-O"
#define PGN_TAG_CASTLING_QUEEN "O-O-O"


enum PGN_RESULT {
	PGN_RESULT_WHITE_WIN = 1,
	PGN_RESULT_BLACK_WIN = -1,
	PGN_RESULT_DRAW = 0,
	PGN_RESULT_UNKNOWN = 128
};

enum PGN_PIECES {
	PGN_PIECE_NONE = 0,
	PGN_PIECE_PAWN = 1,
	PGN_PIECE_KNIGHT = 2,
	PGN_PIECE_BISHOP = 3,
	PGN_PIECE_ROOK = 4,
	PGN_PIECE_QUEEN = 5,
	PGN_PIECE_KING = 6,
	PGN_PIECE_KING_ROOK = 7	// Castling
};

extern char piece_notations[];	// enum PGN_PIECES is index for the corresponding notation in PGN file
extern char file_notations[];	// internal file number is index for corresponding notation in PGN file
extern char rank_notations[];	// internal rank number is index for corresponding notation in PGN file
extern char castlingK_notation[];	// castling King-side PGN notation
extern char castlingQ_notation[];	// castling Queen-side PGN notation

struct PGN_ply {
	struct PGN_ply *next_ply;	// circular list of plies

	unsigned comment : 1;	// comment available
	unsigned white : 1;		// black (0) or white (1)
	unsigned piece : 3;
	unsigned promotion : 3;	// i.e. promotion if != 0

	unsigned from_file : 3;	// a(0)..h(7), same value as to_file means "empty"
	unsigned from_rank : 3;	// 1(0)..8(7), same value as to_rank means "empty"
	unsigned capture : 1;
	unsigned ep : 1;		// (black's pawn capture move and to_rank is 3 or white pawn capture move and to_rank is 4)


	unsigned to_file : 3;	// a(0)..h(8)
	unsigned to_rank : 3;	// 1(0)..8(7)
	unsigned check : 1;
	unsigned checkmate : 1;

	unsigned NAG : 8;
};

struct PGN_game {
	struct PGN_game *next_game;	// circular list of games
	struct PGN_ply *last_ply;
	cstring event;
	cstring site;
	cstring event_date;
	cstring round;
	cstring white;
	cstring black;
	enum PGN_RESULT result;
	unsigned int plies;
};

struct PGN_tree {
	unsigned int games;
	struct PGN_game *last_game;
};

struct PGN_tree * pgn_new_tree();
struct PGN_game * pgn_add_game(struct PGN_tree *game_tree);
struct PGN_ply * pgn_add_ply(struct PGN_game *game);
void pgn_reset_games(struct PGN_tree *game_tree);

#endif // !PGN_H
