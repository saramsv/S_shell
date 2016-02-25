#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string>

using namespace std;

string location="root@sara / $ ";
int file_system_size = 100*1024*1024;
int inode_max_number_of_files = 1024;
int n_inodes = 0;
short inode_table_row_size = 8; //2B for inode_number, 4 for offset and 1 for type
short directory_table_row_size = 258;//256B for file names and 2B for inode numbers which are short int
int block_size = 32*1024;
int header_size= 260; //256B for file names and 4B for address
short current_inode = 0;

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

short create_a_new_inode_number()
{
    return n_inodes+1;
}


void add_to_inode_table(short inode_number, int offset, char type)
{
    
    FILE * fp = f_open();
    fseek(fp, n_inodes * inode_table_row_size , SEEK_SET);
    fwrite((char *)&inode_number, sizeof(short), 1, fp);
    fwrite((char *)&offset, sizeof(int), 1, fp);
    fwrite((char *)&type, sizeof(char), 1, fp);
    n_inodes += 1;
    fclose(fp);
}



int request_a_inode(short inode_id)
{
    FILE * fp = f_open();
    int inode_table_size = inode_max_number_of_files *inode_table_row_size;
    int address = 0;
    short inode_number = 0;
    fseek(fp ,address , SEEK_SET);
    fread(&inode_number, 2, 1, fp);
    //cout <<"found inode number is: "<<inode_number;
    if(inode_number == inode_id)
    {
        fread(&address, 4 , 1, fp);
    }
    else
    {
        while(inode_number != inode_id && address<inode_table_size)
        {
            address += inode_table_row_size-2;
            fseek(fp , address,SEEK_SET);
            fread(&inode_number, 2, 1, fp);
        }
        fread(&address, 4, 1, fp);

    }
    //cout<<"first address:"<<address<<"\n";
    //fread(&address, 4 , 1, fp);
    address += header_size + 256;
    fseek(fp , address , SEEK_SET);
    fread(&inode_number,2 ,1 ,fp);
    //cout <<"second ound inode number is: "<<inode_number;
    while (inode_number!=0)
    {
        
        //cout<<"second address:"<<address<<"\n";
        address += 256;
        fseek(fp , address, SEEK_SET);
        fread(&inode_number,2 ,1 ,fp);
    }
    fclose(fp);
    return address-256;
}

void add_in_directory_table(string name , short inode_number, short inode_id)
{
    FILE * fp = f_open();
    int addr = request_a_inode(inode_id);
    cout <<"first addr"<<addr<<endl;
    fseek(fp, addr, SEEK_SET);
    fwrite(name.c_str(), sizeof(char), sizeof(name), fp); //sizeof(name), fp);
    addr += 256;
    cout <<"second addr"<<addr<<endl;
    fseek(fp, addr, SEEK_SET);
    fwrite(&inode_number, sizeof(short),1 ,fp);

    addr-=256;
    fseek(fp, addr, SEEK_SET);
    char buffer[256];
    fread(buffer, 256,1,fp);
    cout<<"address"<<addr<<endl<<"buffer: "<< buffer<<endl;
    fclose(fp);
}

int request_a_block()
{
    FILE * fp = f_open();
    int address = inode_max_number_of_files * inode_table_row_size + 4; // 4 is for the "next  address" in the header.
    //how to read an int
    /*fseek(fp, address - 4, SEEK_SET);
    int test;
    fread(&test, sizeof(int), 1, fp);
    cout<<"Test: "<<test<<endl;*/

    fseek(fp, address, SEEK_SET);
    bool is_block_empty = false;
    char buffer[256];
    fread(buffer, 256, 1, fp);
    cout<<"buffer[0]: "<<buffer[0]<<buffer[1]<<buffer[2]<<buffer[3]<<endl;
    if(buffer[0]==0)
    {
        cout<< "raft to if"<<endl;
        is_block_empty = true;
        address -=4;
    }
    else
    {
        while(is_block_empty==false)
        {
            address= address+block_size;
            fseek(fp,address,SEEK_SET);
            fread(buffer, 256, 1, fp);
            if(buffer[0]==0)
            {
                is_block_empty = true;
                address -=4;
            }
        }
    }
    fclose(fp);
    return address;
}

void make_a_header(int next_add, string filename)
{
    FILE * fp = f_open();
    int addr = request_a_block();
    fseek(fp, addr, SEEK_SET);
    fwrite(&next_add, sizeof(int), 1, fp);
    fwrite(filename.c_str(), sizeof(char), sizeof(filename), fp);
    fclose(fp);
}
void mkfs()
{
    FILE *fp = f_open(1);
    char c = 0;
    fseek(fp, file_system_size - 1, SEEK_SET);
    fwrite(&c, sizeof(char), 1, fp);
    fclose(fp);

    /*int i = 0;
    while(i < file_system_size)
    {
        //fseek(fp, 1, SEEK_CUR);
        char c = 0;
        fwrite(&c, sizeof(char), 1, fp);
        i++;
    }*/

    int next_block_address = 0;
    string current_directory_name = "/";
    string parent_directory_name = "..";
    short parent_inode = create_a_new_inode_number();
    current_inode = parent_inode;

    add_to_inode_table(current_inode, inode_table_row_size * inode_max_number_of_files, 'd');
    cout<<"after inode table"<<endl;
    make_a_header(next_block_address, current_directory_name);
    cout<<"after header"<<endl;
    add_in_directory_table(current_directory_name, current_inode, current_inode);
    cout<<"after directory table"<<endl;
    add_in_directory_table(parent_directory_name, parent_inode, current_inode);
    cout<<"after dir2 table"<<endl;
}

void display()
{  
    cout << "My File System!" << "\n";
    cout << location;
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
    /*display();
    cout<<"next free block: "<<request_a_block()<<"\n";
    cout<<"next free node in Inode table: "<<request_a_inode(1)<<"\n";*/
    return 0;
}
