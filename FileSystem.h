#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "Value.h"
#include <iostream>
#include <string.h>
#include <time.h>
#include <algorithm>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
using namespace std;

//Buffer
void fflush_Buffer()
{
	for (int i = 0; i < BLOCK_SIZE; ++i)
	{
		FS_Buffer[i] = 0;
	}
}

//Buffer
void fflush_MyBuffer()
{
	for (int i = 0; i < BLOCK_SIZE; ++i)
	{
		My_Buffer[i] = 0;
	}
	MyBufferCount = 0;
}



//File System
void FS_Init()
{
	for (int i = 0; i < NUM_BLOCKS*BLOCK_SIZE; ++i)
		FS_Image[i] = 0;

	for (int i = 0; i < 100; ++i)
	{
		SystemFileTable[i].file_pointer = -1;
		SystemFileTable[i].inode_number = -1;
		FileDescriptorTable[i] = -1;

	}

	for (int i = 0; i < 64; ++i)
	{
		inodeTable[i].inode_number = -1;
	}

	stack[0].path_Inode = -1;
	stack[0].path_Name = "#";
	stack[1].path_Inode = 0;
	stack[1].path_Name = "/";
	stack[2].path_Inode = 4;
	stack[2].path_Name = "home";

	pathIndex = 2;
}

void FS_Write(int block)
{
	//FS_Image에 4096*block-1에 접근하여 buffer를 저장한다.
	int count = 0;
	for (int i = block*BLOCK_SIZE; i < (block + 1)*BLOCK_SIZE; ++i)
		FS_Image[i] = FS_Buffer[count++];

	//memset(FS_Buffer, 0, sizeof(FS_Buffer));
	fflush_Buffer();
}

void FS_Read(int block)
{
	//FS_Image에 4096*block-1에 접근하여 buffer에 저장한다.
	int count = 0;
	for (int i = block*BLOCK_SIZE; i<(block + 1)*BLOCK_SIZE; ++i)
		FS_Buffer[count++] = FS_Image[i];
}



//Buffer <-> Data
void copyDataToBuffer(char* data, int size_Data, int startIndex)
{
	int index = 0;
	for (int i = startIndex; i < size_Data+startIndex; ++i)
	{
		FS_Buffer[i] = data[index++];
	}
}

void copyBufferToData(int size_Data, int startIndex, char* data)
{
	int index = 0;
	for (int i = startIndex; i < size_Data + startIndex; ++i)
	{
		data[index++] = FS_Buffer[i];
	}
	data[index] = '\0';
}
void copyDataToMyBuffer(char* data, int size_Data, int startIndex)
{
	int index = 0;
	for (int i = startIndex; i < size_Data + startIndex; ++i)
	{
		My_Buffer[i] = data[index++];
	}
}

void copyMyBufferToData(int size_Data, int startIndex, char* data)
{
	int index = 0;
	for (int i = startIndex; i < size_Data + startIndex; ++i)
	{
		data[index++] = My_Buffer[i];
	}
	data[index] = '\0';
}

//Char <-> Int
void intToChar(int startIndex, int num)
{
	unsigned char str[33];

	for (int i = 0; i < 32; ++i)
	{
		str[i] = '0';
	}

	//2진수로 변환
	int index = 0;
	while (num)
	{
		str[index++] = (num % 2) + '0';
		num = num / 2;
	}

	//10진수로변환 8개씩끊어서
	int count = 0;
	for (int i = 0; i < 32; ++i)
	{
		//cout << str[i];
		count++;

		if (count % 8 == 0)
		{
			count = 0;
			//cout << ' ';
		}
	}


	//저장

	for (int j = 0; j < 32; j += 8)
	{
		int sum = 0;
		int c = 1;

		for (int i = j; i < j + 8; ++i)
		{
			sum += (str[i] - '0') * c;
			c *= 2;
		}

		//cout << '\n' << sum << '\n';
		FS_Buffer[startIndex++] = sum;
	}

	//확인
	for (int i = startIndex; i < startIndex + 4; ++i)
	{
		//cout << "확인" << FS_Buffer[i] << '\n';
	}

}

int charToInt(int startIndex)
{
	int num[4];
	unsigned char str[33];
	for (int i = 0; i < 32; ++i)
	{
		str[i] = '0';
	}

	int sum = 0;
	int numIndex = 0;

	for (int i = startIndex; i < startIndex + 4; ++i)
	{
		num[numIndex++] = FS_Buffer[i];
		//cout << num[i] << '\n';
	}

	//2진수로 변환
	int k = 0;


	for (int i = 0; i < 4; ++i)
	{
		int index = k;

		while (num[i])
		{
			str[index++] = (num[i] % 2) + '0';
			num[i] = num[i] / 2;
		}
		k += 8;
	}

	//10진수로변환 8개씩끊어서
	int count = 0;
	for (int i = 0; i < 32; ++i)
	{
		//cout << str[i];
		count++;

		if (count % 8 == 0)
		{
			count = 0;
			//cout << ' ';
		}
	}

	int c = 1;
	for (int j = 0; j < 32; j++)
	{
		sum += (str[j] - '0') * c;
		c *= 2;
	}
	//cout << '\n' << sum << '\n';
	return sum;
}

