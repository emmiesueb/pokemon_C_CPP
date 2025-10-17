#include <iostream>
#include <direct.h>
#include <iomanip>
#include <limits.h>
#include <string>
#include <cmath>

#include "pokedex.h"

using namespace std;

/**
 * Here are series of class functions that are being defined.
 * Each class has a constructor and a formatting function, the
 * Deconstructor is only if the object has a member that requires
 * memory allocation.
*/
/* Pokemon : */
Pokemon::Pokemon(int pid, char *identity, int spid, int h, int w, int base_ex, int o, bool def) : pokemon_id(pid), species_id(spid), height(h), weight(w), base_experience(base_ex), order(o), is_default(def) {
            identifier = (char *) malloc(strlen(identity) + 1);
            strcpy(identifier, identity);
        }

Pokemon::~Pokemon() {
    free(identifier);
}

/* Experience : */
Experience::Experience(int g, int l, int ex) : growth_rate_id(g), level(l), experience(ex) {}


/* Stats : */
Stats::Stats(int sid, int dcid, char *identity, bool bonly, int gi) : stat_id(sid), damage_class_id(dcid), is_battle_only(bonly), game_index(gi) {
    identifier = (char *) malloc(strlen(identity) + 1);
    strcpy(identifier, identity);
}


Stats::~Stats() {
    free(identifier);
}

/* Type_Names : */ 
Type_Names::Type_Names(int tid, char *n) : type_id(tid) {
    name = (char *) malloc(strlen(n) + 1);
    strcpy(name, n);
}


Type_Names::~Type_Names() {
    free(name);
}

/* Moves : */
Moves::Moves(int mid, char *identity, int gid, int tid, int po, int p, int acc, int pr, int trid, int8_t dcid,
            int effid, int effch, int ctid, int ceffid, int sceffid) : move_id(mid), generation_id(gid), type_id(tid),
            power(po), pp(p), accuracy(acc), priority(pr), target_id(trid), damage_class_id(dcid), effect_id(effid), effect_chance(effch),
            contest_type_id(ctid), contest_effect_id(ceffid), super_contest_effect_id(sceffid) {
                identifier = (char *) malloc(strlen(identity) + 1);
                strcpy(identifier, identity);
            }


Moves::~Moves() {
    free(identifier);
}


/* Pokemon_Types : */
Pokemon_Types::Pokemon_Types(int pid, int tid, int s) : pokemon_id(pid), type_id(tid), slot(s) {}


/* Pokemon_Moves : */
Pokemon_Moves::Pokemon_Moves(int pid, int vgid, int mid, int pmmid, int l, int o) : pokemon_id(pid), version_group_id(vgid), move_id(mid), 
        pokemon_move_method_id(pmmid), level(l), order(o) {}


/* Pokemon_Stats : */
Pokemon_Stats::Pokemon_Stats(int pid, int sid, int base_s, int eff) : pokemon_id(pid), stat_id(sid), base_stat(base_s), effort(eff) {}


/* Pokemon_Species : */
Pokemon_Species::Pokemon_Species(int pid, char* identity, int g, int e_from, int e_chain, int color, int shape, int habitat, int gender,
            int capture, int base_h, bool baby, int hatch, bool gender_diff, int gid, int forms, bool legend, bool mythical, int conquest) :
            pokemon_id(pid), generation(g), evolves_from_species_id(e_from), evolution_chain_id(e_chain), color_id(color),
            shape_id(shape), habitat_id(habitat), gender_rate(gender), capture_rate(capture), base_happiness(base_h), is_baby(baby),
            hatch_counter(hatch), has_gender_differences(gender_diff), growth_rate_id(gid), forms_switchable(forms), is_legendary(legend),
            is_mythical(mythical), conquest_order(conquest) {
                identifier = (char *) malloc(strlen(identity) + 1);
                strcpy(identifier, identity);
            }


Pokemon_Species::~Pokemon_Species() {
    free(identifier);
}

const char *path = "/Assets/";

std::vector<std::unique_ptr<Pokemon>> pokemon;
std::vector<std::unique_ptr<Type_Names>> type_names;
std::vector<std::unique_ptr<Experience>> experience;
std::vector<std::unique_ptr<Stats>> stats;
std::vector<std::unique_ptr<Pokemon_Species>> pokemon_species;
std::vector<std::unique_ptr<Pokemon_Moves>> pokemon_moves;
std::vector<std::unique_ptr<Pokemon_Stats>> pokemon_stats;
std::vector<std::unique_ptr<Pokemon_Types>> pokemon_types;
std::vector<std::unique_ptr<Moves>> moves;

