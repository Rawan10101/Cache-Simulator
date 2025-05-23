#include <iostream>
#include <iomanip>
#include <vector>
#include <list>
#include <algorithm>
#include <cmath>
using namespace std;

#define DRAM_SIZE       (64 * 1024 * 1024) // 64 MB
#define CACHE_SIZE      (64 * 1024)        // 64 KB
#define NUM_REFERENCES  1000000
#define MISS_FLAG false
#define HIT_FLAG true

enum cacheResType { MISS = 0, HIT = 1 };

// Cache structures
struct CacheLine {
    unsigned int tag;
    bool valid;
};

vector<vector<CacheLine>> cache;
vector<list<int>> lruLists;
int numSets;
int numWays;
int lineSize;

// Random number generator
unsigned int m_w = 0xABABAB55;
unsigned int m_z = 0x05080902;
unsigned int rand_()
{
    m_z = 36969 * (m_z & 65535) + (m_z >> 16);
    m_w = 18000 * (m_w & 65535) + (m_w >> 16);
    return (m_z << 16) + m_w;
}

// Memory reference generators
unsigned int memGen1()
{
    static unsigned int addr=0;
    return (addr++)%(DRAM_SIZE);
}

unsigned int memGen2()
{
    static unsigned int addr=0;
    return rand_()%(24*1024);  //24 KB
}

unsigned int memGen3()
{
    return rand_()%(DRAM_SIZE);
}

unsigned int memGen4()
{
    static unsigned int addr=0;
    return (addr++)%(4*1024); // 4KB
}

unsigned int memGen5()
{
    static unsigned int addr=0;
    return (addr++)%(1024*64); //64 KB
}

unsigned int memGen6()
{
    static unsigned int addr=0;
    return (addr+=32)%(64*4*1024);
}

void resetMemGens() {
    m_w = 0xABABAB55;
    m_z = 0x05080902;
}
void initCache(int sets, int ways, int blockSize) {
    numSets = sets;
    numWays = ways;
    lineSize = blockSize;
    cache.clear();
    lruLists.clear();

    cache.resize(numSets, vector<CacheLine>(numWays, {0, false}));
    lruLists.resize(numSets);
}
int block_size = 4; // example block size
int number_of_blocks = 1024; // number of blocks in the cache
int cash_type = 0; // 0: Direct, 1: Set-associative, 2: Fully associative

int coldstart_misses = 0;
int conflict_misses = 0;
int capcity_misses = 0;

// Cache Simulator
bool cacheSim(unsigned int address, int cache[3][100000], int assoc_type, int &replacement_counter, int index, int tag)
{
    int offset_bits = log2(block_size);
    bool is_hit = false;

    if (cash_type == 0) // Direct Mapped
    {
        if (cache[0][index] == tag)
        {
            return true; // Hit
        }
        else
        {
            // Miss handling
            cache[0][index] = tag;

            bool is_compulsory = false;
            for (int block = 0; block < number_of_blocks; block++)
            {
                if (cache[1][block] != 1)
                {
                    is_compulsory = true;
                    break;
                }
            }

            if (is_compulsory)
                capcity_misses++;
            else if (cache[1][index] == 1)
                conflict_misses++;
            else
                coldstart_misses++;

            cache[1][index] = 1;
            return false;
        }
    }

    else if (cash_type == 1) // Set Associative
    {
        int set_start = index * assoc_type;
        bool found = false;

        for (int i = 0; i < assoc_type; ++i)
        {
            if (cache[0][set_start + i] == tag)
            {
                return true; // Hit
            }
        }

        // Check for empty spot
        for (int i = 0; i < assoc_type; ++i)
        {
            if (cache[1][set_start + i] == -1)
            {
                cache[0][set_start + i] = tag;
                cache[1][set_start + i] = 1;
                coldstart_misses++;
                return false;
            }
        }

        // Replace randomly
        int victim_index = set_start + (rand() % assoc_type);
        cache[0][victim_index] = tag;
        cache[1][victim_index] = 1;
        capcity_misses++;
        return false;
    }

    else if (cash_type == 2) // Fully Associative
    {
        int block_addr = address >> offset_bits;

        // LRU
        if (assoc_type == 0)
        {
            for (int i = 0; i < number_of_blocks; ++i)
            {
                if (cache[0][i] == block_addr)
                {
                    cache[1][i] = replacement_counter++;
                    return true;
                }
            }

            if (replacement_counter < number_of_blocks)
            {
                cache[0][replacement_counter] = block_addr;
                cache[1][replacement_counter] = replacement_counter;
                coldstart_misses++;
                replacement_counter++;
                return false;
            }
            else
            {
                int lru_index = 0;
                for (int i = 1; i < number_of_blocks; ++i)
                {
                    if (cache[1][i] < cache[1][lru_index])
                        lru_index = i;
                }
                cache[0][lru_index] = block_addr;
                cache[1][lru_index] = replacement_counter++;
                capcity_misses++;
                return false;
            }
        }

        // LFU
        else if (assoc_type == 1)
        {
            for (int i = 0; i < number_of_blocks; ++i)
            {
                if (cache[0][i] == block_addr)
                {
                    cache[1][i]++;
                    return true;
                }
            }

            if (replacement_counter < number_of_blocks)
            {
                cache[0][replacement_counter] = block_addr;
                cache[1][replacement_counter] = 1;
                coldstart_misses++;
                replacement_counter++;
                return false;
            }
            else
            {
                int lfu_index = 0;
                for (int i = 1; i < number_of_blocks; ++i)
                {
                    if (cache[1][i] < cache[1][lfu_index])
                        lfu_index = i;
                }
                cache[0][lfu_index] = block_addr;
                cache[1][lfu_index] = 1;
                capcity_misses++;
                return false;
            }
        }

        // FIFO
        else if (assoc_type == 2)
        {
            for (int i = 0; i < number_of_blocks; ++i)
            {
                if (cache[0][i] == block_addr)
                    return true;
            }

            if (replacement_counter < number_of_blocks)
            {
                cache[0][replacement_counter] = block_addr;
                cache[1][replacement_counter] = replacement_counter;
                coldstart_misses++;
                replacement_counter++;
                return false;
            }
            else
            {
                int fifo_index = 0;
                for (int i = 1; i < number_of_blocks; ++i)
                {
                    if (cache[1][i] < cache[1][fifo_index])
                        fifo_index = i;
                }
                cache[0][fifo_index] = block_addr;
                cache[1][fifo_index] = replacement_counter++;
                capcity_misses++;
                return false;
            }
        }

        // Random
        else if (assoc_type == 3)
        {
            for (int i = 0; i < number_of_blocks; ++i)
            {
                if (cache[0][i] == block_addr)
                    return true;
            }

            if (replacement_counter < number_of_blocks)
            {
                cache[0][replacement_counter] = block_addr;
                coldstart_misses++;
                replacement_counter++;
                return false;
            }
            else
            {
                int random_index = rand() % number_of_blocks;
                cache[0][random_index] = block_addr;
                capcity_misses++;
                return false;
            }
        }
    }

    return true;
}


