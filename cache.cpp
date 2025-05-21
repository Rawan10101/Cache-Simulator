#include <iostream>
#include <iomanip>
#include <vector>
#include <list>
#include <algorithm>
using namespace std;

#define DRAM_SIZE       (64 * 1024 * 1024) // 64 MB
#define CACHE_SIZE      (64 * 1024)        // 64 KB
#define NUM_REFERENCES  1000000

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

// Reset static variables
void resetMemGens() {
    m_w = 0xABABAB55;
    m_z = 0x05080902;
}

// Initialize Cache
void initCache(int sets, int ways, int blockSize) {
    numSets = sets;
    numWays = ways;
    lineSize = blockSize;

    cache.clear();
    lruLists.clear();
    
    cache.resize(numSets, vector<CacheLine>(numWays, {0, false}));
    lruLists.resize(numSets);
}

// Cache Simulator
cacheResType cacheSim(unsigned int addr) {
    unsigned int blockAddr = addr / lineSize;
    unsigned int index = blockAddr % numSets;
    unsigned int tag = blockAddr / numSets;

    auto &set = cache[index];
    auto &lru = lruLists[index];

    // Check for hit
    for (int i = 0; i < numWays; ++i) {
        if (set[i].valid && set[i].tag == tag) {
            // Update LRU - remove and add to front
            lru.remove(i);
            lru.push_front(i);
            return HIT;
        }
    }

    // Miss - find empty slot or evict LRU
    int replaceIndex = -1;
    
    // Look for invalid line first
    for (int i = 0; i < numWays; ++i) {
        if (!set[i].valid) {
            replaceIndex = i;
            break;
        }
    }
    
    // If no invalid line, use LRU
    if (replaceIndex == -1) {
        if (lru.size() < numWays) {
            // Find a way that's not in the LRU list
            for (int i = 0; i < numWays; ++i) {
                if (find(lru.begin(), lru.end(), i) == lru.end()) {
                    replaceIndex = i;
                    break;
                }
            }
        } else {
            replaceIndex = lru.back();
            lru.pop_back();
        }
    }
    
    // Update cache
    set[replaceIndex].tag = tag;
    set[replaceIndex].valid = true;
    
    // Update LRU
    lru.remove(replaceIndex); // Remove if exists
    lru.push_front(replaceIndex);
    
    return MISS;
}

// Experiment 1: Fix sets to 4, vary line size
void experimentVaryLineSize(unsigned int (*memGen)(), const string& genName) {
    vector<int> lineSizes = {16, 32, 64, 128};
    const int fixedSets = 4;
    vector<pair<int, double>> results;
    
    cout << "\n--- Experiment 1: Vary Line Size (Fixed Sets = 4) with " << genName << " ---\n";
    
    for (int blockSize : lineSizes) {
        int ways = CACHE_SIZE / (fixedSets * blockSize);
        resetMemGens();
        initCache(fixedSets, ways, blockSize);
        
        unsigned int hits = 0, misses = 0;
        for (int i = 0; i < NUM_REFERENCES; ++i) {
            unsigned int addr = memGen();
            if (cacheSim(addr) == HIT)
                hits++;
            else
                misses++;
        }
        
        double hitRatio = 100.0 * hits / NUM_REFERENCES;
        double missRatio = 100.0 * misses / NUM_REFERENCES;
        results.push_back({blockSize, hitRatio});
        
        cout << "Line size: " << blockSize << " bytes, Ways: " << ways 
             << ", Hit ratio: " << fixed << setprecision(4) << hitRatio 
             << "%, Miss ratio: " << missRatio << "%" << endl;
    }
}