const char *files[9] = {
    "pokemon", 
    "moves", 
    "type_names", 
    "experience", 
    "stats", 
    "pokemon_species", 
    "pokemon_types", 
    "pokemon_moves", 
    "pokemon_stats"
};

/**
 * This function will take a buffer from a row in a CSV file and
 * separate it by the values between each comma, returning it for
 * use in other functions.
*/
char **tokenize(char *line) {
    if (line == NULL) {
        std::cerr << "buffer is empty, exiting..." << endl;
        exit(1);
    }
    if (sizeof(line) <= 1) {
        return NULL;
    }
    char **words = (char **) malloc(sizeof(char *) * 256);
    // malloc error:
    if (words == NULL) {
        std::cerr << "malloc error" << endl;
        free(line);
        return NULL;
    }

    int i, j, k;
    i = j = k = 0;
    // strtok will not be able to be used, need to use getc to check the field
    // if the token between commas is of size 0, then it needs to be represented by INT_MAX
    do {
        while (line[j] != ',' && line[j] != '\n') {
            ++k;
            ++j;
        }

        if (k == 0) {
            words[i] = (char *) malloc(sizeof(char) * 5);
            if (words[i] == NULL) {
                std::cerr << "malloc error" << endl;
                free(line);
                for (int l = 0; l < i; ++l)
                    free(words[l]);
                free(words);
                return NULL;
            }
            strcpy(words[i], "null");
            words[i][4] = '\0';
        }
        else {
            words[i] = (char *) malloc(sizeof(char) * (k + 1));
            // malloc error:
            if (*(words + i) == NULL) {
                std::cerr << "malloc error" << endl;
                free(line);
                for (int l = 0; l < i; ++l)
                    free(words[l]);
                free(words);
                return NULL;
            }
            strncpy(words[i], line + (j - k), k);
            words[i][k] = '\0';
        }
        ++i;
        k = 0;
        ++j;
    }   while (line[j] != '\0');

    return words;
}

/**
 * This goes through a line of a file until a newline character
 * is reached, returning the buffer.
*/
void parse_CSV(FILE *f, char *buff, int size) {
    if (buff == NULL) {
        std::cerr << "buffer is not allocated" << endl;
        return;
    }

    int c, i;
    i = 0;
    // keep going until newline or end of file:
    while ((c = fgetc(f)) != EOF) {
        if (c == '\n') {
            buff[i++] = c;
            buff[i] = '\0';
            return;
        }
        else {
            buff[i++] = c;
        }

        if (i == size) {
            size *= 2;
            buff = (char *) realloc(buff, sizeof(char) * size);
            // reallocation error:
            if (buff == NULL) {
                std::cerr << "realloc error";
                free(buff);
                fclose(f);
                exit(1);
            }
        }
    }
}

// pokemon.csv
static void pokemon_CSV(FILE *f) {
    Pokemon *p;
    char *buff = (char *) malloc(sizeof(char) * 78);
    if (buff == NULL) {
        std::cerr << "malloc error" << endl;
        fclose(f);
        exit(1);
    }
    char **words;
    int i;
    // skip first line:
    if (fgets(buff, 78, f) == NULL) {
        std::cerr << "fgets error" << endl;
        free(buff);
        fclose(f);
        exit(1);
    }
    free(buff);

    // 3, 6, 7, 19
    // evolves_from, shape_id, habitat_id, conquest_order have chance of being NULL
    while (!feof(f)) {
        buff = (char *) malloc(sizeof(char) * 64);
        if (buff == NULL) {
            std::cerr << "malloc error" << endl;
            fclose(f);
            exit(1);
        }
        parse_CSV(f, buff, 64);
        if (feof(f)) {
            free(buff);
            return;
        }
        
        if ((words = tokenize(buff)) != NULL) {
            p = new Pokemon(atoi(words[0]), words[1], atoi(words[2]), atoi(words[3]),
                    atoi(words[4]), atoi(words[5]), atoi(words[6]), (bool) atoi(words[7]));
            pokemon.push_back(std::unique_ptr<Pokemon>(p));
            for (i = 0; i < 8; ++i)
                free(words[i]);
            free(words);
        }
        free(buff);
    }
}

