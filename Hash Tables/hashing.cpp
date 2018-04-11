#include <iostream>
#include <ctime>
#include <chrono>
#include <climits>
#include <cmath>

using namespace std;

int pow_universe = 32; // u
uint32_t universe = ULONG_MAX; // the size of universe (2^32 - 1);
int pow_table_size = 20; // k
int hash_table_size = 1 << pow_table_size; // m - hash table size
uint32_t *hash_table;
bool* cell_free; // array for info if cell is occupied
int occupied_cells = 0;
double elapsed_time;
int steps;
int parts = 4; // c
int bits_per_part = pow_universe / parts;
int tab_table_size = 1 << bits_per_part;
int hash_function_choice;


// part with borrowed code - begin

static long rng_state[2];

static long rng_rotl(const long x, int k) {
    return (x << k) | (x >> (64 - k));
}

static long rng_next_u64(void) {
    long s0 = rng_state[0], s1 = rng_state[1];
    long result = s0 + s1;
    s1 ^= s0;
    rng_state[0] = rng_rotl(s0, 55) ^ s1 ^ (s1 << 14);
    rng_state[1] = rng_rotl(s1, 36);
    return result;
}

static void rng_setup(unsigned int seed) {
    rng_state[0] = seed * 0xdeadbeef;
    rng_state[1] = seed ^ 0xc0de1234;
    for (int i=0; i<100; i++)
        rng_next_u64();
}

static uint32_t rng_next_u32(void) {
    return rng_next_u64() >> 11;
}

// part with borrowed code - end

uint32_t gen_mult_shift_param() {
    // parameter for multiply shift
    uint32_t a = rng_next_u32();
    a = a | uint32_t(1);
    return a;
}

uint32_t** gen_tab_table() {
    // table for tabulation
    uint32_t** T = new uint32_t *[tab_table_size];
    for (int i = 0; i < tab_table_size; i++) {
        T[i] = new uint32_t[parts];
        for (int j = 0; j < parts; j++) {
            //T[i][j] = rng_next_u32() % (1 << pow_table_size);
            T[i][j] = rng_next_u32() & ((1 << pow_table_size) - 1);
        }
    }
    return T;
}

struct hf_params {
    // generation of parameters for hash functions
    uint32_t a1;
    uint32_t a2;
    uint32_t** T1;
    uint32_t** T2;
    
    void regenerate(){
        a1 = gen_mult_shift_param();
        a2 = gen_mult_shift_param();
        T1 = gen_tab_table();
        T2 = gen_tab_table();
    }
};

uint32_t naive_modulo(uint32_t x) {
    //return (x % hash_table_size)
    return (x & (hash_table_size - 1));
}

uint32_t multiply_shift(uint32_t x, uint32_t a) {
    return ((a * x) >> (pow_universe - pow_table_size));
}

uint32_t tabulation(uint32_t x, uint32_t**T) {
    uint32_t hash = 0UL;
    uint32_t mask = (uint32_t(1) << bits_per_part) - uint32_t(1);
    for (int i = 0; i < parts; i++) {
        int j = x & mask; // get last bits
        x = x >> bits_per_part; // shift
        hash = hash ^ T[j][i];
    }
    return hash;
}

uint32_t choose_hash_function(uint32_t x, uint32_t a, uint32_t** T){
    uint32_t hash;
    if (hash_function_choice == 1){
        hash = naive_modulo(x);
    }
    else if (hash_function_choice == 2){
        hash = multiply_shift(x, a);
    }
    else if (hash_function_choice == 3){
        hash = tabulation(x, T);
    }
    return hash;
}

void deletion(uint32_t** arr, int size) {
    for(int i = 0; i < size; ++i) {
        delete[] arr[i];
    }
    delete[] arr;
}

void linear_probing_insert(uint32_t x, hf_params params) {
    uint32_t cell = choose_hash_function(x, params.a1, params.T1);
    if (!(!cell_free[cell] && hash_table[cell] == x)) { // if not already inserted
        uint32_t position = cell;
        while (!cell_free[position]) {
            position = (position + uint32_t(1)) % hash_table_size;
            steps++;
        }
        hash_table[position] = x;
        cell_free[position] = false;
        occupied_cells++;
    }
}

void collision(uint32_t &x, uint32_t &cell, hf_params params){
    // swap
    uint32_t tmp = x;
    x = hash_table[cell];
    hash_table[cell] = tmp;
    steps++;

    // new hash function for old element
    if (cell == choose_hash_function(x, params.a1, params.T1)) {
        cell = choose_hash_function(x, params.a2, params.T2);
    } else {
        cell = choose_hash_function(x, params.a1, params.T1);
    }
}

