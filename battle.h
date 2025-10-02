#include <cmath>
#include <limits.h>

#include "pokedex.h"
#include "character.h"

extern float type_matchups[18][18];

int damage(int, Party_Member *, Party_Member *, float, float, float, bool);
int escape_chance(Party_Member *, Party_Member *, int);
int exp_gain(Party_Member *, float, Party_Member *, int);
int money_gain(Npc *);
int status_effect(Moves *, Party_Member *);
float type_effect(float, float);
int effect_result(Party_Member *, bool);