#include "simple_logger.h"
#include "console.h"
#include "gfc_string.h"
#include "player.h"
#include "monster.h"
#include "world.h"
#include "item.h"
#include <string.h>
#include <stdlib.h>
#include "simple_font.h"
#include "gf2d_draw.h"

static int _open = 0;
static GFC_String* consoleText;
static Font* consoleFont;

void console_init() {
	_open = 0;
	consoleText = gfc_string_new();
	consoleFont = simple_font_load("fonts/minecraft.ttf",16);
	slog("console init");
}

void console_pass_event(SDL_Event* event) {
	if (event->type == SDL_KEYDOWN) {
		if (event->key.keysym.sym == SDLK_F12) {
			_open = !_open;
			if (_open) {
				SDL_StartTextInput();
			}
			else {
				SDL_StopTextInput();
				consoleText->buffer[0] = '\0';
				consoleText->length = 0;
			}
		}

		if (_open) {
			if (event->key.keysym.sym == SDLK_BACKSPACE && consoleText->length > 0) {
				consoleText->length--;
				consoleText->buffer[consoleText->length] = '\0';
			}
			else if (event->key.keysym.sym == SDLK_RETURN && consoleText->length > 0) {
				slog("command entered");

				console_execute(consoleText->buffer);

				consoleText->buffer[0] = '\0';
				consoleText->length = 0;

				_open = 0;
				SDL_StopTextInput();
			}
		}
	}

	else if (event->type == SDL_TEXTINPUT && _open) {
		if (event->text.text[0] != '`') {
			gfc_string_append(consoleText, event->text.text);
		}
	}
}