// moves.csv
static void moves_CSV(FILE *f) {
    Moves *m;
    char *buff = (char *) malloc(sizeof(char) * 176);
    if (buff == NULL) {
        perror("moves buffer malloc error");
        fclose(f);
        exit(1);
    }
    char **words;
    int i, po, p, acc, effch, ctid, ceffid, sceffid;
    po = p = acc = effch = ctid = ceffid = sceffid = 0;
    // get headers:
    if (fgets(buff, 176, f) == NULL) {
        std::cerr << "fgets error" << endl;
        fclose(f);
        free(buff);
        exit(1);
    }
    free(buff);

    while (1) {
        // power, pp, accuracy, effect chance, and the contest ids have a chance of being NULL, set as INT_MAX if so:
        buff = (char *) malloc(sizeof(char) * 64);
        if (buff == NULL) {
            std::cerr << "malloc error" << endl;
            fclose(f);
            exit(1);
        }
        parse_CSV(f, buff, 64);
        if (feof(f)) {
            free(buff);
            return;
        }

        if ((words = tokenize(buff)) != NULL) {
            // possible null values:
            po = strcmp(words[4], "null") == 0 ? INT_MAX : atoi(words[4]);
            p = strcmp(words[5], "null") == 0 ? INT_MAX : atoi(words[5]);
            acc = strcmp(words[6], "null") == 0 ? INT_MAX : atoi(words[6]);
            effch = strcmp(words[11], "null") == 0 ? INT_MAX : atoi(words[11]);
            ctid = strcmp(words[12], "null") == 0 ? INT_MAX : atoi(words[12]);
            ceffid = strcmp(words[13], "null") == 0 ? INT_MAX : atoi(words[13]);
            sceffid = strcmp(words[14], "null") == 0 ? INT_MAX : atoi(words[14]);

            m = new Moves(atoi(words[0]), words[1], atoi(words[2]), atoi(words[3]), po, 
                    p, acc, atoi(words[7]), atoi(words[8]), (int8_t) atoi(words[9]), 
                    atoi(words[10]), effch, ctid, ceffid, sceffid);
            moves.push_back(std::unique_ptr<Moves>(m));
            for (i = 0; i < 3; i++)
                free(words[i]);
            free(words);
        }
        free(buff);
    }
}

// type_names.csv
static void type_names_CSV(FILE *f) {
    Type_Names *t;
    char *buff = (char *) malloc(sizeof(char) * 36);
    if (buff == NULL) {
        std::cerr << "malloc error" << endl;
        fclose(f);
        exit(1);
    }
    char **words;
    int i;
    // skip first line, those are just headers:
    if (fgets(buff, 36, f) == NULL) {
        std::cerr << "fgets error" << endl;
        free(buff);
        fclose(f);
        exit(1);
    }
    free(buff);

    while (1) {
        buff = (char *) malloc(sizeof(char) * 30);
        if (buff == NULL) {
            std::cerr << "malloc error" << endl;
            fclose(f);
            exit(1);
        }
        parse_CSV(f, buff, 30);
        if (feof(f)) {
            free(buff);
            return;
        }

        // check if language id is 9, which means it is in english:
        if ((words = tokenize(buff)) != NULL && atoi(words[1]) == 9) {
            t = new Type_Names(atoi(words[0]), words[2]);
            type_names.push_back(std::unique_ptr<Type_Names>(t));
            for (i = 0; i < 3; i++)
                free(words[i]);
            free(words);
        }
        free(buff);
    }
}

// experience.csv
static void experience_CSV(FILE *f) {
    Experience *e;
    char *buff = (char *) malloc(sizeof(char) * 40);
    if (buff == NULL) {
        std::cerr << "malloc error" << endl;
        fclose(f);
        exit(1);
    }
    char **words;
    int i;
    // skip first line, those are just headers:
    if (fgets(buff, 40, f) == NULL) {
        std::cerr << "fgets error" << endl;
        free(buff);
        fclose(f);
        exit(1);
    }
    free(buff);

    while (1) {
        buff = (char *) malloc(sizeof(char) * 16);
        if (buff == NULL) {
            std::cerr << "malloc error" << endl;
            fclose(f);
            exit(1);
        }
        parse_CSV(f, buff, 16);
        if (feof(f)) {
            free(buff);
            return;
        }

        if ((words = tokenize(buff)) != NULL) {
            e = new Experience(atoi(words[0]), atoi(words[1]), atoi(words[2]));
            experience.push_back(std::unique_ptr<Experience>(e));
            for (i = 0; i < 3; ++i)
                free(words[i]);
            free(words);
        }
        free(buff);
    }
}

