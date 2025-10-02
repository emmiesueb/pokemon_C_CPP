#ifndef CHARACTER_H
# define CHARACTER_H

# include <cstdint>

#include "pair.h"
#include "pokedex.h"

#define DIJKSTRA_PATH_MAX (INT_MAX / 2)
#define NO_NPCS 50

typedef enum __attribute__ ((__packed__)) movement_type {
  move_hiker,
  move_rival,
  move_pace,
  move_wander,
  move_sentry,
  move_explore,
  move_swim,
  move_pc,
  num_movement_types
} movement_type_t;

typedef enum __attribute__ ((__packed__)) character_type {
  char_pc,
  char_hiker,
  char_rival,
  char_swimmer,
  char_other,
  num_character_types
} character_type_t;

extern const char *char_type_name[num_character_types];

class Character {
  public:
    virtual ~Character() {}
    pair_t pos;
    char symbol;
    int next_turn;
    int seq_num;
    int num_in_party;
    int num_pkmn_defeated;
    class Party_Member *party[6]; // party pokemon
};

class Npc : public Character {
  public:
    character_type_t ctype;
    movement_type_t mtype;
    int defeated;
    pair_t dir;
    int base_money;
};

class Pc : public Character {
  public:
    int num_in_pcc;
    class Party_Member *pcc[18]; // all Pokemon the PC owns outside of their party
    class Items *bag[3];
    int wallet;
};

/* character is defined in poke327.h to allow an instance of character
 * in world without including character.h in poke327.h                 */

int32_t cmp_char_turns(const void *key, const void *with);
void delete_character(void *v);

extern void (*move_func[num_movement_types])(Character *, pair_t);

int pc_move(char);
bool is_pc(Character *);

#endif