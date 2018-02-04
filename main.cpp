#include <iostream>
#include "FileDirectory.h"

int main() {
    FileDirectory *fd = new FileDirectory();
    char file1name[8] = {'f','i','l','e','1',0,0,0};
    char file1[40]= { 1 };
    char file2name[8] = {'f','i','l','e','2',0,0,0};
    char file2[200]= { 1 };
    char file3name[8] = {'f','i','l','e','3',0,0,0};
    char file3[300]= { 1 };
    char file4name[8] = {'f','i','l','e','4',0,0,0};
    char file4[500]= { 1 };
    bool createF1 = fd->create(file1name, 40);
    if(createF1){
        std::cout << "Writing file1" << std::endl;
        fd->write(file1name, 40, file1, 2018-1980, 2, 1, 0, 0, 0);
    }else{
        std::cout << "Create file1 failed." << std::endl;
    }
    fd->printData(file1name);

    bool createF2 = fd->create(file2name, 200);
    if(createF2){
        std::cout << "Writing file2" << std::endl;
        fd->write(file2name, 200, file2, 2018-1980, 2, 1, 0, 0, 0);
    }else{
        std::cout << "Create file2 failed." << std::endl;
    }

    bool createF3 = fd->create(file3name, 300);
    if(createF3){
        std::cout << "Writing file1" << std::endl;
        fd->write(file3name, 300, file3, 2018-1980, 2, 1, 0, 0, 0);
    }else{
        std::cout << "Create file3 failed." << std::endl;
    }

    bool createF4 = fd->create(file4name, 500);
    if(createF4){
        std::cout << "Writing file4" << std::endl;
        fd->write(file4name, 500, file4, 2018-1980, 2, 1, 0, 0, 0);
    }else{
        std::cout << "Create file4 failed." << std::endl;
    }

    bool deleteF2 = fd->deleteFile(file2name);
    if(deleteF2){
        std::cout << "Deleted file2" << std::endl;
    }else{
        std::cout << "Failed to delete file2" << std::endl;
    }

    bool createF4_2 = fd->create(file4name, 500);
    if(createF4_2){
        std::cout << "Writing file4" << std::endl;
        fd->write(file4name, 500, file4, 2018-1980, 2, 1, 0, 0, 0);
    }else{
        std::cout << "Create file4 failed." << std::endl;
    }
fd->printDirectory();

    return 0;
}