#include <iostream>
#include <fstream>
#include <stdio.h>

using namespace std;

void config(int fs_size)
{
    ofstream fp;
    fp.open("my_filesys.bin",ios::out |ios::binary); 
    fp.seekp(fs_size*1024*1024-1 , ios::beg);
    fp<<'0';
    fp.close();
}
int main()
{
    config(100);
    return 0;
}

