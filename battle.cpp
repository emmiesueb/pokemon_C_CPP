#include "battle.h"
#include <cmath>
#include <limits.h>

/* Status Effect ID Macros */
#define POISONED 67
#define BADLYPOISONED 3
#define ASLEEP 2
#define PARALYZED 68
#define FROZEN 6
#define BURNED 5
#define WOUND 43
#define CONFUSED 77

using namespace std;

/**
 * The damage function has since been improved to consider
 * just the power of the move and if it is a physical
 * or special move. Special moves will instead calculate
 * damage based on the attacker's special attack and the
 * defender's special defense. This also allows for
 * calculation of 
*/
int damage(int power, Party_Member *p, Party_Member *foe, float crit, float stab, float type, bool is_physical) {
    /* Special move, does no damage: */
    if (power == INT_MAX || power == 0)
        return 0;

    if (is_physical)
        return (int) ceil(((((((float) ((2 * p->level) / 5.0)) + 2) 
        * power 
        * ((float) (p->full_stats[1] / foe->full_stats[2])) / 50.0)) + 2) 
        * crit 
        * ((float) ((rand() % 16 + 85) / 100.0)) * stab * type);
    else
        return (int) ceil(((((((float) ((2 * p->level) / 5.0)) + 2) * power * ((float) (p->full_stats[3] / foe->full_stats[4])) / 50.0)) + 2) * crit * ((float) ((rand() % 16 + 85) / 100.0)) * stab * type);
}

int escape_chance(Party_Member *self, Party_Member *foe, int attempts) {
  return ((self->battle_stats[5] * 32) / (((int) floor(foe->battle_stats[5] / 4)) % 256)) + 30 * attempts;
}

/**
 * This calculates the experience gained from battle
 * for the PC's pokemon. 
*/
int exp_gain(Party_Member *self, float wild_or_trainer, Party_Member *foe, int num_participate) {
  return (int) ceil((float) ((self->base_experience * foe->level) / (7 * num_participate)) * wild_or_trainer);
}

/**
 * This calculates the pokemon dollars gaines from
 * battle for the PC if they win, this is only
 * applicable for trainer battles.
*/
int money_gain(Npc *n) {
  int j = 0;
  for (int i = 0; i < n->num_in_party; ++i) {
    if (n->party[j]->level < n->party[i]->level)
      j = i;
  }
  return n->party[j]->level * n->base_money;
}


float type_effect(float type1, float type2) {
    float type = 0.0;
    
    // super effective against one, no effect or not effective against the other:
    if ((type1 == 0.0 && type1 == 2.0)
        || (type1 == 2.0 && type1 == 0.0)
        || (type1 == 0.5 && type2 == 2.0)
        || (type1 == 2.0 && type2 == 0.5))
        type = 1.0;
    // super effective against one, regular effective against the other:
    else if ((type1 == 1.0 && type2 == 2.0)
        || (type1 == 2.0 && type2 == 1.0))
        type = 2.0;
    // super effective against both:
    else if (type1 == 2.0 && type2 == 2.0)
        type = 4.0;
    // no effect against either:
    else if ((type1 == 0.0 && type2 == 0.0)
        || (type1 == 0.0 && type2 == 1.0)
        || (type1 == 1.0 && type2 == 0.0))
        type = 0.0;
    // not very effective against one or both:
    else if ((type1 == 0.5 && type2 == 0.5)
        || (type1 == 1.0 && type2 == 0.5)
        || (type1 == 0.5 && type2 == 1.0))
        type = 0.5;
    // regular effect on both:
    else
        type = 1.0;

    return type;
}


/**
 * This will determine what happens to the pokemon
 * based on the status effect. This function also
 * returns the effect ID of the move if it
 * successfully inflicts that status effect
 * on the target pokemon. Otherwise, this
 * will return 0 because there will be no
 * status effect.
*/
int status_effect(Moves *m, Party_Member *p) {
    switch(m->effect_id) {
    case POISONED:
    case BADLYPOISONED:
    case ASLEEP:
    case FROZEN:
    case BURNED:
    case WOUND:
    case CONFUSED:
    case PARALYZED:
        if (m->effect_chance == INT_MAX || rand() % 100 <= m->effect_chance) {
            p->status = m->effect_id;
            return m->effect_id;
        }
        break;
    default:
        break;
    }
    return 0;
}


/**
 * This will determine how the effect will
 * affect the pokemon. This considers if it
 * is after the turn or during, which is
 * an important determinating factor for
 * some status conditions.
 * This will return specific values based on
 * what the status of the pokemon is.
 * A 1 will be returned in the case that the
 * pokemon is confused and did not hit itself,
 * but it still remains confused.
 * A 100 will be returned if the pokemon faints
 * due to damage inflicted by the condition.
*/
int effect_result(Party_Member *p, bool after_turn) {
    switch(p->status) {
        // takes the same amount of damage from burned, poisoned, or wound conditions:
        case WOUND:
            if (after_turn)
                p->status_turns++;
            if (p->status_turns == 4 && after_turn) {
                p->status_turns = 0;
                return 0;
            }
        case BURNED:
        case POISONED:
            if (after_turn)
                p->full_stats[0] = std::max(0, (int) floor((float) p->full_stats[0] - (p->maxHP / 16.0)));
            if (p->full_stats[0] == 0) {
                p->status_turns = 0;
                p->defeated = 1;
                return 100;
            }
            return p->status;
            break;
        case BADLYPOISONED:
            // damage increases per turn:
            if (after_turn) {
                p->full_stats[0] = std::max(0, (int) floor((float) p->full_stats[0] - (p->maxHP / 16.0) * p->status_turns));
                if (p->full_stats[0] == 0) {
                    p->status_turns = 0;
                    p->defeated = 1;
                    return 100;
                }
                p->status_turns++;
            }
            return p->status;
        case PARALYZED:
        case ASLEEP:
            // pokemon can wake up or no longer be paralyzed:
            if (after_turn)
                p->status_turns++;
            if ((rand() % 100 <= 33 && !after_turn)
                || (p->status_turns == 3 && after_turn)) {
                p->status_turns = 0;
                return 0;
            }
            return p->status;
        case CONFUSED:
            // pokemon has chance of hitting self, which is equal to a physical move with a power of 40:
            if (rand() % 100 <= 50 && !after_turn) {
                p->full_stats[0] = std::max(0, p->full_stats[0] - damage(40, p, p, 1.0, 1.0, 1.0, 1));
                if (p->full_stats[0] == 0) {
                    p->defeated = 1;
                    return 100;
                }
            }
            // didn't hit itself
            else if (!after_turn)
                return 1;
            // pokemon snaps out of confusion!
            if (rand() % 100 <= 33) {
                p->status_turns = 0;
                return 0;
            }
            return p->status;
        default:
            break;
    }
    return 0;
}