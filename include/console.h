#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include <SDL.h>

/*
* @brief initializes the console
*/
void console_init();

/*
* @brief passes an event to the console
* @param event the event to pass
*/
void console_pass_event(SDL_Event* event);

/*
* @brief gets if the console is open or not
* @returns 1 if the console is open, 0 otherwise
*/
int get_console_open();

/*
* @brief draws the console to the screen
*/
void console_draw();

/*
* @brief executes a console command
* @param buffer the buffer containing the command
*/
void console_execute(const char* cmd_line);

/*
* @brief frees previously initialized console
*/
void console_free();

#endif //__CONSOLE_H__