// Experiment 2: Fix line size to 64B, vary ways
void experimentVaryWays(unsigned int (*memGen)(), const string& genName) {
    vector<int> waysList = {1, 2, 4, 8, 16};
    const int fixedLineSize = 64;
    vector<pair<int, double>> results;
    
    cout << "\n--- Experiment 2: Vary Ways (Fixed Line Size = 64B) with " << genName << " ---\n";
    
    for (int ways : waysList) {
        int sets = CACHE_SIZE / (ways * fixedLineSize);
        resetMemGens();
        initCache(sets, ways, fixedLineSize);
        
        unsigned int hits = 0, misses = 0;
        for (int i = 0; i < NUM_REFERENCES; ++i) {
            unsigned int addr = memGen();
            if (cacheSim(addr) == HIT)
                hits++;
            else
                misses++;
        }
        
        double hitRatio = 100.0 * hits / NUM_REFERENCES;
        double missRatio = 100.0 * misses / NUM_REFERENCES;
        results.push_back({ways, hitRatio});
        
        cout << "Ways: " << ways << ", Sets: " << sets 
             << ", Hit ratio: " << fixed << setprecision(4) << hitRatio 
             << "%, Miss ratio: " << missRatio << "%" << endl;
    }
}


// Test cases for validation

void testConflictMiss() {
    cout << "\n--- Test Case: Conflict Miss ---\n";
    
    // Test with direct-mapped cache (most prone to conflict misses)
    int lineSize = 64;
    int ways = 1;
    int sets = 4;
    cout << "Cache Type: Direct-Mapped" << endl;
    cout << "Cache Size: " << (sets * ways * lineSize) / 1024 << " KB" << endl;
    cout << "Line Size: " << lineSize << " bytes" << endl;
    cout << "Number of Sets: " << sets << endl;
    cout << "Number of Ways: " << ways << endl;
    cout << "Test Description: Testing conflict misses with addresses mapping to the same set\n";
    initCache(sets, ways, lineSize);
    
    unsigned int baseAddr = 0;
    // These addresses all map to the same set (same index, different tags)
    vector<unsigned int> addresses = {
        baseAddr + 0 * sets * lineSize,
        baseAddr + 1 * sets * lineSize,
        baseAddr + 2 * sets * lineSize,
        baseAddr + 3 * sets * lineSize
    };
    
    cout << "First round - all should be MISS:" << endl;
    for (unsigned int addr : addresses) {
        cacheResType res = cacheSim(addr);
        cout << "Access " << addr << ": " << (res == HIT ? "HIT" : "MISS") << endl;
    }
    
    // Re-access all addresses in reverse order - all should be MISS due to conflicts
    cout << "\nSecond round - all should be MISS due to conflict evictions EXCEPT the first one (the last one present in the last round is now first accessed this round and matches the tag ):" << endl;
    for (int i = addresses.size() - 1; i >= 0; i--) {
        cacheResType res = cacheSim(addresses[i]);
        cout << "Re-access " << addresses[i] << ": " << (res == HIT ? "HIT" : "MISS") << endl;
    }
    
    // Now test with 2-way set associative cache (should handle conflicts better)
    ways = 2;
    sets = CACHE_SIZE / (ways * lineSize);
    initCache(sets, ways, lineSize);
    
   cout << "\nWith 2-way set associative cache:" << endl;
    cout << "Cache Type: 2-Way Set Associative" << endl;
    cout << "Number of Sets: " << sets << endl;
    cout << "Number of Ways: " << ways << endl;
    
    cout << "\nFirst round - all should be MISS:" << endl;
    for (unsigned int addr : addresses) {
        cacheResType res = cacheSim(addr);
        cout << "Access " << addr << ": " << (res == HIT ? "HIT" : "MISS") << endl;
    }
    
    // Re-access all addresses - should be HIT since 2-way cache can hold them all
    cout << "\nSecond round - all should be HIT with 2-way cache:" << endl;
    for (unsigned int addr : addresses) {
        cacheResType res = cacheSim(addr);
        cout << "Re-access " << addr << ": " << (res == HIT ? "HIT" : "MISS") << endl;
    }
    
}