void intToChar(unsigned char *buffer, int startIndex, int num)
{
	unsigned char str[33];

	for (int i = 0; i < 32; ++i)
	{
		str[i] = '0';
	}

	//2진수로 변환
	int index = 0;
	while (num)
	{
		str[index++] = (num % 2) + '0';
		num = num / 2;
	}

	//10진수로변환 8개씩끊어서
	int count = 0;
	for (int i = 0; i < 32; ++i)
	{
		//cout << str[i];
		count++;

		if (count % 8 == 0)
		{
			count = 0;
			//cout << ' ';
		}
	}


	//저장

	for (int j = 0; j < 32; j += 8)
	{
		int sum = 0;
		int c = 1;

		for (int i = j; i < j + 8; ++i)
		{
			sum += (str[i] - '0') * c;
			c *= 2;
		}

		//cout << '\n' << sum << '\n';
		buffer[startIndex++] = sum;
	}

	//확인
	for (int i = startIndex; i < startIndex + 4; ++i)
	{
		//cout << "확인" << buffer[i] << '\n';
	}

}

int charToInt(unsigned char *buffer, int startIndex)
{
	int num[4];
	unsigned char str[33];
	for (int i = 0; i < 32; ++i)
	{
		str[i] = '0';
	}

	int sum = 0;

	int numIndex = 0;
	for (int i = startIndex; i < startIndex + 4; ++i)
	{
		num[numIndex] = buffer[i];
		//cout << num[i] << '\n';
	}

	//2진수로 변환
	int k = 0;


	for (int i = 0; i < 4; ++i)
	{
		int index = k;

		while (num[i])
		{
			str[index++] = (num[i] % 2) + '0';
			num[i] = num[i] / 2;
		}
		k += 8;
	}

	//10진수로변환 8개씩끊어서
	int count = 0;
	for (int i = 0; i < 32; ++i)
	{
		//cout << str[i];
		count++;

		if (count % 8 == 0)
		{
			count = 0;
			//cout << ' ';
		}
	}

	int c = 1;
	for (int j = 0; j < 32; j++)
	{

		sum += (str[j] - '0') * c;
		c *= 2;

	}
	//cout << '\n' << sum << '\n';
	return sum;

}

void stringToChar(string data_s, char* data_c)
{
	int size = data_s.length();

	int i;
	for (i = 0; i < size; ++i)
	{
		data_c[i] = data_s[i];
	}
	data_c[i] = '\0';
}

void itoa(int n, char s[])

{

	int i, sign;

	if ((sign = n) <0)

		n = -n;

	i = 0;

	do {

		s[i++] = n % 10 + '0';

	} while ((n /= 10)>0);

	if (sign<0)

		s[i++] = '-';

	s[i] = '\0';

	reverse(s, s + i);

}

void setTime(int &startIndex)
{
	time(&time_now);
	tm = localtime(&time_now);

	int year = tm->tm_year + 1900;
	int month = tm->tm_mon + 1;
	int day = tm->tm_mday;
	int hour = tm->tm_hour;
	int minutes = tm->tm_min;

	char year_char[5];
	itoa(year, year_char);

	FS_Image[startIndex++] = year_char[0];
	FS_Image[startIndex++] = year_char[1];
	FS_Image[startIndex++] = year_char[2];
	FS_Image[startIndex++] = year_char[3];
	FS_Image[startIndex++] = '-';

	if (month < 10)
	{
		char month_char[2];
		itoa(month, month_char);

		FS_Image[startIndex++] = '0';
		FS_Image[startIndex++] = month_char[0];
	}
	else
	{
		char month_char[3];
		itoa(month, month_char);

		FS_Image[startIndex++] = month_char[0];
		FS_Image[startIndex++] = month_char[1];
	}

	FS_Image[startIndex++] = '-';

	if (day < 10)
	{
		char day_char[2];
		itoa(day, day_char);

		FS_Image[startIndex++] = '0';
		FS_Image[startIndex++] = day_char[0];
	}
	else
	{
		char day_char[3];
		itoa(day, day_char);

		FS_Image[startIndex++] = day_char[0];
		FS_Image[startIndex++] = day_char[1];
	}

	FS_Image[startIndex++] = '-';

	if (hour < 10)
	{
		char hour_char[2];
		itoa(hour, hour_char);

		FS_Image[startIndex++] = '0';
		FS_Image[startIndex++] = hour_char[0];
	}
	else
	{
		char hour_char[3];
		itoa(hour, hour_char);

		FS_Image[startIndex++] = hour_char[0];
		FS_Image[startIndex++] = hour_char[1];
	}

	FS_Image[startIndex++] = '-';

	if (minutes < 10)
	{
		char minute_char[2];
		itoa(minutes, minute_char);

		FS_Image[startIndex++] = '0';
		FS_Image[startIndex++] = minute_char[0];
	}
	else
	{
		char minute_char[3];
		itoa(minutes, minute_char);

		FS_Image[startIndex++] = minute_char[0];
		FS_Image[startIndex++] = minute_char[1];
	}
}
void intToChar_My(int startIndex, int num)
{
	unsigned char str[33];

	for (int i = 0; i < 32; ++i)
	{
		str[i] = '0';
	}

	//2진수로 변환
	int index = 0;
	while (num)
	{
		str[index++] = (num % 2) + '0';
		num = num / 2;
	}

	//10진수로변환 8개씩끊어서
	int count = 0;
	for (int i = 0; i < 32; ++i)
	{
		//cout << str[i];
		count++;

		if (count % 8 == 0)
		{
			count = 0;
			//cout << ' ';
		}
	}


	//저장

	for (int j = 0; j < 32; j += 8)
	{
		int sum = 0;
		int c = 1;

		for (int i = j; i < j + 8; ++i)
		{
			sum += (str[i] - '0') * c;
			c *= 2;
		}

		//cout << '\n' << sum << '\n';
		My_Buffer[startIndex++] = sum;
	}

	//확인
	for (int i = startIndex; i < startIndex + 4; ++i)
	{
		//cout << "확인" << FS_Buffer[i] << '\n';
	}

}

