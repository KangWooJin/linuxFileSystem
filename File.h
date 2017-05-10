#ifndef FILE_H
#define FILE_H

#include "FileSystem.h"

int getFdByInodeNumber(int fd)
{
	int SystemFileIndex = FileDescriptorTable[fd];

	int inodeTableIndex = SystemFileTable[SystemFileIndex].inode_number;

	return inodeTable[inodeTableIndex].inode_number;
}

int getFdByFilePointer(int fd)
{
	int SystemFileIndex = FileDescriptorTable[fd];

	return SystemFileTable[SystemFileIndex].file_pointer;
}

void setFilePointer(int fd, int FilePointer)
{
	int SystemFileIndex = FileDescriptorTable[fd];

	SystemFileTable[SystemFileIndex].file_pointer = FilePointer;
}

int File_Open(int workspace_inode, char *file)
{
	/*
	File_Open(char *file): file이 가리키는 이름의 파일을 열고 0 이상의 정수 파일 디스크립터(fd: file descriptor)를 리턴한다.
	같은 파일을 동시에 두 번 이상 열수 없다 (즉, 기존에 열려 있는 파일을 다시 열 수 없음).
	*/

	//현재 디렉토리에 inodeNumber와 파일명으로 파일의 inodeNumber 구하기
	int inode = getInodeByName(workspace_inode, file);

	//파일이 열려있는지 확인
	for (int i = 0; i < NUMBEROFINODES; ++i)
	{
		if (inode == inodeTable[i].inode_number)
		{
			//이미 기존에 열려 있는 파일
			cout << "현재 파일이 열려있습니다.\n";
			return -1;
		}
	}

	//아이노드 테이블에 아이노드 추가
	//0번부터 빈 테이블에 삽입하는 방식으로 구현
	int inodeTableIndex = 0;
	for (inodeTableIndex = 0; inodeTableIndex < NUMBEROFINODES; ++inodeTableIndex)
	{
		if (inodeTable[inodeTableIndex].inode_number == -1)
		{
			inodeTable[inodeTableIndex].inode_number = inode;
			break;
		}
	}
	//시스템 파일 테이블은 inode 테이블 인덱스를 가리키고
	//파일 포인터는 0으로 초기화
	int SystemFileIndex = 0;
	for (SystemFileIndex = 0; SystemFileIndex < 100; ++SystemFileIndex)
	{
		if (SystemFileTable[SystemFileIndex].inode_number == -1)
		{
			SystemFileTable[SystemFileIndex].inode_number = inodeTableIndex;
			SystemFileTable[SystemFileIndex].file_pointer = 0;
			break;
		}
	}


	//파일 디스크립터 테이블은 시스템 파일 테이블 인덱스를 가리린다.
	int fileDescriptorIndex = 3;

	for (fileDescriptorIndex = 3; fileDescriptorIndex < 100; ++fileDescriptorIndex)
	{
		if (FileDescriptorTable[fileDescriptorIndex] == -1)
		{
			FileDescriptorTable[fileDescriptorIndex] = SystemFileIndex;
			break;
		}
	}

	// 파일디스크립터 테이블에 index를 리턴하여 파일디스크립터 리턴

	// 파일오픈시 파일접근시간 수정
	if (inode < 32)
	{
		FS_Read(4);
		getTime(14 + (inode*INODE_SIZE));
		FS_Write(4);
	}
	//inode가 32보다 큰경우는 inodeBlock5번을 리드
	else
	{
		FS_Read(5);
		getTime(14 + ((inode - 32)*INODE_SIZE));
		FS_Write(5);
	}
	return fileDescriptorIndex;

}

void File_Close(int fd)
{
	/*
	File_Close(int fd): 파일 디스크립터 fd가 가리키는 파일을 닫는다.
	*/

	int SystemFileIndex = FileDescriptorTable[fd];

	FileDescriptorTable[fd] = -1;

	int inodeTableIndex = SystemFileTable[SystemFileIndex].inode_number;
	SystemFileTable[SystemFileIndex].file_pointer = -1;
	SystemFileTable[SystemFileIndex].inode_number = -1;

	inodeTable[inodeTableIndex].inode_number = -1;

}

int File_Unlink(char *file, int workspace_inode)
{
	/*
	File_Unlink(char *file): file이 나타내는 파일을 삭제한다.
	또한 해당 파일을 포함하는 디렉토리에서도 관련 내용을 삭제한다.
	해당 파일이 사용하는 data block과 inode도 삭제해야하며 관련 bitmap도 0으로 설정한다.
	단, File_Open() 함수를 호출해 현재 사용 중인 파일은 삭제 못한다.
	*/

	int inode = getInodeByName(workspace_inode, file);

	if (inode == -1)
	{
		cout << "존재하지 않는 파일입니다.\n";
		return false;
	}

	int fd = File_Open(workspace_inode, file);
	if (fd == -1)
		return false;
	
	int original_workspace_inode = workspace_inode;

	int workspaceBlock = getInodeBlock(workspace_inode); //inode count in 1 block is 32

	workspace_inode = workspace_inode - 32 * (workspaceBlock - INODE_BLOCKS);

	


	int myInode = getInodeByName(original_workspace_inode, file);

	//현재 디렉토리 엔트리에 파일을 삭제
	deleteEntry(workspace_inode, file, 1);

	//inode + data Block 초기화
	int inodeBlock = getInodeBlock(myInode); //inode count in 1 block is 32
	int tempInode = myInode - 32 * (inodeBlock - INODE_BLOCKS);
	FS_Read(inodeBlock);
	int blocks = charToInt(myInode * 128 + 33);

	for (int i = 1; i <= blocks; ++i)
	{
		resetAssignedBlock(BLOCK_BITMAP, getDataBlockByInode(myInode, i));
	}

	//inode + data bitmap 초기화
	resetAssignedBlock(INODE_BITMAP, inode);
	
	File_Close(fd);

	return -1;
}


