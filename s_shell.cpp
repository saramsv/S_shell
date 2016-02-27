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
    if(n_inodes * inode_table_row_size > inode_table_row_size * inode_max_number_of_files)
    {
        cout<<"OOPS, This file system can not suport more inodes!"<<endl;
        return;
    }
    fwrite((char *)&inode_number, sizeof(short), 1, fp);
    fwrite((char *)&offset, sizeof(int), 1, fp);
    fwrite((char *)&type, sizeof(char), 1, fp);
    n_inodes += 1;
    cout<<" added in inode_table and n_inodes is: "<<n_inodes<<endl;
    fclose(fp);
}

int find_offset(short inode_id)
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
        fread(&address, sizeof(int) , 1, fp);
        return address;
    }
    else
    {
        while(inode_number != inode_id && address<inode_table_size)
        {
            address += inode_table_row_size;
            fseek(fp , address,SEEK_SET);
            fread(&inode_number, sizeof(short), 1, fp);
        }
        fread(&address, sizeof(int), 1, fp);
        return address;
    }
    return 0;

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

void make_a_header(int next_add, string filename, int addr)
{
    FILE * fp = f_open();
    fseek(fp, addr, SEEK_SET);
    fwrite(&next_add, sizeof(int), 1, fp);
    fwrite(filename.c_str(), sizeof(char), sizeof(filename), fp);
    cout <<"header  "<< filename<<" has been made in " <<addr<<endl;
    fclose(fp);
}

int request_a_inode()
{
    FILE * fp = f_open();
    int max_addr = 0;
    int address = 0;
    int next_addr = 0;
    char name[256];
    short inode_number;
    address = find_offset(current_inode);
    if(address == 0)
    {
        cout<<"There is no such directory"<<endl;
        return 0;
    }
    max_addr = address + block_size;
    fseek(fp , address , SEEK_SET);
    fread(&next_addr, sizeof(int), 1, fp);
    fread(name , 256, 1, fp);
    address += header_size + 256;
    fseek(fp , address , SEEK_SET);
    fread(&inode_number,sizeof(short) ,1 ,fp);
    if (inode_number==0)
    {
        return address - 256;
    }
    else
    {
        while (inode_number!=0 && address < max_addr)
        {
            address += 258;
            fseek(fp , address, SEEK_SET);
            fread(&inode_number,sizeof(short) ,1 ,fp);
            if(inode_number == 0)
            {
                return address - 256;
            }
            else if(inode_number!=0 && address >= max_addr && next_addr == 0)
            {
                address = request_a_block();
                string str(name);
                make_a_header(next_addr, str, address);
                address += header_size;
                return address;
            }
            else if(inode_number!=0 && address >= max_addr && next_addr != 0)
            {
                address = next_addr;
                fseek(fp , address, SEEK_SET);
                fread(&next_addr, sizeof(int), 1, fp);
                max_addr = address + block_size;
                address += header_size + 256;
            }
        }
    }
    fclose(fp);
    return address - 256;
}

void add_in_directory_table(string name , short inode_number)
{
    FILE * fp = f_open();
    int addr = request_a_inode(); // for the parent directory
    fseek(fp, addr, SEEK_SET);
    fwrite(name.c_str(), sizeof(char), sizeof(name), fp); 
    addr += 256;
    fseek(fp, addr, SEEK_SET);
    fwrite(&inode_number, sizeof(short),1 ,fp); //adding a new inode_number
    cout <<"added in dir table " <<endl;

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

    add_to_inode_table(current_inode, inode_table_row_size * inode_max_number_of_files, 'd'); // parameters are inode number, offset and type
   int addr = request_a_block();
    make_a_header(next_block_address, current_directory_name, addr);
   // cout<<"after header"<<endl;
    add_in_directory_table(current_directory_name, current_inode);
    //cout<<"after directory table"<<endl;
    add_in_directory_table(parent_directory_name, parent_inode);
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
    fclose(fp);
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
        int addr = request_a_block();
        make_a_header(0, filename, addr);
        add_in_directory_table(filename ,inode_number);
        cout <<"SUCCESS fd = "<<fd;
    }
    else
    {
        cout <<"SUCCESS fd = "<<fd;
    }
}

void LS()
{
    FILE * fp = f_open();
    int addr = find_the_directory_table(current_inode);
    cout<<addr <<endl;
    char name[256];
    fseek(fp, addr, SEEK_SET);
    int next_addr = 0;
    int max_addr = addr + block_size;
    short inode_number = 0;
    fread(&next_addr , sizeof(int), 1, fp);
    fseek(fp,  header_size - sizeof(int), SEEK_CUR);
    fread(name, 256, 1, fp);
    fread(&inode_number , sizeof(short), 1, fp);
    if(inode_number == 0)
    {
        return;
    }
    else
    {
        cout<<name<<"    ";
        while(addr < max_addr && inode_number !=0)
        {
            fread(name , 256, 1, fp);
            fread(&inode_number , sizeof(short), 1, fp);
            if (inode_number != 0)
            {
                cout<<name<<"    ";
            }
            addr += directory_table_row_size;
            if(addr >= max_addr && next_addr == 0)
            {
                return;
            }
            else if(addr >= max_addr && next_addr != 0)
            {
                addr = next_addr;
                max_addr = addr + block_size;
                fseek(fp, addr, SEEK_SET);
                fread(&next_addr , sizeof(int), 1, fp);
                fseek(fp,  header_size - sizeof(int), SEEK_CUR);
                fread(name, 256, 1, fp);
                fread(&inode_number , sizeof(short), 1, fp);
                if (inode_number != 0)
                {
                    cout<<name<<"    ";
                }
                addr += directory_table_row_size;
            }
        }
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
        getline(cin, more_command);
        if(last_command.compare(more_command) == 0)
           is_more_command = false;
        else
        {
            cout<<"more comand: "<<more_command<<endl;
            string token , line(more_command);
            istringstream iss(line);
            vector<string> tokens;
            int i=0;
            while(getline(iss, token , ' '))
            {
                tokens.push_back(token);
                cout<<"tokens: "<<tokens[i];
                i++;
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
            if(tokens[0].compare("ls")==0)
            {
                LS();
            }
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
