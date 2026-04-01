#include <SDL.h>
#include "simple_logger.h"
#include "gfc_input.h"
#include "gf2d_graphics.h"
#include "gf2d_sprite.h"
#include "gf2d_draw.h"
#include "entity.h"
#include "player.h"
#include "world.h"
#include "camera.h"
#include "simple_ui.h"
#include "item.h"
#include "simple_font.h"
#include "console.h"

typedef enum {
    GS_MAINMENU,
    GS_PLAYING,
    GS_DEATH,
    GS_PAUSED
}GameState;

typedef struct {
    int                 mx;                 //the x coordinate of the mouse
    int                 my;                 //the y coordinate of the mouse
    Uint8               done;               //1 when game quits, 0 otherwise
    Uint8               paused;             //1 when game is paused, 0 otherwise
    Uint8               debug;
    GameState           state;              //tracks the current game state
    const Uint8*        keys;               //tracks the keys pressed
    GenericMenu*        mainMenu;           //the main menu pointer
    GenericMenu*        deathMenu;          //the death menu pointer
    GenericMenu*        pauseMenu;          //the pause menu pointer
    Floor*              floor;              //the current floor
    Entity*             player;             //the player pointer
    Sprite*             mouse;              //the mouse pointer
    GFC_Vector2D        mouseScale;
    Stage*              currentStage;       //the current stage the player is in
    HUD*                hud;
    GFC_Rect            ftb;                //fade to black rect
    float               ftb_alpha;
}System;


void update_game(System* game) {
    switch (game->state) {
        int i;
        int x, y;
        Entity* collider;
        Doors exitSide;
        GFC_Vector2I gridPos;

    case GS_MAINMENU:
        menu_update(game->mainMenu);
        if (game->mainMenu->Menu.start.quitButton.clicked) game->done = 1;
        if (game->mainMenu->Menu.start.startButton.clicked) {
            slog("GAME START! Attempting floor generation");
            game->floor = floor_create(10,2,1,1,rand());
            if (!game->floor) {
                slog("ERROR: Floor Generation Failed");
            }
            else {
                slog("Floor Generated, finding starting room...");
                print_floor(game->floor);
            }
            for (i = 0; i < game->floor->width * game->floor->height; i++) {
                if (game->floor->blueprint[i] == RT_START) {
                    slog("Starting room found: Index: %i",i);
                    game->currentStage = game->floor->floorMap[i];
                    game->currentStage->room = room_load(game->currentStage->filename,get_room_type_string(game->currentStage->type));
                    set_active_room(game->currentStage->room);
                    load_stage(game->floor, game->currentStage);
                    break;
                }
            }
            if (!game->currentStage->room){
                slog("ERROR: current room is NULL. RIP, enjoy the segfault");
            }
            game->player = player_new();
            gridPos.x = game->currentStage->room->width / 2;
            gridPos.y = game->currentStage->room->height - 2;
            set_center(game->player, grid_to_world(gridPos));
            game->state = GS_PLAYING;
        }
        break;

    case GS_DEATH:
        if(game->player){
            if (game->player->frame < 125) {
                game->player->frame = 125;
                game->ftb = gfc_rect(0, 0, 1200, 720);
                game->ftb_alpha = 0;
            }

            game->player->frame += 0.1;
            if (game->player->frame >= 142){
                game->player->frame = 142;
                game->ftb_alpha += 1.5;
            }
        }

        if (game->ftb_alpha >= 255) {
            if (game->currentStage) clear_stage();
            if (game->player) {
                entity_free(game->player);
                game->player = NULL;
            }
            if (game->floor) {
                free_world(game->floor);
                game->floor = NULL;
            }
            if (game->currentStage) {
                game->currentStage = NULL;
            }

            menu_update(game->deathMenu);
            if (game->deathMenu->Menu.death.restartButton.clicked) game->done = 1;
            if (game->deathMenu->Menu.death.menuButton.clicked) game->state = GS_MAINMENU;
        }
        
        break;

    case GS_PAUSED:
        menu_update(game->pauseMenu);
        if (game->pauseMenu->Menu.pause.menuButton.clicked) {
            if (game->player) {
                entity_free(game->player);
                game->player = NULL;
            }
            if (game->floor) {
                free_world(game->floor);
                game->floor = NULL;
            }
            if (game->currentStage) {
                game->currentStage = NULL;
            }
            game->paused = 0;
            game->state = GS_MAINMENU;
        }
        if (game->pauseMenu->Menu.pause.resumeButton.clicked) {
            game->paused = 0;
            game->state = GS_PLAYING;
        }
        break;

    case GS_PLAYING:

        entity_manager_think_all();
        entity_manager_update_all();
        item_manager_think_all();
        hud_update(game->hud, game->player);

        collider = check_entity_collision(game->player);
        if (collider && collider->type == ET_DOOR) {
            slog("player collided with a door");
            exitSide = get_opposite_side(((DoorData*)collider->data)->side);
            slog("Player exits on side: %i", exitSide);
            slog("Target Room Index: %i", ((DoorData*)collider->data)->targetRoom);
            game->currentStage = game->floor->floorMap[((DoorData*)collider->data)->targetRoom];
            slog("current stage set to door target room");
            clear_stage();
            item_manager_free_all();
            if (game->player->currentTiles) {
                gfc_list_delete(game->player->currentTiles);
                game->player->currentTiles = NULL;
            }
            slog("stage cleared of entities");
            floor_update_active_rooms(game->floor, game->currentStage->gridPos.x, game->currentStage->gridPos.y);
            slog("active rooms updated");
            load_stage(game->floor, game->currentStage);
            slog("stage loaded");
            set_active_room(game->currentStage->room);
            slog("active room set to current stage room");
            spawn_at_door_exit(game->player, game->currentStage->room, exitSide);
            slog("player spawned at exit");
        }

        if (((PlayerData*)game->player->data)->health <= 0) {
            game->state = GS_DEATH;
        }
        if (game->keys[SDL_SCANCODE_P]) {
            game->paused = 1;
            game->state = GS_PAUSED;
        }
        break;
    }
}