int charToInt_My(int startIndex)
{
	int num[4];
	unsigned char str[33];
	for (int i = 0; i < 32; ++i)
	{
		str[i] = '0';
	}

	int sum = 0;
	int numIndex = 0;

	for (int i = startIndex; i < startIndex + 4; ++i)
	{
		num[numIndex++] = My_Buffer[i];
		//cout << num[i] << '\n';
	}

	//2진수로 변환
	int k = 0;


	for (int i = 0; i < 4; ++i)
	{
		int index = k;

		while (num[i])
		{
			str[index++] = (num[i] % 2) + '0';
			num[i] = num[i] / 2;
		}
		k += 8;
	}

	//10진수로변환 8개씩끊어서
	int count = 0;
	for (int i = 0; i < 32; ++i)
	{
		//cout << str[i];
		count++;

		if (count % 8 == 0)
		{
			count = 0;
			//cout << ' ';
		}
	}

	int c = 1;
	for (int j = 0; j < 32; j++)
	{
		sum += (str[j] - '0') * c;
		c *= 2;
	}
	//cout << '\n' << sum << '\n';
	return sum;
}

void charToChar(unsigned char *buffer, unsigned char *str, int startIndex, int endIndex)
{
	int index = 0;

	for (int i = startIndex; i < startIndex + endIndex; ++i)
	{
		str[index++] = buffer[i];
	}

	str[index] = '\0';
}





//Getter
int getEntryCount(int block)
{
	FS_Read(block);

	int entryCount = 0;

	for (int i = 0; i < BLOCK_SIZE; i += ENTRYSIZE)
	{
		if (FS_Buffer[i + 0] == '1')
			entryCount++;
	}

	return entryCount;
}

int getInodeBlock(int inode)
{
	int block = 0;

	if (inode < 32)
		block = INODE_BLOCKS;
	else
		block = INODE_BLOCKS + 1;

	return block;
}

int getDataBlockByInode(int inode, int blockIndex)
{
	int inodeBlock = getInodeBlock(inode); //inode count in 1 block is 32

	inode = inode - 32 * (inodeBlock - INODE_BLOCKS);

	FS_Read(inodeBlock);

	int block = charToInt(37 + INODE_SIZE*inode);

	return block;
}

/*
void getNameByInode(int workspace_inode, int inode,char* name)
{
	if (workspace_inode == -1)
	{
		name = "/";
		return;
	}

	int preInode = workspace_inode;

	int block = getInodeBlock(preInode);

	preInode = preInode - 32 * (block - INODE_BLOCKS);

	FS_Read(block);

	int dataBlock = charToInt(37);

	int entryCount = getEntryCount(dataBlock);

	for (int i = 0; i < entryCount; ++i)
	{
		if (charToInt(i*ENTRYSIZE + 1) == inode)
		{
			int namesize = FS_Buffer[i*ENTRYSIZE + 5];
			copyBufferToData(namesize, i*ENTRYSIZE + 6, name);
			break;
		}
	}
}
*/

int getInodeByName(int workspace_inode, char* direc)
{
	if (workspace_inode == -1)
		return 0;

	int inode = -1;

	int preInode = workspace_inode;

	int block = getInodeBlock(preInode);

	preInode = preInode - 32 * (block - INODE_BLOCKS);

	FS_Read(block);

	int dataBlock = charToInt(37+(128*preInode));

	FS_Read(dataBlock);

	for (int i = 0; i < BLOCK_SIZE; i += ENTRYSIZE)
	{
		char name[255];

		copyBufferToData((int)FS_Buffer[5+i], i + 6, name);

		//name[(int)FS_Buffer[5]] = '\0';

		if (strcmp (name,direc) == 0)
		{
			inode = charToInt(i+1);
			break;
		}
	}

	if (block - INODE_BLOCKS == 1)
		inode += 32;

	return inode; // -1이면 존재하지않는 파일
}