string msg[] = {"Miss", "Hit"};
int main(int argc, const char* argv[]) {
    int cash_size;
    int looper = 1000000;
    int addr, flag, shift;

    cout << "Enter cache type (0: Direct Mapped, 1: Set Associative, 2: Fully Associative): ";
    cin >> cash_type;

    cout << "Enter block size (Power of 2, between 4 and 128 bytes): ";
    cin >> block_size;

    cout << "Enter cache size (1KB - 64KB, power of 2 steps): ";
    cin >> cash_size;

    int cash[3][100000];
    int block_counter = 0;
    int hit_counter = 0;
    int index_addr = 0, tag_addr = 0;

    if (cash_type == 0) {

        number_of_blocks = (cash_size * 1024) / block_size;
        for (int i = 0; i < 2; i++)
            for (int j = 0; j < number_of_blocks; j++)
                cash[i][j] = -1;

        for (int i = 0; i < looper; i++) {
            addr = memGen1();
            shift = log2(block_size);
            index_addr = (addr >> shift) % number_of_blocks;
            shift = log2(number_of_blocks + block_size);
            tag_addr = addr >> shift;

            flag = cacheSim(addr, cash, 0, block_counter, index_addr, tag_addr);

            index_addr = 0;
            tag_addr = 0;

            cout << "0x" << setw(8) << setfill('0') << hex << addr << " (" << msg[flag] << ")\n";

            if (msg[flag] == "Hit")
                hit_counter++;
        }

        cout << "Hits: " << hit_counter << "\nCompulsory misses: " << coldstart_misses
             << "\nCapacity misses: " << capcity_misses << "\nConflict misses: " << conflict_misses << endl;

    } else if (cash_type == 2) {

        int replacement_policy;

        number_of_blocks = (cash_size * 1024) / block_size;

        cout << "Choose replacement policy (LRU=0, LFU=1, FIFO=2, RANDOM=3): ";
        cin >> replacement_policy;
        for (int i = 0; i < 2; i++)
            for (int j = 0; j < number_of_blocks; j++)
                cash[i][j] = -10;

        for (int i = 0; i < looper; i++) {
            addr = memGen4();

            flag = cacheSim(addr, cash, replacement_policy, block_counter, index_addr, tag_addr);

            if (msg[flag] == "Hit")
                hit_counter++;

            block_counter++;
        }

        cout << "Hits: " << hit_counter << "\nCompulsory misses: " << coldstart_misses
             << "\nCapacity misses: " << capcity_misses << "\nConflict misses: " << conflict_misses << endl;

    } else if (cash_type == 1) { // Set Associative Cache

        int ways;

        cout << "Specify the number of ways for set associative cache (2,4,8,16): ";
        cin >> ways;

        number_of_blocks = (cash_size * 1024) / (block_size * ways);

        // Initialize cache arrays with -1
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 100000; j++)
                cash[i][j] = -1;

        for (int i = 0; i < looper; i++) {
            addr = memGen5();

            shift = log2(block_size);
            index_addr = (addr >> shift) % number_of_blocks;

            shift = log2(number_of_blocks + block_size);
            tag_addr = addr >> shift;

            flag = cacheSim(addr, cash, ways, block_counter, index_addr, tag_addr);

            index_addr = 0;
            tag_addr = 0;

            if (msg[flag] == "Hit")
                hit_counter++;

            block_counter++;
        }

        cout << "Hits: " << hit_counter << "\nCold star misses: " << coldstart_misses
             << "\nCapacity misses: " << capcity_misses << "\nConflict misses: " << conflict_misses << endl;
    }

    return 0;
}