void draw_game(System* game) {
    gf2d_graphics_clear_screen();

    GFC_Color mouseGFC_Color = gfc_color8(0, 100, 255, 200);

    switch (game->state) {
    case GS_MAINMENU:

        menu_draw(game->mainMenu);

        draw_mouse(game->mainMenu, game->mouse, game->mx, game->my, game->mouseScale);

        break;
    case GS_DEATH:

        if (game->ftb_alpha < 255) {
            room_draw(game->currentStage->room);

            entity_manager_draw_all(game->debug);

            item_manager_draw_all();

            camera_center_on(gfc_vector2d(game->player->position.x + (game->player->sprite->frame_w / 2), game->player->position.y + (game->player->sprite->frame_h / 2)));

            draw_hud(game->hud, game->player);

            gf2d_draw_rect_filled(game->ftb, gfc_color8(0, 0, 0, game->ftb_alpha));

            console_draw();
        }
        else{

            menu_draw(game->deathMenu);

            draw_mouse(game->deathMenu, game->mouse, game->mx, game->my, game->mouseScale);
        }

        break;
    case GS_PAUSED:

        room_draw(game->currentStage->room);

        entity_manager_draw_all(game->debug);

        camera_center_on(gfc_vector2d(game->player->position.x + (game->player->sprite->frame_w / 2), game->player->position.y + (game->player->sprite->frame_h / 2)));

        menu_draw(game->pauseMenu);

        draw_mouse(game->pauseMenu,game->mouse,game->mx,game->my,game->mouseScale);

        console_draw();
        break;
    case GS_PLAYING:
        room_draw(game->currentStage->room);

        entity_manager_draw_all(game->debug);

        item_manager_draw_all();

        camera_center_on(gfc_vector2d(game->player->position.x + (game->player->sprite->frame_w / 2), game->player->position.y + (game->player->sprite->frame_h / 2)));

        draw_hud(game->hud, game->player);

        if (((PlayerData*)game->player->data)->health < 0) {
            gf2d_draw_rect_filled(game->ftb,gfc_color8(0,0,0,game->ftb_alpha));
        }

        console_draw();
        break;
    }

    

    gf2d_graphics_next_frame();
}


int main(int argc, char * argv[])
{
    System* game = gfc_allocate_array(sizeof(System),1);

    /*variable declarations*/

    game->done = 0;
    game->paused=0;
    game->state = GS_MAINMENU;



    /*program initializtion*/

    init_logger("gf2d.log",0);
    slog("---==== BEGIN ====---");
    gf2d_graphics_initialize(
        "GODPUNCH",
        1200,
        720,
        1200,
        720,
        gfc_vector4d(0,0,0,255),
        0);
    camera_set_dimension(gfc_vector2d(1200, 720));
    gf2d_graphics_set_frame_delay(16);
    gf2d_sprite_init(1024);
    simple_font_init();
    console_init();
    game->hud = hud_init();
    entity_manager_init(1024);
    SDL_ShowCursor(SDL_DISABLE);
    item_manager_init(1024);




    /*demo setup*/

    game->mouse = gf2d_sprite_load_all("images/ui/cursor.png",2048,2048,1,0);
    game->mouseScale = gfc_vector2d(0.01562, 0.01562);
    GFC_Vector2D offset = camera_get_offset();
    game->mainMenu = main_menu_init();
    game->deathMenu = death_menu_init();
    game->pauseMenu = pause_menu_init();

    slog("press [escape] to quit");


    /*main game loop*/
    while(!game->done)
    {
        SDL_Event event;
        
        while (SDL_PollEvent(&event)) {
            console_pass_event(&event);

            if (event.type == SDL_QUIT) {
                game->done = 1;
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.repeat == 0 && event.key.keysym.sym == SDLK_F11) {
                    gf2d_graphics_toggle_fullscreen();
                }
                else if (event.key.repeat == 0 && event.key.keysym.sym == SDLK_F6) {
                    game->debug = !game->debug;
                }
            }
        }

        SDL_PumpEvents();   // update SDL's internal event structures
        


        game->keys = SDL_GetKeyboardState(NULL); // get the keyboard state for this frame

        SDL_GetMouseState(&game->mx, &game->my);

        update_game(game);

        draw_game(game);
        
        if (game->keys[SDL_SCANCODE_ESCAPE])game->done = 1; // exit condition
        //slog("Rendering at %f FPS",gf2d_graphics_get_frames_per_second());
    }

    //FREE EVERYTHING

    item_manager_free_all();
    if (game->player) {
        entity_free(game->player);
        game->player = NULL;
    }
    entity_manager_free_all();
    if (game->floor) {
        free_world(game->floor);
        game->floor = NULL;
    }
    if (game->currentStage) {
        game->currentStage = NULL;
    }
    menu_free(game->mainMenu);
    menu_free(game->deathMenu);
    menu_free(game->pauseMenu);
    free(game);
    console_free();
    simple_font_close();
    slog("---==== END ====---");
    return 0;
}
/*eol@eof*/