void getTime(int startIndex)
{
	time(&time_now);
	tm = localtime(&time_now);

	int hour = tm->tm_hour;
	int minutes = tm->tm_min;

	if (hour < 10)
	{
		char hour_char[2];
		itoa(hour, hour_char);

		FS_Buffer[startIndex + 0] = '0';
		FS_Buffer[startIndex + 1] = hour_char[0];
	}
	else
	{
		char hour_char[3];
		itoa(hour, hour_char);

		FS_Buffer[startIndex + 0] = hour_char[0];
		FS_Buffer[startIndex + 1] = hour_char[1];
	}

	FS_Buffer[startIndex + 2] = ':';

	if (minutes < 10)
	{
		char minute_char[2];
		itoa(minutes, minute_char);

		FS_Buffer[startIndex + 3] = '0';
		FS_Buffer[startIndex + 4] = minute_char[0];
	}
	else
	{
		char minute_char[3];
		itoa(minutes, minute_char);

		FS_Buffer[startIndex + 3] = minute_char[0];
		FS_Buffer[startIndex + 4] = minute_char[1];
	}
}

int getUnassignedBlockNum(int block)
{
	int pos = -1;

	if (block == BLOCK_BITMAP) //DataBlock Bitmap
	{

		FS_Read(block);

		for (int i = 0; i < BLOCK_SIZE; ++i)
		{
			if (FS_Buffer[i] == '0')
			{
				FS_Buffer[i] = '1';
				pos = i;
				break;
			}
		}

		if (pos == -1)
			return -1;

		FS_Write(block);

		//미할당된 데이터블락수 감소
		FS_Read(BLOCK_DESCIPTOR_TABLE);

		int unassigned_Blocks = charToInt(12);
		unassigned_Blocks--;
		intToChar(12, unassigned_Blocks);

		FS_Write(BLOCK_DESCIPTOR_TABLE);
	}
	else if (block == INODE_BITMAP) //Inode Bitmap
	{

		FS_Read(block);

		for (int i = 0; i < NUMBEROFINODES; ++i)
		{
			if (FS_Buffer[i] == '0')
			{
				FS_Buffer[i] = '1';
				pos = i;
				break;
			}
		}

		FS_Write(block);

		if (pos == -1)
			return -1;

		//미할당된 inode 수를 감소
		FS_Read(BLOCK_DESCIPTOR_TABLE);

		int unassigned_Blocks = charToInt(16);
		unassigned_Blocks--;
		intToChar(16, unassigned_Blocks);

		FS_Write(BLOCK_DESCIPTOR_TABLE);
	}

	return pos;
}

//check
bool checkDirectory(int inode)
{
	if (inode == -1)
		return false;

	int inodeBlock = getInodeBlock(inode);

	inode = inode - 32 * (inodeBlock - INODE_BLOCKS);

	FS_Read(inodeBlock);

	if (FS_Buffer[INODE_SIZE*inode + 0] == 'd')
		return true;

	else
		return false;
}


string popBack(string str)
{
	return str.substr(0, str.length() - 1);
}

