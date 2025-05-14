#include <iostream>
#include  <iomanip>
#include <vector>
#include <list>
using namespace std;

#define		DBG				1
#define		DRAM_SIZE		(64*1024*1024)
#define		CACHE_SIZE		(64*1024)


#define BLOCK_SIZE 32
#define NUM_LINES (CACHE_SIZE / BLOCK_SIZE)

struct FALine {
	unsigned int tag;
};

vector<FALine> faCache;
list<unsigned int> lruList; // Keeps track of access order

unsigned int cache[NUM_LINES];
bool valid[NUM_LINES];

enum cacheResType {MISS=0, HIT=1};

/* The following implements a random number generator */
unsigned int m_w = 0xABABAB55;    /* must not be zero, nor 0x464fffff */
unsigned int m_z = 0x05080902;    /* must not be zero, nor 0x9068ffff */
unsigned int rand_()
{
    m_z = 36969 * (m_z & 65535) + (m_z >> 16);
    m_w = 18000 * (m_w & 65535) + (m_w >> 16);
    return (m_z << 16) + m_w;  /* 32-bit result */
}

unsigned int memGen1()
{
	static unsigned int addr=0;
	return (addr++)%(DRAM_SIZE);
}

unsigned int memGen2()
{
	static unsigned int addr=0;
	return  rand_()%(24*1024);
}

unsigned int memGen3()
{
	return rand_()%(DRAM_SIZE);
}

unsigned int memGen4()
{
	static unsigned int addr=0;
	return (addr++)%(4*1024);
}

unsigned int memGen5()
{
	static unsigned int addr=0;
	return (addr++)%(1024*64);
}

unsigned int memGen6()
{
	static unsigned int addr=0;
	return (addr+=32)%(64*4*1024);
}


// Direct Mapped Cache Simulator
cacheResType cacheSimDM(unsigned int addr)
{
	// This function accepts the memory address for the memory transaction and
	// returns whether it caused a cache miss or a cache hit

	// The current implementation assumes there is no cache; so, every transaction is a miss
	unsigned int blockAddr = addr / BLOCK_SIZE;
	unsigned int index = blockAddr % NUM_LINES;
	unsigned int tag = blockAddr / NUM_LINES;

	if (valid[index] && cache[index] == tag) {
		return HIT;
	} else {
		cache[index] = tag;
		valid[index] = true;
		return MISS;
	}
}


// Fully Associative Cache Simulator
cacheResType cacheSimFA(unsigned int addr)
{
	// This function accepts the memory address for the read and
	// returns whether it caused a cache miss or a cache hit

	// The current implementation assumes there is no cache; so, every transaction is a miss
	unsigned int blockAddr = addr / BLOCK_SIZE;
	unsigned int tag = blockAddr;

	for (auto it = faCache.begin(); it != faCache.end(); ++it) {
		if (it->tag == tag) {
			lruList.remove(tag);
			lruList.push_front(tag);
			return HIT;
		}
	}
	if (faCache.size() < NUM_LINES) {
		faCache.push_back({tag});
		lruList.push_front(tag);
	} else {
		unsigned int lruTag = lruList.back();
		lruList.pop_back();
		for (auto it = faCache.begin(); it != faCache.end(); ++it) {
			if (it->tag == lruTag) {
				faCache.erase(it);
				break;
			}
		}

		faCache.push_back({tag});
		lruList.push_front(tag);
	}

	return MISS;
}

char *msg[2] = {"Miss","Hit"};

#define		NO_OF_Iterations	1000		// CHange to 1,000,000
int main()
{
	unsigned int hit = 0;
	cacheResType r;

	unsigned int addr;
	cout << "Direct Mapped Cache Simulator\n";

	for(int inst=0;inst<NO_OF_Iterations;inst++)
	{
		addr = memGen2();
		r = cacheSimFA(addr);
		if(r == HIT) hit++;
		cout <<"0x" << setfill('0') << setw(8) << hex << addr <<" ("<< msg[r] <<")\n";
	}
	cout << "Hit ratio = " << (100*hit/NO_OF_Iterations)<< endl;
}