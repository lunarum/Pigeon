#include "pgn.h"
#include <stdlib.h>

char piece_notations[] = " PNBRQK";
char file_notations[] = "abcdefgh";
char rank_notations[] = "12345678";
char castlingK_notation[] = "O-O";
char castlingQ_notation[] = "O-O-O";

/// <summary>
/// pgn_new_tree - create a new (empty) game_tree
/// </summary>
/// <returns>
/// the newly created game tree/>
/// </returns>

struct PGN_tree *pgn_new_tree() {
	// Allocate zeroed block of memory
	struct PGN_tree *game_tree = calloc(1, sizeof(struct PGN_tree));
	return game_tree;
}

/// <summary>
/// pgn_add_game - add a new game to the given game_tree
/// </summary>
/// <param name="game_tree">A PGN game_tree where the newly created game must be added to</param>
/// <returns>
/// the newly (appended) game struct added to the tree of <paramref name="game_tree"/>
/// </returns>
struct PGN_game *pgn_add_game(struct PGN_tree *game_tree) {
	struct PGN_game *game = NULL;

	if (game_tree != NULL) {
		// Allocate zeroed block of memory
		game = calloc(1, sizeof(struct PGN_game));
		if (game != NULL) {
			if (game_tree->last_game == NULL) {
				game->next_game = game;
			}
			else {
				game->next_game = game_tree->last_game->next_game;
				game_tree->last_game->next_game = game;
			}
			game_tree->last_game = game;
			++game_tree->games;
		}
	}

	return game;
}

/// <summary>
/// pgn_add_ply - add a new ply to the given game
/// </summary>
/// <param name="game">A PGN game where the newly created ply must be added to</param>
/// <returns>
/// the newly (appended) ply struct added to the plies of <paramref name="game"/>
/// </returns>
struct PGN_ply * pgn_add_ply(struct PGN_game *game) {
	struct PGN_ply *ply = NULL;

	if (game != NULL) {
		// Allocate zeroed block of memory
		ply = calloc(1, sizeof(struct PGN_ply));
		if (ply != NULL) {
			if (game->last_ply == NULL) {
				ply->next_ply = ply;
			}
			else {
				ply->next_ply = game->last_ply->next_ply;
				game->last_ply->next_ply = ply;
			}
			game->last_ply = ply;
			++game->plies;
		}
	}

	return ply;
}

/// <summary>
/// pgn_reset_games - reset game_tree, remove / free all games
/// </summary>
/// <param name="game_tree">The PGN game_tree to be reset</param>
void pgn_reset_games(struct PGN_tree *game_tree) {
	struct PGN_game *game, *next_game;
	struct PGN_ply *ply, *next_ply;

	if (game_tree != NULL) {
		// destroy all game entries
		for (game = game_tree->last_game; game != NULL; game = next_game) {
			// save next game
			next_game = game->next_game;

			// destroy all plies
			for (ply = game->last_ply; ply != NULL; ply = next_ply) {
				// save next ply
				next_ply = ply->next_ply;
				// now destroy ply
				free(ply);
				if (next_ply == game->last_ply)
					break;	// last reached
			}

			// now destroy game itself
			free(game);
			if (next_game == game_tree->last_game)
				break;	// last reached
		}
		game_tree->games = 0;
		game_tree->last_game = NULL;
	}
}