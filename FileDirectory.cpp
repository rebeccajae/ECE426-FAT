//
// Created by becca on 2/3/18.
//

#include "FileDirectory.h"

#include <iostream>

#define ZERO_UNUSED 0x0000
#define ZERO_USED 0x0001
#define LAST_CLUSTER 0xFFFF
#define CLUSTER_SIZE 4

/**
 * Create a new instance of FileDirectory and initialize all FAT entries to 0
 */
FileDirectory::FileDirectory() {
    //Initialize all entries in FAT16 to zero.
    for (int i = 0; i < 256; i++) {
        FAT16[i] = ZERO_UNUSED; // Set to zero, unused
    }
}

/**
 * Check if a file of the size described can fit into the directory object
 *
 * @param filename Name of desired file, 8 chars
 * @param numberBytes Size of desired file, in bytes
 * @return true if a file can be created with the size given, false if not
 */
bool FileDirectory::create(char *filename, int numberBytes) {
    //Check if there's an entry in the directory
    int unusedEntryCount = 0;
    for (int i = 0; i < 4; i++) {
        if(fileDirectory[i][0] == 0){
            unusedEntryCount++; //Count all unused entries.
        }
    }
    if(unusedEntryCount > 0){
        //Check if there are enough unused clusters.
        int numberClustersNeeded = (numberBytes/CLUSTER_SIZE) + 1;
        int numberClustersAvail = 0;
        for (int j = 0; j < 256; j++) {
            if(FAT16[j] == ZERO_UNUSED || FAT16[j] == ZERO_USED){
                //It's a free-marked cluster. Count it.
                numberClustersAvail++;
            }
        }
        if(numberClustersNeeded <= numberClustersAvail){

            //We have space for it!
            return true;
        }else{
            //No space in the FAT.
            return false;
        }
    }else{
        //No entries available.
        return false;
    }
}

/**
 * Delete the file given in filename
 *
 * @param filename The name of the file to delete
 * @return true if successfully deleted file, false if the file does not exist
 */
bool FileDirectory::deleteFile(char *filename) {
    unsigned short int firstSector;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            if (fileDirectory[i][j] != filename[j]) {
                break;
            }
            if (j == 7) {
                //There is a file!
                //Delete it.
                fileDirectory[i][0] = 0x0; // Change first character of name to 0. (NUL)
                firstSector = fileDirectory[i][27] << 8 | fileDirectory[i][26];
                unsigned short int next_sector = firstSector;
                unsigned short int temp;
                while (next_sector != LAST_CLUSTER) {
                    temp = FAT16[next_sector];
                    FAT16[next_sector] = 0x01;
                    next_sector = temp;

                }
                return true;
            }
        }
    }
    return false;
}

bool FileDirectory::write(char filename[], int numberBytes, char fileData[], int year, int month, int day, int hour, int minute, int second)
{
    unsigned char record[32] = { 0 };
    unsigned short int i, j, sectorAddress, unused[256], filesec[256], time, date;
    //Generate FAT Record template
    record[31] = numberBytes >> 24;
    record[30] = numberBytes  >> 16;
    record[29] = numberBytes >> 8;
    record[28] = numberBytes;
    date = (year - 1980) << 9 + month << 5 + day;
    record[25] = date >> 8;
    record[24] = date;
    time = (hour << 11) + (minute << 5) + (second / 2);
    record[23] = time >> 8;
    record[22] = time;
    for (int l = 0; l < 8; l++)
    {

        record[l] = filename[l];
    }

    for (i = 2; i < 256; i++)
    {
        if (FAT16[i] == ZERO_USED || FAT16[i] == ZERO_UNUSED) {
            break;
        }
    }

    sectorAddress = i;
    record[27] = sectorAddress >> 8;
    record[26] = sectorAddress;

    for (int k = 0; k < 4; k++) {
        if(fileDirectory[k][0] == 0){
            for (int l = 0; l < 32; ++l) {
                fileDirectory[k][l] = record[l];
            }
            break;
        }
    }
    j = 1;
    unused[0] = i;
    for (i = unused[0]+1; i < 256 && j*CLUSTER_SIZE <= numberBytes; i++) {
        if (FAT16[i] == ZERO_USED || FAT16[i] == ZERO_UNUSED) {
            unused[j++] = i;
        }
    }

    for (i = 0; i < j-2; i++) {
        FAT16[unused[i]] = unused[i + 1];
    }

    FAT16[unused[i]] = LAST_CLUSTER;

    //Unused has each sector. Let's traverse and toss em along.

    for (int m = 0; m <= i+1; ++m) {
        int currentSector = unused[m];
        for (int k = 0; k < 4; ++k) {
            data[currentSector*4 + k] = fileData[m*4 + k];
        }
    }
    return true;
}


void FileDirectory::printClusters(char filename[])
{
    unsigned short int firstSector;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++)
        {
            if (fileDirectory[i][j] != filename[j]) {
                break;
            }
            if (j == 7) {
                //There is a file!
                firstSector = fileDirectory[i][27] << 8 | fileDirectory[i][26];

            }
        }
    }
    if(firstSector != 0) {
        unsigned short int next_sector = firstSector;
        while (next_sector < 0xFFF8) {
            std::cout << next_sector << "->";
            next_sector = FAT16[next_sector];

        }
        std::cout << "EOF" << std::endl;
    }
}

bool FileDirectory::read(char *filename, char *fileData) {
    return false;
}

void FileDirectory::printDirectory() {
    char filename[8];
    for (int i = 0; i < 4; i++) {
        if(fileDirectory[i][0] != 0){
            for (int j = 0; j < 8; ++j) {
                filename[j] = fileDirectory[i][j];

            }
            std::cout << filename << ":" << std::endl;
            this->printClusters(filename);
        }

    }
}

void FileDirectory::printData(char *filename) {
    unsigned short int firstSector, sectors[256], r;
    unsigned int fileSz;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++)
        {
            if (fileDirectory[i][j] != filename[j]) {
                break;
            }
            if (j == 7) {
                //There is a file!
                firstSector = fileDirectory[i][27] << 8 | fileDirectory[i][26];
                fileSz += fileDirectory[i][31] << 24;
                fileSz += fileDirectory[i][30] << 16;
                fileSz += fileDirectory[i][29] << 8;
                fileSz += fileDirectory[i][28];
            }
        }
    }
    r = 0;
    if(firstSector != 0) {
        unsigned short int next_sector = firstSector;
        while (next_sector < 0xFFF8) {
            sectors[r++] = next_sector;
            next_sector = FAT16[next_sector];

        }
        for (int i = 0; i < fileSz; ++i) {
            unsigned short int currentSector = sectors[i/4];
            int dataIdx = currentSector*4 + i%4;
            std::cout << (unsigned int)data[dataIdx] << ",";

        }
        std::cout << "EOF" << std::endl;
    }
}







