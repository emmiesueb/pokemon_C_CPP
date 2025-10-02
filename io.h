#ifndef IO_H
# define IO_H

#define GRASS       1
#define WATER       2
#define MART        3
#define CENTER      4
#define PATHS       5
#define ROCK        6
#define HIGHLIGHT   7
#define TREE        8
#define SURPRISE    9

#define ENCOUNTER_PROB  40
#define CAPTURE_PROB    60

#include "character.h"

typedef int16_t pair_t[2];

void io_init_terminal(void);
void io_reset_terminal(void);
void io_display(void);
void io_handle_input(pair_t dest);
void io_queue_message(const char *format, ...);
void io_trainer_battle(Character *aggressor, Character *defender);

#endif