int getWorkspaceInodeByPath(string path, char* target)
{
	if (path == "/")
	{
		target[0] = '/';
		target[1] = '\0';

		return -1;
	}

	pathInfo tmp[100];

	//tmpIndex 0 은 디폴트값
	tmp[0].path_Inode = -1;
	tmp[0].path_Name = "#";

	int tmpIndex = 0;

	int len = path.length();

	//절대경로 -> /home/data
	if (path[0] == '/') //절대경로
	{
		tmpIndex += 1;

		tmp[tmpIndex].path_Name += '/';
		tmp[tmpIndex].path_Inode = 0;

		string tmpName = "";

		//tmpIndex 2 부터 입력 시작
		for (int i = 1; i < len; ++i)
		{
			tmpName += path[i];

			if (path[i] == '/')
			{
				tmpName = popBack(tmpName);

				if (i + 1 == len)
				{
					int size = tmpName.length();

					int i;

					for (i = 0; i < size; ++i)
					{
						target[i] = tmpName.at(i);
					}

					target[i] = '\0';

					break;
				}

				if (tmpName == ".") // .
				{
					tmpName = "";

					continue;
				}
				else if (tmpName == "..") // ..
				{
					tmpIndex -= 1;

					if (tmpIndex == 1)
					{
						cout << "잘못된 경로입니다." << endl;
						return -1;
					}

					tmpName = "";


					continue;
				}

				char name[255];

				stringToChar(tmpName, name);

				int inode = getInodeByName(tmp[tmpIndex].path_Inode, name);

				if (!checkDirectory(inode)) //false - Not D
				{
					cout << "Error : 존재하지않는 디렉토리 명 포함" << endl;

					return -1;
				}

				int inode_t = inode;

				int block = getInodeBlock(inode_t); //inode count in 1 block is 32

				inode_t = inode_t - (32 * (block - INODE_BLOCKS));

				FS_Read(block);

				getTime(INODE_SIZE*(inode_t)+14);

				FS_Write(block);

				tmpIndex += 1;

				tmp[tmpIndex].path_Inode = inode;
				tmp[tmpIndex].path_Name = tmpName;

				tmpName = "";

				continue;
			}
			else if (i + 1 == len)
			{
				if (tmpName == ".") // .
				{
					tmpName = "";

					int size = tmp[tmpIndex].path_Name.length();

					int i;

					for (i = 0; i < size; ++i)
					{
						target[i] = tmp[tmpIndex].path_Name.at(i);
					}

					target[i] = '\0';
				}
				else if (tmpName == "..") // ..
				{
					if (tmpIndex == 1)
					{
						cout << "잘못된 경로입니다." << endl;
						return -1;
					}

					tmpName = "";

					tmpIndex -= 1;

					int size = tmp[tmpIndex].path_Name.length();

					int i;

					for (i = 0; i < size; ++i)
					{
						target[i] = tmp[tmpIndex].path_Name.at(i);
					}

					target[i] = '\0';
				}
				else
				{
					int size = tmpName.length();

					int i;

					for (i = 0; i < size; ++i)
					{
						target[i] = tmpName.at(i);
					}

					target[i] = '\0';
				}
			}
		}
	}
	//data
	//상대경로 -> ./home/data
	else
	{
		tmpIndex = pathIndex;

		for (int i = 0; i <= tmpIndex; ++i)
			tmp[i] = stack[i];


		string tmpName = "";

		for (int i = 0; i < len; ++i)
		{
			tmpName += path[i];


			if (path[i] == '/')
			{

				tmpName = popBack(tmpName);

				if (i + 1 == len)
				{
					int size = tmpName.length();

					int i;

					for (i = 0; i < size; ++i)
					{
						target[i] = tmpName.at(i);
					}

					target[i] = '\0';

					break;
				}

				if (tmpName == ".") // .
				{
					tmpName = "";

					continue;
				}
				else if (tmpName == "..") // ..
				{

					if (tmpIndex == 1)
					{
						return -1;
					}

					tmpIndex -= 1;


					tmpName = "";


					continue;
				}


				char name[255];

				stringToChar(tmpName, name);

				int inode = getInodeByName(tmp[tmpIndex].path_Inode, name);


				if (!checkDirectory(inode)) //false - Not D
				{
					cout << "Error : 존재하지않는 디렉토리 명 포함" << endl;

					return -1;
				}

				int inode_t = inode;

				int block = getInodeBlock(inode_t); //inode count in 1 block is 32

				inode_t = inode_t - (32 * (block - INODE_BLOCKS));

				FS_Read(block);

				getTime(INODE_SIZE*(inode_t)+14);

				FS_Write(block);

				tmpIndex += 1;

				tmp[tmpIndex].path_Inode = inode;
				tmp[tmpIndex].path_Name = tmpName;

				tmpName = "";

			}
			else if (i + 1 == len)
			{
				if (tmpName == ".") // .
				{
					tmpName = "";

					int size = tmp[tmpIndex].path_Name.length();

					int i;

					for (i = 0; i < size; ++i)
					{
						target[i] = tmp[tmpIndex].path_Name.at(i);
					}

					target[i] = '\0';
				}
				else if (tmpName == "..") // ..
				{
					if (tmpIndex == 1)
					{
						return -1;
					}

					tmpIndex -= 1;


					tmpName = "";


					int size = tmp[tmpIndex].path_Name.length();

					int i;

					for (i = 0; i < size; ++i)
					{
						target[i] = tmp[tmpIndex].path_Name.at(i);
					}

					target[i] = '\0';
				}
				else
				{
					int size = tmpName.length();

					int i;

					for (i = 0; i < size; ++i)
					{
						target[i] = tmpName.at(i);
					}

					target[i] = '\0';
				}
			}
		}
	}

	//현재경로 -> data/etc
	//else
	//{
	//   tmpIndex = pathIndex;

	//   for (int i = 0; i <= tmpIndex; ++i)
	//      tmp[i] = stack[i];

	//   string tmpName = "";

	//   for (int i = 0; i < len; ++i)
	//   {
	//      tmpName += path[i];

	//      if (path[i] == '/')
	//      {
	//   
	//         tmpName = popBack(tmpName);

	//         if (tmpName == ".") // .
	//         {
	//            tmpName = "";

	//            continue;
	//         }
	//         else if (tmpName == "..") // ..
	//         {
	//            tmpName = "";

	//            tmpIndex -= 1;

	//            continue;
	//         }

	//         char name[255];

	//         stringToChar(tmpName, name);

	//         int inode = getInodeByName(tmp[tmpIndex].path_Inode, name);

	//         if (!checkDirectory(inode)) //false - Not D
	//         {
	//            cout << "Error : 존재하지않는 디렉토리 명 포함" << endl;

	//            return -1;
	//         }

	//         tmpIndex += 1;

	//         tmp[tmpIndex].path_Inode = inode;
	//         tmp[tmpIndex].path_Name = tmpName;

	//         tmpName = "";

	//      }
	//      else if (i + 1 == len)
	//      {
	//         int size = tmpName.length();

	//         int i;

	//         for (i = 0; i < size; ++i)
	//         {
	//            target[i] = tmpName.at(i);
	//         }

	//         target[i] = '\0';
	//      }
	//   }
	//}

	if (tmpIndex < 1)
	{
		cout << "루트 디렉토리 위로는 접근 불가" << endl;
		return -1;
	}

	if (tmpIndex == 1 && strcmp(target, "/") == 0)
		return -1;
	return tmp[tmpIndex].path_Inode;
}



