#include <crypto/verthash_datfile.h>
#include "clientversion.h"
#include <random.h>
#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <validation.h>

#include "verthash_constants.h"

uint32_t VerthashDatFile::Fnv1a(uint32_t a, uint32_t b)
{
    return (a ^ b) * 0x1000193;
}

CCriticalSection VerthashDatFile::cs_Datfile;
void VerthashDatFile::UpdateMiningDataFile() {
    TRY_LOCK(cs_Datfile, lockDatfile);
    if (!lockDatfile)
    {
        LogPrintf("Skipping Verthash data update because one is already running");
        return;
    }

    fs::path targetFile = GetDataDir() / "verthash.dat";

    // Default to writing the whole file
    CBlockIndex* startIndex = chainActive.Genesis();

    FILE* blockFile;
    uint256 prevBlockHash;
    int startPos = 0;

    unsigned char prev_row[VERTHASH_MULTIPLICATION][32], curr_row[VERTHASH_MULTIPLICATION][32], double_buf[64];

    memset(prev_row, 0, sizeof(prev_row));

    if(boost::filesystem::exists(targetFile)) {
        blockFile = fsbridge::fopen(targetFile.c_str(),"rb+");


        CAutoFile filein(blockFile, SER_DISK, CLIENT_VERSION);
        fseek(blockFile, 0, SEEK_END);

        // Find common ancestor
        while(ftell(blockFile) >= VERTHASH_MULTIPLICATION*32){

            // Read previousBlockHash, the first 32 bytes of the expanded header
            fseek(blockFile, -VERTHASH_MULTIPLICATION*32, SEEK_CUR);
            filein >> FLATDATA(prevBlockHash);
            // Rewind what we read
            fseek(blockFile, -32, SEEK_CUR);

            // Now find this in our blockchain
            bool found = false;

            CBlockIndex *idx = pindexBestHeader;
            CBlockIndex *next = NULL;
            while(idx) {
                if((idx->pprev != NULL && idx->pprev->GetBlockHash() == prevBlockHash) || (idx->pprev == NULL && prevBlockHash.IsNull())) {
                    found = true;
                    break;
                }
                next = idx;
                idx = idx->pprev;
            }

            if(found) {

                startIndex = next;
                // startIndex is now equal to the first block we have to write.
                // The file pointer is before the last block in the file, because we need to read
                // in the last row.
                filein >> FLATDATA(prev_row);
                startPos = ftell(blockFile);
                break;
            }
        }
        filein.fclose();

    }

    if(startIndex == NULL) {
        return;
    }

    // Start at the tip and go back to startIndex
    CBlockIndex* idx = pindexBestHeader;
    std::stack<CBlockIndex*> blocksToWrite;
    while(idx) {
        if(startIndex->pprev != NULL && idx->GetBlockHash() == startIndex->pprev->GetBlockHash()) {
            // We found startIndex - we already have that in the file
            break;
        }
        blocksToWrite.push(idx);
        idx = idx->pprev;
    }

    if(blocksToWrite.size() == 0) {
        return;
    }

    blockFile = fsbridge::fopen(targetFile.c_str(),startPos > 0 ? "rb+" : "wb");
    CAutoFile fileout(blockFile, SER_DISK, CLIENT_VERSION);
    fseek(blockFile, startPos, SEEK_CUR);

    LogPrintf("Appending %d blocks to mining datafile\n", blocksToWrite.size());

    while(!blocksToWrite.empty()) {
        idx = blocksToWrite.top();

        CDataStream ssHeader(SER_DISK, CLIENT_VERSION);
        ssHeader << idx->GetBlockHeader();

        memset(curr_row, 0, sizeof(curr_row));
        memcpy(curr_row, ssHeader.data()+4, 32);

        for(int j=1; j<VERTHASH_MULTIPLICATION; j++)
        {
                memcpy(double_buf, prev_row[j-1], 32);
                memcpy(double_buf+32, curr_row[j-1], 32);

                sha3(double_buf, 64, curr_row[j], 32);
        }

        fileout << FLATDATA(curr_row);
        memcpy(prev_row, curr_row, VERTHASH_MULTIPLICATION*32);

        blocksToWrite.pop();
    }


    TruncateFile(blockFile, ftell(blockFile));
    FileCommit(blockFile);
    fileout.fclose();
    // Done.
}
