#include <SDL.h>
#include "simple_logger.h"

#include "gf2d_graphics.h"
#include "gf2d_sprite.h"
#include "entity.h"
#include "player.h"
#include "world.h"
#include "camera.h"

int main(int argc, char * argv[])
{
    /*variable declarations*/
    int done = 0;
    const Uint8 * keys;
    //Sprite *sprite;
    
    Level* level;

    int mx,my;
    float mf = 0;
    Sprite *mouse;
    //Level* level;
    Entity* player;
    GFC_Color mouseGFC_Color = gfc_color8(0,100,255,200);
    
    /*program initializtion*/
    init_logger("gf2d.log",0);
    slog("---==== BEGIN ====---");
    gf2d_graphics_initialize(
        "gf2d",
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
    //level = level_test_new();
    mouse = gf2d_sprite_load_all("images/pointer.png",32,32,16,0);
    //sprite = gf2d_sprite_load_image("images/backgrounds/bg_flat.png");
    player = player_new();

    level = level_test_new();
    //level_create("images/backgrounds/bg_flat.png","images/placeholder/basictileset.png",32,32,16,16,1);

    slog("press [escape] to quit");
    /*main game loop*/
    while(!done)
    {
        SDL_PumpEvents();   // update SDL's internal event structures
        keys = SDL_GetKeyboardState(NULL); // get the keyboard state for this frame
        /*update things here*/
        SDL_GetMouseState(&mx,&my);
        mf+=0.1;
        if (mf >= 16.0)mf = 0;

        entity_manager_think_all();
        entity_manager_update_all();

        gf2d_graphics_clear_screen();// clears drawing buffers
        // all drawing should happen betweem clear_screen and next_frame
            //backgrounds drawn first
           // gf2d_sprite_draw_image(sprite,gfc_vector2d(0,0));
           level_draw(level);
           level_add_border(level, 1);
            
            entity_manager_draw_all();

            //UI elements last
            gf2d_sprite_draw(
                mouse,
                gfc_vector2d(mx,my),
                NULL,
                NULL,
                NULL,
                NULL,
                &mouseGFC_Color,
                (int)mf);

        gf2d_graphics_next_frame();// render current draw frame and skip to the next frame
        
        if (keys[SDL_SCANCODE_ESCAPE])done = 1; // exit condition
        //slog("Rendering at %f FPS",gf2d_graphics_get_frames_per_second());
    }
    entity_free(player);
    level_free(level);
    slog("---==== END ====---");
    return 0;
}
/*eol@eof*/