int File_Create(char *file, int workspace_inode, int flag = 0)
{
	/*
	File_Create(char *file): 파라미터 file이 가리키는 이름으로 크기 0바이트의 새로운 파일을 생성한다.
	경로에 있는 파일이 이미 존재하면 에러를 리턴한다. 
	파일명의 최대 길이는 255바이트이다. 기본 접근 권한은 666이다.
	*/

	int inode = getInodeByName(workspace_inode, file);

	if (inode != -1 )
	{
		int check = -1;
		if ( flag  == 0 )
			check = File_Unlink(file, workspace_inode);
		
		if (check == 0)
			return -99;

		return check; // ==>존재하는 파일
	}

	inode = getUnassignedBlockNum(INODE_BITMAP);
	
	//inode 블럭에 기본데이터값 추가
	addInode(inode, -1, 1);

	//디렉토리 엔트리에 추가
	int len = strlen(file);
	addEntry(workspace_inode, -1, inode, len, file,1);
	
	return 1; // 존재하지 않는 파일
}




int File_Read(int fd, int size)
{
	/*
	File_Read(int fd, void *buffer, int size): 파일 디스크립터 fd가 가리키는 파일로부터 size 바이트만큼 읽은 후 buffer에 저장한다. 
	파일 포인터(file_pointer)가 가리키는 위치부터 읽어온다. 
	파일 포인터는 읽은 후에 새로운 위치로 갱신된다.
	*/
	// fd를 통해서inodeNumber를 구해온다.
	int inode = getFdByInodeNumber(fd);
	int filePointer = getFdByFilePointer(fd);
	int dataBlocks;
	int maxSize;
	int flag = 1;

	//inode가 32보다 작은경우는 inodeBlock4번을 리드
	if (inode < 32)
	{
		FS_Read(4);
		dataBlocks = charToInt((inode * 128) + 33);
		maxSize = charToInt(10 + (inode * 128));
	}
	//inode가 32보다 큰경우는 inodeBlock5번을 리드
	else
	{
		FS_Read(5);
		dataBlocks = charToInt(((inode - 32) * 128) + 33);
		maxSize = charToInt(10 + ((inode-32) * 128));
	}

	//datablocks를 통해서 inode가 가리키고 있는 datablocks의 갯수를 구하여
	int datablock[100] = { 0, };
	int s = 0;
	
	//datablock을 구한다.
	for (int i = 0; i < dataBlocks; ++i)
	{
		if (inode < 32)
		{
			datablock[i] = charToInt(37 + s + (inode * 128));
			s += 4;
		}
		else
		{
			datablock[i] = charToInt(37 + s + ((inode-32) * 128));
			s += 4;
		}
		
	}

	//filePointer로 시작 dataBlock을 계산
	int pBlockCount = filePointer / BLOCK_SIZE;
	//filePointer로 누적 파일포인터 계산
	int pFilePointer = filePointer % BLOCK_SIZE;
	
	
	//FS_read를 통해서 My_Buffer에 집어넣는다.

	FS_Read(datablock[pBlockCount]);

	for (int i = 0; i < size; ++i)
	{
		//파일 사이즈보다 읽는 사이즈가 커지는 경우 방지
		if (maxSize == filePointer)
		{
			flag = 0;
			break;
		}


		//한 블록을 다읽은경우 다음블록으로 변경
		if (pFilePointer == 4096)
		{
			pFilePointer = 0;
			pBlockCount++;
			FS_Read(datablock[pBlockCount]);
		}
		

		My_Buffer[MyBufferCount++] = FS_Buffer[pFilePointer];

		pFilePointer++;
		filePointer++;
	}

	//filePointer를 현재까지 읽은곳으로 설정

	setFilePointer(fd, filePointer);
	//fflush_MyBuffer();
	
	return flag;
}


