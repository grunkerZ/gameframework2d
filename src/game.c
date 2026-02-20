#include <SDL.h>
#include "simple_logger.h"

#include "gf2d_graphics.h"
#include "gf2d_sprite.h"
#include "entity.h"
#include "player.h"
#include "world.h"
#include "camera.h"
#include "monster.h"
#include "simple_ui.h"

int main(int argc, char * argv[])
{
    typedef enum {
        GS_MAINMENU,
        GS_PLAYING,
        GS_DEATH
    }GameState;


    /*variable declarations*/

    int done = 0;
    const Uint8 * keys;
    Level* level;
    int mx,my;
    float mf = 0;
    Sprite *mouse;
    Entity* player;
    Entity* monster;
    GFC_Color mouseGFC_Color = gfc_color8(0,100,255,200);
    GameState state = GS_MAINMENU;
    GenericMenu* mainMenu;
    GenericMenu* deathMenu;
    

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
    entity_manager_init(1024);
    SDL_ShowCursor(SDL_DISABLE);



    /*demo setup*/

    mouse = gf2d_sprite_load_all("images/pointer.png",32,32,16,0);
    player = player_new();
    monster = monster_new();
    level = level_load("maps/testworld.map");
    GFC_Vector2D offset = camera_get_offset();
    mainMenu = main_menu_init();
    deathMenu = death_menu_init();

    slog("press [escape] to quit");


    /*main game loop*/
    while(!done)
    {
        SDL_PumpEvents();   // update SDL's internal event structures
        keys = SDL_GetKeyboardState(NULL); // get the keyboard state for this frame

        SDL_GetMouseState(&mx, &my);
        mf += 0.1;
        if (mf >= 16.0)mf = 0;

        gf2d_graphics_clear_screen(); //draw after updates

        switch (state) {
            case GS_MAINMENU:
                //updates
                menu_update(mainMenu);
               
                //draw
                menu_draw(mainMenu);
                gf2d_sprite_draw(
                    mouse,
                    gfc_vector2d(mx, my),
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    &mouseGFC_Color,
                    (int)mf);

                if (mainMenu->Menu.start.startButton.clicked) {
                    state = GS_PLAYING;
                    entity_free(player);
                    entity_free(monster);
                    level_free(level);
                    player = player_new();
                    monster = monster_new();
                    level = level_load("maps/testworld.map");
                }
                if (mainMenu->Menu.start.exitButton.clicked) {
                    done = 1;
                }
                break;

            case GS_PLAYING:
                //updates
                entity_manager_think_all();
                entity_manager_update_all();

                //draw
                level_draw(level);
                entity_manager_draw_all();
                camera_center_on(gfc_vector2d(player->position.x + (player->sprite->frame_w / 2), player->position.y + (player->sprite->frame_h / 2)));

                if (((PlayerData*)player->data)->health <= 0) {
                    state = GS_DEATH;
                }

                break;

            case GS_DEATH:
                //updates
                menu_update(deathMenu);

                //draw
                menu_draw(deathMenu);

                gf2d_sprite_draw(
                    mouse,
                    gfc_vector2d(mx, my),
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    &mouseGFC_Color,
                    (int)mf);

                if (deathMenu->Menu.death.mainMenuButton.clicked) {
                    state = GS_MAINMENU;
                }
                if (deathMenu->Menu.death.exitButton.clicked) {
                    done = 1;
                }
                break;

        }

        gf2d_graphics_next_frame();// render current draw frame and skip to the next frame
        
        if (keys[SDL_SCANCODE_ESCAPE])done = 1; // exit condition
        //slog("Rendering at %f FPS",gf2d_graphics_get_frames_per_second());
    }
    menu_free(mainMenu);
    menu_free(deathMenu);
    entity_free(player);
    level_free(level);
    slog("---==== END ====---");
    return 0;
}
/*eol@eof*/
