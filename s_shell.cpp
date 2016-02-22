#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string>

using namespace std;

string location="root@sara / $ ";
int file_system_size = 100*1024*1024;
int inode_max_number_of_files = 1024;
int n_inodes = 0;
int block_siz = 2048;

FILE* f_open(int truncate=0)
{
    FILE *fp;
    if (truncate)
    {
        fp = fopen("/home/sara/s_shell/my_filesys.bin", "wb");
    }
    else
    {
        fp = fopen("/home/sara/s_shell/my_filesys.bin", "rb+");
    }
    return fp;
}

void add_to_inode_table(short inode_number, int offset, char type)
{
    
    FILE * fp = f_open();
   // fp.open("my_filesys.bin", ios::in| ios::out | ios::binary | ios::ate);
//    fp.seekp(n_inodes*8 , ios::beg);
    fseek(fp, n_inodes * 8 , SEEK_SET);
    fwrite((char *)&inode_number, sizeof(short), 1, fp);
    fwrite((char *)&offset, sizeof(int), 1, fp);
    fwrite((char *)&type, sizeof(char), 1, fp);
    // fp.write((char *) &offset, sizeof(offset));
    // fp.write((char *) &type, sizeof(type));
    n_inodes += 1;
    fclose(fp);
}

void make_a_header(string filename, short inode_number)
{
    FILE * fp = f_open();
    fseek(fp, inode_max_number_of_files * 8, SEEK_SET);
    fwrite(filename.c_str(), sizeof(char), sizeof(filename), fp);
    fwrite(&inode_number, sizeof(inode_number), 1, fp);
    cout << "sara!!";
    fclose(fp);
}

void mkfs()
{
    /*ofstream fp;
    fp.open("my_filesys.bin", ios::out |ios::in| ios::binary | ios::ate); 
    fp.seekp(file_system_size - 1, ios::beg);
    fp<<'0';
    fp.close();
    make_a_header("/",1);*/

    FILE *fp = f_open(1);
    fseek(fp, file_system_size-1, SEEK_SET);
    char c = '0';
    fwrite(&c, sizeof(char), 1, fp);
    short inode_number = 1;
    fclose(fp);


    make_a_header("/", inode_number);
    add_to_inode_table(1,8*inode_max_number_of_files,'d');
    cout << "My File System!" << "\n";
    cout << location;
}

void display()
{  
    string more_command = "";
    string last_command = "exit";
    bool is_more_command = true;
    while (is_more_command)
    {
        cin >> more_command;
        if(last_command.compare(more_command) == 0)
           is_more_command = false;
        else
            cout << location;
    }
}



void open_file(string filename, int flag)
{
    
}


int main()
{
    mkfs();
    display();
    return 0;
}