void File_Write(int fd, int size, int flag )
{
	/*
	File_Write(int fd, void *buffer, int size): buffer에 있는 데이터를 size 바이트만큼 파일 디스크립터 fd가 가리키는 파일에 파일 포인터(file_pointer) 위치부터 저장한다. 
	저장한 후에 파일 포인터 값도 size만큼 증가한다.
	위에 사용된 buffer 파라미터는 동적으로 임시 할당된 메모리로 크기로 사용자가 정의한 크기이다.
	파일 시스템 관련 함수에서의 buffer 파라미터와 다르다. 만약 파일 크기가 6KB이면 4KB 단위로 하여 FS_Write() 함수를 두 번 호출해야 한다. 
	Read할 때도 이를 고려해야 한다.
	*/


	int inode = getFdByInodeNumber(fd);
	int filePointer = getFdByFilePointer(fd);
	int dataBlocks = 0;
	//int maxSize;
	//inode가 32보다 작은경우는 inodeBlock4번을 리드
	if (inode < 32)
	{
		FS_Read(4);
		dataBlocks = charToInt((inode * 128) + 33);
		
		//maxSize = charToInt(10 + (inode * 128)) + size;
		
	}
	//inode가 32보다 큰경우는 inodeBlock5번을 리드
	else
	{
		FS_Read(5);
		dataBlocks = charToInt(((inode - 32) * 128) + 33);
		//maxSize = charToInt(10 + ((inode-32) * 128)) + size;
	}

	//datablocks를 통해서 inode가 가리키고 있는 datablocks의 갯수를 구하여
	int datablock[100] = { 0, };
	int s = 0;
	 

	//dataBlocks가 0인경우 dataBlock을 할당한다
	if (dataBlocks == 0)
	{
		datablock[dataBlocks] = getUnassignedBlockNum(2);
		dataBlocks++;
		if (datablock[0] == -1)
		{
			cout << "DataBlock Full\n";
			return;
		}
	}
	
	else
	{
		//datablock을 구한다.
		for (int i = 0; i < dataBlocks; ++i)
		{
			if (inode < 32)
			{
				datablock[i] = charToInt(37 + s + (inode * 128));
				s += 4;
			}
			else
			{
				datablock[i] = charToInt(37 + s + ((inode - 32) * 128));
				s += 4;
			}

		}
	}
	
	
	//filePointer로 시작 dataBlock을 계산
	int pBlockCount = filePointer / BLOCK_SIZE;
	//filePointer로 누적 파일포인터 계산
	int pFilePointer = filePointer % BLOCK_SIZE;

	FS_Read(datablock[pBlockCount]);

	for (int i = 0; i < size; ++i)
	{

		FS_Buffer[pFilePointer] = My_Buffer[MyBufferCount++];

		pFilePointer++;
		filePointer++;

		// dataBlock사이즈를 넘어가는경우
		if ((pFilePointer) % 4096 == 0 && pFilePointer != 0)
		{
			FS_Write(datablock[pBlockCount]); // dataBlock을 쓰기
			datablock[dataBlocks++] = getUnassignedBlockNum(2); // 새로운 dataBlock할당
			pFilePointer = 0;
			pBlockCount++;
		}
		
	}

	if ((filePointer) % 4096 != 0)
		FS_Write(datablock[pBlockCount]);

	//inode의 size를 증가
	setFilePointer(fd, filePointer);
	fflush_MyBuffer();

	if (inode < 32)
	{
		FS_Read(4);
		intToChar(33+(inode*INODE_SIZE), dataBlocks);
		int ss = 0;
		for (int i = 0; i < dataBlocks; ++i)
		{
			intToChar(37 + ss + (inode*INODE_SIZE), datablock[i]);
			ss += 4;
		}
		intToChar(10 + (inode*INODE_SIZE), filePointer);
		getTime(24 + (inode*INODE_SIZE));
		FS_Write(4);
	}
	//inode가 32보다 큰경우는 inodeBlock5번을 리드
	else
	{
		FS_Read(5);
		intToChar(33 + (inode*INODE_SIZE), dataBlocks);
		int ss = 0;
		for (int i = 0; i < dataBlocks; ++i)
		{
			intToChar(37 + ss + (inode*INODE_SIZE), datablock[i]);
			ss += 4;
		}
		intToChar(10 + ((inode - 32)*INODE_SIZE), filePointer);
		getTime(24 + ((inode-32)*INODE_SIZE));
		FS_Write(5);
	}
}

void File_Seek(int fd, int offset)
{
	/*
	File_Seek(int fd, int offset): 파일 디스크립터 fd가 가리키는 파일의 현재 파일 포인터의 값을 갱신한다. 
	offset은 해당 파일의 첫 번째 위치(0번지)부터의 떨어진 거리이다. 
	즉, 파일 포인터 값을 offset로 설정한다 (file_pointer = offset). 
	예로, offset이 150이면 해당 파일의 150번째 바이트를 가리키는 위치를 의미한다.
	음수이면 파일의 끝을 리턴한다.
	*/
	int inode = getFdByInodeNumber(fd);
	

	if (offset < 0)
	{
		if (inode < 32)
		{
			FS_Read(4);
			
			offset = charToInt((inode * 128) + 10);
			
		}
		//inode가 32보다 큰경우는 inodeBlock5번을 리드
		else
		{
			FS_Read(5);
			offset = charToInt(((inode-32) * 128) + 10);
			
		}
	}
	
	setFilePointer(fd, offset);
	
}





#endif