#ifndef POKE327_H
#define POKE327_H

#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <cassert>

#include "heap.h"
#include "character.h"
#include "pair.h"

#define malloc(size) ({          \
  void *_tmp;                    \
  assert((_tmp = malloc(size))); \
  _tmp;                          \
})

/* Returns true if random float in [0,1] is less than *
 * numerator/denominator.  Uses only integer math.    */
# define rand_under(numerator, denominator) \
  (rand() < ((RAND_MAX / denominator) * numerator))

/* Returns random integer in [min, max]. */
# define rand_range(min, max) ((rand() % (((max) + 1) - (min))) + (min))

# define UNUSED(f) ((void) f)

#define MAP_X              80
#define MAP_Y              21
#define MIN_TREES          10
#define MIN_BOULDERS       10
#define TREE_PROB          95
#define BOULDER_PROB       95
#define WORLD_SIZE         401

#define MIN_TRAINERS     7
#define ADD_TRAINER_PROB 60
#define MAX_PARTY        6 /* Max number of party Pokemon */

#define MOUNTAIN_SYMBOL       '%'
#define BOULDER_SYMBOL        '0'
#define TREE_SYMBOL           '4'
#define FOREST_SYMBOL         '^'
#define GATE_SYMBOL           '#'
#define BAILEY_SYMBOL         '#'
#define PATH_SYMBOL           '#'
#define POKEMART_SYMBOL       'M'
#define POKEMON_CENTER_SYMBOL 'C'
#define TALL_GRASS_SYMBOL     ':'
#define SHORT_GRASS_SYMBOL    '.'
#define WATER_SYMBOL          '~'
#define ERROR_SYMBOL          '&'

#define PC_SYMBOL       '@'
#define HIKER_SYMBOL    'h'
#define RIVAL_SYMBOL    'r'
#define EXPLORER_SYMBOL 'e'
#define SENTRY_SYMBOL   's'
#define PACER_SYMBOL    'p'
#define SWIMMER_SYMBOL  'm'
#define WANDERER_SYMBOL 'w'

#define mappair(pair) (m->map[pair[dim_y]][pair[dim_x]])
#define mapxy(x, y) (m->map[y][x])
#define heightpair(pair) (m->height[pair[dim_y]][pair[dim_x]])
#define heightxy(x, y) (m->height[y][x])

/* Min and Max level macros: */
#define MINLEVEL      1
#define MAXLEVEL      100

typedef enum __attribute__ ((__packed__)) terrain_type {
  ter_boulder,
  ter_tree,
  ter_path,
  ter_mart,
  ter_center,
  ter_grass,
  ter_clearing,
  ter_mountain,
  ter_forest,
  ter_water,
  ter_gate,
  ter_bailey,
  num_terrain_types,
  ter_debug
} terrain_type_t;

extern int32_t move_cost[num_character_types][num_terrain_types];

class Map {
  public:
    terrain_type_t map[MAP_Y][MAP_X];
    uint8_t height[MAP_Y][MAP_X];
    Character *cmap[MAP_Y][MAP_X];
    heap_t turn;
    int32_t num_trainers;
    int8_t n, s, e, w;
};

class worldData {
public:
    Map *world[WORLD_SIZE][WORLD_SIZE];
    pair_t cur_idx;
    Map *cur_map;
    /* Please distance maps in world, not map, since *
    * we only need one pair at any given time.      */
    int hiker_dist[MAP_Y][MAP_X];
    int rival_dist[MAP_Y][MAP_X];
    class Pc *pc;
    int quit;
    int add_trainer_prob;
    int char_seq_num;
};

/* Even unallocated, a WORLD_SIZE x WORLD_SIZE array of pointers is a very *
 * large thing to put on the stack.  To avoid that, world is a global.     */
extern class worldData world;
extern int min_lvl;
extern int max_lvl;

extern pair_t all_dirs[8];

#define rand_dir(dir) {     \
  int _i = rand() & 0x7;    \
  dir[0] = all_dirs[_i][0]; \
  dir[1] = all_dirs[_i][1]; \
}

typedef struct path {
  heap_node_t *hn;
  uint8_t pos[2];
  uint8_t from[2];
  int32_t cost;
} path_t;

int new_map(int teleport);
void pathfind(Map *);

#endif