// stats.csv
static void stats_CSV(FILE *f) {
    Stats *s;
    char *buff = (char *) malloc(sizeof(char) * 60);
    if (buff == NULL) {
        std::cerr << "stats buff malloc error" << endl;
        fclose(f);
        exit(1);
    }
    char **words;
    int i, dcid, gi;
    dcid = gi = 0;
    // skip first line:
    if (fgets(buff, 60, f) == NULL) {
        std::cerr << "fgets error, couldn't read headers" << endl;
        fclose(f);
        free(buff);
        exit(1);
    }
    free(buff);

    while (1) {
        buff = (char *) malloc(sizeof(char) * 25);
        if (buff == NULL) {
            std::cerr << "stats loop buff malloc error" << endl;
            fclose(f);
            exit(1);
        }
        parse_CSV(f, buff, 25);
        if (feof(f)) {
            free(buff);
            break;
        }

        if ((words = tokenize(buff)) != NULL) {
            dcid = ((strcmp(words[1], "null") == 0) ? INT_MAX : atoi(words[1]));
            gi = ((strcmp(words[4], "null") == 0) ? INT_MAX : atoi(words[4]));
            s = new Stats(atoi(words[0]), dcid, words[2], (bool) atoi(words[3]), gi);
            stats.push_back(std::unique_ptr<Stats>(s));
            for (i = 0; i < 5; ++i)
                free(words[i]);
            free(words);
        }
        free(buff);
    }
}

// pokemon_species.csv
static void pokemon_species_CSV(FILE *f) {
    Pokemon_Species *ps;
    char *buff = (char *) malloc(sizeof(char) * 265);
    if (buff == NULL) {
        std::cerr << "malloc error" << endl;
        fclose(f);
        exit(1);
    }
    char **words;
    int i, e_from, shape, hid, conquest;
    e_from = shape = hid = conquest = 0;
    // skip first line:
    if (fgets(buff, 265, f) == NULL) {
        std::cerr << "fgets error" << endl;
        free(buff);
        fclose(f);
        exit(1);
    }
    free(buff);

    while (1) {
        buff = (char *) malloc(sizeof(char) * 64);
        if (buff == NULL) {
            std::cerr << "malloc error" << endl;
            fclose(f);
            exit(1);
        }
        parse_CSV(f, buff, 64);
        if (feof(f)) {
            free(buff);
            break;
        }

        // check potentially null values:
        if ((words = tokenize(buff)) != NULL) {
            e_from = ((strcmp(words[3], "null") == 0) ? INT_MAX : atoi(words[3]));
            shape = ((strcmp(words[6], "null") == 0) ? INT_MAX : atoi(words[6]));
            hid = ((strcmp(words[7], "null") == 0) ? INT_MAX : atoi(words[7]));
            conquest = ((strcmp(words[19], "null") == 0) ? INT_MAX : atoi(words[18]));
            
            // create and add to array:
            ps = new Pokemon_Species(atoi(words[0]), words[1], atoi(words[2]), e_from, atoi(words[4]),
                    atoi(words[5]), shape, hid, atoi(words[8]), atoi(words[9]), atoi(words[10]),
                    (bool) atoi(words[11]), atoi(words[12]), (bool) atoi(words[13]), atoi(words[14]), atoi(words[16]),
                    (bool) atoi(words[17]), (bool) atoi(words[18]), conquest);
            pokemon_species.push_back(std::unique_ptr<Pokemon_Species>(ps));
            for (i = 0; i < 20; ++i)
                free(words[i]);
            free(words);
        }
        free(buff);
    }
}

