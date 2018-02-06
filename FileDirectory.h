//
// Created by becca on 2/3/18.
//


class FileDirectory {
private:
    unsigned char fileDirectory[4][32]; // 4 file records, each 32 bits long.
    unsigned short int FAT16[256]; // 256 clusters
    unsigned char data[1024]; // 1024 bytes storage, thus 4 bytes per cluster
public:
    FileDirectory();
    bool create(char filename[], int numberBytes);
    bool deleteFile(char filename[]);
    bool read(char filename[], char fileData[]);
    bool write(char filename[], int numberBytes, char fileData[], int year, int month, int day, int hour, int minute, int second);
    void printClusters(char filename[]);
    void printDirectory();
    void printData(char filename[]);
    bool compare(char file1[], char file2[]);
};
