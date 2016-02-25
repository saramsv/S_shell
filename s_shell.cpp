#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string>
#include <vector>
#include <sstream>

using namespace std;

string location="root@sara / $ ";
int file_system_size = 100*1024*1024;
int inode_max_number_of_files = 1024;
int n_inodes = 0;
short inode_table_row_size = 8; //2B for inode_number, 4 for offset and 1 for type
short directory_table_row_size = 258;//256B for file names and 2B for inode numbers which are short int
int block_size = 1*1024;
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
    cout<<" added in inode_table and n_inodes is: "<<n_inodes<<endl;
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
            address += inode_table_row_size;
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
        address += 258;
        fseek(fp , address, SEEK_SET);
        fread(&inode_number,2 ,1 ,fp);
    }
    fclose(fp);
    cout <<"request a inode is: "<< address-256 <<endl;
    return address-256;
}

void add_in_directory_table(string name , short inode_number, short inode_id)
{
    FILE * fp = f_open();
    int addr = request_a_inode(inode_id); // for the parent directory
    //cout <<"first addr"<<addr<<endl;
    fseek(fp, addr, SEEK_SET);
    fwrite(name.c_str(), sizeof(char), sizeof(name), fp); 
    addr += 256;
    //cout <<"second addr"<<addr<<endl;
    fseek(fp, addr, SEEK_SET);
    fwrite(&inode_number, sizeof(short),1 ,fp); //adding a new inode_number
    cout <<"added in dir table " <<endl;

/*    addr-=256;
    fseek(fp, addr, SEEK_SET);
    char buffer[256];
    fread(buffer, 256,1,fp);
    cout<<"address"<<addr<<endl<<"buffer: "<< buffer<<endl;*/
    fclose(fp);
}

int request_a_block()
{
    FILE * fp = f_open();
    int address = inode_max_number_of_files * inode_table_row_size + 4; // 4 is for the "next  address" in the header.

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
    cout <<"request a block is: "<< address <<endl;
    return address;
}

void make_a_header(int next_add, string filename)
{
    FILE * fp = f_open();
    int addr = request_a_block();
    fseek(fp, addr, SEEK_SET);
    fwrite(&next_add, sizeof(int), 1, fp);
    fwrite(filename.c_str(), sizeof(char), sizeof(filename), fp);
    cout <<"header  "<< filename<<" has been made in " <<next_add<<endl;
    fclose(fp);
}
void mkfs()
{
    FILE *fp = f_open(1);
    char c = 0;
    fseek(fp, file_system_size - 1, SEEK_SET);
    fwrite(&c, sizeof(char), 1, fp);
    fclose(fp);

    int next_block_address = 0;
    string current_directory_name = "/";
    string parent_directory_name = "..";
    short parent_inode = create_a_new_inode_number();
    current_inode = parent_inode;

    add_to_inode_table(current_inode, inode_table_row_size * inode_max_number_of_files, 'd');
   // cout<<"after inode table"<<endl;
    make_a_header(next_block_address, current_directory_name);
   // cout<<"after header"<<endl;
    add_in_directory_table(current_directory_name, current_inode, current_inode);
    //cout<<"after directory table"<<endl;
    add_in_directory_table(parent_directory_name, parent_inode, current_inode);
    //cout<<"after dir2 table"<<endl;
}


int find_the_directory_table(short inode_id)
{
    FILE * fp = f_open();
    int inode_table_size = inode_max_number_of_files *inode_table_row_size;
    int address = 0;
    short inode_number = 0;
    fseek(fp ,address , SEEK_SET);
    fread(&inode_number, 2, 1, fp);
    if(inode_number == inode_id)
    {
        fread(&address, 4 , 1, fp);
    }
    else
    {
        while(inode_number != inode_id && address<inode_table_size)
        {
            address += inode_table_row_size;
            fseek(fp , address,SEEK_SET);
            fread(&inode_number, 2, 1, fp);
        }
        fread(&address, 4, 1, fp);
    }
    cout <<"braye in file bayad toye in addr begarde: "<< address <<endl;
    return address;
}

