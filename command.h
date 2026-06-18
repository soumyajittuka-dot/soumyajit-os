#ifndef COMMAND_H
#define COMMAND_H

void command_loop();
void process_keyboard_input(char c);
void execute_command(char* cmd);

#endif