// pokemon_types.csv
static void pokemon_types_CSV(FILE *f) {
    Pokemon_Types *pt;
    char *buff = (char *) malloc(sizeof(char) * 32);
    if (buff == NULL) {
        std::cerr << "malloc error" << endl;
        fclose(f);
        exit(1);
    }
    char **words;
    int i;
    // skip first line:
    if(fgets(buff, 32, f) == NULL) {
        std::cerr << "fgets error" << endl;
        free(buff);
        fclose(f);
        exit(1);
    }
    free(buff);

    while (1) {
        buff = (char *) malloc(sizeof(char) * 16);
        if (buff == NULL) {
            std::cerr << "malloc error" << endl;
            fclose(f);
            exit(1);
        }
        parse_CSV(f, buff, 16);
        if (feof(f)) {
            free(buff);
            break;
        }
        words = tokenize(buff);
        
        if ((words = tokenize(buff)) != NULL) {
            pt = new Pokemon_Types(atoi(words[0]), atoi(words[1]), atoi(words[2]));
            pokemon_types.push_back(std::unique_ptr<Pokemon_Types>(pt));
        }

        for (i = 0; i < 3; ++i)
            free(words[i]);
        free(buff);
    }
}

// pokemon_moves.csv
static void pokemon_moves_CSV(FILE *f) {
    Pokemon_Moves *pm;
    char *buff = (char *) malloc(sizeof(char) * 72);
    if (buff == NULL) {
        std::cerr << "malloc error" << endl;
        fclose(f); 
        exit(1);
    }
    char **words;
    int i, o;
    o = 0;
    // skip first line:
    if (fgets(buff, 72, f) == NULL) {
        std::cerr << "fgets error" << endl;
        fclose(f);
        free(buff);
        exit(1);
    }
    free(buff);

    while (1) {
        // order has a chance of being empty
        buff = (char *) malloc(sizeof(char) * 20);
        if (buff == NULL) {
            std::cerr << "malloc error" << endl;
            fclose(f);
            exit(1);
        }
        parse_CSV(f, buff, 20);
        if (feof(f)) {
            free(buff);
            break;
        }

        if ((words = tokenize(buff)) != NULL) {
            o = ((strcmp(words[5], "null")) ? INT_MAX : atoi(words[5]));
            pm = new Pokemon_Moves(atoi(words[0]), atoi(words[1]), atoi(words[2]), atoi(words[3]), atoi(words[4]), o);
            pokemon_moves.push_back(std::unique_ptr<Pokemon_Moves>(pm));
            for (i = 0; words[i] != NULL; ++i)
                free(words[i]);
        }
        free(buff);
    }
}

// pokemon_stats.csv
static void pokemon_stats_CSV(FILE *f) {
    Pokemon_Stats *ps;
    char *buff = (char *) malloc(sizeof(char) * 40);
    if (buff == NULL) {
        std::cerr << "malloc error" << endl;
        fclose(f);
        exit(1);
    }
    char **words;
    int i = 0;
    // print headers for first line:
    if (fgets(buff, 40, f) == NULL) {
        std::cerr << "fgets error" << endl;
        free(buff);
        fclose(f);
        exit(1);
    }
    free(buff);

    while (1) {
        buff = (char *) malloc(sizeof(char) * 16);
        if (buff == NULL) {
            std::cerr << "malloc error" << endl;
            fclose(f);
            exit(1);
        }
        parse_CSV(f, buff, 16);
        if (feof(f)) {
            free(buff);
            return;
        }

        if ((words = tokenize(buff)) != NULL) {
            ps = new Pokemon_Stats(atoi(words[0]), atoi(words[1]), atoi(words[2]), atoi(words[3]));
            pokemon_stats.push_back(std::unique_ptr<Pokemon_Stats>(ps));
            for (i = 0; words[i] != NULL; ++i)
                free(words[i]);
        }
        free(buff);
    }
}


// parse through the CSV file, store it in an array, and print the data:
static void (*csv_func[9])(FILE *) = {
    pokemon_CSV,
    moves_CSV,
    type_names_CSV,
    experience_CSV,
    stats_CSV,
    pokemon_species_CSV,
    pokemon_types_CSV,
    pokemon_moves_CSV,
    pokemon_stats_CSV,
};


// open CSV files:
void open_CSV(int type) {
    char buffer[1024];
    FILE *f = NULL;

    // get current working directory:
    if (_getcwd(buffer, 1024) != NULL) {
        const char *CWD = buffer;
        int size = strlen(files[type]) + strlen(path) + strlen(CWD);
        char f1[size];
        sprintf(f1, "%s%s%s.csv", CWD, path, files[type]);
        f = fopen(f1, "rb");
        // file failed to open, tell:
        if (f == NULL) {
            std::cerr << "File " << f1 << " does not exist. Exiting program.";
            exit(1);
        }
    }
    else
        std::cerr << "Could not get current working directory.";

    // parse through CSV file:
    csv_func[type](f);
    fclose(f);
}


