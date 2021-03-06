#include "pgn_reader.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>

#ifdef _DEBUG
//#define DEBUG_PRINT_TOKENS 1
#define DEBUG_PRINT_PLY 1
#endif

#define ERROR_BUFFER 256

FILE *fp;
char saved_character;
int current_column;
int current_line;
char error_message[ERROR_BUFFER];
enum PGN_READER_RESULT result;

enum PGN_TOKEN next_token = PGN_TOKEN_NONE;
cstring next_token_word;
unsigned int next_token_number;

cstring symbol_event;
cstring symbol_site;
cstring symbol_date;
cstring symbol_round;
cstring symbol_white;
cstring symbol_black;
cstring symbol_result;
cstring symbol_result_black;
cstring symbol_result_white;
cstring symbol_result_draw;
cstring symbol_result_unknown;


enum PGN_READER_RESULT pgn_read_result(int *line, int *column, char ** message) {
	if (line != NULL)
		*line = current_line;
	if (column != NULL)
		*column = current_column;
	if (message != NULL)
		*message = error_message;
	return result;
}

char read_char() {
	char c;
	if (saved_character) {
		c = saved_character;
		saved_character = 0;
	}
	else {
		c = fgetc(fp);
		if (c == PGN_TAG_EOL) {
			current_column = 1;
			++current_line;
		}
		else
			++current_column;
	}
	return c;
}

/*
 * read_token - read next token
 */
bool read_token() {
	char c, buffer[PGN_MAX_LINE + 1];
	int n;

	next_token = PGN_TOKEN_NONE;
	next_token_word = CSTRING_NULL;
	next_token_number = 0;

	// Skip whitespace
	do {
		c = read_char();
		if (c == EOF)
			break;
	} while (iscntrl(c) || isspace(c));

	switch (c) {
	case EOF:
		next_token = PGN_TOKEN_EOF;
		result = PGN_READER_EOF;
		return true;
	case PGN_TAG_TAG_START:
		next_token = PGN_TOKEN_TAG_START;
		break;
	case PGN_TAG_TAG_END:
		next_token = PGN_TOKEN_TAG_END;
		break;
	case PGN_TAG_PERIOD:
		next_token = PGN_TOKEN_PERIOD;
		break;
	case PGN_TAG_ASTERIX:
		next_token = PGN_TOKEN_ASTERIX;
		break;
	case PGN_TAG_STRING:
		next_token = PGN_TOKEN_STRING;
		c = read_char();
		for (n = 0; n < PGN_MAX_LINE && c != EOF && c != PGN_TAG_STRING; ++n, c = read_char()) {
			if (c == PGN_TAG_STRING_ESCAPE) {
				c = read_char();
			}
			buffer[n] = c;
		}
		buffer[n] = 0;
		next_token_word = cstring_add(buffer);
		break;
	default:
		if (isalnum(c)) {
			next_token = isdigit(c) ? PGN_TOKEN_INTEGER : PGN_TOKEN_SYMBOL;
			for (n = 0; n < PGN_MAX_LINE && c != EOF && (isalnum(c) || strchr("_+#=:-/", c) != NULL); ++n, c = read_char()) {
				buffer[n] = c;
				if (isdigit(c))
					next_token_number = next_token_number * 10 + (c - '0');
				else
					next_token = PGN_TOKEN_SYMBOL;
			}
			buffer[n] = 0;
			if (next_token == PGN_TOKEN_SYMBOL) {
				next_token_word = cstring_add(buffer);
				next_token_number = 0;
			}
				
			saved_character = c;
		}
		else {
			result = PGN_READER_ERROR_SYNTAX;
			sprintf_s(error_message, ERROR_BUFFER, "SYNTAX ERROR: unknown character %x '%c' found", (int)c, c);
			return false;
		}
	}

#ifdef DEBUG_PRINT_TOKENS
	printf("token(%d)", next_token);
	if(next_token_word != CSTRING_NULL)
		printf(" string(%s)", cstring_get(next_token_word));
	if(next_token_number != 0)
		printf(" int(%d)", next_token_number);
	putchar('\n');
#endif

	return true;
}

