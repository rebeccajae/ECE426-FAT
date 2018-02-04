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
    //Initialize all entries in FAT16 to zero. Blank all storage arrays.
    for (int i = 0; i < 256; i++) {
        FAT16[i] = ZERO_UNUSED; // Set to zero, unused
    }
    for (int i = 0; i < 1024; i++) {
        data[i] = 0x0; // Set to zero, unused
    }
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 32; j++) {
            fileDirectory[i][j] = 0x0;
        }
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
            //Not enough space in the FAT.
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
    unsigned short int firstSector = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            //Check if file exists
            if (fileDirectory[i][j] != filename[j]) {
                break;
            }
            if (j == 7) {
                //There is a file!
                //Delete it.
                fileDirectory[i][0] = 0x0; // Change first character of name to 0
                firstSector = fileDirectory[i][27] << 8 | fileDirectory[i][26];
                unsigned short int next_sector = firstSector;
                unsigned short int temp;
                //Change all entries of its clusters to unused.
                while (next_sector != LAST_CLUSTER) {
                    temp = FAT16[next_sector];
                    FAT16[next_sector] = ZERO_USED;
                    next_sector = temp;

                }
                return true;
            }
        }
    }
    return false;
}

/**
 * Reads a file specified by filename into the array pointed to by fileData.
 * @param filename The filename to read
 * @param fileData An array to store the read data
 * @return true on successful read, false if file doesn't exist.
 */
bool FileDirectory::read(char *filename, char *fileData) {
    unsigned short int firstSector, sectors[256], r;
    firstSector = 0;
    unsigned int fileSz = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++)
        {
            //Check if file exists
            if (fileDirectory[i][j] != filename[j]) {
                break;
            }
            if (j == 7) {
                //There is a file! Get the information from the fileDirectory.
                //Get first cluster address
                firstSector = fileDirectory[i][27] << 8 | fileDirectory[i][26];
                //Read file size
                fileSz += fileDirectory[i][31] << 24;
                fileSz += fileDirectory[i][30] << 16;
                fileSz += fileDirectory[i][29] << 8;
                fileSz += fileDirectory[i][28];
            }
        }
        if(i == 3 && firstSector == 0){
            return false; //File doesn't exist.
        }
    }
    r = 0;
    //Use the cluster addresses to read the data from the disk.
    if(firstSector != 0) {
        unsigned short int next_sector = firstSector;
        while (next_sector != LAST_CLUSTER) {
            sectors[r++] = next_sector;
            next_sector = FAT16[next_sector];

        }
        for (int i = 0; i < fileSz; ++i) {
            unsigned short int currentSector = sectors[i/4];
            int dataIdx = currentSector*4 + i%4;
            fileData[i] = data[dataIdx];

        }
    }
    return true;
}

/**
 * Writes a file to the directory. This does not sanity-check file writes, you should sanity-check using create first.
 * @param filename Name of the file to write
 * @param numberBytes Size of the file to write
 * @param fileData byte array of the file to write
 * @param year Year modified
 * @param month Month Modified
 * @param day Day Modified
 * @param hour Hour Modified
 * @param minute Minute Modified
 * @param second Second Modified
 * @return true on successful write.
 */
bool FileDirectory::write(char filename[], int numberBytes, char fileData[], int year, int month, int day, int hour, int minute, int second)
{
    unsigned char record[32] = { 0 };
    unsigned short int i, j, sectorAddress, unused[256], time, date;
    //Generate FAT Record template
    date = 0;
    time = 0;
    record[31] = numberBytes >> 24;
    record[30] = numberBytes  >> 16;
    record[29] = numberBytes >> 8;
    record[28] = numberBytes;
    date += ((year - 1980) << 9);
    date += month << 5;
    date += day;
    record[25] = date >> 8;
    record[24] = date;
    time += (hour << 11);
    time += (minute << 5);
    time += (second / 2);
    record[23] = time >> 8;
    record[22] = time;
    for (int l = 0; l < 8; l++)
    {

        record[l] = filename[l];
    }
    //Find the first unused entry in the FAT.
    for (i = 2; i < 256; i++)
    {
        if (FAT16[i] == ZERO_USED || FAT16[i] == ZERO_UNUSED) {
            break;
        }
    }

    sectorAddress = i;
    record[27] = sectorAddress >> 8;
    record[26] = sectorAddress;

    //Write temp record into fileDirectory
    for (int k = 0; k < 4; k++) {
        if(fileDirectory[k][0] == 0){
            for (int l = 0; l < 32; ++l) {tha
                fileDirectory[k][l] = record[l];
            }
            break;
        }
    }
    j = 1;
    //Find and traverse all unused clusters.
    unused[0] = i;
    for (i = unused[0]+1; i < 256 && j*CLUSTER_SIZE <= numberBytes; i++) {
        if (FAT16[i] == ZERO_USED || FAT16[i] == ZERO_UNUSED) {
            unused[j++] = i;
        }
    }
    //Link the clusters you want to use
    for (i = 0; i < j-2; i++) {
        FAT16[unused[i]] = unused[i + 1];
    }
    //Set a EOF on the last cluster.
    FAT16[unused[i]] = LAST_CLUSTER;

    //Write data into the data array
    for (int m = 0; m <= i+1; ++m) {
        int currentSector = unused[m];
        for (int k = 0; k < 4; ++k) {
            data[currentSector*4 + k] = fileData[m*4 + k];
        }
    }
    return true;
}

