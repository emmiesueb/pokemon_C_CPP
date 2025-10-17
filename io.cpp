#include <iostream>
#include <unistd.h>
#include <ncurses/ncurses.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include <cmath>

#include "io.h"
#include "character.h"
#include "poke327.h"
#include "pokedex.h"
#include "battle.h"

#define DEF_POTIONS 5
#define DEF_REVIVES 5
#define DEF_PKBALL  10

using namespace std;

int costs[3] = {
  300, 1500, 300,
};

/**
 * Type matchup 2D matrix, represents move
 * effectiveness during battle for moves that
 * do damage.
*/
float type_matchups[18][18] = {
    {1.0, 1.0, 1.0, 1.0, 1.0, 0.5, 1.0, 0.0, 0.5, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, // 1 Normal   
    {2.0, 1.0, 0.5, 0.5, 1.0, 2.0, 0.5, 0.0, 2.0, 1.0, 1.0, 1.0, 1.0, 0.5, 2.0, 1.0, 2.0, 0.5}, // 2 Fighting   
    {1.0, 2.0, 1.0, 1.0, 1.0, 0.5, 2.0, 1.0, 0.5, 1.0, 1.0, 2.0, 0.5, 1.0, 1.0, 1.0, 1.0, 1.0}, // 3 Flying   
    {1.0, 1.0, 1.0, 0.5, 0.5, 0.5, 1.0, 0.5, 0.0, 1.0, 1.0, 2.0, 1.0, 1.0, 1.0, 1.0, 1.0, 2.0}, // 4 Poison
    {1.0, 1.0, 0.0, 2.0, 1.0, 2.0, 0.5, 1.0, 2.0, 2.0, 1.0, 0.5, 2.0, 1.0, 1.0, 1.0, 1.0, 1.0}, // 5 Ground
    {1.0, 0.5, 2.0, 1.0, 0.5, 1.0, 2.0, 1.0, 0.5, 2.0, 1.0, 1.0, 1.0, 1.0, 2.0, 1.0, 1.0, 1.0}, // 6 Rock
    {1.0, 0.5, 0.5, 0.5, 1.0, 1.0, 1.0, 0.5, 0.5, 0.5, 1.0, 2.0, 1.0, 2.0, 1.0, 1.0, 2.0, 0.5}, // 7 Bug 
    {0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 2.0, 1.0, 1.0, 1.0, 1.0, 1.0, 2.0, 1.0, 1.0, 0.5, 1.0}, // 8 Ghost
    {1.0, 0.5, 1.0, 1.0, 1.0, 2.0, 1.0, 1.0, 0.5, 0.5, 0.5, 1.0, 0.5, 1.0, 2.0, 1.0, 1.0, 2.0}, // 9 Steel 
    {1.0, 1.0, 1.0, 1.0, 0.5, 0.5, 2.0, 1.0, 2.0, 0.5, 0.5, 2.0, 1.0, 1.0, 2.0, 0.5, 1.0, 1.0}, // 10 Fire
    {1.0, 1.0, 1.0, 1.0, 2.0, 2.0, 1.0, 1.0, 1.0, 2.0, 0.5, 0.5, 1.0, 1.0, 1.0, 0.5, 1.0, 1.0}, // 11 Water
    {1.0, 1.0, 0.5, 0.5, 2.0, 2.0, 0.5, 1.0, 0.5, 0.5, 2.0, 0.5, 1.0, 1.0, 1.0, 0.5, 1.0, 1.0}, // 12 Grass
    {1.0, 1.0, 2.0, 1.0, 0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 2.0, 0.5, 0.5, 1.0, 1.0, 0.5, 1.0, 1.0}, // 13 Electric
    {1.0, 2.0, 1.0, 2.0, 1.0, 1.0, 1.0, 1.0, 0.5, 1.0, 1.0, 1.0, 1.0, 0.5, 1.0, 1.0, 0.0, 1.0}, // 14 Psychic
    {1.0, 1.0, 2.0, 1.0, 2.0, 1.0, 1.0, 1.0, 0.5, 0.5, 0.5, 0.5, 1.0, 1.0, 0.5, 2.0, 1.0, 1.0}, // 15 Ice
    {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.5, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 2.0, 1.0, 0.0}, // 16 Dragon
    {1.0, 0.5, 1.0, 1.0, 1.0, 1.0, 1.0, 2.0, 1.0, 1.0, 1.0, 1.0, 1.0, 2.0, 1.0, 1.0, 0.5, 0.5}, // 17 Dark
    {1.0, 2.0, 1.0, 0.5, 1.0, 1.0, 1.0, 1.0, 0.5, 0.5, 1.0, 1.0, 1.0, 1.0, 1.0, 2.0, 2.0, 1.0}, // 18 Fairy
};

typedef struct io_message {
  /* Will print " --more-- " at end of line when another message follows. *
   * Leave 10 extra spaces for that.                                      */
  char msg[71];
  struct io_message *next;
} io_message_t;

const char *btns[4] = {
  "ITEMS     ",
  "FIGHT     ",
  "RUN       ",
  "PARTY     "
};

static io_message_t *io_head, *io_tail;
WINDOW *menu = NULL;  // pop-up menu terminal
WINDOW *battle = NULL; // battle window
int move_used = -1; // move to use in battle
int attempts; // attempts to flee wild combat
int item_used = -1;
int cur_pkmn = 0;

void io_init_terminal(void)
{
  initscr();
  raw();
  noecho();
  curs_set(0);
  keypad(stdscr, TRUE);
  start_color();
  init_pair(GRASS, COLOR_GREEN, COLOR_BLACK);
  init_pair(PATHS, COLOR_YELLOW, COLOR_BLACK);
  init_pair(WATER, COLOR_BLUE, COLOR_BLACK);
  init_pair(CENTER, COLOR_WHITE, COLOR_RED);
  init_pair(MART, COLOR_WHITE, COLOR_BLUE);
  init_pair(ROCK, COLOR_RED, COLOR_BLACK);
  init_pair(TREE, COLOR_CYAN, COLOR_BLACK);
  init_pair(HIGHLIGHT, COLOR_BLACK, COLOR_WHITE);
  init_pair(SURPRISE, COLOR_RED, COLOR_WHITE);
}

void io_reset_terminal(void)
{
  endwin();

  while (io_head) {
    io_tail = io_head;
    io_head = io_head->next;
    free(io_tail);
  }
  io_tail = NULL;
}

void io_queue_message(const char *format, ...)
{
  io_message_t *tmp;
  va_list ap;

  if (!(tmp = (io_message_t *) malloc(sizeof (*tmp)))) {
    cerr << "malloc" << endl;
    exit(1);
  }

  tmp->next = NULL;

  va_start(ap, format);

  vsnprintf(tmp->msg, sizeof (tmp->msg), format, ap);

  va_end(ap);

  if (!io_head) {
    io_head = io_tail = tmp;
  } else {
    io_tail->next = tmp;
    io_tail = tmp;
  }
}

static void io_print_message_queue(uint32_t y, uint32_t x)
{
  while (io_head) {
    io_tail = io_head;
    attron(COLOR_PAIR(TREE));
    mvprintw(y, x, "%-80s", io_head->msg);
    attroff(COLOR_PAIR(TREE));
    io_head = io_head->next;
    if (io_head) {
      attron(COLOR_PAIR(TREE));
      mvprintw(y, x + 70, "%10s", " --more-- ");
      attroff(COLOR_PAIR(TREE));
      refresh();
      getch();
    }
    free(io_tail);
  }
  io_tail = NULL;
}

/**************************************************************************
 * Compares trainer distances from the PC according to the rival distance *
 * map.  This gives the approximate distance that the PC must travel to   *
 * get to the trainer (doesn't account for crossing buildings).  This is  *
 * not the distance from the NPC to the PC unless the NPC is a rival.     *
 *                                                                        *
 * Not a bug.                                                             *
 **************************************************************************/
static int compare_trainer_distance(const void *v1, const void *v2)
{
  const Character *const *c1 = (const Character *const *) v1;
  const Character *const *c2 = (const Character *const *) v2;

  return (world.rival_dist[(*c1)->pos[dim_y]][(*c1)->pos[dim_x]] -
          world.rival_dist[(*c2)->pos[dim_y]][(*c2)->pos[dim_x]]);
}

static Character *io_nearest_visible_trainer()
{
  Character **c, *n;
  uint32_t x, y, count;

  c = (Character **) malloc(world.cur_map->num_trainers * sizeof(*c));

  /* Get a linear list of trainers */
  for (count = 0, y = 1; y < MAP_Y - 1; y++) {
    for (x = 1; x < MAP_X - 1; x++) {
      if (world.cur_map->cmap[y][x] && world.cur_map->cmap[y][x] !=
          world.pc) {
        c[count++] = world.cur_map->cmap[y][x];
      }
    }
  }

  /* Sort it by distance from PC */
  qsort(c, count, sizeof(*c), compare_trainer_distance);

  n = c[0];

  free(c);

  return n;
}

void io_display()
{
  uint32_t y, x;
  Character *c;

  for (y = 0; y < MAP_Y; y++) {
    for (x = 0; x < MAP_X; x++) {
      if (world.cur_map->cmap[y][x]) {
        mvaddch(y + 1, x, world.cur_map->cmap[y][x]->symbol);
      } else {
        switch (world.cur_map->map[y][x]) {
        case ter_boulder:
          attron(COLOR_PAIR(ROCK));
          mvaddch(y + 1, x, BOULDER_SYMBOL);
          attroff(COLOR_PAIR(ROCK));
          break;
        case ter_mountain:
          attron(COLOR_PAIR(ROCK));
          mvaddch(y + 1, x, MOUNTAIN_SYMBOL);
          attroff(COLOR_PAIR(ROCK));
          break;
        case ter_tree:
          attron(COLOR_PAIR(TREE));
          mvaddch(y + 1, x, TREE_SYMBOL);
          attroff(COLOR_PAIR(TREE));
          break;
        case ter_forest:
          attron(COLOR_PAIR(TREE));
          mvaddch(y + 1, x, FOREST_SYMBOL);
          attroff(COLOR_PAIR(TREE));
          break;
        case ter_path:
        case ter_bailey:
          attron(COLOR_PAIR(PATHS));
          mvaddch(y + 1, x, PATH_SYMBOL);
          attroff(COLOR_PAIR(PATHS));
          break;
        case ter_gate:
          attron(COLOR_PAIR(PATHS));
          mvaddch(y + 1, x, GATE_SYMBOL);
          attroff(COLOR_PAIR(PATHS));
          break;
        case ter_mart:
          attron(COLOR_PAIR(MART));
          mvaddch(y + 1, x, POKEMART_SYMBOL);
          attroff(COLOR_PAIR(MART));
          break;
        case ter_center:
          attron(COLOR_PAIR(CENTER));
          mvaddch(y + 1, x, POKEMON_CENTER_SYMBOL);
          attroff(COLOR_PAIR(CENTER));
          break;
        case ter_grass:
          attron(COLOR_PAIR(GRASS));
          mvaddch(y + 1, x, TALL_GRASS_SYMBOL);
          attroff(COLOR_PAIR(GRASS));
          break;
        case ter_clearing:
          attron(COLOR_PAIR(GRASS));
          mvaddch(y + 1, x, SHORT_GRASS_SYMBOL);
          attroff(COLOR_PAIR(GRASS));
          break;
        case ter_water:
          attron(COLOR_PAIR(WATER));
          mvaddch(y + 1, x, WATER_SYMBOL);
          attroff(COLOR_PAIR(WATER));
          break;
        default:
 /* Use zero as an error symbol, since it stands out somewhat, and it's *
  * not otherwise used.                                                 */
          attron(COLOR_PAIR(TREE));
          mvaddch(y + 1, x, ERROR_SYMBOL);
          attroff(COLOR_PAIR(TREE)); 
       }
      }
    }
  }

  mvprintw(23, 1, "PC position is (%2d,%2d) on map %d%cx%d%c.",
           world.pc->pos[dim_x],
           world.pc->pos[dim_y],
           abs(world.cur_idx[dim_x] - (WORLD_SIZE / 2)),
           world.cur_idx[dim_x] - (WORLD_SIZE / 2) >= 0 ? 'E' : 'W',
           abs(world.cur_idx[dim_y] - (WORLD_SIZE / 2)),
           world.cur_idx[dim_y] - (WORLD_SIZE / 2) <= 0 ? 'N' : 'S');
  mvprintw(22, 1, "%d known %s.", world.cur_map->num_trainers,
           world.cur_map->num_trainers > 1 ? "trainers" : "trainer");
  mvprintw(22, 30, "Nearest visible trainer: ");
  if ((c = io_nearest_visible_trainer())) {
    attron(COLOR_PAIR(ROCK));
    mvprintw(22, 55, "%c at vector %d%cx%d%c.",
             c->symbol,
             abs(c->pos[dim_y] - world.pc->pos[dim_y]),
             ((c->pos[dim_y] - world.pc->pos[dim_y]) <= 0 ?
              'N' : 'S'),
             abs(c->pos[dim_x] - world.pc->pos[dim_x]),
             ((c->pos[dim_x] - world.pc->pos[dim_x]) <= 0 ?
              'W' : 'E'));
    attroff(COLOR_PAIR(ROCK));
  } else {
    attron(COLOR_PAIR(WATER));
    mvprintw(22, 55, "NONE.");
    attroff(COLOR_PAIR(WATER));
  }

  io_print_message_queue(0, 0);

  refresh();
}

uint32_t io_teleport_pc(pair_t dest)
{
  do {
    dest[dim_x] = rand_range(1, MAP_X - 2);
    dest[dim_y] = rand_range(1, MAP_Y - 2);
  } while (world.cur_map->cmap[dest[dim_y]][dest[dim_x]]                  ||
           move_cost[char_pc][world.cur_map->map[dest[dim_y]]
                                                [dest[dim_x]]] ==
             DIJKSTRA_PATH_MAX                                            ||
           world.rival_dist[dest[dim_y]][dest[dim_x]] < 0);

  return 0;
}

// build basic list window
static void io_menu_display(WINDOW *win, int type, char **list, int cursor, int startidx) {
  box(win, ACS_VLINE, ACS_HLINE);
  mvwdelch(win, 2, 0);
  mvwaddch(win, 2, 0, ACS_LTEE);
  mvwhline(win, 2, 1, ACS_HLINE, 34);
  mvwdelch(win, 2, 35);
  mvwaddch(win, 2, 35, ACS_RTEE);

  // print which kind of menu:
  //TODO: fix this because i expanded the length of the window
  int i = type == MART ? (mvwprintw(win, 1, 14, "Pokemart")) : ( type == CENTER ? mvwprintw(win, 1, 11, "Pokemon Center") : mvwprintw(win, 1, 12, "Trainer List"));
  
  // int j = 0;
  // print out the list:
  for (i = startidx; list[i] && i <= startidx + 5; ++i) {
    // pokemon center doesn't have more than 2 lines, this accounts for that:
    if ((type == CENTER && i == 2)
      || (type == MART && i == 3))
      break;
    if (cursor >= 0 && cursor == i)
      wattron(win, COLOR_PAIR(HIGHLIGHT));

    // clear out the line:
    mvwaddstr(win, (3 + (i - startidx)), 1, "                                 ");
    mvwprintw(win, (3 + (i - startidx)), 1, list[i]);

    if (cursor >= 0 && cursor == i)
      wattroff(win, COLOR_PAIR(HIGHLIGHT));
  }
  wrefresh(win);
}

static void io_list_trainers()
{
  menu = newwin(10, 36, 6, 22); // create new list winodw
  keypad(stdscr, TRUE);
  keypad(menu, TRUE);
  
  int n = world.cur_map->num_trainers;
  Character *characters[n];
  int i;

  for (i = 0; i < n; ++i)
    characters[i] = NULL;

  i = 0;
  // get all characters in the map:
  for (int y = 0; y < MAP_Y; ++y) {
    for (int x = 0; x < MAP_X; ++x) {
      if (world.cur_map->cmap[y][x] && !(world.pc->pos[dim_x] == x && world.pc->pos[dim_y] == y))
        characters[i++] = world.cur_map->cmap[y][x];
    }
  }
  char **list = (char **) calloc(n, sizeof(char *));

  // determine the string to print:
  for (i = 0; i < n; ++i) {
    *(list + i) = (char *) malloc(sizeof(char) * 33);
    if (*(list + i) == NULL) {
      for (int j = 0; j < i; ++j)
        free(*(list + j));
      free(list);
      delwin(menu);
      menu = NULL;
      return;
    }
    snprintf(*(list + i), 33, "%c, %d %s and %d %s", characters[i]->symbol, 
        abs(world.pc->pos[dim_y] - characters[i]->pos[dim_y]), 
        world.pc->pos[dim_y] > characters[i]->pos[dim_y] ? "South" : "North" ,
        abs(world.pc->pos[dim_x] - characters[i]->pos[dim_x]), 
        world.pc->pos[dim_x] > characters[i]->pos[dim_x] ? "East" : "West");
  }

  i = 0;

  // now take inputs:
  do {
    io_menu_display(menu, 0, list, -1, i);
    wrefresh(menu);
    int c = wgetch(menu);
    switch (c) {
      case KEY_UP:
        if (n > 6 && i > 0)
          i--;
        break;
      case KEY_DOWN:
        if (n > 6 && i < (n - 6))
          i++;
        break;
      case 27:
        keypad(menu, FALSE);
        delwin(menu);
        menu = NULL;
        break;
    }
  } while (menu);
  for (i = 0; i < n; ++i)
    free(*(list + i));
  free(list);

  // redraw map:
  io_display();
}


/**
 * Allows the PC to check their party and scroll.
 * If type == 0, it's to swap out a party pokemon.
 * If type == 1, it's to use a potion on a pokemon.
 * If type == 2, it's to swap out pokemon since a previous one fainted.
 * If type == 3, it's to swap out pokemon with a PCC pokemon
 * from the PCC to put it in the party.
 * - return cursor value for which pokemon to switch with or
 * - return if the item was used.
*/
int io_party_select(int type) {
  WINDOW *win = newwin(8, 12, 7, 67);
  box(win, ACS_VLINE, ACS_HLINE);
  int i = 0;
  int32_t c;
  int enter = 0;
  int cursor = 0;
  bool break_loop = 0;
  keypad(stdscr, TRUE);
  keypad(battle, TRUE);
  keypad(win, TRUE);
  do {
    wrefresh(win);
    enter = 0;
    for (i = 0; i < 6; i++) {
      if (cursor == i)
        wattron(win, COLOR_PAIR(HIGHLIGHT));
      
      if (i >= world.pc->num_in_party)
        mvwprintw(win, i + 1, 1, "----------");
      else 
        mvwprintw(win, i + 1, 1, "%s", world.pc->party[i]->species->identifier);

      if (cursor == i)
        wattroff(win, COLOR_PAIR(HIGHLIGHT));
    }

    switch(c = wgetch(win)) {
      case KEY_DOWN:
        cursor = min(cursor + 1, 6);
        break;
      case KEY_UP:
        cursor = max(cursor - 1, 0);
        break;
      case 10:
        enter = 1;
        break;
      case 27:
        break_loop = 1;
        break;
    }

    if ((type == 0 || type == 2 || type == 3) && enter) {
      mvwhline(battle, 16, 1, ' ', 78);
      if ((world.pc->party[cursor] != NULL && world.pc->party[cursor]->full_stats[0] > 0)
        || (type == 3 && world.pc->party[cursor] != NULL)) {
        keypad(win, FALSE);
        wclear(win);
        wrefresh(win);
        delwin(win);
        win = NULL;
        return cursor;
      }
      else if (world.pc->party[cursor] != NULL && world.pc->party[cursor]->defeated == 1)
        mvwprintw(battle, 16, 2, "That pokemon is fainted.");
      else
        return -1;
      wrefresh(battle);
      wrefresh(win);
      sleep(1);
    }
    else if (type == 1 && enter) {
      if (world.pc->num_in_party > cursor) {
        mvwhline(battle, 16, 1, ' ', 78);
        /* Heal for at most 20 HP: */
        if (world.pc->party[cursor]->full_stats[0] > 0 
          && item_used == 0) {
          int healed = min(world.pc->party[cursor]->maxHP - world.pc->party[cursor]->full_stats[0], 20);
          world.pc->party[cursor]->full_stats[0] += healed;
          mvwprintw(battle, 16, 2, "Pokemon was restored for %d HP.", healed);
          keypad(win, FALSE);
          wclear(win);
          wrefresh(win);
          delwin(win);
          win = NULL;
          return 1;
        }
        /* Revive to half health:*/
        else if (world.pc->party[cursor]->full_stats[0] == 0 && item_used == 1) {
          world.pc->party[cursor]->full_stats[0] = floor(world.pc->party[cursor]->maxHP / 2);
          world.pc->party[cursor]->defeated = 0;
          mvwprintw(battle, 16, 2, "Pokemon was revived to half health.");
          keypad(win, FALSE);
          wclear(win);
          wrefresh(win);
          delwin(win);
          win = NULL;
          return 1;
        }
        else {
          mvwprintw(battle, 16, 2, "It won't have any effect.");
          wrefresh(battle);
          wrefresh(win);
          sleep(2);
          keypad(win, FALSE);
          wclear(win);
          wrefresh(win);
          delwin(win);
          win = NULL;
          return 0;
        }
      }
    }
  } while (!break_loop);
  keypad(win, FALSE);
  wclear(win);
  wrefresh(win);
  delwin(win);
  win = NULL;
  return -1;
}


/**
 * PCC display generation
*/
void io_display_pcc(WINDOW *pcc, int box, int cursor) {
  
  for (int i = 0; i < 10; ++i) {
    if (i == 0 || i == 9)
      mvwhline(pcc, i, 1, ACS_HLINE, 34);

    if (i == 0) {
      mvwaddch(pcc, i, 0, ACS_ULCORNER);
      mvwaddch(pcc, i, 35, ACS_URCORNER);
    }
    if (i == 9) {
      mvwaddch(pcc, i, 0, ACS_LLCORNER);
      mvwaddch(pcc, i, 35, ACS_LRCORNER);
    }

    if (i > 0 && i < 9) {
      mvwaddch(pcc, i, 0, ACS_VLINE);
      mvwaddch(pcc, i, 35, ACS_VLINE);
    }
  }

  mvwdelch(pcc, 2, 0);
  mvwaddch(pcc, 2, 0, ACS_LTEE);
  mvwhline(pcc, 2, 1, ACS_HLINE, 34);
  mvwdelch(pcc, 2, 35);
  mvwaddch(pcc, 2, 35, ACS_RTEE);
  mvwprintw(pcc, 1, 17, "PCC");
  mvwprintw(pcc, 2, 16, "Box %d", (box + 1));
  
  for (int i = 0; i < 6; i++) {
    if (i == cursor)
      wattron(pcc, COLOR_PAIR(HIGHLIGHT));
    if (world.pc->pcc[(box * 6) + i] != NULL)
      mvwprintw(pcc, 3 + i, 1, world.pc->pcc[(box * 6) + i]->species->identifier);
    else
      mvwhline(pcc, 3 + i, 1, ' ', 34);
    if (i == cursor)
      wattroff(pcc, COLOR_PAIR(HIGHLIGHT));
  }
}


/**
 * This will create the PCC window, which will
 * open a new window to display all of them. It
 * will display empty boxes if the player has not
 * filled those boxes with more pokemon. If the
 * player wishes to swap a pokemon in their PCC,
 * this will also handle that interaction.
*/
void io_pcc() {
  WINDOW *pcc = newwin(10, 36, 6, 22);
  keypad(stdscr, TRUE);
  keypad(pcc, TRUE);

  bool exiting = 0;
  int c;
  int cursor = 0;
  int box = 0;

  do {
    io_display_pcc(pcc, box, cursor);
    wrefresh(pcc);
    switch (c = getch()) {
      case KEY_DOWN:
        cursor = min(5, cursor + 1);
        break;
      case KEY_UP:
        cursor = max(0, cursor - 1);
        break;
      case KEY_LEFT:
        box = max(0, box - 1);
        break;
      case KEY_RIGHT:
        box = min(2, box + 1);
        break;
      // player is switching out pokemon:
      case 10:
        int i;
        if ((i = io_party_select(3)) > -1) {
          world.pc->party[i]->full_stats[0] = world.pc->party[i]->maxHP;
          Party_Member *temp = world.pc->party[i];
          world.pc->party[i] = world.pc->pcc[(box * 6) + cursor];
          world.pc->pcc[(box * 6) + cursor] = temp;
          io_display();
        }
        break;
      case 27:
        exiting = 1;
        break;
    }
  } while (!exiting);
  keypad(pcc, FALSE);
  wclear(pcc);
  wrefresh(pcc);
  delwin(pcc);
  pcc = NULL;
}


/**
 * This function will handle input interactions
 * for both the Pokemart and Pokemon Center menus,
 * which it will allow for different occurrences
 * based on the item selected by the user.
*/
void handle_menu_input(int type, int cursor) {
  // PC is making a purchase:
  if (type == MART) {
    // handle which item:
    WINDOW *win = newwin(4, 36, 15, 22);
    int amt = 0;
    int cost = 0;
    mvwaddch(win, 0, 0, ACS_LTEE);
    mvwhline(win, 0, 1, ACS_HLINE, 34);
    mvwaddch(win, 0, 35, ACS_RTEE);
    mvwaddch(win, 1, 0, ACS_VLINE);
    mvwaddch(win, 1, 35, ACS_VLINE);
    mvwaddch(win, 2, 0, ACS_VLINE);
    mvwaddch(win, 2, 35, ACS_VLINE);
    mvwhline(win, 3, 1, ACS_HLINE, 34);
    mvwaddch(win, 3, 0, ACS_LLCORNER);
    mvwaddch(win, 3, 35, ACS_LRCORNER);
    keypad(stdscr, TRUE);
    keypad(win, TRUE);
    int c;
    bool exiting = 0;
    do {
      mvwhline(win, 1, 1, ' ', 34);
      mvwprintw(win, 1, 1, "How many?");
      mvwprintw(win, 1, 30, "%d", amt);
      mvwhline(win, 2, 1, ' ', 34);
      mvwprintw(win, 2, 1, "P%d", world.pc->wallet);
      mvwprintw(win, 2, 29, "P%d", cost);
      wrefresh(win);
      switch (c = getch()) {
        case KEY_UP:
          amt = min(amt + 1, 10);
          cost = amt * costs[cursor];
          break;
        case KEY_DOWN:
          amt = max(0, amt - 1);
          cost = max(0, cost - costs[cursor]);
          break;
        case 10:
          world.pc->wallet -= cost;
          world.pc->bag[cursor]->num += amt;
          exiting = 1;
          break;
        case 27:
        case '>':
          exiting = 1;
          break;
      }
    } while (!exiting);
    keypad(win, FALSE);
    wclear(win);
    wrefresh(win);
    delwin(win);
    win = NULL;
  }
  else if (type == CENTER)  {
    switch (cursor) {
      // heal all the PC's pokemon:
      case 0:
        for (int i = 0; i < world.pc->num_in_party; ++i) {
          world.pc->party[i]->defeated = 0;
          world.pc->party[i]->full_stats[0] = world.pc->party[i]->maxHP;
          world.pc->party[i]->status = 0;
        }
        world.pc->num_pkmn_defeated = 0;
        break;
      // check the PC's PCC, where all their other pokemon are:
      case 1:
        io_pcc();
        break;
    }
  }
}


void io_building_menu(int type)
{
  mvprintw(0, 0, ((type == MART) ? "Welcome to the Pokemart! Could I interest you in some Pokeballs?" : "Welcome to the Pokemon Center! How can Nurse Joy assist you?"));
  refresh();

  menu = newwin(10, 36, 6, 22); // create new list window
  keypad(stdscr, TRUE);
  keypad(menu, TRUE);
  // read file for respective building type:
  FILE *f = type == MART ? fopen("./pokemart.txt", "rb") : fopen("./pokemon_center.txt", "rb") ;
  // file didn't open:
  if (!f) {
    delwin(menu);
    menu = NULL;
    return;
  }
  // file opened, read from file:
  int i, j;
  i = type == MART ? 30 : 2; // only 2 options for the pokemon center
  char **line = (char **) calloc(i, sizeof(char *));
  for (j = 0; j < i; ++j) {
    *(line + j) = (char *) malloc(sizeof(char) * 33);
    if (*(line + j) == NULL) {
      //malloc error
      for (int k = 0; k < j; ++k)
        free(*(line + k));
      free(line);
      keypad(menu, FALSE);
      delwin(menu);
      menu = NULL;
      return;
    }
  }
  char c[1];
  i = j = 0;
  while (!feof(f)) {
    fread(c, sizeof(char), 1, f);
    if (c[0] != '\n' && j < 27) {
      line[i][j++] = c[0];
    }
    else {
      line[i++][j] = '\0';
      j = 0;
    }
  }
  // close file:
  fclose(f);
  line[i][j] = '\0'; // needs to terminate string at the end

  int cursor = 0; // this will highlight text at this specific idx of the list display
  j = 0;  // this will be the idx of the list that is at the top in the display
  
  // now take inputs:
  do {
    io_menu_display(menu, type, line, cursor, j);
    wrefresh(menu);
    int ch = wgetch(menu);
    switch (ch) {
      // scroll up:
      case KEY_UP:
        if (cursor > 0)
          cursor--;
        if (i > 6 && j > cursor)
          j--;
        break;
      // scroll down:
      case KEY_DOWN:
        if (cursor < i)
          cursor++;
        if (i > 6 && cursor > (j + 5))
          j++;
        break;
      // exit building:
      case '<':
      case 27:
        keypad(menu, FALSE);
        delwin(menu);
        menu = NULL;
        break;
      // hit enter, do what this allows:
      case 10:
        handle_menu_input(type, cursor);
        io_display();
        wrefresh(menu);
        break;
    }
  } while (menu);
  for (j = 0; j < i; ++j)
    free(*(line + j));
  free(line);
}


/**
 * This displays the health, level, and name of both
 * the opposing pokemon and the PC's current pokemon.
*/
void io_display_battle(Party_Member *foe, Party_Member *self) {
  /* Opposing pokemon: */
  mvwaddch(battle, 4, 2, ACS_ULCORNER);
  mvwhline(battle, 4, 3, ACS_HLINE, 31);
  mvwaddch(battle, 4, 34, ACS_URCORNER);
  mvwaddch(battle, 5, 2, ACS_VLINE);
  mvwhline(battle, 5, 3, (int) ' ', 31);
  if (foe->is_shiny)
    wattron(battle, COLOR_PAIR(TREE));
  mvwprintw(battle, 5, 4, "%s", foe->species->identifier);
  mvwaddch(battle, 5, 59, foe->species->identifier[0]);
  if (foe->is_shiny)
    wattroff(battle, COLOR_PAIR(TREE));
  mvwprintw(battle, 5, 16, "lv.%d", foe->level);
  float pcnt_hp = (100.0 * foe->full_stats[0]) / foe->maxHP;
  wattron(battle, COLOR_PAIR((pcnt_hp > 35 ? GRASS : (pcnt_hp < 10 ? ROCK : PATHS))));
  mvwprintw(battle, 5, 23, "hp: %d/%d", foe->full_stats[0], foe->maxHP);
  wattroff(battle, COLOR_PAIR((pcnt_hp > 35 ? GRASS : (pcnt_hp < 10 ? ROCK : PATHS))));
  mvwaddch(battle, 5, 34, ACS_VLINE);
  mvwaddch(battle, 6, 2, ACS_LLCORNER);
  mvwhline(battle, 6, 3, ACS_HLINE, 31);
  mvwaddch(battle, 6, 34, ACS_LRCORNER);

  /* PC's pokemon: */
  mvwaddch(battle, 10, 46, ACS_ULCORNER);
  mvwhline(battle, 10, 47, ACS_HLINE, 31);
  mvwaddch(battle, 10, 78, ACS_URCORNER);
  mvwaddch(battle, 11, 46, ACS_VLINE);
  mvwhline(battle, 11, 47, (int) ' ', 31);
  if (self->is_shiny)
    wattron(battle, COLOR_PAIR(TREE));
  mvwprintw(battle, 11, 48, "%s", self->species->identifier);
  mvwaddch(battle, 11, 20, self->species->identifier[0]);
  if (self->is_shiny)
    wattroff(battle, COLOR_PAIR(TREE));
  mvwprintw(battle, 11, 59, "lv.%d", self->level);
  pcnt_hp = (100.0 * self->full_stats[0]) / self->maxHP;
  wattron(battle, COLOR_PAIR((pcnt_hp > 35 ? GRASS : (pcnt_hp < 10 ? ROCK : PATHS))));
  mvwprintw(battle, 11, 66, "hp: %d/%d", self->full_stats[0], self->maxHP);
  wattroff(battle, COLOR_PAIR((pcnt_hp > 35 ? GRASS : (pcnt_hp < 10 ? ROCK : PATHS))));
  mvwaddch(battle, 11, 78, ACS_VLINE);
  mvwaddch(battle, 12, 46, ACS_LLCORNER);
  mvwhline(battle, 12, 47, ACS_HLINE, 31);
  mvwaddch(battle, 12, 78, ACS_LRCORNER);
}


/**
 * This allows the PC to choose what to do with the item
 * they want to use.
 * various return values are used to mean different things:
 * - 0, not used up
 * - 1, used up
 * - 2, pokemon caught
*/
int io_use_item() {
  mvwhline(battle, 16, 1, ' ', 78);
  do {
    /* Potion: */
    if (item_used == 0) {
      mvwprintw(battle, 16, 2, "You're using a potion.");
      wrefresh(battle);
      if (io_party_select(1) == 1) {
        world.pc->bag[item_used]->num--;
        return 1;
      }
      return 0;
    }
    else if (item_used == 1) {
      mvwprintw(battle, 16, 2, "You're using a revive.");
      wrefresh(battle);
      if (io_party_select(1) == 1) {
        world.pc->bag[item_used]->num--;
        world.pc->num_pkmn_defeated--;
        return 1;
      }
      return 0;
    }
    else if (item_used == 2) {
      world.pc->bag[item_used]->num--;
      mvwprintw(battle, 16, 2, "You threw a pokeball!");
      wrefresh(battle);
      sleep(2);
      mvwhline(battle, 16, 1, ' ', 78);
      if (world.pc->num_in_party < 6 && (rand() % 100) <= 30)
        return 2;
      else {
        mvwprintw(battle, 16, 2, "The wild pokemon broke out.");
        wrefresh(battle);
        sleep(2);
      }
      return 1;
    }
  } while (1);
}


/**
 * Displays the buttons at the bottom of the battle screen to either use an item,
 * fight, run, or check out party
 * screens:
 * - 0 is the main screen
 * - 1 is the item screen
 * - 2 is the moves screen
 * - 4 is the party screen
 * - 3 is run
*/
int io_battle_btns(int scrn, int cursor, bool wild, int attempts) {

  if (scrn == 3 && wild)
    return 1;

  int i = 0;

  // clear these out:
  mvwhline(battle, 19, 1, (int) ' ', 78);
  mvwhline(battle, 20, 1, (int) ' ', 78);
  mvwhline(battle, 21, 1, (int) ' ', 78);
  /* Main Screen: */
  if (scrn == 0) {
    // display main buttons:
    for (i = 0; i < 4; ++i) {
      if (cursor == i)
        wattron(battle, COLOR_PAIR(HIGHLIGHT));
      mvwprintw(battle, 20, (i * 20) + 6, btns[i]);
      if (cursor == i)
        wattroff(battle, COLOR_PAIR(HIGHLIGHT));
    }
    wrefresh(battle);
  }
  /* Items: */
  else if (scrn == 1) {
    for (i = 0; i < 3; i++) {
      if (cursor == i)
        wattron(battle, COLOR_PAIR(HIGHLIGHT));
      mvwprintw(battle, 19 + i, 4, "%d %s", world.pc->bag[i]->num, world.pc->bag[i]->identifier.c_str());
      if (cursor == i)
        wattroff(battle, COLOR_PAIR(HIGHLIGHT));
    }
    wrefresh(battle);
  }
  /* Moves: */
  else if (scrn == 2) {
    for (i = 0; i < 4; ++i) {
      if (cursor == i)
        wattron(battle, COLOR_PAIR(HIGHLIGHT));
      
      if (world.pc->party[cur_pkmn]->moves[i] == NULL)
        mvwprintw(battle, 20, (i * 20) + 6, "----------");
      else
        mvwprintw(battle, 20, (i * 20) + 6, "%s", world.pc->party[cur_pkmn]->moves[i]->identifier);
      
      if (cursor == i)
        wattroff(battle, COLOR_PAIR(HIGHLIGHT));
      
      if (world.pc->party[cur_pkmn]->moves[i] != NULL)
        mvwprintw(battle, 21, (i * 20) + 6, "%s", type_names[world.pc->party[cur_pkmn]->moves[i]->type_id - 1].get()->name);
    }
    wrefresh(battle);
  }
  /* Party: */
  else if (scrn == 4) {
    // do nothing here
  }
  /* Tried to run during a trainer battle, not allowed! */
  else {
    mvwprintw(battle, 16, 2, "You can't run away from a trainer battle!");
    wrefresh(battle);
    scrn = 0;
    cursor = 2;
    sleep(2);
  }
  return 0;
}


/**
 * This function will take input for the battle screen
 * Returns indicate specific scenarios:
 * - 0, no turn used
 * - 1, turn used
 * - 2, fled battle
 * - 3-8, pokemon to switch to
 * - 9, escape (debugging purposes)
 * - 10, caught a wild pokemon!
*/
int io_battle_input(int scrn, int cursor, bool wild, Party_Member *self, Party_Member *foe) {
  int32_t c;
  move_used = -1;
  item_used = -1;
  int i = 0;
  do {
    /* Main screen: */
    if (scrn == 0) {
      mvwhline(battle, 16, 1, ' ', 78);
      mvwprintw(battle, 16, 2, "What will you do?");
      wrefresh(battle);
      switch (c = getch()) {
        case KEY_LEFT:
          cursor = max(cursor - 1, 0);
          break;
        case KEY_RIGHT:
          cursor = min(cursor + 1, 3);
          break;
        case 10:
          scrn = cursor + 1;
          cursor = 0;
          break;
        // for debugging
        case 27:
          return 9;
        default:
          break;
      }
    }
    /* Items: */
    else if (scrn == 1) {
      mvwhline(battle, 16, 1, ' ', 78);
      mvwprintw(battle, 16, 2, "Items in your bag.");
      wrefresh(battle);
      switch(c = getch()) {
        case KEY_UP:
          cursor = max(cursor - 1, 0);
          break;
        case KEY_DOWN:
          cursor = min(cursor + 1, 2);
          break;
        case 10:
          mvwhline(battle, 16, 1, ' ', 78);
          if (cursor == 2 && wild == 0) {
            mvwprintw(battle, 16, 2, "You can't use that here!");
            io_display_battle(foe, self);
            wrefresh(battle);
            sleep(1);
            return 0;
          }
          else if (world.pc->bag[cursor]->num > 0) {
            item_used = cursor;
            i = io_use_item();
            io_display_battle(foe, self);
            wrefresh(battle);
            sleep(1);
            if (i == 2) {
              io_display_battle(foe, self);
              wrefresh(battle);
              sleep(1);
              return 10;
            }
            return i;
          }
          else if (world.pc->bag[cursor]->num == 0) {
            mvwprintw(battle, 16, 2, "You don't have any.");
            io_display_battle(foe, self);
            wrefresh(battle);
            sleep(2);
            return 0;
          }
          io_display_battle(foe, self);
          wrefresh(battle);
          sleep(1);
          break;
        case 27:
          scrn = 0;
          cursor = 0;
          break;
        default:
          break;
      }
    }
    /* Fight: */
    else if (scrn == 2) {
      switch (c = getch()) {
        case KEY_LEFT:
          cursor = max(cursor - 1, 0);
          break;
        case KEY_RIGHT:
          cursor = min(cursor + 1, 3);
          break;
        case 10:
          if (self->moves[cursor] != NULL) {
            scrn = 0;
            move_used = cursor;
            cursor = 0;
            return 1;
          }
          break;
        case 27:
          scrn = 0;
          cursor = 1;
          break;
        default:
          break;
      }
    }
    /* Party: */
    else if (scrn == 4) {
      i = 0;
      if ((i = io_party_select(0)) != cur_pkmn && i > -1) {
        cur_pkmn = i;
        return 3 + i;
      }
      else
        return 0;
    }
      
    if (io_battle_btns(scrn, cursor, wild, attempts++) == 1) {
      if (rand() % 256 < escape_chance(self, foe, attempts++))
        return 2;
      else {
        mvwprintw(battle, 16, 2, "You failed to escape!");
        wrefresh(battle);
        sleep(2);
      }
      return 1;
    }
    wrefresh(battle);
  } while (1);
  return 0;
}


/**
 * This will print out different status messages
 * depending on when in the turn this happens and
 * what the status is.
*/
int io_status_print(Party_Member *p, int status, bool after_turn) {
  int i = 0;
  switch(p->status) {
  // poisoned:
  case 67:
    if (after_turn && status != 0) {
      mvwprintw(battle, 16, 2, "%s is poisoned.", p->species->identifier);
      i = 1;
    }
    break;
  // badly poisoned:
  case 3:
    if (after_turn && status != 0) {
      mvwprintw(battle, 16, 2, "%s is badly poisoned.", p->species->identifier);
      i = 1;
    }
    break;
  // asleep:
  case 2:
    // pokemon is trying to wake up
    if (status == 0) {
      mvwprintw(battle, 16, 2, "%s woke up!", p->species->identifier);
      p->status = 0;
    }
    else
      mvwprintw(battle, 16, 2, "%s is fast asleep.", p->species->identifier);
    i = 1;
    break;
  // paralyzed:
  case 68:
    if (status == 0) {
      mvwprintw(battle, 16, 2, "%s shook off it's paralysis!", p->species->identifier);
      p->status = 0;
    }
    else
      mvwprintw(battle, 16, 2, "%s is still paralyzed.", p->species->identifier);
    i++;
    break;
  // frozen:
  case 6:
    if (status == 0) {
      mvwprintw(battle, 16, 2, "%s thawed from being frozen!", p->species->identifier);
      p->status = 0;
    }
    else
      mvwprintw(battle, 16, 2, "%s is still frozen.", p->species->identifier);
    i++;
    break;
  // burned:
  case 5:
    if (after_turn && status != 0) {
      mvwprintw(battle, 16, 2, "%s is burned.", p->species->identifier);
      i++;
    }
    break;
  // wound:
  case 43:
    if (after_turn && status != 0) {
      mvwprintw(battle, 16, 2, "%s is wound up!", p->species->identifier);
      i++;
    }
    break;
  // confused:
  case 77:
    if (!after_turn && status != 0) {
      mvwprintw(battle, 16, 2, "%s is confused.", p->species->identifier);
      i++;
    }
    else if (status == 0) {
      mvwprintw(battle, 16, 2, "%s snapped out of its confusion!", p->species->identifier);
      p->status = 0;
      i++;
    }
    break;
  default:
    break;
  }

  return i;
}


/**
 * This will print out a status message
 * declaring what pokemon has been affected and
 * what status it has been inflicted with
 * after a move that causes a status effect.
*/
void io_new_status(Party_Member *p) {
  int i = 0;
  switch(p->status) {
  // poisoned:
  case 67:
    mvwprintw(battle, 16, 2, "%s was poisoned!", p->species->identifier);
    i++;
    break;
  // badly poisoned:
  case 3:
    mvwprintw(battle, 16, 2, "%s was badly poisoned!", p->species->identifier);
    i++;
    break;
  // asleep:
  case 2:
    mvwprintw(battle, 16, 2, "%s fell asleep!", p->species->identifier);
    i++;
    break;
  // paralyzed:
  case 68:
    mvwprintw(battle, 16, 2, "%s was paralyzed!", p->species->identifier);
    i++;
    break;
  // frozen:
  case 6:
    mvwprintw(battle, 16, 2, "%s was frozen!", p->species->identifier);
    i++;
    break;
  // burned:
  case 5:
    mvwprintw(battle, 16, 2, "%s was burned!", p->species->identifier);
    i++;
    break;
  // wound:
  case 43:
    mvwprintw(battle, 16, 2, "%s is wound up!", p->species->identifier);
    i++;
    break;
  // confused:
  case 77:
    mvwprintw(battle, 16, 2, "%s was confused!", p->species->identifier);
    i++;
    break;
  default:
    break;
  }

  if (i == 1) {
    wrefresh(battle);
    sleep(2);
  }
}


/**
 * This is a selection screen that will be
 * displayed when a pokemon levels up, is
 * able to learn a new move, and already
 * has 4 moves in its learned move list.
*/
int io_move_selection(Party_Member *p) {
  int selected = 0;
  int c;
  WINDOW *win = newwin(6, 12, 10, 67);
  box(win, ACS_VLINE, ACS_HLINE);
  keypad(stdscr, TRUE);
  keypad(battle, TRUE);
  keypad(win, TRUE);
  bool entered = 0;
  
  do {
    for (int i = 0; i < 4; i++) {
      if (i == selected)
        wattron(win, COLOR_PAIR(HIGHLIGHT));
      mvwprintw(win, 11, 68, "%s", p->moves[i]->identifier);
      if (i == selected)
        wattroff(win, COLOR_PAIR(HIGHLIGHT));
    }
    wrefresh(win);

    switch(c = wgetch(win)) {
      case KEY_UP:
        selected = max(0, selected - 1);
        break;
      case KEY_DOWN:
        selected = min(3, selected + 1);
        break;
      case 27:
        selected = -1;
      case 10:
        entered = 1;
        break;
    }
  } while (!entered);

  keypad(win, FALSE);
  wclear(win);
  wrefresh(win);
  delwin(win);
  win = NULL;
  return selected;
}


/**
 * This will consider the level-up moveset for
 * the pokemon currently leveling up. If it has a move it can learn, 
*/
void io_level_up(Party_Member *p) {
  // update each stat:
  for (int k = 1; k < 6; ++k)
    p->full_stats[k] = floor(((((p->stats[k] + p->ivs[k]) * 2) * p->level) / 100) + 5);
  float pcnt_hp = ((float) p->full_stats[0]) / ((float) p->maxHP);
  p->maxHP = floor(((((p->stats[0] + p->ivs[0]) * 2) * p->level)/ 100) + p->level + 10);
  p->full_stats[0] = (int) floor(p->maxHP * pcnt_hp);

  unsigned long i;
  int j, selected;
  mvwhline(battle, 16, 1, ' ', 78);
  bool decided = 0;
  bool can_learn = 0;
  for (i = 0; i < pokemon_moves.size(); ++i) {
    can_learn = 0;
    // associated with this pokemon and can be learned by leveling up:
    if (p->pid == pokemon_moves[i].get()->pokemon_id
      && pokemon_moves[i].get()->pokemon_move_method_id == 1) {
      if (p->level <= pokemon_moves[i].get()->level) {
      // check if there are any empty move slots:
        for (j = 0; j < 4; ++j) {
          // already learned this move:
          if (p->moves[j] != NULL 
            && p->moves[j]->move_id == pokemon_moves[i].get()->move_id) {
            can_learn = 0;
            break;
          }
          // has empty slots:
          else if (p->moves[j] == NULL) {
            can_learn = 1;
            break;
          }
          can_learn = 1;
        }
        // has an empty move slot:
        if (j < 4 && can_learn) {
          decided = 1;
          selected = j;
          break;
        }

        // does not know this move already, can learn it:
        if (can_learn && j == 4) {
          mvwprintw(battle, 16, 2, "%s would like to learn %s.", p->species->identifier, moves[pokemon_moves[i].get()->move_id - 1].get()->identifier);
          wrefresh(battle);
          sleep(2);
          mvwhline(battle, 16, 1, ' ', 78);

          mvwprintw(battle, 16, 2, "Which move would you like to replace?");
          wrefresh(battle);

          if ((selected = io_move_selection(p)) == -1)
            break;
          else {
            decided = 1;
            break;
          }
        }

      }
      else
        break;
    }
  }

  mvwhline(battle, 16, 1, ' ', 78);
  // PC decided to learn this move:
  if (decided) {
    p->moves[selected] = moves[pokemon_moves[i].get()->move_id - 1].get();
    mvwprintw(battle, 16, 2, "%s learned %s!", p->species->identifier, moves[pokemon_moves[i].get()->move_id - 1].get()->identifier);
  }
  // PC did not learn this move.
  else if (!decided && can_learn) {
    mvwprintw(battle, 16, 2, "%s did not learn %s.", p->species->identifier, moves[pokemon_moves[i].get()->move_id - 1].get()->identifier);
  }
}


/**
 * This displays a message in the battle window
 * based on the effectiveness of the move used
 * if it does damage to another pokemon.
*/
void io_type_message(float type) {
  mvwhline(battle, 16, 1, ' ', 78);

  if (type >= 2.0)
    mvwprintw(battle, 16, 2, "It was super effective!");
  else if (type < 1.0 && type > 0)
    mvwprintw(battle, 16, 2, "It wasn't very effective.");
  else if (type == 0)
    mvwprintw(battle, 16, 2, "It had no effect.");
}


/**
 * This will simulate a turn in a pokemon battle.
 * various return values will mean different things:
 * - 0, neither fainted
 * - 1, first fainted
 * - 2, second fainted
 * - 3, both fainted
*/
int io_battle_turns(int first_dmg, int scnd_dmg, Party_Member *p1, Party_Member *p2, int m1, int m2, bool pc_first, float p1_type, float p2_type) {
  int hit_chance = rand() % 100;
  int status = 0;

  mvwhline(battle, 16, 1, ' ', 78);

  /* pokemon has a status condition: */
  if (p1->status != 0) {
    status = effect_result(p1, 0);
    if (io_status_print(p1, status, 0) == 1) {
      wrefresh(battle);
      sleep(2);
    }
    // status changed, so change p1's status:
    if (status != 1 && status != 100)
      p1->status = status;
  }

  // pokemon cannot be asleep, frozen, or paralyzed to use a move:
  if (p1->status != 2 && p1->status != 68 && p1->status != 6) {
    mvwhline(battle, 16, 1, ' ', 78);
    // status returned was 77, so it hit itself
    if (p1->status == 77 && status != 1)
      mvwprintw(battle, 16, 2, "%s hit itself in its confusion!", p1->species->identifier);
    // status returned was 1, it did not hit itself but it did not snap out of confusion.
    else if (status != 100) {
      //TODO: determine status effect chances.
      if (p1->moves[m1]->power == INT_MAX) {
        mvwprintw(battle, 16, 2, "%s used %s!", p1->species->identifier, p1->moves[m1]->identifier);
      }
      else if (hit_chance <= p1->moves[m1]->accuracy) {
        p2->full_stats[0] = max(0, (p2->full_stats[0] - first_dmg));
        mvwprintw(battle, 16, 2, "%s used %s for %d damage!", p1->species->identifier, p1->moves[m1]->identifier, first_dmg);
      }
      else {
        mvwprintw(battle, 16, 2, "%s used %s and missed.", p1->species->identifier, p1->moves[m1]->identifier);
      }
      wrefresh(battle);
      sleep(2);

      if (p1_type != 1.0 && hit_chance < p1->moves[m1]->accuracy && p1->moves[m1]->power != INT_MAX)
        io_type_message(p1_type);

      if (status_effect(p1->moves[m1], p2) != 0) {
        wrefresh(battle);
        sleep(2);
        mvwhline(battle, 16, 1, ' ', 78);
        io_new_status(p2);
      }
    }

    if (pc_first)
      io_display_battle(p2, p1);
    else
      io_display_battle(p1, p2);
    wrefresh(battle);
    sleep(2);

    // fainted on its turn:
    if (status == 100) {
      mvwhline(battle, 16, 2, ' ', 78);
      mvwprintw(battle, 16, 2, "%s fainted.", p1->species->identifier);
      p1->defeated = 1;
      wrefresh(battle);
      sleep(2);
    }
  }

  // defeated opposing pokemon:
  if (p2->full_stats[0] == 0) {
    mvwhline(battle, 16, 1, (int) ' ', 78);
    mvwprintw(battle, 16, 2, "%s fainted.", p2->species->identifier);
    p2->defeated = 1;
    wrefresh(battle);
    sleep(2);
    return 2;
  }

  hit_chance = rand() % 100;

  mvwhline(battle, 16, 1, (int) ' ', 78);

  /* pokemon has a status condition: */  
  if (p2->status != 0) {
    status = effect_result(p2, 0);
    if (io_status_print(p2, status, 0) == 1) {
      wrefresh(battle);
      sleep(2);
    }
    // status changed, so change p1's status:
    if (status != 1 && status != 100)
      p2->status = status;
  }

  if (p2->status != 2 && p2->status != 68 && p2->status != 6) {
    mvwhline(battle, 16, 1, ' ', 78);
    // pokemon hit itself while confused:
    if (p2->status == 77 && status != 1)
      mvwprintw(battle, 16, 2, "%s hit itself in its confusion!", p2->species->identifier);
    // can move, can't attack if the first pokemon in the order has fainted already:
    else if (status != 100 && p1->defeated != 1) {
      if (p2->moves[m2]->power == INT_MAX)
        mvwprintw(battle, 16, 2, "%s used %s!", p2->species->identifier, p2->moves[m2]->identifier);
      else if (hit_chance <= p2->moves[m2]->accuracy) {
        p1->full_stats[0] = max(0, p1->full_stats[0] - scnd_dmg);
        mvwprintw(battle, 16, 2, "%s used %s for %d damage!", p2->species->identifier, p2->moves[m2]->identifier, scnd_dmg);
      }
      else {
        mvwprintw(battle, 16, 2, "%s used %s and missed.", p2->species->identifier, p2->moves[m2]->identifier);
      }
      wrefresh(battle);
      sleep(2);
      if (p2_type != 1.0 && hit_chance <= p2->moves[m2]->accuracy && p2->moves[m2]->power != INT_MAX)
        io_type_message(p2_type);

      if (status_effect(p2->moves[m2], p1) != 0) {
        mvwhline(battle, 16, 1, ' ', 78);
        io_new_status(p1);
      }
    }
    
    if (pc_first)
      io_display_battle(p2, p1);
    else
      io_display_battle(p1, p2);
    wrefresh(battle);
    sleep(2);

    // fainted on its turn:
    if (status == 100) {
      mvwhline(battle, 16, 1, ' ', 78);
      mvwprintw(battle, 16, 2, "%s fainted.", p2->species->identifier);
      p2->defeated = 1;
      wrefresh(battle);
      sleep(2);
    }
  }
  
  // defeated opposing pokemon:
  if (p1->full_stats[0] == 0 && p1->defeated != 1) {
    mvwhline(battle, 16, 1, ' ', 78);
    mvwprintw(battle, 16, 2, "%s fainted.", p1->species->identifier);
    p1->defeated = 1;
    wrefresh(battle);
    sleep(2);
    return 1;
  }

  /* After battle effects by statuses: */
  if (p1->status != 0 && p1->defeated != 1) {
    mvwhline(battle, 16, 1, ' ', 78);
    status = effect_result(p1, 1);
    if (pc_first)
      io_display_battle(p2, p1);
    else
      io_display_battle(p1, p2);
    if (io_status_print(p1, status, 1) == 1) {
      wrefresh(battle);
      sleep(2);
    }
  }

  if (p2->status != 0 && p2->defeated != 1) {
    mvwhline(battle, 16, 1, ' ', 78);
    status = effect_result(p2, 1);
    if (pc_first)
      io_display_battle(p2, p1);
    else
      io_display_battle(p1, p2);
    if (io_status_print(p2, status, 1) == 1) {
      wrefresh(battle);
      sleep(2);
    }
  }

  // determine 
  if (p1->defeated == 0 && p2->defeated == 1)
    return 2;
  if (p1->defeated == 1 && p2->defeated == 0)
    return 1;
  // both fainted due to status effects:
  if (p1->defeated == 1 && p2->defeated == 1)
    return 3;

  return 0;
}


/**
 * This is a generic function that can be used
 * for either a trainer or wild pokemon battle.
 * This will return which pokemon has fainted, which
 * determines what will happen after the round.
*/
int io_battle(Party_Member *p1, Party_Member *p2, int turn, bool trainer_battle) {
  bool is_physical = 0;
  int turn_effect = 0;
  int fainted = 0;
  int foe_move = 0;
  int foe_dmg, self_dmg;
  float stab, crit1, crit2;
  float type2;
  stab = crit1 = crit2 = 1.0;
  foe_dmg = self_dmg = 0;
  float foe_type, self_type;
  foe_type = self_type = 0;

    if (turn == 9 || ((turn == 2 || turn == 10) && !trainer_battle))
      return 0;

    if (turn > 0) {
      foe_move = rand() % 2;
      if (p2->moves[foe_move] == NULL)
        foe_move = 0;
      stab = 1;
      if (p2->moves[foe_move]->type_id == p2->type[0]->type_id
        || (p2->type[1] != NULL
            && p2->moves[foe_move]->type_id == p2->type[1]->type_id))
        stab = 1.5;
      crit2 = floor(p2->stats[5] / 2) < (rand() % 256) ? 1.5 : 1;
      is_physical = (p2->moves[foe_move]->damage_class_id == 2 ? 1 : 0);
      
      if (p1->type[1] == NULL)
        type2 = 1.0;
      else
        type2 = type_matchups[p2->moves[foe_move]->type_id - 1][p1->type[1]->type_id - 1];

      foe_type = type_effect(type_matchups[p2->moves[foe_move]->type_id - 1][p1->type[0]->type_id - 1], type2);

      foe_dmg = damage(p2->moves[foe_move]->power, p2, p1, crit2, stab,
                        foe_type, is_physical);
      
      /* Implement move priority: */
      if (move_used > -1) {
        stab = 1;
        if (p1->moves[move_used]->type_id == p1->type[0]->type_id
          || (p1->type[1] != NULL
              && p1->moves[move_used]->type_id == p1->type[1]->type_id))
          stab = 1.5;
        crit1 = floor(p1->stats[5] / 2) < (rand() % 256) ? 1.5 : 1;
        is_physical = (p1->moves[move_used]->damage_class_id == 2 ? 1 : 0);
        if (p2->type[1] == NULL)
          type2 = 1.0;
        else
          type2 = type_matchups[p1->moves[move_used]->type_id - 1][p2->type[1]->type_id - 1];

        self_type = type_effect(type_matchups[p1->moves[move_used]->type_id - 1][p2->type[0]->type_id - 1], type2);
        self_dmg = damage(p1->moves[move_used]->power, p1, p2, crit1, stab, 
                          self_type, is_physical);
        
        /* Move priority: */
        if (p1->moves[move_used]->priority < p2->moves[foe_move]->priority)
          fainted = io_battle_turns(self_dmg, foe_dmg, p1, p2, move_used, foe_move, 1, self_type, foe_type);
        else if (p1->moves[move_used]->priority > p2->moves[foe_move]->priority) {
          turn_effect = io_battle_turns(foe_dmg, self_dmg, p2, p1, foe_move, move_used, 0, foe_type, self_type);
          fainted = ((turn_effect == 1) ? 2 : ((turn_effect == 2) ? 1 : (turn_effect == 3 ? 3 : 0)));
        }
        else if (p1->moves[move_used]->priority == p2->moves[foe_move]->priority) {
          // determine by speed:
          if (p1->full_stats[5] > p2->full_stats[5])
            fainted = io_battle_turns(self_dmg, foe_dmg, p1, p2, move_used, foe_move, 1, self_type, foe_type);
          else {
            turn_effect = io_battle_turns(foe_dmg, self_dmg, p2, p1, foe_move, move_used, 0, self_type, foe_type);
            fainted = ((turn_effect == 1) ? 2 : ((turn_effect == 2) ? 1 : (turn_effect == 3 ? 3 : 0)));
          }
        }
      }
      /* Player switched out pokemon */
      else {
        mvwhline(battle, 16, 1, ' ', 78);
        int hit_chance = rand() % 100;
        if (p2->moves[foe_move]->power != INT_MAX) {
          if (hit_chance < p2->moves[foe_move]->accuracy) {
            mvwprintw(battle, 16, 2, "%s used %s for %d damage!", p2->species->identifier, p2->moves[foe_move]->identifier, foe_dmg);
            wrefresh(battle);
            sleep(2);
            if (crit2 == 1.5) {
              mvwhline(battle, 16, 1, ' ', 78);
              mvwprintw(battle, 16, 2, "It was a critical hit!");
              wrefresh(battle);
              sleep(2);
            }
            if (foe_type >= 2.0) {
              mvwhline(battle, 16, 1, ' ', 78);
              mvwprintw(battle, 16, 2, "It was super effective!");
              wrefresh(battle);
              sleep(2);
            }
            else if (foe_type < 1.0 && foe_type > 0) {
              mvwhline(battle, 16, 1, ' ', 78);
              mvwprintw(battle, 16, 2, "It wasn't very effective.");
              wrefresh(battle);
              sleep(2);
            }
            else if (foe_type == 0.0) {
              mvwhline(battle, 16, 1, ' ', 78);
              mvwprintw(battle, 16, 2, "It had no effect.");
              sleep(2);
              foe_dmg = 0;
            }
          }
          else {
            mvwprintw(battle, 16, 2, "%s used %s and missed!", p2->species->identifier, p2->moves[foe_move]->identifier);
            foe_dmg = 0;
          }
        }
        else {
          mvwprintw(battle, 16, 2, "%s used %s!", p2->species->identifier, p2->moves[foe_move]->identifier);
          foe_dmg = 0;
        }
        p1->full_stats[0] = std::max(0, p1->full_stats[0] - foe_dmg);
        io_display_battle(p2, p1);
        wrefresh(battle);
        sleep(2);
        if (p1->full_stats[0] == 0)
          return 1;
      }
    }

    return fainted;
}


/**
 * This will go through the sequence of
 * adding experience points to each pokemon in the
 * party that has not fainted and has participated
 * in the battle with this opposing pokemon.
*/
void exp_after(Party_Member *p, int num_participate, float wild_or_trainer) {
  for (int i = 0; i < world.pc->num_in_party; ++i) {
    if (world.pc->party[i]->participate 
      && world.pc->party[i]->level < 100
      && !world.pc->party[i]->defeated) {
      int exp = exp_gain(world.pc->party[i], wild_or_trainer, p, num_participate);
      world.pc->party[i]->experience += exp;
      mvwhline(battle, 16, 1, ' ', 78);
      mvwprintw(battle, 16, 2, "Your %s gained %d exp!", world.pc->party[i]->species->identifier, exp);
      wrefresh(battle);
      sleep(2);
      /* Level up: */
      while (world.pc->party[i]->experience >= experience[((world.pc->party[i]->species->growth_rate_id - 1) * 100) + world.pc->party[i]->level].get()->experience) {
        world.pc->party[i]->level++;
        mvwhline(battle, 16, 1, ' ', 78);
        mvwprintw(battle, 16, 2, "Your %s is now level %d!", world.pc->party[i]->species->identifier, world.pc->party[i]->level);
        io_display_battle(p, world.pc->party[cur_pkmn]);
        wrefresh(battle);
        sleep(2);
        io_level_up(world.pc->party[i]);
        io_display_battle(p, world.pc->party[cur_pkmn]);
        wrefresh(battle);
        sleep(2);
      }
      if (i != cur_pkmn)
        world.pc->party[i]->participate = 0;
    }
  }
}


/**
 * Trainer battle interactions handler, displays
 * the full battle window.
*/
void io_trainer_battle(Character *aggressor, Character *defender)
{
  Npc *n = (Npc *) ((aggressor == world.pc) ? defender : aggressor);

  // build battle window:
  battle = newwin(23, 80, 1, 0);
  box(battle, ACS_VLINE, ACS_HLINE);
  mvwdelch(battle, 15, 79);
  mvwdelch(battle, 15, 0);
  mvwaddch(battle, 15, 0, ACS_LTEE);
  mvwhline(battle, 15, 1, ACS_HLINE, 78);
  mvwaddch(battle, 15, 79, ACS_RTEE);
  mvwdelch(battle, 18, 79);
  mvwdelch(battle, 18, 0);
  mvwaddch(battle, 18, 0, ACS_LTEE);
  mvwhline(battle, 18, 1, ACS_HLINE, 78);
  mvwaddch(battle, 18, 79, ACS_RTEE);
  keypad(stdscr, TRUE);
  keypad(battle, TRUE);

  int turn;
  bool battling = 1;
  int scrn = 0;
  int cursor = 0;
  int self, foe;
  self = foe = 0;
  int fainted = 0;
  int num_participate[n->num_in_party];
  for (int i = 0; i < n->num_in_party; ++i)
    num_participate[i] = 0;
  num_participate[foe] = 1;
  world.pc->party[cur_pkmn]->participate = 1;
  self = cur_pkmn;
    
  do {
    for (int i = 0; i < 6; i++) {
      if (i < n->num_in_party) {
        if (n->party[i]->defeated)
          wattron(battle, COLOR_PAIR(WATER));
        else if (n->party[i]->status > 0)
          wattron(battle, COLOR_PAIR(PATHS));
        else
          wattron(battle, COLOR_PAIR(ROCK));
        mvwaddch(battle, 3, 2 + i, ACS_BULLET);
        if (n->party[i]->defeated)
          wattroff(battle, COLOR_PAIR(WATER));
        else if (n->party[i]->status > 0)
          wattroff(battle, COLOR_PAIR(PATHS));
        else
          wattroff(battle, COLOR_PAIR(ROCK));
      }
      else
        mvwaddch(battle, 3, 2 + i, ACS_BULLET);

      if (i < world.pc->num_in_party) {
        if (world.pc->party[i]->defeated)
          wattron(battle, COLOR_PAIR(WATER));
        else if (world.pc->party[i]->status > 0)
          wattron(battle, COLOR_PAIR(PATHS));
        else
          wattron(battle, COLOR_PAIR(ROCK));
        mvwaddch(battle, 9, 46 + i, ACS_BULLET);
        if (world.pc->party[i]->defeated)
          wattroff(battle, COLOR_PAIR(WATER));
        else if (world.pc->party[i]->status > 0)
          wattroff(battle, COLOR_PAIR(PATHS));
        else
          wattroff(battle, COLOR_PAIR(ROCK));
      }
      else
        mvwaddch(battle, 9, 46 + i, ACS_BULLET);
    }
    move_used = -1;
    io_battle_btns(scrn, cursor, 0, 0);
    io_display_battle(n->party[foe], world.pc->party[self]);
    wrefresh(battle);
    // determine if a turn is being used or not:
    turn = io_battle_input(scrn, cursor, 0, world.pc->party[self], n->party[foe]);

    if (turn == 9)
      break;

    if (turn > 2) {
      io_display_battle(n->party[foe], world.pc->party[self]);
      self = turn - 3;
      cur_pkmn = self;
      mvwhline(battle, 16, 1, ' ', 78);
      mvwprintw(battle, 16, 2, "You sent out %s!", world.pc->party[self]->species->identifier);
      if (!world.pc->party[self]->participate)
        num_participate[foe] = min(world.pc->num_in_party, num_participate[foe] + 1);
      world.pc->party[self]->participate = 1;
      wrefresh(battle);
      sleep(2);
      io_display_battle(n->party[foe], world.pc->party[self]);
      wrefresh(battle);
      sleep(2);
    }

    if (turn != 0) {
      fainted = io_battle(world.pc->party[self], n->party[foe], turn, 1);

      /* PC pokemon fainted: */
      if (fainted == 1 || fainted == 3) {
        world.pc->num_pkmn_defeated++;
        world.pc->party[self]->defeated = 1;
        io_display_battle(n->party[foe], world.pc->party[self]);
        // PC chooses which pokemon to send out:
        if (world.pc->num_pkmn_defeated < world.pc->num_in_party)
          cur_pkmn = io_party_select(2);
        // all pokemon have fainted, lost the battle:
        else
          break;

        self = cur_pkmn;
        if (world.pc->party[self]->participate)
          num_participate[foe] = max(1, num_participate[foe] - 1);
        world.pc->party[self]->participate = 1;
        mvwhline(battle, 16, 1, ' ', 78);
        mvwprintw(battle, 16, 2, "Go, %s!", world.pc->party[self]->species->identifier);
        wrefresh(battle);
        sleep(2);

        /* If the opposing pokemon faints as well: */
        if (n->party[foe]->status != 0 && !n->party[foe]->defeated) {
          io_status_print(n->party[foe], n->party[foe]->status, 1);
          if (effect_result(n->party[foe], 1) == 100) {
            mvwhline(battle, 16, 1, ' ', 78);
            mvwprintw(battle, 16, 2, "%s fainted.", n->party[foe]->species->identifier);
            wrefresh(battle);
            sleep(2);
            exp_after(n->party[foe], num_participate[foe], 1.5);
          }
        }
      }
      /* NPC pokemon fainted: */
      if (fainted == 2 || fainted == 3) {
        n->num_pkmn_defeated++;

        // gain exp:
        exp_after(n->party[foe], num_participate[foe], 1.5);

        /* NPC can send out another pokemon: */
        if (n->num_pkmn_defeated < n->num_in_party) {
          foe++;
          mvwhline(battle, 16, 1, ' ', 78);
          mvwprintw(battle, 16, 2, "Your opponent sent out %s!", n->party[foe]->species->identifier);
          num_participate[foe] = 1;
          wrefresh(battle);
          sleep(2);
        }
      }
      cursor = 0;
      scrn = 0;
    }

    /* Either the NPC has lost or the PC has lost */
    if (n->num_pkmn_defeated == n->num_in_party
      || world.pc->num_pkmn_defeated == n->num_in_party)
        battling = 0;
  } while (battling);

  /* NPC lost: */
  if (n->num_pkmn_defeated == n->num_in_party || turn == 9) {
    for (int i = 0; i < world.pc->num_in_party; ++i) {
      if (cur_pkmn > i && world.pc->party[i]->defeated == 0)
        cur_pkmn = i;
      world.pc->party[i]->participate = 0;
    }
    world.pc->wallet += money_gain(n);

    n->defeated = 1;
    if (n->ctype == char_hiker || n->ctype == char_rival)
      n->mtype = move_wander;
    
    mvwhline(battle, 16, 1, ' ', 78);
    mvwprintw(battle, 16, 2, "You earned P%d!", money_gain(n));
    wrefresh(battle);
    sleep(2);

    io_queue_message("You're really strong!");
  }
  /* PC lost: */
  else {
    world.cur_map->cmap[world.pc->pos[dim_y]][world.pc->pos[dim_x]] = NULL;
    world.pc->num_pkmn_defeated = 0;
    
    /* Heal party pokemon: */
    for (int i = 0; i < world.pc->num_in_party; ++i) {
      world.pc->party[i]->full_stats[0] = world.pc->party[i]->maxHP;
      world.pc->party[i]->defeated = world.pc->party[i]->participate = 0;
    }
    world.cur_idx[dim_x] = 200;
    world.cur_idx[dim_y] = 200;
    world.cur_map = world.world[world.cur_idx[dim_y]][world.cur_idx[dim_x]];
    /* PC needs to be at the pokemon center: */
    pair_t dest;
    do {
      dest[dim_y] = rand() % (MAP_Y - 3) + 1;
      dest[dim_x] = rand() % (MAP_X - 3) + 1;
    } while (world.cur_map->map[dest[dim_y]][dest[dim_x]] != ter_center);
    world.pc->pos[dim_y] = dest[dim_y];
    world.pc->pos[dim_x] = dest[dim_x];
    world.cur_map->cmap[world.pc->pos[dim_y]][world.pc->pos[dim_x]] = world.pc;

    io_queue_message("You fled to the nearest Pokemon Center.");
  }
  keypad(battle, FALSE);
  wclear(battle);
  wrefresh(battle);
  delwin(battle);
  battle = NULL;
}


/**
 * For wild encounters, a random pokemon will be generated and the PC
 * can either fight, run, or use an item.
*/
void wild_battle() {
  Party_Member *p = generate_pkmn(min_lvl, max_lvl);

  // build battle window:
  battle = newwin(23, 80, 1, 0);
  box(battle, ACS_VLINE, ACS_HLINE);
  mvwdelch(battle, 15, 79);
  mvwdelch(battle, 15, 0);
  mvwaddch(battle, 15, 0, ACS_LTEE);
  mvwhline(battle, 15, 1, ACS_HLINE, 78);
  mvwaddch(battle, 15, 79, ACS_RTEE);
  mvwdelch(battle, 18, 79);
  mvwdelch(battle, 18, 0);
  mvwaddch(battle, 18, 0, ACS_LTEE);
  mvwhline(battle, 18, 1, ACS_HLINE, 78);
  mvwaddch(battle, 18, 79, ACS_RTEE);
  keypad(stdscr, TRUE);
  keypad(battle, TRUE);

  int turn;
  bool battling = 1;
  int scrn = 0;
  int cursor = 0;
  int self = cur_pkmn;
  int fainted;
  attempts = 0;
  int num_participate = 1;
  world.pc->party[self]->participate = 1;
  do {
    for (int i = 0; i < 6; i++) {
      if (i < world.pc->num_in_party) {
        if (world.pc->party[i]->defeated)
          wattron(battle, COLOR_PAIR(WATER));
        else if (world.pc->party[i]->status > 0)
          wattron(battle, COLOR_PAIR(PATHS));
        else
          wattron(battle, COLOR_PAIR(ROCK));
        mvwaddch(battle, 9, 46 + i, ACS_BULLET);
        if (world.pc->party[i]->defeated)
          wattroff(battle, COLOR_PAIR(WATER));
        else if (world.pc->party[i]->status > 0)
          wattroff(battle, COLOR_PAIR(PATHS));
        else
          wattroff(battle, COLOR_PAIR(ROCK));
      }
      else
        mvwaddch(battle, 9, 46 + i, ACS_BULLET);
    }
    mvwhline(battle, 16, 1, ' ', 78);
    mvwprintw(battle, 16, 2, "What will you do?");
    io_battle_btns(scrn, cursor, 1, attempts);
    io_display_battle(p, world.pc->party[self]);
    wrefresh(battle);
    // determine if a turn is being used or not:
    turn = io_battle_input(scrn, cursor, 1, world.pc->party[self], p);

    if (turn == 9 || turn == 10)
      break;

    if (turn > 2) {
      io_display_battle(p, world.pc->party[self]);
      self = turn - 3;
      cur_pkmn = self;
      mvwprintw(battle, 16, 2, "You sent out %s!", world.pc->party[self]->species->identifier);
      if (!world.pc->party[self]->participate)
        num_participate = min(world.pc->num_in_party, num_participate + 1);
      world.pc->party[self]->participate = 1;
      wrefresh(battle);
      sleep(2);
      mvwhline(battle, 16, 1, ' ', 78);
      io_display_battle(p, world.pc->party[self]);
      wrefresh(battle);
      sleep(2);
    }

    if (turn != 0) {
      fainted = io_battle(world.pc->party[self], p, turn, 0);

      /* PC pokemon fainted: */
      if (fainted == 1 || fainted == 3) {
        world.pc->num_pkmn_defeated++;
        world.pc->party[self]->defeated = 1;
        io_display_battle(p, world.pc->party[self]);
        // PC chooses which pokemon to send out:
        if (world.pc->num_pkmn_defeated < world.pc->num_in_party)
          cur_pkmn = io_party_select(2);
        // all pokemon have fainted, lost the battle:
        else
          break;
        
        self = cur_pkmn;
        mvwhline(battle, 16, 1, ' ', 78);
        mvwprintw(battle, 16, 2, "Go, %s!", world.pc->party[self]->species->identifier);
        if (world.pc->party[self]->participate)
          num_participate = max(1, num_participate - 1);
        world.pc->party[self]->participate = 1;
        wrefresh(battle);
        sleep(2);
      }
      /* wild pokemon fainted: */
      else if (fainted == 2 || fainted == 3) {
        for (int i = 0; i < world.pc->num_in_party; ++i) {
          if (cur_pkmn > i && world.pc->party[i]->defeated == 0)
            cur_pkmn = i;
          if (world.pc->party[i]->participate == 1 && world.pc->party[i]->defeated == 0) {
            int exp = exp_gain(world.pc->party[i], 1.0, p, num_participate);
            world.pc->party[i]->experience += exp;
            mvwhline(battle, 16, 1, ' ', 78);
            mvwprintw(battle, 16, 2, "%s earned %d exp!", world.pc->party[i]->species->identifier, exp);
            wrefresh(battle);
            sleep(2);
            if (world.pc->party[i]->level < 100 
            && world.pc->party[i]->experience >= experience[((world.pc->party[i]->species->growth_rate_id - 1) * 100) + world.pc->party[i]->level].get()->experience) {
              world.pc->party[i]->level++;
              mvwhline(battle, 16, 1, ' ', 78);
              mvwprintw(battle, 16, 2, "Your %s is now level %d!", world.pc->party[i]->species->identifier, world.pc->party[i]->level);
              io_display_battle(p, world.pc->party[self]);
              wrefresh(battle);
              sleep(2);
              io_level_up(world.pc->party[self]);
              io_display_battle(p, world.pc->party[self]);
              wrefresh(battle);
              sleep(2);
            }
          }
          if (i != self)
            world.pc->party[i]->participate = 0;
        }
        battling = 0;
      }
      cursor = 0;
      scrn = 0;
    }

    // PC party defeated, fled from battle, or caught pokemon:
    if (world.pc->num_pkmn_defeated == world.pc->num_in_party
      || turn == 2 || turn == 10)
      battling = 0;
  } while (battling);

  /* PC was defeated by wild pokemon: */
  if (world.pc->num_pkmn_defeated == world.pc->num_in_party) {
    world.cur_map->cmap[world.pc->pos[dim_y]][world.pc->pos[dim_x]] = NULL;
    world.pc->num_pkmn_defeated = 0;
    
    /* Heal party pokemon: */
    for (int i = 0; i < world.pc->num_in_party; ++i) {
      world.pc->party[i]->full_stats[0] = world.pc->party[i]->maxHP;
      world.pc->party[i]->defeated = world.pc->party[i]->participate = 0;
    }
    /* PC needs to be at the pokemon center: */
    pair_t dest;
    do {
      dest[dim_y] = rand() % (MAP_Y - 3) + 1;
      dest[dim_x] = rand() % (MAP_X - 3) + 1;
    } while (world.cur_map->map[dest[dim_y]][dest[dim_x]] != ter_center);
    world.pc->pos[dim_y] = dest[dim_y];
    world.pc->pos[dim_x] = dest[dim_x];
    world.cur_map->cmap[world.pc->pos[dim_y]][world.pc->pos[dim_x]] = world.pc;

    io_queue_message("You fled to the nearest Pokemon Center.");
  }
  /* Caught the wild pokemon: */
  else if (turn == 10 || turn == 9) {
    mvwhline(battle, 16, 1, ' ', 78);
    mvwprintw(battle, 16, 2, "You caught the wild %s!", p->species->identifier);
    wrefresh(battle);
    sleep(2);
    mvwhline(battle, 16, 1, ' ', 78);
    if (world.pc->num_in_party < 6) {
      world.pc->party[world.pc->num_in_party++] = p;
      mvwprintw(battle, 16, 2, "The %s was added to your party.", p->species->identifier);
    }
    else if (world.pc->num_in_pcc < 18) {
      world.pc->pcc[world.pc->num_in_pcc++] = p;
      mvwprintw(battle, 16, 2, "The %s was sent to your PC.", p->species->identifier);
    }
    wrefresh(battle);
    sleep(2);
    io_queue_message("You caught the wild %s!", p->species->identifier);
  }
  /* Fled battle: */
  else if (turn == 2)
    io_queue_message("You fled!");
  /* Defeated wild pokemon: */
  else
    io_queue_message("You defeated the wild %s", p->species->identifier);

  keypad(battle, FALSE);
  wclear(battle);
  wrefresh(battle);
  delwin(battle);
  battle = NULL;
}


/**
 * This funciton will display the PC's party pokemon.
 * this returns a 0 if the turn has been consumed and
 * a 1 if the turn has not been consumed.
*/
void io_party_display() {
  mvprintw(0, 0, "PC's Pokemon Party");
  refresh();

  /* Create menu pop-up: */
  int i;
  menu = newwin(13, 36, 5, 22);
  keypad(stdscr, TRUE);
  keypad(menu, TRUE);
  box(menu, ACS_VLINE, ACS_HLINE);
  for (i = 0; i < world.pc->num_in_party; ++i) {
    // pokemon name:
    if (world.pc->party[i]->is_shiny)
      wattron(menu, COLOR_PAIR(TREE));
    mvwprintw(menu, (i * 2) + 1, 1, "%s", world.pc->party[i]->species->identifier);
    if (world.pc->party[i]->is_shiny)
      wattroff(menu, COLOR_PAIR(TREE));

    // pokemon gender:
    if (world.pc->party[i]->gender != 'N') {
      wattron(menu, COLOR_PAIR(((world.pc->party[i]->gender == 'M') ? WATER : ROCK)));
      mvwprintw(menu, (i * 2) + 1, 12, "%c", world.pc->party[i]->gender);
      wattroff(menu, COLOR_PAIR(((world.pc->party[i]->gender == 'M') ? WATER : ROCK)));
    }
    // pokemon level:
    mvwprintw(menu, (i * 2) + 1, 14, "lv.%d", world.pc->party[i]->level);

    // pokemon hp:
    float pcnt_hp = (100.0 * world.pc->party[i]->full_stats[0]) / world.pc->party[i]->maxHP;
    wattron(menu, COLOR_PAIR((pcnt_hp > 35 ? GRASS : (pcnt_hp > 10 ? PATHS : ROCK))));
    mvwprintw(menu, (i * 2) + 1, 21, "hp: %d/%d", world.pc->party[i]->full_stats[0], world.pc->party[i]->maxHP);
    wattroff(menu, COLOR_PAIR((pcnt_hp > 35 ? GRASS : (pcnt_hp > 10 ? PATHS : ROCK))));

    if (i < 5) {
      mvwdelch(menu, (i * 2) + 2, 35);
      mvwdelch(menu, (i * 2) + 2, 0);
      mvwaddch(menu, (i * 2) + 2, 0, ACS_LTEE);
      mvwhline(menu, (i * 2) + 2, 1, ACS_HLINE, 34);
      mvwaddch(menu, (i * 2) + 2, 35, ACS_RTEE);
    }
  }

  wrefresh(menu);

  int c;
  i = 1;
  do {
    switch(c = getch()) {
      case 27:
        i = 0;
        break;
      default:
        break;
    }
  } while (i);
  keypad(menu, FALSE);
  delwin(menu);
  menu = NULL;
  /* Clear top message */
  move(0, 0);
  clrtoeol();
  io_display();
  refresh();
}

uint32_t move_pc_dir(uint32_t input, pair_t dest)
{
  move(0, 0);
  clrtoeol();
  dest[dim_y] = world.pc->pos[dim_y];
  dest[dim_x] = world.pc->pos[dim_x];

  switch (input) {
  case 1:
  case 2:
  case 3:
    dest[dim_y]++;
    break;
  case 4:
  case 5:
  case 6:
    break;
  case 7:
  case 8:
  case 9:
    dest[dim_y]--;
    break;
  }
  switch (input) {
  case 1:
  case 4:
  case 7:
    dest[dim_x]--;
    break;
  case 2:
  case 5:
  case 8:
    break;
  case 3:
  case 6:
  case 9:
    dest[dim_x]++;
    break;
  case '>':
    if (world.cur_map->map[world.pc->pos[dim_y]][world.pc->pos[dim_x]] == ter_mart
        || world.cur_map->map[world.pc->pos[dim_y]][world.pc->pos[dim_x]] == ter_center) {
          io_building_menu((world.cur_map->map[world.pc->pos[dim_y]][world.pc->pos[dim_x]] == ter_mart ? MART : CENTER));
          move(0, 0);
          clrtoeol();
        }
    break;
  }
  if (world.cur_map->map[dest[dim_y]][dest[dim_x]] == ter_gate) {
    world.cur_map->cmap[dest[dim_y]][dest[dim_x]] = NULL;
    // go to a different map:
    if (dest[dim_y] == 0 && dest[dim_x] == world.cur_map->n) {
      world.cur_idx[dim_y]--;
    }
    else if (dest[dim_y] == MAP_Y - 1 && dest[dim_x] == world.cur_map->s) {
      world.cur_idx[dim_y]++;
    }
    else if (dest[dim_x] == 0 && dest[dim_y] == world.cur_map->w) {
      world.cur_idx[dim_x]--;
    }
    else if (dest[dim_x] == MAP_X - 1 && dest[dim_y] == world.cur_map->e) {
      world.cur_idx[dim_x]++;
    }

    new_map(0);
    world.cur_map->cmap[world.pc->pos[dim_y]][world.pc->pos[dim_x]] = world.pc;
    io_display();
  }

  if (world.cur_map->cmap[dest[dim_y]][dest[dim_x]]) {
    if (dynamic_cast<Npc *> (world.cur_map->cmap[dest[dim_y]][dest[dim_x]]) &&
        ((Npc *) world.cur_map->cmap[dest[dim_y]][dest[dim_x]])->defeated) {
      mvprintw(0, 0, "Hello there, friend!");
      return 1;
    } else if (dynamic_cast<Npc *> (world.cur_map->cmap[dest[dim_y]][dest[dim_x]])) {
      attron(COLOR_PAIR(SURPRISE));
      mvaddch(world.pc->pos[dim_y], world.pc->pos[dim_x], '!');
      attroff(COLOR_PAIR(SURPRISE));
      refresh();
      sleep(1);
      io_trainer_battle(world.pc, world.cur_map->cmap[dest[dim_y]][dest[dim_x]]);
      // Not actually moving, so set dest back to PC position
      dest[dim_x] = world.pc->pos[dim_x];
      dest[dim_y] = world.pc->pos[dim_y];
      io_display();
      return 1;
    }
  }
  
  if (move_cost[char_pc][world.cur_map->map[dest[dim_y]][dest[dim_x]]] ==
      DIJKSTRA_PATH_MAX) {
    mvprintw(0, 0, "You can't go that way!");
    return 1;
  }

  if (world.cur_map->map[dest[dim_y]][dest[dim_x]] == ter_grass
    && (rand() % 100) <= ENCOUNTER_PROB) {
      attron(COLOR_PAIR(SURPRISE));
      mvaddch(world.pc->pos[dim_y], world.pc->pos[dim_x], '!');
      attroff(COLOR_PAIR(SURPRISE));
      refresh();
      sleep(1);
      wild_battle();
      io_display();
  }

  return 0;
}

void io_teleport_world(pair_t dest)
{
  /* mvscanw documentation is unclear about return values.  I believe *
   * that the return value works the same way as scanf, but instead   *
   * of counting on that, we'll initialize x and y to out of bounds   *
   * values and accept their updates only if in range.                */
  int x = INT_MAX, y = INT_MAX;
  
  world.cur_map->cmap[world.pc->pos[dim_y]][world.pc->pos[dim_x]] = NULL;

  echo();
  curs_set(1);
  do {
    mvprintw(0, 0, "Enter x [-200, 200]:           ");
    refresh();
    mvscanw(0, 21, (char *) "%d", &x);
  } while (x < -200 || x > 200);
  do {
    mvprintw(0, 0, "Enter y [-200, 200]:          ");
    refresh();
    mvscanw(0, 21, (char *) "%d", &y);
  } while (y < -200 || y > 200);

  refresh();
  noecho();
  curs_set(0);

  x = min(x + 200, 400);
  y = min (y + 200, 400);

  world.cur_idx[dim_x] = x;
  world.cur_idx[dim_y] = y;

  new_map(1);
  io_teleport_pc(dest);
}

void io_handle_input(pair_t dest)
{
  uint32_t turn_not_consumed;
  int key;

  do {
    switch (key = getch()) {
    case '7':
    case 'y':
    case KEY_HOME:
      turn_not_consumed = move_pc_dir(7, dest);
      break;
    case '8':
    case 'k':
    case KEY_UP:
      turn_not_consumed = move_pc_dir(8, dest);
      break;
    case '9':
    case 'u':
    case KEY_PPAGE:
      turn_not_consumed = move_pc_dir(9, dest);
      break;
    case '6':
    case 'l':
    case KEY_RIGHT:
      turn_not_consumed = move_pc_dir(6, dest);
      break;
    case '3':
    case 'n':
    case KEY_NPAGE:
      turn_not_consumed = move_pc_dir(3, dest);
      break;
    case '2':
    case 'j':
    case KEY_DOWN:
      turn_not_consumed = move_pc_dir(2, dest);
      break;
    case '1':
    case 'b':
    case KEY_END:
      turn_not_consumed = move_pc_dir(1, dest);
      break;
    case '4':
    case 'h':
    case KEY_LEFT:
      turn_not_consumed = move_pc_dir(4, dest);
      break;
    case '5':
    case ' ':
    case '.':
    case KEY_B2:
      dest[dim_y] = world.pc->pos[dim_y];
      dest[dim_x] = world.pc->pos[dim_x];
      turn_not_consumed = 0;
      break;
    case '>':
      turn_not_consumed = move_pc_dir('>', dest);
      break;
    /* See pokemon party: */
    case 'x':
      io_party_display();
      turn_not_consumed = 1;
      break;
    case 'Q':
      dest[dim_y] = world.pc->pos[dim_y];
      dest[dim_x] = world.pc->pos[dim_x];
      world.quit = 1;
      turn_not_consumed = 0;
      break;
      break;
    case 't':
      io_list_trainers();
      turn_not_consumed = 1;
      break;
    case 'f':
      /* Fly to any map in the world. */
      io_teleport_world(dest);
      turn_not_consumed = 0;
      break;    
    case 'm':
      
    case 'q':
      /* Demonstrate use of the message queue.  You can use this for *
       * printf()-style debugging (though gdb is probably a better   *
       * option.  Not that it matters, but using this command will   *
       * waste a turn.  Set turn_not_consumed to 1 and you should be *
       * able to figure out why I did it that way.                   */
      io_queue_message("This is the first message.");
      io_queue_message("Since there are multiple messages, "
                       "you will see \"more\" prompts.");
      io_queue_message("You can use any key to advance through messages.");
      io_queue_message("Normal gameplay will not resume until the queue "
                       "is empty.");
      io_queue_message("Long lines will be truncated, not wrapped.");
      io_queue_message("io_queue_message() is variadic and handles "
                       "all printf() conversion specifiers.");
      io_queue_message("Did you see %s?", "what I did there");
      io_queue_message("When the last message is displayed, there will "
                       "be no \"more\" prompt.");
      io_queue_message("Have fun!  And happy printing!");
      io_queue_message("Oh!  And use 'Q' to quit!");

      dest[dim_y] = world.pc->pos[dim_y];
      dest[dim_x] = world.pc->pos[dim_x];
      turn_not_consumed = 0;
      break;
    default:
      /* Also not in the spec.  It's not always easy to figure out what *
       * key code corresponds with a given keystroke.  Print out any    *
       * unhandled key here.  Not only does it give a visual error      *
       * indicator, but it also gives an integer value that can be used *
       * for that key in this (or other) switch statements.  Printed in *
       * octal, with the leading zero, because ncurses.h lists codes in *
       * octal, thus allowing us to do reverse lookups.  If a key has a *
       * name defined in the header, you can use the name here, else    *
       * you can directly use the octal value.                          */
      mvprintw(0, 0, "Unbound key: %#o ", key);
      turn_not_consumed = 1;
    }
    refresh();
  } while (turn_not_consumed);
}