bool match_token(enum PGN_TOKEN tok, cstring *value_str, int *value_int) {
	if (next_token != tok) {
		result = PGN_READER_ERROR_SYNTAX;
		sprintf_s(error_message, ERROR_BUFFER, "SYNTAX ERROR: token %d expected", (int)tok);
		return false;
	}
	if (value_str != NULL)
		*value_str = next_token_word;
	if (value_int != NULL)
		*value_int = next_token_number;
	return read_token();
}

bool convert_result(struct PGN_game *game, cstring value) {
	if (value == symbol_result_black)
		game->result = PGN_RESULT_BLACK_WIN;
	else if (value == symbol_result_white)
		game->result = PGN_RESULT_WHITE_WIN;
	else if (value == symbol_result_draw)
		game->result = PGN_RESULT_DRAW;
	else if (value == symbol_result_unknown)
		game->result = PGN_RESULT_UNKNOWN;
	else
		return false;
	return true;
}

bool fetch_tags(struct PGN_game *game) {
	cstring value;

	while (next_token == PGN_TOKEN_TAG_START) {
		if (match_token(PGN_TOKEN_TAG_START, &value, NULL)) {
			if (match_token(PGN_TOKEN_SYMBOL, &value, NULL)) {
				if (value == symbol_event) {
					if (!match_token(PGN_TOKEN_STRING, &game->event, NULL))
						return false;
				}
				else if (value == symbol_site) {
					if (!match_token(PGN_TOKEN_STRING, &game->site, NULL))
						return false;
				}
				else if (value == symbol_date) {
					if (!match_token(PGN_TOKEN_STRING, &game->event_date, NULL))
						return false;
				}
				else if (value == symbol_round) {
					if (!match_token(PGN_TOKEN_STRING, &game->round, NULL))
						return false;
				}
				else if (value == symbol_white) {
					if (!match_token(PGN_TOKEN_STRING, &game->white, NULL))
						return false;
				}
				else if (value == symbol_black) {
					if (!match_token(PGN_TOKEN_STRING, &game->black, NULL))
						return false;
				}
				else if (value == symbol_result) {
					if (match_token(PGN_TOKEN_STRING, &value, NULL)) {
						if (!convert_result(game, value)) {
							result = PGN_READER_ERROR_SYNTAX;
							sprintf_s(error_message, ERROR_BUFFER, "SYNTAX ERROR: unknown value %s", cstring_get(value));
							return false;
						}
					}
					else
						return false;
				}
				if (!match_token(PGN_TOKEN_TAG_END, NULL, NULL))
					return false;
			}
		}
	}

	return true;
}

const char *parse_ply_piece(struct PGN_ply *ply, const char *str) {
	const char *index;

	if (str == NULL || !*str)
		return NULL;

	if ((index = strchr(piece_notations + PGN_PIECE_PAWN, *str)) != NULL) {	// Does ply start with a valid piece letter?
		ply->piece = index - piece_notations + PGN_PIECE_PAWN;
		++str;
	}
	else {
		if (strchr(file_notations, *str) == NULL)			// Then does ply not start with a file letter i.e. omitting the piece info?
			return NULL;
		// Piece info is missing = > it's a pawn!
		ply->piece = PGN_PIECE_PAWN;
	}
	return str;
}

