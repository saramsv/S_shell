#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string>

using namespace std;

string location="root@sara / $ ";
int file_system_size = 100*1024*1024;
int inode_max_number_of_files = 1024;
int n_inodes = 0;


void add_to_inode_table(short inode_number, int offset, char type)
{
    
    ofstream fp;
    fp.open("my_filesys.bin", ios::in| ios::out | ios::binary | ios::ate);
    fp.seekp(n_inodes*8 , ios::beg);
    fp.write((char *)&inode_number,sizeof(inode_number));
    fp.write((char *)&offset,sizeof(offset));
    fp.write((char *)&type,sizeof(type));
    n_inodes += 1;
    fp.close();
}

void make_a_header(string filename, short inode_number)
{
    ofstream fp("my_filesys.bin", ios::out | ios::in | ios::binary | ios::ate); 
    fp.seekp(inode_max_number_of_files * 8, ios::beg);
    fp.write(filename.c_str(), filename.size());
    fp.write((char *)&inode_number, sizeof(inode_number));
    fp.close();
}

void mkfs()
{
    ofstream fp;
    fp.open("my_filesys.bin", ios::out |ios::in| ios::binary | ios::ate); 
    fp.seekp(file_system_size - 1, ios::beg);
    fp<<'0';
    fp.close();
    make_a_header("/",1);
    add_to_inode_table(1,8*inode_max_number_of_files,'d');
    cout<<"My File System!"<<"\n";
    cout<<location;
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