void testSequentialAccess() {
    cout << "\n--- Test Case: Sequential Access ---\n";
    
    int lineSize = 64;
    int ways = 1;
    int sets = CACHE_SIZE / (ways * lineSize);
    
    unsigned int hits = 0, misses = 0;
    cout << "Cache Type: " << (ways == 1 ? "Direct-Mapped" : to_string(ways) + "-Way Set Associative") << endl;
    cout << "Cache Size: " << CACHE_SIZE / 1024 << " KB" << endl;
    cout << "Line Size: " << lineSize << " bytes" << endl;
    cout << "Number of Sets: " << sets << endl;
    cout << "Number of Ways: " << ways << endl;
    cout << "Test Description: Accessing sequential addresses within the same cache line\n";
        initCache(sets, ways, lineSize);

    
    // Access sequential addresses within the same cache line
    for (int i = 0; i < 10; i++) {
        unsigned int addr = 1000 + i;
        cacheResType result = cacheSim(addr);
        if (result == HIT) hits++;
        else misses++;
        cout << "Access " << addr << ": " << (result == HIT ? "HIT" : "MISS") << endl;
    }
    
    cout << "Sequential access within same line - Hits: " << hits << ", Misses: " << misses << endl;
     cout << "Hit Ratio: " << fixed << setprecision(2) << (100.0 * hits / (hits + misses)) << "%" << endl;

}
void testLRUPolicy() {
    cout << "\n--- Test Case: LRU Replacement Policy ---\n";
    
    int lineSize = 64;
    int ways = 2;
    int sets = CACHE_SIZE / (ways * lineSize);
     cout << "Cache Type: " << (ways == 1 ? "Direct-Mapped" : to_string(ways) + "-Way Set Associative") << endl;
    cout << "Cache Size: " << CACHE_SIZE / 1024 << " KB" << endl;
    cout << "Line Size: " << lineSize << " bytes" << endl;
    cout << "Number of Sets: " << sets << endl;
    cout << "Number of Ways: " << ways << endl;
    cout << "Test Description: Testing LRU replacement with addresses mapping to the same set\n";
    
    initCache(sets, ways, lineSize);
    
    // Calculate addresses that map to the same set
    unsigned int setIndex = 5;
    unsigned int addr1 = setIndex * lineSize;
    unsigned int addr2 = addr1 + numSets * lineSize;
    unsigned int addr3 = addr2 + numSets * lineSize;
     cout << "Address " << addr1 << " maps to set " << setIndex << ", tag " << (addr1/lineSize)/sets << endl;
    cout << "Address " << addr2 << " maps to set " << setIndex << ", tag " << (addr2/lineSize)/sets << endl;
    cout << "Address " << addr3 << " maps to set " << setIndex << ", tag " << (addr3/lineSize)/sets << endl;
    
    cout << "Expected sequence: MISS, MISS, HIT, MISS, MISS, MISS\n";
    
    cout << "Access " << addr1 << ": " << (cacheSim(addr1) == HIT ? "HIT" : "MISS") << " cold start"<<endl;
    cout << "Access " << addr2 << ": " << (cacheSim(addr2) == HIT ? "HIT" : "MISS") <<" cold start" <<endl;
    cout << "Access " << addr1 << ": " << (cacheSim(addr1) == HIT ? "HIT" : "MISS") << endl;
    cout << "Access " << addr3 << ": " << (cacheSim(addr3) == HIT ? "HIT" : "MISS") << " 33088 is LRU so it is evicted and replaced by 65856"<<endl;
    cout << "Access " << addr2 << ": " << (cacheSim(addr2) == HIT ? "HIT" : "MISS") <<" 33088 is not in the set any more (evicted) so it will overwrite 320 (LRU so evicted)" <<endl;
    cout << "Access " << addr1 << ": " << (cacheSim(addr1) == HIT ? "HIT" : "MISS") <<" 320 not in the set anymore so it overwrites 65856(gets evicted using LRU)"<< endl;
   cout<< " Now set 5 contains addresses 320 and 33088"<<endl;

}

int main() {
    // Run validation tests

    testSequentialAccess();
    testLRUPolicy();
    testConflictMiss();
    
    // Run experiments with all memory generators
    experimentVaryLineSize(memGen1, "memGen1");
    experimentVaryWays(memGen1, "memGen1");
    
    experimentVaryLineSize(memGen2, "memGen2");
    experimentVaryWays(memGen2, "memGen2");
    
    experimentVaryLineSize(memGen3, "memGen3");
    experimentVaryWays(memGen3, "memGen3");
    
    experimentVaryLineSize(memGen4, "memGen4");
    experimentVaryWays(memGen4, "memGen4");
    
    experimentVaryLineSize(memGen5, "memGen5");
    experimentVaryWays(memGen5, "memGen5");
    
    experimentVaryLineSize(memGen6, "memGen6");
    experimentVaryWays(memGen6, "memGen6");
    
    return 0;
}
