#ifndef PGN_READER_H
#define PGN_READER_H

#include "pgn.h"

enum PGN_READER_RESULT {
	PGN_READER_ERROR_NOFILE = -1,	// file not opened / found
	PGN_READER_ERROR_MEMORY = -2,	// PGN file contains syntax error / non confirmation to PGN standard
	PGN_READER_ERROR_SYNTAX = -3,	// out of memory
	PGN_READER_EOF = 1,				// EOF reached (no more tokens / data)
	PGN_READER_OK = 0
};

enum PGN_TOKEN {
	PGN_TOKEN_NONE = 0,
	PGN_TOKEN_STRING,	// sequence of zero or more printing characters (ISO 8859/1) delimited by a pair of quote characters. Backslash is excape character. Max. length 255.
	PGN_TOKEN_INTEGER,	// sequence of one or more decimal digit characters.  It is terminated just prior to the first non-digit character.
	PGN_TOKEN_COMMENT,	// rest of line comment starts with a ; and continues to the end of the line. The second kind starts with a { and continues to the next }. Comments cannot appear inside any token.
	PGN_TOKEN_ESCAPE,	// The escape mechanism is triggered by a % appearing in the first column of a line; the data on the rest of the line is ignored by publicly available PGN scanning software.
	PGN_TOKEN_SYMBOL,	// A symbol token starts with a letter or digit character and is immediately followed by a sequence of zero or more "A-Za-z", "0-9", "_", "+", "#", "=", ":", "-". A symbol token is terminated just prior to the first non-symbol character. Currently, a symbol is limited to a maximum of 255 characters in length.
	PGN_TOKEN_PERIOD,
	PGN_TOKEN_ASTERIX,
	PGN_TOKEN_NAG,
	PGN_TOKEN_TAG_START,
	PGN_TOKEN_TAG_END,
	PGN_TOKEN_COMMENT_START,
	PGN_TOKEN_COMMENT_END,
	PGN_TOKEN_LIST_START,
	PGN_TOKEN_LIST_END,
	PGN_TOKEN_FUTURE_START,
	PGN_TOKEN_FUTURE_END,
	PGN_TOKEN_EOF
};

struct PGN_tree *pgn_reader(const char*filename);
enum PGN_READER_RESULT pgn_read_result(int *line, int *column, char **message);
#endif