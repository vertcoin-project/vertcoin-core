#include <crypto/verthash.h>
#include "clientversion.h"
#include <random.h>
#include <hash.h>
#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <system_error>

#ifdef WIN32
#include <io.h>
#include <windows.h>
#else
#include <sys/mman.h>
#endif

#define HEADER_SIZE 80
#define HASH_OUT_SIZE 32
#define P0_SIZE 64
#define N_ITER 8
#define N_SUBSET P0_SIZE*N_ITER
#define N_ROT 32
#define N_INDEXES 4096
#define BYTE_ALIGNMENT 16

unsigned char *Verthash::datFile;
size_t Verthash::datFileSize;
bool Verthash::datFileInRam;
bool Verthash::datFileMapped;
const uint256 verthashDatFileHash = uint256{"48aa21d7afededb63976d48a8ff8ec29d5b02563af4a1110b056cd43e83155a5"};
inline uint32_t fnv1a(const uint32_t a, const uint32_t b) {
    return (a ^ b) * 0x1000193;
}

bool Verthash::VerifyDatFile()
{
    CSHA256 ctx;
    if(!datFileInRam) {
        std::filesystem::path dataFile{gArgs.GetDataDirNet() / "verthash.dat"};
        if(!std::filesystem::exists(dataFile)) {
            throw std::runtime_error("Verthash datafile not found");
        }
        FILE* datfile = fsbridge::fopen(dataFile.c_str(),"rb");
        fseek(datfile, 0, SEEK_SET);
        const int bufSize = 32768;
        unsigned char *buffer = (unsigned char *)malloc(bufSize);
        size_t bytes_read;
        while((bytes_read = fread(buffer, 1, 32768, datfile))) {
            ctx.Write(buffer, bytes_read);
        }
        fclose(datfile);
    } else {
        ctx.Write(datFile, datFileSize);
    }

    uint256 hash;
    ctx.Finalize((unsigned char*)&hash);
    if(hash == verthashDatFileHash) {
        return true;
    }
    LogInfo("Verthash Datafile's hash is invalid - got %s expected %s", hash.GetHex(), verthashDatFileHash.GetHex());
    return false;
}

void Verthash::LoadInRam() {
    std::filesystem::path dataFile{gArgs.GetDataDirNet() / "verthash.dat"};
    if(!std::filesystem::exists(dataFile)) {
        throw std::runtime_error("Verthash datafile not found");
    }
    Unload();
    FILE* datfile = fsbridge::fopen(dataFile.c_str(),"rb");
    if(datfile == nullptr) {
        throw std::runtime_error("Verthash datafile could not be opened");
    }
    fseek(datfile, 0, SEEK_END);
    datFileSize = ftell(datfile);

    fseek(datfile, 0, SEEK_SET);
    datFile = (unsigned char *)malloc(datFileSize);

    const size_t bytes_read = fread(datFile, 1, datFileSize, datfile);
    if(bytes_read != datFileSize) {
        throw std::runtime_error("Verthash datafile could not be loaded into RAM");
    }
    fclose(datfile);

    datFileInRam = true;
}

bool Verthash::MapDatFile() {
    const fs::path dataFile{gArgs.GetDataDirNet() / "verthash.dat"};
    Unload();
    std::error_code ec;
    const uintmax_t size = fs::file_size(dataFile, ec);
    FILE* datfile = fsbridge::fopen(dataFile, "rb");
    if(ec || size == 0 || datfile == nullptr) {
        if(datfile != nullptr) fclose(datfile);
        return false;
    }
#ifdef WIN32
    void* view = nullptr;
    HANDLE mapping = CreateFileMappingW((HANDLE)_get_osfhandle(_fileno(datfile)), nullptr, PAGE_READONLY, 0, 0, nullptr);
    if(mapping != nullptr) {
        view = MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0);
        // The view keeps the mapping and the file open.
        CloseHandle(mapping);
    }
#else
    void* view = mmap(nullptr, size, PROT_READ, MAP_SHARED, fileno(datfile), 0);
    if(view == MAP_FAILED) {
        view = nullptr;
    } else {
        // Each hash reads 32 bytes at 4096 random offsets. Without this, every page fault also reads
        // in the pages around it, which evicts the pages hashes need when the file doesn't fit in RAM.
        posix_madvise(view, size, POSIX_MADV_RANDOM);
    }
#endif
    fclose(datfile);
    if(view == nullptr) {
        return false;
    }
    datFile = (unsigned char*)view;
    datFileSize = size;
    datFileInRam = true;
    datFileMapped = true;
    return true;
}

void Verthash::SetDatFile(unsigned char* data, size_t size) {
    Unload();
    datFile = data;
    datFileSize = size;
    datFileInRam = true;
}

void Verthash::Unload() {
    if(datFileMapped) {
#ifdef WIN32
        UnmapViewOfFile(datFile);
#else
        munmap(datFile, datFileSize);
#endif
    } else {
        free(datFile);
    }
    datFile = nullptr;
    datFileSize = 0;
    datFileInRam = false;
    datFileMapped = false;
}

bool Verthash::IsLoaded() {
    return datFileInRam;
}

void Verthash::Hash(const char* input, char* output)
{
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

    size_t datfile_sz = datFileSize;
    FILE* VerthashDatFile;

    if(!datFileInRam) {
        std::filesystem::path dataFile{gArgs.GetDataDirNet() / "verthash.dat"};
        if(!std::filesystem::exists(dataFile)) {
            throw std::runtime_error("Verthash datafile not found");
        }
        VerthashDatFile = fsbridge::fopen(dataFile.c_str(),"rb");
        fseek(VerthashDatFile, 0, SEEK_END);
        datfile_sz = ftell(VerthashDatFile);
    }

    uint32_t* p1_32 = (uint32_t*)p1;
    uint32_t value_accumulator = 0x811c9dc5;
    const uint32_t mdiv = ((datfile_sz - HASH_OUT_SIZE)/BYTE_ALIGNMENT) + 1;

    if(!datFileInRam) {
        for(size_t i = 0; i < N_INDEXES; i++) {
            const long offset = (fnv1a(seek_indexes[i], value_accumulator) % mdiv) * BYTE_ALIGNMENT;
            fseek(VerthashDatFile, offset, SEEK_SET);
            for(size_t i2 = 0; i2 < HASH_OUT_SIZE/sizeof(uint32_t); i2++) {
                uint32_t value = 0;
                size_t read_len = fread(&value, sizeof(uint32_t), 1, VerthashDatFile);
                assert(read_len == 1);
                uint32_t* p1_ptr = p1_32 + i2;
                *p1_ptr = fnv1a(*p1_ptr, value);

                value_accumulator = fnv1a(value_accumulator, value);
            }
        }
    } else {
        uint32_t* blob_bytes_32 = (uint32_t*)datFile;
        for(size_t i = 0; i < N_INDEXES; i++) {
            const uint32_t offset = (fnv1a(seek_indexes[i], value_accumulator) % mdiv) * BYTE_ALIGNMENT/sizeof(uint32_t);
            for(size_t i2 = 0; i2 < HASH_OUT_SIZE/sizeof(uint32_t); i2++) {
                const uint32_t value = *(blob_bytes_32 + offset + i2);
                uint32_t* p1_ptr = p1_32 + i2;
                *p1_ptr = fnv1a(*p1_ptr, value);
                value_accumulator = fnv1a(value_accumulator, value);
            }
        }
    }

    memcpy(output, &p1[0], HASH_OUT_SIZE);
    if(!datFileInRam) {
        fclose(VerthashDatFile);
    }
}
