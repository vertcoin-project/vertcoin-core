#include <crypto/verthash_datfile.h>
#include <logging.h>
#include <util/fs_helpers.h>
#include <util/time.h>

#include <cstring>
#include <system_error>
#include <vector>

#ifdef WIN32
#include <io.h>
#include <windows.h>
#else
#include <sys/mman.h>
#endif

#define NODE_SIZE 32

// The datafile is the graph for this index: numXi(17) nodes of NODE_SIZE bytes.
static const int64_t GRAPH_INDEX = 17;

struct Graph
{
    uint8_t *data;
    int64_t log2;
    int64_t pow2;
    const uint8_t *pk;
    int64_t index;
};

static int64_t Log2(int64_t x)
{
    int64_t r = 0;
    for (; x > 1; x >>= 1)
    {
        r++;
    }

    return r;
}

static int64_t bfsToPost(const struct Graph *g, const int64_t node)
{
    return node & ~g->pow2;
}

static int64_t numXi(int64_t index)
{
    return (1 << ((uint64_t)index)) * (index + 1) * index;
}

static uint8_t *GetNode(struct Graph *g, const int64_t id)
{
    return g->data + bfsToPost(g, id) * NODE_SIZE;
}

static uint32_t WriteVarInt(uint8_t *buffer, int64_t val)
{
    memset(buffer, 0, NODE_SIZE);
    uint64_t uval = ((uint64_t)(val)) << 1;
    if (val < 0)
    {
        uval = ~uval;
    }
    uint32_t i = 0;
    while (uval >= 0x80)
    {
        buffer[i] = (uint8_t)uval | 0x80;
        uval >>= 7;
        i++;
    }
    buffer[i] = (uint8_t)uval;
    return i;
}

// Hashes node id from the public key, the id and up to two parents, straight into the graph.
// Parents always come before id in the graph, so they never overlap the node being written.
static void NewNode(struct Graph *g, const int64_t id, const uint8_t *parent0, const uint8_t *parent1)
{
    uint8_t hashInput[NODE_SIZE * 4];
    size_t hashInputSize = NODE_SIZE * 2;
    memcpy(hashInput, g->pk, NODE_SIZE);
    WriteVarInt(hashInput + NODE_SIZE, id);
    if (parent0 != nullptr)
    {
        memcpy(hashInput + hashInputSize, parent0, NODE_SIZE);
        hashInputSize += NODE_SIZE;
    }
    if (parent1 != nullptr)
    {
        memcpy(hashInput + hashInputSize, parent1, NODE_SIZE);
        hashInputSize += NODE_SIZE;
    }
    sha3(hashInput, hashInputSize, GetNode(g, id), NODE_SIZE);
}

static void ButterflyGraph(struct Graph *g, int64_t index, int64_t *count)
{
    if (index == 0)
    {
        index = 1;
    }

    int64_t numLevel = 2 * index;
    int64_t perLevel = (int64_t)(1 << (uint64_t)index);
    int64_t begin = *count - perLevel;
    int64_t level, i;

    for (level = 1; level < numLevel; level++)
    {
        for (i = 0; i < perLevel; i++)
        {
            int64_t prev;
            int64_t shift = index - level;
            if (level > numLevel / 2)
            {
                shift = level - numLevel / 2;
            }
            if (((i >> (uint64_t)shift) & 1) == 0)
            {
                prev = i + (1 << (uint64_t)shift);
            }
            else
            {
                prev = i - (1 << (uint64_t)shift);
            }

            const uint8_t *parent0 = GetNode(g, begin + (level - 1) * perLevel + prev);
            const uint8_t *parent1 = GetNode(g, *count - perLevel);
            NewNode(g, *count, parent0, parent1);
            (*count)++;
        }
    }
}

