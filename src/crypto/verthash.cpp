#include <crypto/verthash.h>
#include "clientversion.h"
#include <random.h>
#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <validation.h>
#include <boost/filesystem.hpp>

#define HEADER_SIZE 80
#define HASH_OUT_SIZE 32
#define P0_SIZE 64
#define N_ITER 8 
#define N_SUBSET P0_SIZE*N_ITER
#define N_ROT 32
#define N_INDEXES 4096
#define BYTE_ALIGNMENT 16

inline uint32_t fnv1a(const uint32_t a, const uint32_t b) {
    return (a ^ b) * 0x1000193;
}

void Verthash::Hash(const char* input, char* output)
{
    fs::path dataFile = GetDataDir() / "verthash.dat";
    if(!boost::filesystem::exists(dataFile)) {
        throw std::runtime_error("Verthash datafile not found");
    }

    FILE* VerthashDatFile = fsbridge::fopen(dataFile.c_str(),"rb");
    fseek(VerthashDatFile, 0, SEEK_END);
    const size_t datfile_sz = ftell(VerthashDatFile);

    unsigned char input_header[HEADER_SIZE];

    memcpy(&input_header[0], input, HEADER_SIZE);   

    unsigned char p1[HASH_OUT_SIZE];
    sha3(&input_header[0], HEADER_SIZE, &p1[0], HASH_OUT_SIZE);
	
    unsigned char p0[N_SUBSET];

    for(size_t i = 0; i < N_ITER; i++) {
    	input_header[0] += 1;
    	sha3(&input_header[0], HEADER_SIZE, p0+i*P0_SIZE, P0_SIZE);
    }

    uint32_t* p0_index = (uint32_t*)p0;
    uint32_t seek_indexes[N_INDEXES];

    for(size_t x = 0; x < N_ROT; x++) {
    	memcpy(seek_indexes+x*(N_SUBSET/sizeof(uint32_t)), p0, N_SUBSET);
    	for(size_t y = 0; y < N_SUBSET/sizeof(uint32_t); y++) {
	    *(p0_index + y) = (*(p0_index + y) << 1) | (1 & (*(p0_index + y) >> 31));
	}
    }

    uint32_t* p1_32 = (uint32_t*)p1;
    uint32_t value_accumulator = 0x811c9dc5;
    const uint32_t mdiv = ((datfile_sz - HASH_OUT_SIZE)/BYTE_ALIGNMENT) + 1;
    for(size_t i = 0; i < N_INDEXES; i++) {
	const long offset = (fnv1a(seek_indexes[i], value_accumulator) % mdiv) * BYTE_ALIGNMENT;
        fseek(VerthashDatFile, offset, SEEK_SET);
        for(size_t i2 = 0; i2 < HASH_OUT_SIZE/sizeof(uint32_t); i2++) {
            uint32_t value = 0;
            fread(&value, sizeof(uint32_t), 1, VerthashDatFile);
            uint32_t* p1_ptr = p1_32 + i2;
            *p1_ptr = fnv1a(*p1_ptr, value);

            value_accumulator = fnv1a(value_accumulator, value);
        }
    }

    memcpy(output, &p1[0], HASH_OUT_SIZE);
    fclose(VerthashDatFile);
}