const char *parse_ply_square(struct PGN_ply *ply, const char *str) {
	const char *index;
	bool from_file_detected = false, from_rank_detected = false;

	// [a-h]?[1-8]?[x]?[a-h][1-8]

	if (str == NULL || !*str)
		return NULL;

	if (((index = strchr(file_notations, *str)) != NULL) && *index) { // Valid file letter?
		from_file_detected = true;
		ply->from_file = ply->to_file = index - file_notations;
		++str;
	}

	if (((index = strchr(rank_notations, *str)) != NULL) && *index) { // Valid rank number?
		from_rank_detected = true;
		ply->from_rank = ply->to_rank = index - rank_notations;
		++str;
			//return (from_file_detected && from_rank_detected) ? str : NULL;
	}

	// A capture symbol before square?
	if (*str == PGN_TAG_CAPTURE) {
		ply->capture = 1;
		++str;
	}

	if (((index = strchr(file_notations, *str)) != NULL) && *index) {	// Valid file letter?
		ply->to_file = index - file_notations;
		if (!from_file_detected)
			ply->from_file = ply->to_file;
		++str;
	}

	if (((index = strchr(rank_notations, *str)) != NULL) && *index) {	// No valid rank number?
		ply->to_rank = index - rank_notations;
		if (!from_rank_detected)
			ply->from_rank = ply->to_rank;
		++str;
	}

	return str;
}

const char *parse_ply_promotion_piece(struct PGN_ply *ply, const char *str) {
	const char *index;

	if (str == NULL || !*str)
		return NULL;

	if ((index = strchr(piece_notations + PGN_PIECE_KNIGHT, *str)) == NULL)	// Does ply not start with a valid piece letter?
		return NULL;
	ply->promotion = index - piece_notations + PGN_PIECE_KNIGHT;
	++str;
	if (ply->promotion < PGN_PIECE_KNIGHT || ply->promotion > PGN_PIECE_QUEEN)
		return NULL;
	return str;
}

struct PGN_ply * fetch_ply(struct PGN_game *game) {
	cstring value;
	const char *str;
	struct PGN_ply *ply;

	// White move
	if (!match_token(PGN_TOKEN_SYMBOL, &value, NULL))
		return NULL;
	ply = pgn_add_ply(game);
	str = cstring_get(value);
	if (strcmp(PGN_TAG_CASTLING_KING, str) == 0) {
		ply->piece = PGN_PIECE_KING_ROOK;
		ply->to_file = 6;
	}
	else if (strcmp(PGN_TAG_CASTLING_QUEEN, str) == 0) {
		ply->piece = PGN_PIECE_KING_ROOK;
		ply->to_file = 2;
	}
	else {
		if ((str = parse_ply_piece(ply, str)) == NULL)
			return NULL;
		if ((str = parse_ply_square(ply, str)) == NULL)
			return NULL;
		if (*str) {	// more to come?
			if (*str == PGN_TAG_PROMOTION) {
				ply->promotion = PGN_PIECE_QUEEN;
				if (!*(++str))
					return NULL;
				if ((str = parse_ply_promotion_piece(ply, str)) == NULL)
					return NULL;
			}
			if (*str == PGN_TAG_CHECK) {
				ply->check = 1;
				++str;
			}
			else if (*str == PGN_TAG_CHECKMATE) {
				ply->check = 1;
				ply->checkmate = 1;
				++str;
			}
		}
	}

#ifdef DEBUG_PRINT_PLY
	if (ply->piece == PGN_PIECE_KING_ROOK) {
		if (ply->to_file == 6)
			printf(PGN_TAG_CASTLING_KING);
		else if (ply->to_file == 2)
			printf(PGN_TAG_CASTLING_QUEEN);
	} else {
		if (ply->piece > PGN_PIECE_PAWN)
			putchar(piece_notations[ply->piece - PGN_PIECE_PAWN]);
		if (ply->from_file != ply->to_file)
			putchar(file_notations[ply->from_file]);
		if (ply->from_rank != ply->to_rank)
			putchar(rank_notations[ply->from_rank]);
		if (ply->capture)
			putchar(PGN_TAG_CAPTURE);
		if (ply->promotion) {
			putchar(PGN_TAG_PROMOTION);
			putchar(piece_notations[ply->promotion - PGN_PIECE_PAWN]);
		}
		putchar(file_notations[ply->to_file]);
		putchar(rank_notations[ply->to_rank]);
		if (ply->check)
			putchar(PGN_TAG_CHECK);
		if (ply->checkmate)
			putchar(PGN_TAG_CHECKMATE);
	}
	putchar(' ');
#endif

