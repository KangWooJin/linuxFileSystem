#ifndef VALUE_H
#define VALUE_H

#include <string>
#include <time.h>
using namespace std;

const int NOT_FOUND = -1;

const int NUM_BLOCKS = 100;
const int BLOCK_SIZE = 4096;
const int ENTRYSIZE = 261;


time_t time_now;
struct tm *tm; 

unsigned char FS_Image[NUM_BLOCKS*BLOCK_SIZE];

unsigned char FS_Buffer[BLOCK_SIZE];
unsigned char My_Buffer[BLOCK_SIZE];
int MyBufferCount = 0;

/////////////
// SUPERBLOCK
char inodeSize[] = "128";
const int INODE_SIZE = 128;
const int INODE_BLOCK_COUNT = 2;


char FINISHDATE[] = { "2016-06-06-14-12" };
const char FIRSTINODE = '/';
/////////////

const int SUPER_BLOCK = 0;
const int BLOCK_DESCIPTOR_TABLE = 1;
const int DATA_BLOCKS = 6;

///////////////
/*BLOCK_DESCRIPTOR_TABLE*/
const int BLOCK_BITMAP = 2; // 0
const int INODE_BITMAP = 3; // 4
const int INODE_BLOCKS = 4; // 8
int Unassigned_Blocks = 100 - 6; //12
int Unassigned_Inodes = 64 ; // 16
///////////////

///////////////
/*SUPER_BLOCK*/
char MagicNumber[] = "20141125";
char BlockSize[] = "4KB";
char BlocksPerGroup[] = "100";
char numberOfInodes[] = "64";


const int NUMBEROFINODES = (INODE_BLOCK_COUNT * BLOCK_SIZE) / INODE_SIZE; //64
///////////////

///////////////
/*BlockBitmap&InodeBitmap*/
char BlockBitmap[NUM_BLOCKS] = { 0, };
char InodeBitmap[NUMBEROFINODES] = { 0, };
///////////////


///////////////
/*Inode Blocks*/

class Inode
{
public :
	char mode[10]; //0
	int size ;  //10
	char time[5];// = {} //14
	char ctime[5]; //19
	char mtime[5]; //24
	int links_count ; //29
	int blocks ; //33
	int block[22];// = blockIndex; //37
	//128-37
};

class Entry
{
public :
	char flag[1]; //0
	int inodeNumber; //1
	char nameLength; //5
	char name[255]; //6
};

class EntryList
{
public :
	Entry entry[NUMBEROFINODES];
	int entryCount;
	
};

Inode InodeList[NUMBEROFINODES];


///////////////

///////////////
/*Table*/
int FileDescriptorTable[100];


class SystemFile
{
public :
	int inode_number;
	int file_pointer;
};

SystemFile SystemFileTable[100];


class FileInode
{
public :
	int inode_number;
	int inode_index;
	Inode inode_data;

};

FileInode inodeTable[NUMBEROFINODES];

///////////////

class pathInfo
{
public:
	int path_Inode;
	string path_Name;
};

pathInfo stack[100];

int pathIndex = 0;


#endif