void console_execute(const char* cmd_line) {
	char cmd[256];
	char* token;
	char* args[10] = { NULL };
	int argc = 0;

	if (!cmd_line || strlen(cmd_line) == 0) return;

	strncpy(cmd, cmd_line, 255);
	cmd[255] = '\0';

	token = strtok(cmd, " ");
	while (token != NULL && argc < 10) {
		args[argc] = token;
		argc++;
		token = strtok(NULL, " ");
	}

	if (argc == 0) return;

	if (strcmp(args[0], "spawn") == 0) {
		Entity* player = get_player_entity();
		GFC_Vector2D position = { 0,0 };
		if (player) {
			position.x = player->position.x;
			position.y = player->position.y;
		}
		if (argc >= 4 && strcmp(args[3], "~") != 0) {
			position.x = atoi(args[3]);
		}
		if (argc >= 5 && strcmp(args[4], "~") != 0) {
			position.y = atoi(args[4]);
		}

		if(argc >=3 && strcmp(args[1], "monster")==0) {
			if (strcmp(args[2], "damned") == 0) {
				monster_spawn(MT_DAMNED, position);
			}
			else if (strcmp(args[2], "fiend") == 0) {
				monster_spawn(MT_FIEND, position);
			}
			else if (strcmp(args[2], "imp") == 0) {
				monster_spawn(MT_IMP, position);
			}
			else if (strcmp(args[2], "hellhound") == 0) {
				monster_spawn(MT_HELLHOUND, position);
			}
			else if (strcmp(args[2], "repenter") == 0) {
				monster_spawn(MT_REPENTER, position);
			}
			else {
				slog("unknown monster: '%s'", args[2]);
			}
		}
	}
	else if (strcmp(args[0], "give") == 0) {
		Entity* player = get_player_entity();
		PlayerData* stats;
		Item* item;
		GFC_Vector2D position = { 0,0 };
		
		if (argc < 5) {
			slog("not enough arguments");
			return;
		}
		if (!player || !player->data) {
			slog("no active player/stats to give to");
			return;
		}
		if (player) {
			position.x = player->position.x;
			position.y = player->position.y;
		}
		if (argc >= 4 && strcmp(args[3], "~") != 0) {
			position.x = atoi(args[3]);
		}
		if (argc >= 5 && strcmp(args[4], "~") != 0) {
			position.y = atoi(args[4]);
		}

		stats = player->data;

		if(strcmp(args[1], "item")==0) {
			if (strcmp(args[2], "hair_trigger") == 0) {
				item = item_create(ITEM_HAIR_TRIGGER);
				item->position = position;
			}
			else if (strcmp(args[2], "combat_boots") == 0) {
				item = item_create(ITEM_COMBAT_BOOTS);
				item->position = position;
			}
			else if (strcmp(args[2], "commando_bandana") == 0) {
				item = item_create(ITEM_COMMANDO_BANDANA);
				item->position = position;
			}
			else if (strcmp(args[2], "reinforced_ribcage") == 0) {
				item = item_create(ITEM_REINFORCED_RIBCAGE);
				item->position = position;
			}
			else if (strcmp(args[2], "sulfur_tipped_rounds") == 0) {
				item = item_create(ITEM_SULFUR_TIPPED_ROUNDS);
				item->position = position;
			}
			else if (strcmp(args[2], "lead_halo") == 0) {
				item = item_create(ITEM_LEAD_HALO);
				item->position = position;
			}
			else if (strcmp(args[2], "forbidden_knowledge") == 0) {
				item = item_create(ITEM_FORBIDDEN_KNOWLEDGE);
				item->position = position;
			}
			else {
				slog("unknown item: '%s'", args[2]);
			}

		}
		else {
			slog("unknown command");
		}
	}
	else if (strcmp(args[0], "stat") == 0) {
		Entity* player = get_player_entity();
		PlayerData* stats;
		int value;
		stats = player->data;
		if (!player || !stats) {
			slog("no active player/stats to change stats");
		}
		else if (argc>=4) {
			value = atoi(args[3]);
			if(strcmp(args[1], "max_health") == 0) {
				if (strcmp(args[2], "add") == 0) {
					stats->maxHealth += value;
				}
				else if (strcmp(args[2], "sub") == 0) {
					stats->maxHealth -= value;
				}
				else if (strcmp(args[2], "set") == 0) {
					stats->maxHealth = value;
				}
				else {
					slog("unknown action: '%s'", args[1]);
				}
				if (stats->health > stats->maxHealth) stats->health = stats->maxHealth;
			}
			else if (strcmp(args[1], "health") == 0) {
				if (strcmp(args[2], "add") == 0) {
					stats->health += value;
				}
				else if (strcmp(args[2], "sub") == 0) {
					stats->health -= value;
				}
				else if (strcmp(args[2], "set") == 0) {
					stats->health = value;
				}
				else {
					slog("unknown action: '%s'", args[1]);
				}
				if (stats->health > stats->maxHealth) stats->health = stats->maxHealth;
			}
			else if (strcmp(args[1], "move_speed") == 0) {
				if (strcmp(args[2], "add") == 0) {
					stats->moveSpeed += value;
				}
				else if (strcmp(args[2], "sub") == 0) {
					stats->moveSpeed -= value;
				}
				else if (strcmp(args[2], "set") == 0) {
					stats->moveSpeed = value;
				}
				else {
					slog("unknown action: '%s'", args[1]);
				}
			}
			else if (strcmp(args[1], "touch_damage") == 0) {
				if (strcmp(args[2], "add") == 0) {
					stats->touchDamage += value;
				}
				else if (strcmp(args[2], "sub") == 0) {
					stats->touchDamage -= value;
				}
				else if (strcmp(args[2], "set") == 0) {
					stats->touchDamage = value;
				}
				else {
					slog("unknown action: '%s'", args[1]);
				}
			}
			else if (strcmp(args[1], "jumps") == 0) {
				if (strcmp(args[2], "add") == 0) {
					stats->jumps += value;
				}
				else if (strcmp(args[2], "sub") == 0) {
					stats->jumps -= value;
				}
				else if (strcmp(args[2], "set") == 0) {
					stats->jumps = value;
				}
				else {
					slog("unknown action: '%s'", args[1]);
				}
			}
			else if (strcmp(args[1], "shot_speed") == 0) {
				if (strcmp(args[2], "add") == 0) {
					stats->shotSpeed += value;
				}
				else if (strcmp(args[2], "sub") == 0) {
					stats->shotSpeed -= value;
				}
				else if (strcmp(args[2], "set") == 0) {
					stats->shotSpeed = value;
				}
				else {
					slog("unknown action: '%s'", args[1]);
				}
			}
			else if (strcmp(args[1], "range") == 0) {
				if (strcmp(args[2], "add") == 0) {
					stats->range += value;
				}
				else if (strcmp(args[2], "sub") == 0) {
					stats->range -= value;
				}
				else if (strcmp(args[2], "set") == 0) {
					stats->range = value;
				}
				else {
					slog("unknown action: '%s'", args[1]);
				}
			}
			else if (strcmp(args[1], "damage") == 0) {
				if (strcmp(args[2], "add") == 0) {
					stats->damage += value;
				}
				else if (strcmp(args[2], "sub") == 0) {
					stats->damage -= value;
				}
				else if (strcmp(args[2], "set") == 0) {
					stats->damage = value;
				}
				else {
					slog("unknown action: '%s'", args[1]);
				}
			}
			else if (strcmp(args[1], "temp_health") == 0) {
				if (strcmp(args[2], "add") == 0) {
					stats->tempHealth += value;
				}
				else if (strcmp(args[2], "sub") == 0) {
					stats->tempHealth -= value;
				}
				else if (strcmp(args[2], "set") == 0) {
					stats->tempHealth = value;
				}
				else {
					slog("unknown action: '%s'", args[1]);
				}
			}
			else if (strcmp(args[1], "fire_rate") == 0) {
				if (strcmp(args[2], "add") == 0) {
					stats->fireRate += value;
				}
				else if (strcmp(args[2], "sub") == 0) {
					stats->fireRate -= value;
				}
				else if (strcmp(args[2], "set") == 0) {
					stats->fireRate = value;
				}
				else {
					slog("unknown action: '%s'", args[1]);
				}
			}
			else if (strcmp(args[1], "dash_duration") == 0) {
				if (strcmp(args[2], "add") == 0) {
					stats->dashDuration += value;
				}
				else if (strcmp(args[2], "sub") == 0) {
					stats->dashDuration -= value;
				}
				else if (strcmp(args[2], "set") == 0) {
					stats->dashDuration = value;
				}
				else {
					slog("unknown action: '%s'", args[1]);
				}
			}
			else if (strcmp(args[1], "slam_damage") == 0) {
				if (strcmp(args[2], "add") == 0) {
					stats->slamDamage += value;
				}
				else if (strcmp(args[2], "sub") == 0) {
					stats->slamDamage -= value;
				}
				else if (strcmp(args[2], "set") == 0) {
					stats->slamDamage = value;
				}
				else {
					slog("unknown action: '%s'", args[1]);
				}
			}
			else if (strcmp(args[1], "slam_cooldown") == 0) {
				if (strcmp(args[2], "add") == 0) {
					stats->slamCooldown += value;
				}
				else if (strcmp(args[2], "sub") == 0) {
					stats->slamCooldown -= value;
				}
				else if (strcmp(args[2], "set") == 0) {
					stats->slamCooldown = value;
				}
				else {
					slog("unknown action: '%s'", args[1]);
				}
			}
			else if (strcmp(args[1], "shove_cooldown") == 0) {
				if (strcmp(args[2], "add") == 0) {
					stats->shoveCooldown += value;
				}
				else if (strcmp(args[2], "sub") == 0) {
					stats->shoveCooldown -= value;
				}
				else if (strcmp(args[2], "set") == 0) {
					stats->shoveCooldown = value;
				}
				else {
					slog("unknown action: '%s'", args[1]);
				}
			}
			else if (strcmp(args[1], "pull_cooldown") == 0) {
				if (strcmp(args[2], "add") == 0) {
					stats->pullCooldown += value;
				}
				else if (strcmp(args[2], "sub") == 0) {
					stats->pullCooldown -= value;
				}
				else if (strcmp(args[2], "set") == 0) {
					stats->pullCooldown = value;
				}
				else {
					slog("unknown action: '%s'", args[1]);
				}
			}
			else if (strcmp(args[1], "dash_cooldown") == 0) {
				if (strcmp(args[2], "add") == 0) {
					stats->dashCooldown += value;
				}
				else if (strcmp(args[2], "sub") == 0) {
					stats->dashCooldown -= value;
				}
				else if (strcmp(args[2], "set") == 0) {
					stats->dashCooldown = value;
				}
				else {
					slog("unknown action: '%s'", args[1]);
				}
			}
		}
	}
	else if (strcmp(args[0], "flag") == 0) {
		Entity* player = get_player_entity();
		PlayerData* stats;
		int value;
		if (player) stats = player->data;
		if (!player) {
			slog("no active player to change stats");
		}
		else if (argc >= 3) {
			value = atoi(args[2]);
			if (value == 1 || value == 0) {
				if (strcmp(args[1], "gravity") == 0) {
					player->gravity = value;
				}
			}
		}
	}
	else {
		slog("unknown command");
	}
	return;
}

int get_console_open() {
	return _open;
}

void console_free() {
	if (consoleText) gfc_string_free(consoleText);
	if (consoleFont)simple_font_free(consoleFont);
	slog("console freed");
}

void console_draw() {
	GFC_Rect consoleBar;
	GFC_Color overlay;
	GFC_String* display;
	SDL_Color textColor = { 255,255,255,255 };

	if (!_open) return;

	overlay = gfc_color(0, 0, 0, 0.7);
	consoleBar.x = 0;
	consoleBar.y = 0;
	consoleBar.w = 1200;
	consoleBar.h = 40;

	gf2d_draw_rect_filled(consoleBar, overlay);

	display = gfc_string_new();

	gfc_string_appendf(display, "] %s", consoleText->buffer);

	if (consoleFont) {
		simple_font_draw(consoleFont, display->buffer, gfc_vector2d(10, 10), textColor);
	}

	gfc_string_free(display);
}


/*eol@eof*/