static void XiGraphIter(struct Graph *g, int64_t index)
{
    int64_t count = g->pow2;

    std::vector<int64_t> stack(5, index);
    std::vector<int32_t> graphStack{4, 3, 2, 1, 0};

    int64_t i = 0;
    int64_t graph = 0;
    int64_t pow2index = 1 << ((uint64_t)index);

    for (i = 0; i < pow2index; i++)
    {
        NewNode(g, count, nullptr, nullptr);
        count++;
    }

    if (index == 1)
    {
        ButterflyGraph(g, index, &count);
        return;
    }

    while (!stack.empty())
    {
        index = stack.back();
        stack.pop_back();
        graph = graphStack.back();
        graphStack.pop_back();

        int64_t pow2indexInner = 1 << ((uint64_t)index);
        int64_t pow2indexInner_1 = 1 << ((uint64_t)index - 1);

        if (graph == 0)
        {
            uint64_t sources = count - pow2indexInner;
            for (i = 0; i < pow2indexInner_1; i++)
            {
                const uint8_t *parent0 = GetNode(g, sources + i);
                const uint8_t *parent1 = GetNode(g, sources + i + pow2indexInner_1);
                NewNode(g, count, parent0, parent1);
                count++;
            }
        }
        else if (graph == 1 || graph == 2 || graph == 3)
        {
            // The first Xi graph, the second Xi graph and the second butterfly graph
            // are each a chain of nodes with one parent.
            uint64_t first = count;
            for (i = 0; i < pow2indexInner_1; i++)
            {
                uint64_t nodeId = first + i;
                const uint8_t *parent = GetNode(g, first - pow2indexInner_1 + i);
                NewNode(g, nodeId, parent, nullptr);
                count++;
            }
        }
        else
        {
            uint64_t sinks = count;
            uint64_t sources = sinks + pow2indexInner - numXi(index);
            for (i = 0; i < pow2indexInner_1; i++)
            {
                uint64_t nodeId0 = sinks + i;
                uint64_t nodeId1 = sinks + i + pow2indexInner_1;
                const uint8_t *parent0 = GetNode(g, sinks - pow2indexInner_1 + i);
                const uint8_t *parent1_0 = GetNode(g, sources + i);
                const uint8_t *parent1_1 = GetNode(g, sources + i + pow2indexInner_1);
                NewNode(g, nodeId0, parent0, parent1_0);
                NewNode(g, nodeId1, parent0, parent1_1);
                count += 2;
            }
        }

        if ((graph == 0 || graph == 3) ||
            ((graph == 1 || graph == 2) && index == 2))
        {
            ButterflyGraph(g, index - 1, &count);
        }
        else if (graph == 1 || graph == 2)
        {
            stack.insert(stack.end(), 5, index - 1);
            graphStack.insert(graphStack.end(), {4, 3, 2, 1, 0});
        }
    }
}

// Builds the whole datafile into data, which must hold DatFileSize() bytes.
static void GenerateGraph(uint8_t *data)
{
    const char *hashInput = "Verthash Proof-of-Space Datafile";
    uint8_t pk[NODE_SIZE];
    sha3(hashInput, 32, pk, NODE_SIZE);

    struct Graph g;
    g.data = data;
    g.log2 = Log2(numXi(GRAPH_INDEX)) + 1;
    g.pow2 = 1 << ((uint64_t)g.log2);
    g.pk = pk;
    g.index = GRAPH_INDEX;

    XiGraphIter(&g, GRAPH_INDEX);
}

static size_t DatFileSize()
{
    return numXi(GRAPH_INDEX) * NODE_SIZE;
}

// Sizes file, which must be open for reading and writing, and maps it for writing.
static uint8_t *MapFile(FILE *file, const fs::path& path, size_t size)
{
    AllocateFileRange(file, 0, size);
    if (fflush(file) != 0)
    {
        return nullptr;
    }
    std::error_code ec;
    if (fs::file_size(path, ec) < size || ec)
    {
        return nullptr;
    }
#ifdef WIN32
    HANDLE mapping = CreateFileMappingW((HANDLE)_get_osfhandle(_fileno(file)), nullptr, PAGE_READWRITE,
                                        (DWORD)((uint64_t)size >> 32), (DWORD)size, nullptr);
    if (mapping == nullptr)
    {
        return nullptr;
    }
    void *view = MapViewOfFile(mapping, FILE_MAP_WRITE, 0, 0, size);
    // The view keeps the mapping open.
    CloseHandle(mapping);
    return (uint8_t *)view;
#else
    void *view = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fileno(file), 0);
    return view == MAP_FAILED ? nullptr : (uint8_t *)view;