/**
 * Prints all clusters occupied by a file specified in filename
 * @param filename The filename to print clusters of
 */

//TODO: the project requirement says use type void, but also mentions returns.
void FileDirectory::printClusters(char filename[])
{
    unsigned short int firstSector = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++)
        {
            //Check if file exists in directory
            if (fileDirectory[i][j] != filename[j]) {
                break;
            }
            if (j == 7) {
                //There is a file! Get the first sector information from the directory.
                firstSector = fileDirectory[i][27] << 8 | fileDirectory[i][26];

            }
        }
    }
    //Iterate through the sectors and print them.
    if(firstSector != 0) {
        unsigned short int next_sector = firstSector;
        while (next_sector < 0xFFF8) {
            std::cout << next_sector << "->";
            next_sector = FAT16[next_sector];

        }
        std::cout << "EOF" << std::endl;
    }
}



/**
 * Prints the directory contents of the current object, including filenames, dates modified and cluster information.
 */
void FileDirectory::printDirectory() {
    char filename[8];
    for (int i = 0; i < 4; i++) {
        if(fileDirectory[i][0] != 0){
            //Read one filename from the directory
            for (int j = 0; j < 8; ++j) {
                filename[j] = fileDirectory[i][j];

            }
            //Get the other information about the file from the directory.
            unsigned int fileSz, hour, minute, second, year, month, day, time, date;
            time = 0;
            date = 0;
            fileSz = 0;

            date += fileDirectory[i][25] << 8;
            date += fileDirectory[i][24];

            day = (date & 0x1F);
            month = ((date >> 5) & 0xF);
            year = ((date >> 9) & 0x7F) + 1980;

            time += fileDirectory[i][23] << 8;
            time += fileDirectory[i][22];

            second = (time & 0x1F)*2;
            minute = ((time >> 5) & 0x3F);
            hour = ((time >> 11) & 0x1F);


            fileSz += fileDirectory[i][31] << 24;
            fileSz += fileDirectory[i][30] << 16;
            fileSz += fileDirectory[i][29] << 8;
            fileSz += fileDirectory[i][28];
            //Print it
            std::cout << filename << " - " << month << "/" << day << "/" << year << " - " << hour << ":" << minute << ":" << second << std::endl;
            std::cout << fileSz << " bytes" << std::endl;
            this->printClusters(filename);
        }

    }
}

/**
 * Prints the data stored within filename
 * @param filename The filename of interest.
 */
void FileDirectory::printData(char *filename) {
    unsigned short int firstSector, sectors[256], r;
    unsigned int fileSz;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++)
        {
            //Check if file exists
            if (fileDirectory[i][j] != filename[j]) {
                break;
            }
            if (j == 7) {
                //There is a file! Get the size and first cluster
                firstSector = fileDirectory[i][27] << 8 | fileDirectory[i][26];
                fileSz += fileDirectory[i][31] << 24;
                fileSz += fileDirectory[i][30] << 16;
                fileSz += fileDirectory[i][29] << 8;
                fileSz += fileDirectory[i][28];
            }
        }
    }
    r = 0;
    //Iterate through the sectors and print the data out.
    if(firstSector != 0) {
        unsigned short int next_sector = firstSector;
        while (next_sector != LAST_CLUSTER) {
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