//set
void addInode(int inode, int blockIndex, int flag = 0)
{
	int block = getInodeBlock(inode); //inode count in 1 block is 32

	inode = inode - (32 * (block - INODE_BLOCKS));

	FS_Read(block);

	//int size_Data = 54;

	/*
	char mode[10]; //0
	int size = 0;  //10
	char time[5] = {} //14
	char ctime[5]; //19
	char mtime[5]; //24
	int links_count = 2; //29
	int blocks = 1; //33
	int block[1] = blockIndex; //37
	*/
	if (flag)
	{
		char mode[11] = "-rw-rw-rw-";
		copyDataToBuffer(mode, 10, INODE_SIZE*(inode));
		intToChar(INODE_SIZE*(inode)+10, 0);
		getTime(INODE_SIZE*(inode)+14);
		getTime(INODE_SIZE*(inode)+19);
		getTime(INODE_SIZE*(inode)+24);
		intToChar(INODE_SIZE*(inode)+29, 1);
		intToChar(INODE_SIZE*(inode)+33, 0);
		intToChar(INODE_SIZE*(inode)+37, blockIndex);
	}
	else
	{
		
		char mode[11] = "drwxrwxrwx";
		copyDataToBuffer(mode, 10, INODE_SIZE*(inode));
		intToChar(INODE_SIZE*(inode)+10, 0);
		getTime(INODE_SIZE*(inode)+14);
		getTime(INODE_SIZE*(inode)+19);
		getTime(INODE_SIZE*(inode)+24);
		intToChar(INODE_SIZE*(inode)+29, 2);
		intToChar(INODE_SIZE*(inode)+33, 1);
		intToChar(INODE_SIZE*(inode)+37, blockIndex);

		if (inode == 0)
		{
			copyDataToBuffer(mode, 10, INODE_SIZE*(inode));
			intToChar(INODE_SIZE*(inode)+10, 0);
			getTime(INODE_SIZE*(inode)+14);
			getTime(INODE_SIZE*(inode)+19);
			getTime(INODE_SIZE*(inode)+24);
			intToChar(INODE_SIZE*(inode)+29, 1);
			intToChar(INODE_SIZE*(inode)+33, 1);
			intToChar(INODE_SIZE*(inode)+37, blockIndex);
		}
	}


	FS_Write(block);

}
void addEntry(int workspace_inode, int blockIndex, int inode, int size_Direc, char* direc, int flag = 0)
{

	//root일경우만 예외
	if (inode == 0)
	{
		FS_Read(blockIndex);

		FS_Buffer[0] = '1';
		intToChar(1, inode);
		FS_Buffer[5] = 1;
		FS_Buffer[6] = '.'; // .

		FS_Write(blockIndex);

		int inode_t = inode;

		int block = getInodeBlock(inode_t);

		FS_Read(block);

		intToChar(INODE_SIZE*(inode_t)+10, ENTRYSIZE * 1);

		FS_Write(block);

		return;
	}
	int preInode = workspace_inode;

	//디렉토리 초기설정

	if (!flag)
	{
		FS_Read(blockIndex);

		FS_Buffer[0] = '1';
		intToChar(1, inode);
		FS_Buffer[5] = 1;
		FS_Buffer[6] = '.'; // .

		FS_Buffer[0 + ENTRYSIZE] = '1';
		intToChar(1 + ENTRYSIZE, preInode);
		FS_Buffer[5 + ENTRYSIZE] = 2;
		FS_Buffer[6 + ENTRYSIZE] = '.';
		FS_Buffer[7 + ENTRYSIZE] = '.';// ..

		FS_Write(blockIndex);

		int inode_t = inode;

		int block = getInodeBlock(inode_t);

		FS_Read(block);

		intToChar(INODE_SIZE*(inode_t)+10, ENTRYSIZE * 2);

		FS_Write(block);
	}


	//상위디렉토리에 엔트리 추가

	int preInodeBlock = getInodeBlock(preInode); //inode count in 1 block is 32

	preInode = preInode - (32 * (preInodeBlock - INODE_BLOCKS));


	//상위 디렉토리의 하드링크 추가

	FS_Read(preInodeBlock);

	int preBlock = charToInt(preInode*INODE_SIZE + 37);

	if (!flag)
	{
		int preLink = charToInt(preInode*INODE_SIZE + 29);

		intToChar(preInode*INODE_SIZE + 29, preLink + 1);

		FS_Write(preInodeBlock);
	}

	int entryPos = -1;

	FS_Read(preBlock);

	for (int i = 0; i < BLOCK_SIZE; i += ENTRYSIZE)
	{
		if (FS_Buffer[0 + i] == 0)
		{
			entryPos = i / ENTRYSIZE;
			break;
		}
	}

	if (entryPos == -1)
	{
		cout << "공간이 없습니다.\n";
		return;
	}

	FS_Read(preBlock);

	FS_Buffer[0 + ENTRYSIZE*entryPos] = '1';
	intToChar(1 + ENTRYSIZE*entryPos, inode);
	FS_Buffer[5 + ENTRYSIZE*entryPos] = size_Direc;
	copyDataToBuffer(direc, size_Direc, 6 + ENTRYSIZE*entryPos);;

	FS_Write(preBlock);

	int inode_t = workspace_inode;

	int block = getInodeBlock(inode_t);;

	int dataBlock = getDataBlockByInode(inode_t, block);

	int entryCount = getEntryCount(dataBlock);

	FS_Read(block);

	intToChar(INODE_SIZE*(inode_t)+10, ENTRYSIZE * entryCount);

	getTime(INODE_SIZE*(inode_t)+14);
	getTime(INODE_SIZE*(inode_t)+24);

	FS_Write(block);

}