bool rehash(uint32_t &x, hf_params params){
    //insert kicked out elemnt
    for (int i = 0; i < hash_table_size; i++) {
        if (cell_free[i]) {
            hash_table[i] = x;
            cell_free[i] = false;
            occupied_cells++;
            break;
        }
    }

    // reinsertion
    for (int i = 0; i < hash_table_size; i++) {
        uint32_t cell = choose_hash_function(hash_table[i], params.a1, params.T1);
        uint32_t cell2 = choose_hash_function(hash_table[i], params.a2, params.T2);
        if (!(cell == i or cell2 == i)) { // inplace rehash
            x = hash_table[i];
            cell_free[i] = true;
            for (int j = 0; j <= occupied_cells; j++) {
                // insert to empty cell
                if (cell_free[cell]) {
                    hash_table[cell] = x;
                    cell_free[cell] = false;
                    break;
                }
                //if collision
                collision(x, cell, params);

                if (j == occupied_cells){
                    occupied_cells--;
                    return true;
                }
            }
        }
    }
    return false;

    //bool rehash_need = occupied_cells != rehashed_elements; // true if we haven`t reinserted all elements
    //occupied_cells = original_n_cell;
    //return rehash_need;
}

void cuckoo_insert(uint32_t x, hf_params &params) {
    uint32_t cell = choose_hash_function(x, params.a1, params.T1);
    uint32_t cell2 = choose_hash_function(x, params.a2, params.T2);	// just for check

    auto begin_inside = std::chrono::high_resolution_clock::now(); // used only to output time when rehashes exceed

    if (!(!cell_free[cell] && hash_table[cell] == x) && !(!cell_free[cell2] && hash_table[cell2] == x)) { // if not already inserted
        for (int i = 0; i <= occupied_cells; i++) {
            // insert to empty cell
            if (cell_free[cell]) {
                hash_table[cell] = x;
                cell_free[cell] = false;
                occupied_cells++;
                return;
            }
            // if collision
            collision(x, cell, params);
        }

        // rehash
        bool rehash_need;
        for (int r = 1; r < 20; r++) {
            // generate new parameters for hash functions
            params.regenerate();
            rehash_need = rehash(x, params);
            if (!rehash_need) {
                return;
            }
        }

        // last print
        if (rehash_need) {
            double load_factor = (double) occupied_cells / hash_table_size;
            auto end_inside = std::chrono::high_resolution_clock::now();
            elapsed_time = std::chrono::duration_cast<std::chrono::nanoseconds>(end_inside - begin_inside).count();
            cout << load_factor*100 << " " << elapsed_time << " " << steps << endl;
            exit(0);
        }
    }
}

int main(int argc, char *argv[]) {
    int mode = atoi(argv[1]);
    int hash_table_choice = atoi(argv[2]);
    hash_function_choice = atoi(argv[3]);

    uint32_t seed = (unsigned int) time(NULL);
    rng_setup(seed);

    // Random case study
    if (mode == 1){
        hash_table = new uint32_t[hash_table_size];
        cell_free = (bool*)malloc(hash_table_size * sizeof(bool));
        fill_n(cell_free, hash_table_size, true);

        // initialization of parameters for hash functions
        hf_params params;
        params.regenerate();

        uint32_t x; // input number
        // 100 subsequences
        for (int s = 0; s < 100; s++) {
            double time_per_100 = 0;
            double steps_per_100 = 0;
            for (int i = 0; i < (hash_table_size/100); i++) {
                steps = 0;
                elapsed_time = 0;

                auto begin = std::chrono::high_resolution_clock::now();

                x = rng_next_u32(); // input
                if (hash_table_choice == 1) {
                    cuckoo_insert(x, params);
                }
                else if (hash_table_choice == 2) {
                    linear_probing_insert(x, params);
                }

                auto end = std::chrono::high_resolution_clock::now();
                elapsed_time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();

                steps_per_100 += steps;
                time_per_100 += elapsed_time;
            }
            double load_factor = (double) occupied_cells / hash_table_size;
            cout << load_factor*100 << " " << time_per_100 / (hash_table_size/100) <<
                 " " << steps_per_100 / (hash_table_size/100) << endl;
        }
        deletion(params.T1, tab_table_size);
        deletion(params.T2, tab_table_size);
        delete[] hash_table;
        delete[] cell_free;
    }
        // Case study of sequential inserts into a table using linear probing
    else {
        for (pow_table_size = 10; pow_table_size < 26; pow_table_size++){
            cout << pow_table_size;

            hash_table_size = 1 << pow_table_size;

            int first_m = (int)(hash_table_size * 0.89);
            int last_m = (int)(hash_table_size * 0.91);

            for (int l = 0; l < 20; l++){
                steps = 0;

                hash_table = new uint32_t[hash_table_size];
                cell_free = (bool*)malloc(hash_table_size * sizeof(bool));
                fill_n(cell_free, hash_table_size, true);


                // generate parameters for hash functions
                hf_params params;

                // simple insert
                for (int x = 1; x < first_m; x++) {
                    linear_probing_insert(x, params);
                }

                int total = 0;
                int inserted = 0;

                // measurements
                for (int x = first_m; x <= last_m; x++) {
                    steps = 0;
                    linear_probing_insert(x, params);
                    total += steps;
                    inserted++;
                }
                cout << " " << (double) total / inserted;

                deletion(params.T1, tab_table_size);
                deletion(params.T2, tab_table_size);
                delete[] hash_table;
                delete[] cell_free;
            }
            cout << endl;
        }
    }
    return 0;
}