	return ply;
}

bool fetch_moves(struct PGN_game *game) {
	struct PGN_ply *ply;

	while (next_token != PGN_TOKEN_EOF) {
		// Move number found?
		if (next_token == PGN_TOKEN_INTEGER) {
			// Skip this number and...
			if (!match_token(PGN_TOKEN_INTEGER, NULL, NULL))
				return false;
			// ...the required following period
			if (!match_token(PGN_TOKEN_PERIOD, NULL, NULL))
				return false;
		}

		if (next_token == PGN_TOKEN_ASTERIX) // An asterix parsed?
			break;	// Then the end of the game is detected.

		// Does another type of result follows?
		if(next_token == PGN_TOKEN_SYMBOL && convert_result(game, next_token_word))
			break;	// Then the end of the game is detected.

#ifdef DEBUG_PRINT_PLY
		printf("%d.", 1+(int)(game->plies / 2));
#endif
		// White move
		ply = fetch_ply(game);
		if (ply == NULL)
			return false;
		ply->white = 1;

		// Does another type of result follows?
		if (next_token == PGN_TOKEN_SYMBOL && convert_result(game, next_token_word))
			break;	// Then the end of the game is detected.

		// Black move
		ply = fetch_ply(game);
		if (ply == NULL)
			return false;
#ifdef DEBUG_PRINT_PLY
		putchar('\n');
#endif
	}

	return true;
}

/*
 * PGN_read - read PGN file from given filename and return new game_tree
 */
struct PGN_tree *pgn_reader(const char*filename) {
	struct PGN_tree *game_tree = pgn_new_tree();
	if (game_tree == NULL) {
		result = PGN_READER_ERROR_MEMORY;
		sprintf_s(error_message, ERROR_BUFFER, "OUT OF MEMORY");
		goto finally;
	}

	saved_character = 0;
	result = PGN_READER_OK;
	current_column = 0;
	current_line = 0;
	sprintf_s(error_message, ERROR_BUFFER, "OK");

	symbol_event = cstring_add("Event");
	symbol_site = cstring_add("Site");
	symbol_date = cstring_add("Date");
	symbol_round = cstring_add("Round");
	symbol_white = cstring_add("White");
	symbol_black = cstring_add("Black");
	symbol_result = cstring_add("Result");
	symbol_result_black = cstring_add("0-1");
	symbol_result_white = cstring_add("1-0");
	symbol_result_draw = cstring_add("1/2-1/2");
	symbol_result_unknown = cstring_add("*");

	if(fopen_s(&fp, filename, "r")) {
		result = PGN_READER_ERROR_NOFILE;
		sprintf_s(error_message, ERROR_BUFFER, "File \"%s\" not found / openend.", filename);
		goto finally;
	}
	current_line = 1;

	if (read_token()) {
		for (;;) {
			if (next_token == PGN_TOKEN_EOF)
				break;
			else if (next_token == PGN_TOKEN_TAG_START) {
				struct PGN_game *game = pgn_add_game(game_tree);
				if (game == NULL) {
					result = PGN_READER_ERROR_MEMORY;
					sprintf_s(error_message, ERROR_BUFFER, "OUT OF MEMORY");
					goto finally;
				}
				if (!fetch_tags(game))
					break;
				if (!fetch_moves(game))
					break;
			}
			else
				if (!read_token())
					break;
		}
	}

finally:
	if (fp != NULL) {
		fclose(fp);
		fp = NULL;
	}

	if (game_tree != NULL && result != PGN_READER_EOF && result != PGN_READER_OK) {
		free(game_tree);
		game_tree = NULL;
	}

	return game_tree;
}