void find_the_file(int &addr , short &inode_id, string filename)
{
    FILE * fp = f_open();
    int address = addr;
    fseek(fp, address , SEEK_SET);
    bool exist = false;
    char name[256];
    fread(&addr, 4, 1, fp);
    int max_addr = address + block_size;
    fseek(fp, addr + address + header_size , SEEK_SET);
    while(!exist && address < max_addr)
    {
        fread(name, 256, 1, fp);
        string str(name);
        if(str.compare(filename)==0)
        {
            exist = true;
            fread(&inode_id , 2, 1, fp);
           // return 0;        
        }
        address += directory_table_row_size;

    }
   // return 0;
}

short finding_file(string filename)
{
    int next_addr = 0;
    short inode_number = 0;
    int dir_table_addr = find_the_directory_table(current_inode);
    char name[256];
    int max_addr = dir_table_addr + block_size;
    FILE * fp = f_open();
    fseek(fp, dir_table_addr , SEEK_SET);
    fread(&next_addr, 4, 1, fp);
    fseek(fp, 256 , SEEK_CUR);
    fread(name, 256, 1, fp);
    string str(name);
    cout<<"name= "<<str<<endl;
    cout<<"NA= "<<next_addr<<endl;
    if (str.compare(filename)==0)
    {
        fread(&inode_number , 2, 1, fp);
        return inode_number;        
    }
    else
    {
        bool exist = false;
        if (next_addr==0)
        {
            while(!exist && dir_table_addr<max_addr)
            {
                //cout<<"raft to while"<<endl;
                fseek(fp, 2 , SEEK_CUR);
                fread(name, 256, 1, fp);
                string str(name);
                if(str.compare(filename)==0)
                {
                    exist = true;
                    fread(&inode_number , 2, 1, fp);
                    return inode_number;        

                }
                else
                {
                    dir_table_addr += directory_table_row_size;
                }

            }
        }
        else
        {
           find_the_file(next_addr ,inode_number, filename);
           while (inode_number==0 && next_addr!=0)
           {
               find_the_file(next_addr ,inode_number, filename);
           }
        }
    }
    cout <<"inode hasel az search baraye file: "<< filename <<"ine: " <<inode_number<<endl;
    return inode_number;
}

void open(string filename , char flag)
{
    int fd = finding_file(filename);
    cout<<"fd= "<<fd<<endl;
    if(fd == 0)
    {
        short inode_number = create_a_new_inode_number();
        cout<<"inode numbere jadid: "<< inode_number<<endl;
        add_to_inode_table(inode_number, request_a_block(), 'f');
        make_a_header(0, filename);
        add_in_directory_table(filename ,inode_number, current_inode);
        cout <<"SUCCESS fd = "<<fd;
    }
    else
    {
        cout <<"SUCCESS fd = "<<fd;
    }
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
        {
            string token , line(more_command);
            istringstream iss(line);
            vector<string> tokens;
            while(getline(iss, token , ' '))
            {
                tokens.push_back(token);
            }
            if(tokens[0].compare("open")==0)
            {
                open(tokens[1], 'r');
            }
            /*else if(tokens[0].compare("read")==0)
            {
                read(tokens[1], 'r');
            }*/
            if(tokens[0].compare("mkfs")==0)
            {
                mkfs();
            }
            /*switch(more_command)
            {
                string command = tokens[0];
                if(command.compare("open")==0)
                {
                    more_command = "open";
                }
                else if(command.compare("read")==0)
                {
                    more_command = "read";
                }
                case open: open(tokens[1], 'r'); break;
                case read: read(); break;
                case mkfs: mkfs(); break;
            }*/
            cout << location;
        }
    }
}
int main()
{
    //mkfs();
    display();
    //cout<<"next free block: "<<request_a_block()<<"\n";
    //cout<<"next free node in Inode table: "<<request_a_inode(1)<<"\n";
    //cout<<find_the_directory_table(1);
    //open("test", 'r');
    //open ("test2", 'w');
    return 0;
}
