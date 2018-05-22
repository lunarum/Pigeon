#include "pgn_reader.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>

//#define DEBUG_PRINT_TOKENS 1


#define ERROR_BUFFER 256

FILE *fp;
char saved_character;
int current_column;
int current_line;
char error_message[ERROR_BUFFER];
enum PGN_READER_RESULT result;

enum PGN_TOKEN token = PGN_TOKEN_NONE;
cstring token_value;
unsigned int token_integer_value;

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

	token = PGN_TOKEN_NONE;
	token_value = CSTRING_NULL;
	token_integer_value = 0;

	// Skip whitespace
	do {
		c = read_char();
		if (c == EOF)
			break;
	} while (iscntrl(c) || isspace(c));

	switch (c) {
	case EOF:
		token = PGN_TOKEN_EOF;
		result = PGN_READER_EOF;
		return true;
	case PGN_TAG_TAG_START:
		token = PGN_TOKEN_TAG_START;
		break;
	case PGN_TAG_TAG_END:
		token = PGN_TOKEN_TAG_END;
		break;
	case PGN_TAG_PERIOD:
		token = PGN_TOKEN_PERIOD;
		break;
	case PGN_TAG_ASTERIX:
		token = PGN_TOKEN_ASTERIX;
		break;
	case PGN_TAG_STRING:
		token = PGN_TOKEN_STRING;
		c = read_char();
		for (n = 0; n < PGN_MAX_LINE && c != EOF && c != PGN_TAG_STRING; ++n, c = read_char()) {
			if (c == PGN_TAG_STRING_ESCAPE) {
				c = read_char();
			}
			buffer[n] = c;
		}
		buffer[n] = 0;
		token_value = cstring_add(buffer);
		break;
	default:
		if (isalnum(c)) {
			token = isdigit(c) ? PGN_TOKEN_INTEGER : PGN_TOKEN_SYMBOL;
			for (n = 0; n < PGN_MAX_LINE && c != EOF && (isalnum(c) || strchr("_+#=:-/", c) != NULL); ++n, c = read_char()) {
				buffer[n] = c;
				if (isdigit(c))
					token_integer_value = token_integer_value * 10 + (c - '0');
				else
					token = PGN_TOKEN_SYMBOL;
			}
			buffer[n] = 0;
			if (token == PGN_TOKEN_SYMBOL) {
				token_value = cstring_add(buffer);
				token_integer_value = 0;
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
	printf("token(%d)", token);
	if(token_value != CSTRING_NULL)
		printf(" string(%s)", cstring_get(token_value));
	if(token_integer_value != 0)
		printf(" int(%d)", token_integer_value);
	putchar('\n');
#endif

	return true;
}

bool match_token(enum PGN_TOKEN tok, cstring *value_str, int *value_int) {
	if (token != tok) {
		result = PGN_READER_ERROR_SYNTAX;
		sprintf_s(error_message, ERROR_BUFFER, "SYNTAX ERROR: token %d expected", (int)tok);
		return false;
	}
	if (value_str != NULL)
		*value_str = token_value;
	if (value_int != NULL)
		*value_int = token_integer_value;
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

	while (token == PGN_TOKEN_TAG_START) {
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

bool fetch_moves(struct PGN_game *game) {
	cstring value;
	struct PGN_ply *ply;

	while (token != PGN_TOKEN_EOF) {
		if (token == PGN_TOKEN_INTEGER) {
			// Move number found; skip this number and the required following period
			if (!match_token(PGN_TOKEN_INTEGER, NULL, NULL))
				return false;
			if (!match_token(PGN_TOKEN_PERIOD, NULL, NULL))
				return false;
		}
		if (token == PGN_TOKEN_ASTERIX) // result (unnkown) is last ply of this game
			break;
		// White move
		if (!match_token(PGN_TOKEN_SYMBOL, &value, NULL))
			return false;
		// Not a move but a result?
		if (convert_result(game, value))
			break;
		ply = pgn_add_ply(game);
		if (token == PGN_TOKEN_ASTERIX) // result (unnkown) is last ply of this game
			break;
		// Black move
		if (!match_token(PGN_TOKEN_SYMBOL, &value, NULL))
			return false;
		// Not a move but a result?
		if (convert_result(game, value))
			break;
		ply = pgn_add_ply(game);
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
			if (token == PGN_TOKEN_EOF)
				break;
			else if (token == PGN_TOKEN_TAG_START) {
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