Party_Member *generate_pkmn(int min, int max) {
    Party_Member *p = new Party_Member;
    p->pid = rand() % 898 + 1;
    p->level = rand() % (max - min) + min;
    p->is_shiny = (rand() % 8192) == 0 ? 1 : 0;
    p->base_experience = pokemon[p->pid - 1].get()->base_experience;
    p->defeated = 0;
    p->participate = 0;
    p->status = 0;
    p->status_turns = 0;

    /* Pokemon Species: */
    p->species = pokemon_species[p->pid - 1].get();
    p->gender = (p->species->gender_rate == -1) ? 'N' : 
            ((p->species->gender_rate == 0) ? 'M' : 
            ((p->species->gender_rate == 8) ? 'F' : 
            ((rand() % 2) == 0 ? 'F' : 'M')));

    /* Base stats & battle stats: */
    p->stats[0] = pokemon_stats[(p->pid - 1) * 6].get()->base_stat; // hp
    p->stats[1] = pokemon_stats[(p->pid - 1) * 6 + 1].get()->base_stat; // attack
    p->stats[2] = pokemon_stats[(p->pid - 1) * 6 + 2].get()->base_stat; // defense
    p->stats[3] = pokemon_stats[(p->pid - 1) * 6 + 3].get()->base_stat; // special-attack
    p->stats[4] = pokemon_stats[(p->pid - 1) * 6 + 4].get()->base_stat; // special-defense
    p->stats[5] = pokemon_stats[(p->pid - 1) * 6 + 5].get()->base_stat; // speed
    p->battle_stats[0] = p->stats[0];
    p->battle_stats[1] = p->stats[1];
    p->battle_stats[2] = p->stats[2];
    p->battle_stats[3] = p->stats[3];
    p->battle_stats[4] = p->stats[4];
    p->battle_stats[5] = p->stats[5];
    p->battle_stats[6] = 100;
    p->battle_stats[7] = 100;

    /* IVs: */
    p->ivs[0] = rand() % 16;
    p->ivs[1] = rand() % 16;
    p->ivs[2] = rand() % 16;
    p->ivs[3] = rand() % 16;
    p->ivs[4] = rand() % 16;
    p->ivs[5] = rand() % 16;

    /* Value of stats: */
    p->full_stats[0] = p->maxHP = floor(((((p->stats[0] + p->ivs[0]) * 2) * p->level)/ 100) + p->level + 10);
    p->full_stats[1] = floor(((((p->stats[1] + p->ivs[1]) * 2) * p->level) / 100) + 5);
    p->full_stats[2] = floor(((((p->stats[2] + p->ivs[2]) * 2) * p->level) / 100) + 5);
    p->full_stats[3] = floor(((((p->stats[3] + p->ivs[3]) * 2) * p->level) / 100) + 5);
    p->full_stats[4] = floor(((((p->stats[4] + p->ivs[4]) * 2) * p->level) / 100) + 5);
    p->full_stats[5] = floor(((((p->stats[5] + p->ivs[5]) * 2) * p->level) / 100) + 5);

    /* Experience based on level: */
    p->experience = experience[((p->species->growth_rate_id - 1) * 100) + p->level - 1].get()->experience;

    /* Moves, which will be up to 2: */
    unsigned long i;
    int j;
    j = 0;
    for (i = 0; i < pokemon_moves.size(); ++i) {
        if (p->pid == pokemon_moves[i].get()->pokemon_id
            && pokemon_moves[i].get()->pokemon_move_method_id == 1
            && pokemon_moves[i].get()->level <= p->level
            && ((j == 1 && strcmp(moves[pokemon_moves[i].get()->move_id].get()->identifier, p->moves[0]->identifier) != 0)
                || j == 0))
            p->moves[j++] = moves[pokemon_moves[i].get()->move_id].get();

        if (pokemon_moves[i].get()->pokemon_id > p->pid || j >= 2)
            break;
    }
    // assign struggle as its only move:
    if (j == 0) {
        p->moves[0] = moves[164].get();
        p->moves[1] = NULL;
    }
    p->moves[2] = p->moves[3] = NULL;

    // gather pokemon type:
    j = 0;
    for (i = 0; i < pokemon_types.size(); ++i) {
        if (p->pid == pokemon_types[i].get()->pokemon_id)
            p->type[j++] = pokemon_types[i].get();
    }
    if (j == 1)
        p->type[1] = NULL;
    return p;
}