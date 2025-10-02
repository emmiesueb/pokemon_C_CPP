#ifndef POKEDEX_H
#define POKEDEX_H

#include <stdbool.h>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <cstring>
#include <memory>
#include <string>

// classes that reflect data in CSV files:
// Generic classes:
class Pokemon {
    public:
        int pokemon_id;
        char *identifier;
        int species_id;
        int height;
        int weight;
        int base_experience;
        int order;
        bool is_default;
        Pokemon(int pid, char *identity, int spid, int h, int w, int base_ex, int o, bool def);

        ~Pokemon();
};


class Experience {
    public:
        int growth_rate_id;
        int level;
        int experience;
        Experience(int g, int l, int ex);
};


class Stats {
    public:
        int stat_id;
        int damage_class_id;
        char *identifier;
        bool is_battle_only;
        int game_index;
        Stats(int sid, int dcid, char *identity, bool bonly, int gi);

        ~Stats();
};


class Type_Names {
    public:
        int type_id;
        char *name;
        Type_Names(int tid, char *n);

        ~Type_Names();
};


class Moves {
    public:
        int move_id;
        char *identifier;
        int generation_id;
        int type_id;
        int power;
        int pp;
        int accuracy;
        int priority;
        int target_id;
        int8_t damage_class_id;
        int effect_id;
        int effect_chance;
        int contest_type_id;
        int contest_effect_id;
        int super_contest_effect_id;
        Moves(int mid, char *identity, int gid, int tid, int po, int p, int acc, int pr, int trid, int8_t dcid,
            int effid, int effch, int ctid, int ceffid, int sceffid);

        ~Moves();
};

// Pokemon-specific classes:
class Pokemon_Types {
    public:
        int pokemon_id;
        int type_id;
        int slot;
        Pokemon_Types(int pid, int tid, int s);
};


class Pokemon_Moves {
    public:
        int pokemon_id;
        int version_group_id;
        int move_id;
        int pokemon_move_method_id;
        int level;
        int order;
        Pokemon_Moves(int pid, int vgid, int mid, int pmmid, int l, int o);
};


class Pokemon_Stats {
    public:
        int pokemon_id;
        int stat_id;
        int base_stat;
        int effort;
        Pokemon_Stats(int pid, int sid, int base_s, int eff);
};


class Pokemon_Species {
    public:
        int pokemon_id;
        char *identifier;
        int generation;
        int evolves_from_species_id;
        int evolution_chain_id;
        int color_id;
        int shape_id;
        int habitat_id;
        int gender_rate;
        int capture_rate;
        int base_happiness;
        bool is_baby;
        int hatch_counter;
        bool has_gender_differences;
        int growth_rate_id;
        int forms_switchable;
        bool is_legendary;
        bool is_mythical;
        int conquest_order;
        Pokemon_Species(int pid, char* identity, int g, int e_from, int e_chain, int color, int shape, int habitat, int gender,
            int capture, int base_h, bool baby, int hatch, bool gender_diff, int gid, int forms, bool legend, bool mythical, int conquest);

        ~Pokemon_Species();
};

class Party_Member {
    public:
        int pid; // makes it easy to find this pokemon in some vectors
        int base_experience; // base exp gain during battle
        Pokemon_Species *species;
        Pokemon_Types *type[2];
        char gender; // 'm' 'f' or 'n' for no gender
        int experience; // current number of exp points
        int level; // current level
        class Moves *moves[4]; // moves this Party Member has learned, can only be up to 4
        int maxHP;
        int stats[6]; // consists of modifiers to base stats, only comprised of 6 stats
        int ivs[6];
        int battle_stats[8]; // stats for battle usage, can be changed during battle and includes accuracy and evasion
        int full_stats[6]; // calculated stats
        bool is_shiny;
        bool defeated; // for use later
        bool participate; // for tracking exp gain
        int status; // for determining pokemon status
        int8_t status_turns; // for turn-based statuses.
};

class Items {
    public:
        int num;
        std::string identifier;
};

// array containing CSV filenames:
extern const char *files[9];

extern std::vector<std::unique_ptr<Pokemon>> pokemon;
extern std::vector<std::unique_ptr<Type_Names>> type_names;
extern std::vector<std::unique_ptr<Experience>> experience;
extern std::vector<std::unique_ptr<Stats>> stats;
extern std::vector<std::unique_ptr<Pokemon_Species>> pokemon_species;
extern std::vector<std::unique_ptr<Pokemon_Moves>> pokemon_moves;
extern std::vector<std::unique_ptr<Pokemon_Stats>> pokemon_stats;
extern std::vector<std::unique_ptr<Pokemon_Types>> pokemon_types;
extern std::vector<std::unique_ptr<Moves>> moves;

extern Party_Member *generate_pkmn(int, int);
extern void open_CSV(int);
#endif