#endif
}

// Writes a mapping from MapFile back to its file and unmaps it.
static bool UnmapFile(uint8_t *data, size_t size)
{
#ifdef WIN32
    const bool flushed = FlushViewOfFile(data, size);
    return UnmapViewOfFile(data) && flushed;
#else
    const bool flushed = msync(data, size, MS_SYNC) == 0;
    return munmap(data, size) == 0 && flushed;
#endif
}

// Generates the datafile into file, either in memory and written out at once, or through
// a memory map of the file, so the OS can page it out on low-memory systems.
static bool WriteDatFile(FILE *file, const fs::path& path, bool in_ram)
{
    const size_t size = DatFileSize();
    if (in_ram)
    {
        uint8_t *data = (uint8_t *)malloc(size);
        if (data == nullptr)
        {
            LogError("Could not allocate %u bytes for the Verthash datafile\n", size);
            return false;
        }
        GenerateGraph(data);
        const bool written = fwrite(data, 1, size, file) == size;
        free(data);
        return written;
    }

    uint8_t *data = MapFile(file, path, size);
    if (data == nullptr)
    {
        LogError("Could not map %s into memory\n", fs::PathToString(path));
        return false;
    }
    GenerateGraph(data);
    return UnmapFile(data, size);
}

RecursiveMutex VerthashDatFile::cs_Datfile;

void VerthashDatFile::DeleteMiningDataFile() {
    const std::filesystem::path targetFile{gArgs.GetDataDirNet() / "verthash.dat"};
    if(std::filesystem::exists(targetFile)) {
        std::filesystem::remove(targetFile);
    }
}

bool VerthashDatFile::CreateMiningDataFile(bool in_ram) {
    TRY_LOCK(cs_Datfile, lockDatfile);
    if (!lockDatfile)
    {
        LogInfo("Skipping Verthash data generation because one is already running\n");
        return true;
    }

    const fs::path targetFile{gArgs.GetDataDirNet() / "verthash.dat"};
    if (fs::exists(targetFile)) {
        return true;
    }

    // Generate into a temporary file and rename it when complete, so an interrupted
    // generation never leaves a partial datafile behind.
    const fs::path tmpFile{gArgs.GetDataDirNet() / "verthash.dat.tmp"};
    LogInfo("Starting Proof-of-Space datafile generation at %s.\n", fs::PathToString(targetFile));
    const auto start{SteadyClock::now()};

    if (!CheckDiskSpace(gArgs.GetDataDirNet(), DatFileSize())) {
        LogError("Not enough disk space for the Verthash datafile at %s\n", fs::PathToString(targetFile));
        return false;
    }

    FILE *file = fsbridge::fopen(tmpFile, in_ram ? "wb" : "wb+");
    if (file == nullptr) {
        LogError("Could not create %s\n", fs::PathToString(tmpFile));
        return false;
    }
    bool written = WriteDatFile(file, tmpFile, in_ram);
    written = FileCommit(file) && written;
    written = fclose(file) == 0 && written;
    if (!written || !RenameOver(tmpFile, targetFile)) {
        LogError("Could not write the Verthash datafile at %s\n", fs::PathToString(targetFile));
        std::error_code ec;
        fs::remove(tmpFile, ec);
        return false;
    }

    LogInfo("Finished Proof-of-Space datafile generation in %.1f s.\n", Ticks<SecondsDouble>(SteadyClock::now() - start));
    return true;
}