void controlLinkCount(int inode, int value)
{
	int inodeBlock = getInodeBlock(inode);

	inode = inode - 32 * (inodeBlock - INODE_BLOCKS);

	FS_Read(inodeBlock);

	int linkCount = charToInt(inode*INODE_SIZE + 29);

	intToChar(inode*INODE_SIZE + 29, linkCount + value);

	FS_Write(inodeBlock);
}

int deleteEntry(int workspace_inode, char* direc, int flag = 0)
{
	int inode = workspace_inode;

	int inodeBlock = getInodeBlock(workspace_inode);

	inode = inode - 32 * (inodeBlock - INODE_BLOCKS);

	FS_Read(inodeBlock);

	int dataBlock = charToInt(inode*INODE_SIZE + 37);

	FS_Read(dataBlock);

	int index = -1;

	for (int i = 0; i < BLOCK_SIZE; i += ENTRYSIZE)
	{
		char name[255];

		copyBufferToData((int)FS_Buffer[i + 5], i + 6, name);

		if (strcmp(name, direc) == 0)
		{
			FS_Buffer[i] = '0';
			for (int j = i + 1; j < i + ENTRYSIZE; ++j)
			{
				FS_Buffer[j] = 0;
			}

			index = i / ENTRYSIZE;
			break;
		}
	}
	FS_Write(dataBlock);

	if (!flag)
		controlLinkCount(workspace_inode, -1);

	int inode_t = workspace_inode;

	int block = getInodeBlock(inode_t);;

	int dataBlock_t = getDataBlockByInode(inode_t, block);

	int entryCount = getEntryCount(dataBlock_t);

	FS_Read(block);

	intToChar(INODE_SIZE*(inode_t)+10, ENTRYSIZE * entryCount);

	getTime(INODE_SIZE*(inode_t)+14);
	getTime(INODE_SIZE*(inode_t)+24);

	FS_Write(block);

	return index;
}
int resetAssignedBlock(int block, int index)
{
	int pos = -1;

	FS_Read(block);

	if (FS_Buffer[index] == '1')
	{
		FS_Buffer[index] = '0';
		pos = index;
	}

	FS_Write(block);

	if (pos == -1)
		return -1;

	//할당 해제된 블럭 0으로 초기화
	if (block == BLOCK_BITMAP)
	{
		FS_Read(index);

		for (int i = 0; i < BLOCK_SIZE; ++i)
			FS_Buffer[i] = 0;

		FS_Write(index);

		//미할당된 데이터블락수 증가
		FS_Read(BLOCK_DESCIPTOR_TABLE);

		int unassigned_Blocks = charToInt(12);
		unassigned_Blocks++;
		intToChar(12, unassigned_Blocks);

		FS_Write(BLOCK_DESCIPTOR_TABLE);
	}
	//할당해제된 inode 초기화
	else if (block == INODE_BITMAP)
	{
		int inode = index;

		int inodeBlock = getInodeBlock(index);

		inode = inode - 32 * (inodeBlock - INODE_BLOCKS);

		FS_Read(inodeBlock);

		for (int i = 0; i < INODE_SIZE; ++i)
			FS_Buffer[i + INODE_SIZE*index] = 0;

		FS_Write(inodeBlock);

		//미할당된 inode 수를 증가
		FS_Read(BLOCK_DESCIPTOR_TABLE);

		int unassigned_Blocks = charToInt(16);
		unassigned_Blocks++;
		intToChar(16, unassigned_Blocks);

		FS_Write(BLOCK_DESCIPTOR_TABLE);
	}
	return pos;
}

void FS_Save(char* FS_File)
{
	//FS_Image 데이터를 저장한다.
	
	int filedesc = open(FS_File, O_RDWR);

	if (write(filedesc, FS_Image, 409600) != 409600)
	{
	cout << "에러에러에러 삐삐삑";
	}

	if (close(filedesc) < 0)
	{
	return ;
	}
	
} //준하

