// main.c : main project file.

#include "pgn_reader.h"
#include <stdio.h>


int main(int argc, char *argv[])
{
	int a, l, c;
	unsigned int n;
	char *m;
	struct PGN_tree *tree;
	struct PGN_game *game;

	printf("PiGeoN-Reader v0.2\n");
	for (a = 1; a < argc; ++a) {
		printf("Parsing '%s'\n", argv[a]);
		tree = pgn_reader(argv[a]);
		pgn_read_result(&l, &c, &m);
		printf("\nread until line %d column %d: %s\n", l, c, m);
		if (tree != NULL) {
			printf("Total of %d games read\n", tree->games);
			for (n = 0, game = tree->last_game->next_game; n < tree->games && n < 10; ++n, game = game->next_game) {
				switch (game->result) {
				case PGN_RESULT_WHITE_WIN:
					m = "1-0";
					break;
				case PGN_RESULT_BLACK_WIN:
					m = "0-1";
					break;
				case PGN_RESULT_DRAW:
					m = "1/2-1/2";
					break;
				default:
					m = "?";
				}
				printf("Event '%s', '%s' vs. '%s' (%s after %d plies)\n",
					cstring_get(game->event),
					cstring_get(game->white),
					cstring_get(game->black),
					m,
					game->plies);
			}
			if(tree->games > 10)
				puts("...\n");
		}
	}
    return 0;
}
