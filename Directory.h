#ifndef DIRECTORY_H
#define DIRECTORY_H

#include "FileSystem.h"


bool checkEntryEmpty(int workspace_inode, char* direc)
{
	int inode = getInodeByName(workspace_inode, direc);

	int inodeBlock = getInodeBlock(workspace_inode);

	inode = inode - 32 * (inodeBlock - INODE_BLOCKS);

	FS_Read(inodeBlock);

	int workspaceBlock = charToInt(inode*INODE_SIZE + 37);

	FS_Read(workspaceBlock);

	int entryCount = 0;

	for (int i = 0; i < BLOCK_SIZE; i += ENTRYSIZE)
	{
		if (FS_Buffer[i] == '1')
			entryCount++;
	}

	if (entryCount > 2)
		return false;

	return true;
}




void Dir_Create(char *direc, int workspace_inode)
{
	/*
	Dir_Create(char *direc): direc에 있는 이름으로 새로운 디렉토리를 만든다.
	새로운 디렉토리를 만드는 과정은 다음과 같다.
	첫 째, 파일 종류가 디렉토리인 새로운 파일을 만든다.
	둘 째, 새로 만들어진 디렉토리의 상위 디렉토리에 새로운 디렉토리 엔트리를 추가한다.
	모든 디렉토리는 기본적으로 두 개의 엔트리를 갖는다.
	하나는 현재 디렉토리를 나타내는 .과 상위 디렉토리를 나타내는 ..이다.
	기본 접근 권한은 777이다.
	*/

	//버퍼에 저장된 데이터비트맵에서 가장 빠른 빈칸 찾아서 인덱스(dataIndex)를 기록후 0->1
	int blockIndex = getUnassignedBlockNum(BLOCK_BITMAP);

	//버퍼에 저장된 아이노드 비트맵에서 가장 빠른 빈칸 찾아서 인덱스(inodeIndex)를 기록후 0->1
	int inode = getUnassignedBlockNum(INODE_BITMAP);

	//inodeList[inodeIndex]에 inodeData(mode,block(dataIndex) 이하 기술) 추가

	addInode(inode, blockIndex);

	//새로운 엔트리 정보를 새롭게 만든 디렉토리 데이터블락에 추가
	//FS_READ로 현재경로 디렉토리의 데이터블럭을 버퍼에 저장
	//버퍼의 데이터에서 엔트리 카운트를 구해서 그 뒤에 새로운 엔트리 정보 추가
	//FS_WRITE로 현재경로 디렉토리의 데이터블럭 저장

	//해당경로의 엔트리에 새로운 디렉토리 엔트리 추가
	//FS_READ로 현재경로 디렉토리의 데이터블럭을 버퍼에 저장
	//버퍼의 데이터에서 엔트리 카운트를 구해서 그 뒤에 새로운 엔트리 정보 추가
	//FS_WRITE로 해당경로 디렉토리의 데이터블럭 저장

	//하드링크 설정

	addEntry(workspace_inode, blockIndex, inode, strlen(direc), direc);



	//파일사이즈

}

void Dir_Read(char *direc, int size, int workspace_inode)
{
	/*
	Dir_Read(char *direc, void *buffer, int size): 디렉토리 direc에 있는 내용을 읽어 buffer에 저장한다. size는 buffer의 크기이다.
	*/

	int inode = getInodeByName(workspace_inode, direc);

	if (inode == -1)
	{
		cout << "현재 경로에 해당 디렉토리가 존재하지않습니다." << endl;
		return;
	}

	if (!checkDirectory(inode))
	{
		cout << "파일은 ls명령을 실행 할 수 없습니다," << endl;
		return;
	}

	fflush_MyBuffer();

	int block = getInodeBlock(inode);

	inode = inode - 32 * (block - INODE_BLOCKS);

	FS_Read(block);

	int dataBlock = charToInt(37 + INODE_SIZE * inode);

	FS_Read(dataBlock);

	for (int i = 0; i < BLOCK_SIZE; ++i)
		My_Buffer[i] = FS_Buffer[i];
}

void Dir_Unlink(char *direc, int workspace_inode)
{
	/*
	Dir_Unlink(char *direc): direc가 나타내는 디렉토리를 삭제한다.
	또한 해당 디렉토리를 포함하는 상위 디렉토리에서도 관련 내용을 삭제한다.
	해당 디렉토리가 사용하는 data block과 inode도 삭제해야하며 관련 bitmap도 0으로 설정한다.
	*/

	//즉, 빈 디렉토리만 삭제 가능하다.루트(/ ) 디렉토리는 삭제 불가능이다.
	if (direc == "/")
	{
		cout << "Error : Cannot delete Root Directory." << endl;
		return;
	}

	//디렉토리 direc 내에 파일이 존재하면 디렉토리를 삭제하지 않고 에러 메시지를 출력한다.
	bool isEmpty = checkEntryEmpty(workspace_inode, direc);

	if (!isEmpty) //엔트리가 남아있을 때
	{
		cout << "Error : Cannot delete occupied Directory." << endl;
		return;
	}

	//타겟의 indoe
	int inode = getInodeByName(workspace_inode, direc);
	//타겟의 inodeblock
	int inodeBlock = getInodeBlock(inode); //inode count in 1 block is 32
	//타켓의 inodeblock 안의 inode
	inode = inode - 32 * (inodeBlock - INODE_BLOCKS);

	//상위디렉토리의 inodeblock
	int workspaceBlock = getInodeBlock(workspace_inode);

	//해당 디렉토리가 속한 상위 디렉토리에서 엔트리 삭제
	if (deleteEntry(workspace_inode, direc) == -1)
	{
		cout << "ERROR : 디렉토리 제거 실패" << endl;
	}

	//해당 디렉토리가 속한 상위 디렉토리에서 하드링크 -1
	//controlLinkCount(workspace_inode, -1);


	//inode + data Block 초기화
	resetAssignedBlock(BLOCK_BITMAP, getDataBlockByInode(inode, 1));
	//inode + data bitmap 초기화
	resetAssignedBlock(INODE_BITMAP, inode);
}

#endif