void FS_Load(char* FS_File)
{
	//FS_File이 내용이 있으면 불러오고
	
	int filedesc = open(FS_File, O_RDWR);

	if (filedesc >= 0)
	{
		if (read(filedesc, FS_Image, 409600) < 0)
		{
			cout << "에러에러 삑삑";
		}
		if (close(filedesc) < 0)
		{
			return ;
		}
	}
	else
	{
		 int filedesc = open(FS_File, O_RDWR | O_CREAT, 0777);
	
	///////////////////////// 0
	int tmpIndex = 0;
	int len = strlen(MagicNumber);
	for (int i = 0; i < len; ++i)
	{
		FS_Image[tmpIndex++] = MagicNumber[i];
	}

	len = strlen(BlockSize);
	for (int i = 0; i < len; ++i)
	{
		FS_Image[tmpIndex++] = BlockSize[i];
	}

	len = strlen(BlocksPerGroup);
	for (int i = 0; i < len; ++i)
	{
		FS_Image[tmpIndex++] = BlocksPerGroup[i];
	}

	len = strlen(numberOfInodes);
	for (int i = 0; i < len; ++i)
	{
		FS_Image[tmpIndex++] = numberOfInodes[i];
	}

	len = strlen(inodeSize);
	for (int i = 0; i < len; ++i)
	{
		FS_Image[tmpIndex++] = inodeSize[i];
	}

	setTime(tmpIndex);
	
	FS_Image[tmpIndex++] = '6';
	FS_Image[tmpIndex++] = '0';

	//////////////////////////////////// 1

	tmpIndex = 4096;
	intToChar(FS_Image, tmpIndex, 2);
	intToChar(FS_Image, tmpIndex + 4, 3);
	intToChar(FS_Image, tmpIndex + 8, 4);
	intToChar(FS_Image, tmpIndex +12, 94);
	intToChar(FS_Image, tmpIndex + 16, 64);

	/////////////////////////////////// 2

	tmpIndex = 8192;
	for (int i = 0; i < 100; i++)
	{
		FS_Image[tmpIndex + i] = '1';
		if ( i > 5 )
			FS_Image[tmpIndex + i] = '0';
	}

	/////////////////////////////////// 3

	tmpIndex = 12288;
	for (int i = 0; i < 64; i++)
	{
		FS_Image[tmpIndex + i] = '0';
	}

	/////////////////////////////////// 4

	/// root디렉토리
	int blockIndex = getUnassignedBlockNum(BLOCK_BITMAP);
	int inode = getUnassignedBlockNum(INODE_BITMAP);

	char fileName[5] = "/";
	addInode(inode, blockIndex);
	addEntry(inode, blockIndex, inode, strlen(fileName), fileName);

	// bin 디렉토리
	blockIndex = getUnassignedBlockNum(BLOCK_BITMAP);
	inode = getUnassignedBlockNum(INODE_BITMAP);

	char fileName1[5] = "bin";
	addInode(inode, blockIndex);
	addEntry(0, blockIndex, inode, strlen(fileName1), fileName1);
	// dev 디렉토리
	blockIndex = getUnassignedBlockNum(BLOCK_BITMAP);
	inode = getUnassignedBlockNum(INODE_BITMAP);

	char fileName2[5] = "dev";
	addInode(inode, blockIndex);
	addEntry(0, blockIndex, inode, strlen(fileName2), fileName2);
	// etc 디렉토리
	blockIndex = getUnassignedBlockNum(BLOCK_BITMAP);
	inode = getUnassignedBlockNum(INODE_BITMAP);
	char fileName3[5] = "etc";
	addInode(inode, blockIndex);
	addEntry(0, blockIndex, inode, strlen(fileName3), fileName3);
	//home 디렉토리

	blockIndex = getUnassignedBlockNum(BLOCK_BITMAP);
	inode = getUnassignedBlockNum(INODE_BITMAP);
	char fileName4[5] = "home";
	addInode(inode, blockIndex);
	addEntry(0, blockIndex, inode, strlen(fileName4), fileName4);
	// lib 디렉토리
	blockIndex = getUnassignedBlockNum(BLOCK_BITMAP);
	inode = getUnassignedBlockNum(INODE_BITMAP);
	char fileName5[5] = "lib";
	addInode(inode, blockIndex);
	addEntry(0, blockIndex, inode, strlen(fileName5), fileName5);
	// bar 디렉토리
	blockIndex = getUnassignedBlockNum(BLOCK_BITMAP);
	inode = getUnassignedBlockNum(INODE_BITMAP);
	char fileName6[5] = "var";
	addInode(inode, blockIndex);
	addEntry(0, blockIndex, inode, strlen(fileName6), fileName6);

	
	
	
	if (close(filedesc) < 0)
	{
	return ;
	}

	FS_Save(FS_File);

	}
	
	


	//없으면 기본셋팅을 시작한다.
	//3페이지에있는 그림으로 셋팅을 하고



	//FS_SAVE를 함수를 호출하여 현재 셋팅값을 저장한다.

} //준하

#endif
