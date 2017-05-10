#ifndef COMMAND_H
#define COMMAND_H

#include "FileSystem.h"
#include "File.h"
#include "Directory.h"
#include <fstream>
using namespace std;

void command_ls(string path)
{
	/*
	Dir_Read() 함수를 이용하여 해당 디렉토리의 엔트리 정보를 출력한다.
	디렉토리와 일반 파일의 정보를 리눅스 명령어 "ls -ali" 결과처럼 출력한다.

	사용 예
	ls - 현재 디렉토리 엔트리 출력
	ls /a/b - 루트 밑에 디렉토리 a 밑에 디렉토리 b의 엔트리 출력 (절대경로)
	ls ../a - 현 디렉토리의 상위 디렉토리에 있는 디렉토리 a의 엔트리 출력 (상대경로)
	*/

	if (path == "")
		//경로 입력X - 현재
	{
		int workspace_inode = stack[pathIndex - 1].path_Inode;

		char target[255];

		stringToChar(stack[pathIndex].path_Name, target);

		Dir_Read(target, BLOCK_SIZE, workspace_inode);

		int inode_t = getInodeByName(workspace_inode, target);

		int block = getInodeBlock(inode_t); //inode count in 1 block is 32

		inode_t = inode_t - (32 * (block - INODE_BLOCKS));

		FS_Read(block);

		getTime(INODE_SIZE*(inode_t)+14);

		FS_Write(block);
	}
	else
		//경로 입력
	{
		char target[255];

		int workspace_inode = getWorkspaceInodeByPath(path, target);

		if (workspace_inode == -1 && strcmp("/", target) != 0)
			return;

		Dir_Read(target, BLOCK_SIZE, workspace_inode);

		int inode_t = getInodeByName(workspace_inode, target);

		int block = getInodeBlock(inode_t); //inode count in 1 block is 32

		inode_t = inode_t - (32 * (block - INODE_BLOCKS));

		FS_Read(block);

		getTime(INODE_SIZE*(inode_t)+14);

		FS_Write(block);
	}

	// inode  모드  하드링크수  파일사이즈  시간  파일명

	for (int i = 0; i < BLOCK_SIZE; i += ENTRYSIZE)
	{
		if (My_Buffer[0 + i] == '1')
		{
			char name[255];

			int name_size = My_Buffer[i + 5];

			copyMyBufferToData(name_size, i + 6, name);

			int inode = charToInt_My(i + 1);

			int inodeBlock = getInodeBlock(inode); //inode count in 1 block is 32

			inode = inode - 32 * (inodeBlock - INODE_BLOCKS);

			FS_Read(inodeBlock);

			char mode[11];

			copyBufferToData(10, 0 + INODE_SIZE*inode, mode);

			int link = charToInt(29 + INODE_SIZE*inode);

			int size = charToInt(10 + INODE_SIZE*inode);

			char time[6];

			copyBufferToData(5, 14 + INODE_SIZE*inode, time);


			cout << inode << "\t" << mode << "\t" << link << "\t" << "JunHa" << "\t" << "JunHa" << "\t" << size << "\t" << time << "\t" << name << endl;
		}
	}
	cout << endl;
}

void command_mkdir(string path)
{
	/*
	Dir_Create() 함수를 이용하여 새로운 디렉토리를 생성한다.

	사용 예
	mkdir a - 현재 디렉토리에 새로운 디렉토리 a 생성
	mkdir /a/b - 절대경로에 맞는 위치에 디렉토리 b 생성
	mkdir ../a - 상대경로에 맞는 위치에 디렉토리 a 생성

	*/

	if (path == "")
		return;;

	char target[255];

	int workspace_inode = getWorkspaceInodeByPath(path, target);

	if (workspace_inode == -1)
		return;

	int check = getInodeByName(workspace_inode, target);

	if (check != -1)
	{
		if (checkDirectory(check))
			cout << "같은 이름의 디렉토리가 존재합니다." << endl;

		else
			cout << "같은 이름의 파일이 존재합니다." << endl;

		return;
	}

	Dir_Create(target, workspace_inode);
}

void command_rmdir(string path)
{
	/*
	Dir_Unlink() 함수를 이용하여 해당 디렉토리를 삭제한다.

	사용 예
	rmdir a - 현재 디렉토리에 있는 디렉토리 a 삭제
	rmdir /a/b - 절대경로에 맞는 위치에 디렉토리 b 삭제
	rmdir ../a - 상대경로에 맞는 위치에 디렉토리 a 삭제

	*/

	if (path == "")
		return;;

	char target[255];

	int workspace_inode = getWorkspaceInodeByPath(path, target);
	if (workspace_inode == -1)
		return;

	Dir_Unlink(target, workspace_inode);
}

void command_cat(string path)
{
	/*
	cat a - 파일 a의 내용을 20줄씩 화면에 출력한다.엔터를 치면 다음 20줄을 출력한다.
		"q"를 입력하면 종료한다.
	*/

	char file[255] = { 0, };

	char cPath[255] = { 0, };

	stringToChar(path, cPath);

	int workspace_inode = getWorkspaceInodeByPath(cPath, file);
	if (workspace_inode == -1)
		return;
	int fd = File_Open(workspace_inode, file);
	if (fd == -1)
		return;
	int count = 0;
	int index = 0;

	File_Read(fd, 4096);

	while (1)
	{
		
		for  (; index < MyBufferCount; ++index)
		{

			printf("%c", My_Buffer[index]);

			if (My_Buffer[index] == '\n')
			{
				count++;
			}
			
			if (count == 20)
			{
				count = 0;
				break;
			}

			if (count != 20 && index == 4096)
			{
				fflush_MyBuffer();
				index = 0;
				File_Read(fd, 4096);
			}
		}

		

		cout << "\nq : quit enter : continue =>";
		char menu = (char)(getchar());
		
		if (menu == 'q')
		{
			
			setFilePointer(fd, getFdByFilePointer(fd));
			fflush_MyBuffer();
			cin.ignore(1);
			break;
		}
		else if (menu == '\n')
		{
			//if (MyBufferCount == 4096)
				
			if (index == 4096)
			{
				fflush_MyBuffer();
				index = 0;
				File_Read(fd, 4096);
			}
			//cin.ignore(1);
			continue;
		}

	}

}

void command_cat(string s1, string path)
{

	/*
	cat > a - 키보드로 데이터를 입력받아 새로운 파일 a 생성, 기존 파일이 있는 경우 덮어쓰기
	File_Create(), File_Open(), File_Write() 함수 등을 이용하여 구현
	"/quit"을 입력할 때까지 입력한 모든 내용을 저장한다.

	cat >> a - 키보드로 데이터를 입력받아 새로운 파일 a 생성, 기존 파일이 있는 경우 이어쓰기
	File_Create(), File_Open(), File_Write(), File_Seek() 함수 등을 이용하여 구현
	"/quit"을 입력할 때까지 입력한 모든 내용을 저장한다.
	*/

	char file[255] = { 0, };

	char cPath[255] = { 0, };

	stringToChar(path, cPath);


	bool flag = 0;

	int workspace_inode = getWorkspaceInodeByPath(cPath, file);

	if (workspace_inode == -1)
		return;

	int dirCheck = getInodeByName(workspace_inode, file);

	if (checkDirectory(dirCheck))
	{
		cout << "cat: cannot overwrite directory \n";
		return;
	}
	
	if (s1.length() == 2 && s1.at(0) == '>' && s1.at(1) == '>')
	{

		if (File_Create(file, workspace_inode, 1) == -1) // True면 존재하지않는것, False면 존재하는것
			flag = 1;
		
		int fd = File_Open(workspace_inode, file);
		if (fd == -1)
			return;

		if ( flag )
			File_Seek(fd, -1);
		

		const int MAX = 4096 * 10;
		char tempBuffer[MAX];
		int index = 0;

		while (1)
		{
			tempBuffer[index++] = getchar();
			if (index >= 5 && tempBuffer[index - 1] == 't' && tempBuffer[index - 2] == 'i' && tempBuffer[index - 3] == 'u' && tempBuffer[index - 4] == 'q' && tempBuffer[index - 5] == '/')
			{
				tempBuffer[index - 5] = '\0';
				cin.ignore(1);
				break;
			}
		}

		int len = strlen(tempBuffer);
		int size = len;
		int pointer = 0;

		for (int i = 0; i <= len / 4096; ++i)
		{
			if (size > 4096)
			{
				for (int j = 0; j < 4096; ++j)
				{
					My_Buffer[j] = tempBuffer[pointer++];
				}

				File_Write(fd, 4096, flag);
				size -= 4096;
			}
			else
			{
				for (int j = 0; j < size; ++j)
				{
					My_Buffer[j] = tempBuffer[pointer++];
				}
				File_Write(fd, size, flag);
			}
		}
		//File_Close(fd);
	}
	else if (s1.length() == 1 && s1.at(0) == '>')
	{
		int check = File_Create(file, workspace_inode);
		while ( check != 1) // True면 존재하지않는것, False면 존재하는것
		{
			if (check == -99)
			{
				return;
			}
			check = File_Create(file, workspace_inode);
		}
		int fd = File_Open(workspace_inode, file);
		
		if (fd == -1)
			return;

		const int MAX = 4096 * 10;
		char tempBuffer[MAX];
		int index = 0;

		while (1)
		{
			tempBuffer[index++] = getchar();
			if (index >= 5 && tempBuffer[index - 1] == 't' && tempBuffer[index - 2] == 'i' && tempBuffer[index - 3] == 'u' && tempBuffer[index - 4] == 'q' && tempBuffer[index - 5] == '/')
			{
				tempBuffer[index-5] = '\0';
				cin.ignore(1);

				break;
			}
		}

		int len = strlen(tempBuffer);
		int size = len;
		int pointer = 0;

		for (int i = 0; i <= len / 4096; ++i)
		{
			if (size > 4096)
			{
				for (int j = 0; j < 4096; ++j)
				{
					My_Buffer[j] = tempBuffer[pointer++];
				}

				File_Write(fd, 4096, flag);
				size -= 4096;
			}
			else
			{
				for (int j = 0; j < size; ++j)
				{
					My_Buffer[j] = tempBuffer[pointer++];
				}
				File_Write(fd, size, flag);
			}
		}
		//File_Close(fd);
	}
	else
	{
		//cout << "잘못된 명령어!!\n";
		return;
	}
	
	
	
}

void command_rm(int workspace_inode, char* file)
{
	int fileInode = getInodeByName(workspace_inode, file); //?

	int block = getInodeBlock(fileInode);
	fileInode = fileInode - 32 * (block - INODE_BLOCKS);
	FS_Read(block);

	if (FS_Buffer[0 + INODE_SIZE * fileInode] == 'd')
	{
		//a가 디렉토리인 경우에는 해당 디렉토리 및 하위 모든 내용을 같이 삭제한다.

		//삭제할 디렉토리의 데이터블록 가져오기
		int datablock = charToInt( 37 + INODE_SIZE*fileInode);
		//for문을 돌면서 디렉토리와 파일 탐색
		FS_Read(datablock);

		for (int i = 2 * ENTRYSIZE; i < 4096; i += ENTRYSIZE)
		{
			if (FS_Buffer[i] == 0 || FS_Buffer[i] == '0')
				continue;

			int entryInode = charToInt(1 + i);
			int inodeblock = getInodeBlock(entryInode);
			entryInode = entryInode - 32 * (inodeblock - INODE_BLOCKS);
			char name[255];

			copyBufferToData((int)FS_Buffer[5 + i], i + 6, name);

			FS_Read(inodeblock);

			if (FS_Buffer[entryInode*INODE_SIZE] == 'd')
			{
				char c[] = ".";

				command_rm(entryInode, c);

				//디렉토리 삭제부분
				Dir_Unlink(name, fileInode);
				FS_Read(datablock);
				//workspace_inode의 수정, 접근시간, fileSize, 하드링크수 변경
			}
			else
			{
				File_Unlink(name, fileInode);
			}
		}
		//workspace_inode의 수정, 접근시간, fileSize, 하드링크수 변경
	}
	else
	{
		File_Unlink(file, workspace_inode);
	}
}
void command_rm(string path)
{
	/*
	rm a - 파일 a 삭제
	a가 디렉토리인 경우에는 해당 디렉토리 및 하위 모든 내용을 같이 삭제한다
	*/

	if (path == "")
		return;

	char file[255] = { 0, };
	char cPath[255] = { 0, };
	stringToChar(path, cPath);
	int workspace_inode = getWorkspaceInodeByPath(cPath, file);

	if (workspace_inode == -1)
		return;

	int fileInode = getInodeByName(workspace_inode, file); //?

	int block = getInodeBlock(fileInode);
	fileInode = fileInode - 32 * (block - INODE_BLOCKS);
	FS_Read(block);
	if (FS_Buffer[0 + INODE_SIZE * fileInode] == 'd')
	{
		command_rm(workspace_inode, file);
		Dir_Unlink(file, workspace_inode);
	}
	else
	{
		command_rm(workspace_inode, file);

	}
	
}


void command_pwd()
{
	//  /home/data/test

	cout << stack[1].path_Name;

	for (int i = 2; i <= pathIndex; ++i)
	{
		cout << stack[i].path_Name ;

		if (i != pathIndex)
			cout << "/";
	}

	cout << endl;
}

void command_chmod(string mode_s, string path)
{
	/*
	파일 및 디렉토리의 접근권한을 변경한다.
	chmod 222 a - 현재 디렉토리에 있는 파일 a의 접근권한을 -W--W--W-로 변경한다.
	숫자를 이용하여 변경하는 것만 지원한다
	*/

	if (path == "" | mode_s == "")
		return;

	char target[255];

	int workspace_inode = getWorkspaceInodeByPath(path, target);

	if (workspace_inode == -1)
		return;

	int targetInode = getInodeByName(workspace_inode, target);

	string mode = "";

	if (checkDirectory(targetInode))
		mode += 'd';
	else
		mode += '-';

	int size = mode_s.length();

	for (int i = 0; i < size; ++i)
	{
		switch (mode_s[i])
		{
		case '0':
			mode += "---";
			break;
		case '1':
			mode += "--x";
			break;
		case '2':
			mode += "-w-";
			break;
		case '4':
			mode += "r--";
			break;

		case '3':
			mode += "-wx";
			break;
		case '5':
			mode += "r-x";
			break;
		case '6':
			mode += "rw-";
			break;

		case '7':
			mode += "rwx";
			break;
		}
	}

	int targetInodeBlock = getInodeBlock(targetInode);

	targetInode = targetInode - 32 * (targetInodeBlock - INODE_BLOCKS);

	char mode_c[11];

	stringToChar(mode, mode_c);

	FS_Read(targetInodeBlock);

	copyDataToBuffer(mode_c, 10, INODE_SIZE*targetInode + 0);

	FS_Write(targetInodeBlock);

}

void command_mv(string fileName1, string fileName2)
{
	/*
	mv a b - 파일 a를 파일 b로 이동한다.
	*/
	if (fileName1 == "" || fileName2 == "")
		return;

	//file1에 디렉토리의inodeNumber 구해오기
	char file1[255] = { 0, };
	char cPath1[255] = { 0, };
	stringToChar(fileName1, cPath1);
	int workspace_inode1 = getWorkspaceInodeByPath(cPath1, file1);
	if (workspace_inode1 == -1)
		return;

	//file2에 디렉토리의inodeNumber 구해오기
	char file2[255] = { 0, };
	char cPath2[255] = { 0, };
	stringToChar(fileName2, cPath2);
	int workspace_inode2 = getWorkspaceInodeByPath(cPath2, file2);
	if (workspace_inode2 == -1)
		return;

	int dirCheck = getInodeByName(workspace_inode1, file2);

	if (checkDirectory(dirCheck))
	{
		cout << "mv: cannot overwrite directory \n";
		return;
	}

	dirCheck = getInodeByName(workspace_inode2, file2);

	if (checkDirectory(dirCheck))
	{
		cout << "mv: cannot overwrite directory \n";
		return;
	}


	//file1이 현재 open중인지 check open시 retrun;
	int fd1 = File_Open(workspace_inode1, file1);
	if (fd1 == -1)
		return;

	//file2가 현재 open중인지 check open시 retrun ;
	int inode2 = getInodeByName(workspace_inode2, file2);
	if (inode2 != -1)
	{
		//file2로 된 file이 존재하는경우 삭제
		if (!File_Unlink(file2, workspace_inode2)) 
		{
			//현재 open중인 file2 mv 실패
			return;
		}
	}
	

	//파일명변경
	if (workspace_inode1 == workspace_inode2)
	{
		int inode = getInodeByName(workspace_inode1, file1);
		int block = getInodeBlock(inode);
		inode = inode - 32 * (block - INODE_BLOCKS);

		//file1의 수정시간+접근시간변경

		FS_Read(block);
		getTime(14 + inode*INODE_SIZE);
		getTime(24 + inode*INODE_SIZE);
		FS_Write(block);

		//workspace_inode1의 디렉토리 엔트리에 들어가서 file1의 entryCount를 읽은 후

		
		FS_Read(block);

		int dataBlock = charToInt(workspace_inode1*INODE_SIZE + 37);

		FS_Read(dataBlock);

		//file1의 엔트리값을 file2의 파일명으로 변경 + file2의 파일길이로 변경
		for (int i = 0; i < BLOCK_SIZE; i += ENTRYSIZE)
		{
			char name[255];

			copyBufferToData((int)FS_Buffer[i + 5], i + 6, name);

			if (strcmp(name, file1) == 0)
			{
				//이름길이변경
				int len = strlen(file2);
				FS_Buffer[i + 5] = (char)len;
				//이름변경
				int count = 0;
				for (int j = 0; j < len; ++j)
				{
					FS_Buffer[i + 6 + count] = file2[j];
					count++;
				}
				break;
			}
		}
		FS_Write(dataBlock);

		
		//workspace_inode1의 엔트리 변경으로 디렉토리 수정시간변경 + 접근시간변경
		block = getInodeBlock(workspace_inode1);
		FS_Read(block);
		getTime(14 + workspace_inode1*INODE_SIZE);
		getTime(24 + workspace_inode1*INODE_SIZE);
		FS_Write(block);
		

	}
	//파일 이동
	else
	{
		//inodeNumber,file명,file길이
		int inode = getInodeByName(workspace_inode1, file1);
		int block = getInodeBlock(inode);
		inode = inode - 32 * (block - INODE_BLOCKS);
		char len = strlen(file1);

		//file1의 수정시간+접근시간변경
		FS_Read(block);
		getTime(14 + inode*INODE_SIZE);
		getTime(24 + inode*INODE_SIZE);
		FS_Write(block);

		//workspace_inode1에 entry에 file1의 엔트리를 삭제 ==> 엔트리앞번호만 날릴경우 문제생겨서 맨앞은 '0' 나머지는 0으로 초기화 하는
		//반복문을 꼭 돌릴것 현재 구현되어있음
		if (deleteEntry(workspace_inode1, file1, 1) == -1)
		{
			cout << "디렉토리에 파일이 없습니다.\n";
			return;
		}

		//workspace_inode2에 inode, len, file1을 entry에 추가
		addEntry(workspace_inode2, -1, inode, strlen(file1), file1, 1);

		//workspace_inode1의 엔트리 변경으로 디렉토리 수정시간변경 + 접근시간변경
		block = getInodeBlock(workspace_inode1);
		FS_Read(block);
		getTime(14 + workspace_inode1*INODE_SIZE);
		getTime(24 + workspace_inode1*INODE_SIZE);
		FS_Write(block);
		//workspace_inode2의 수정시간+접근시간변경
		block = getInodeBlock(workspace_inode2);
		FS_Read(block);
		getTime(14 + workspace_inode2*INODE_SIZE);
		getTime(24 + workspace_inode2*INODE_SIZE);
		FS_Write(block);
		
	}
}

void command_cp(string fileName1, string fileName2)
{
	/*
	cp a b - 파일 a를 파일 b로 복사한다.
	*/

	if (fileName1 == "" || fileName2 =="")
		return;

	//file1에 디렉토리의inodeNumber 구해오기
	char file1[255] = { 0, };
	char cPath1[255] = { 0, };
	stringToChar(fileName1, cPath1);
	int workspace_inode1 = getWorkspaceInodeByPath(cPath1, file1);
	if (workspace_inode1 == -1)
		return;

	//file2를 만들 디렉토리의 inodeNumber 구해오기
	char file2[255] = { 0, };
	char cPath2[255] = { 0, };
	stringToChar(fileName2, cPath2);
	int workspace_inode2 = getWorkspaceInodeByPath(cPath2, file2);
	if (workspace_inode2 == -1)
		return;

	int dirCheck = getInodeByName(workspace_inode1, file2);

	if (checkDirectory(dirCheck))
	{
		cout << "cp: cannot overwrite directory \n";
		return;
	}

	dirCheck = getInodeByName(workspace_inode2, file2);

	if (checkDirectory(dirCheck))
	{
		cout << "cp: cannot overwrite directory \n";
		return;
	}

	//fileName1이 오픈중인지 체크
	int fd1 = File_Open(workspace_inode1, file1);
	if (fd1 == -1)
	{
		return;
	}

	//fileName2 파일 생성
	int check = File_Create(file2, workspace_inode2);
	while ( check != 1) // True면 존재하지않는것, False면 존재하는것
	{
		if (check == -99)
		{
			return;
		}
		check = File_Create(file2, workspace_inode2);
	}
	int fd2 = File_Open(workspace_inode2, file2);

	if (fd2 == -1)
	{
		return;
	}

	//file의 inodenumber를 가져옴
	int inodeFile1 = getFdByInodeNumber(fd1);
	int inodeFile2 = getFdByInodeNumber(fd2);

	int block = getInodeBlock(inodeFile1);

	inodeFile1 = inodeFile1 - 32 * (block - INODE_BLOCKS);

	//file1의 inodeBlock을 읽어 데이터 쓸준비
	FS_Read(block);

	int maxSize1 = charToInt(10 + INODE_SIZE*inodeFile1);
	int blocks1 = charToInt(33 + INODE_SIZE*inodeFile1);
	
	int s = 0;
	for (int i = 0; i < blocks1; ++i)
	{
		FS_Read(block);

		//파일1의 데이터를 읽어서 파일2에 덮어쓰기
		FS_Read(charToInt(37 + s + INODE_SIZE*inodeFile1));
		int j = 0;
		s += 4;

		for (; j < 4096; ++j)
		{
			My_Buffer[j] = FS_Buffer[j];
			maxSize1--;

			if (maxSize1 == 0)
			{
				j++;
				break;
			}
		}

		//a의 내용을 b에 write
		File_Write(fd2, j, 1);
	}

	File_Seek(fd1, -1);
	//close 의도적인 주석처리
	//File_Close(fd1);
	//File_Close(fd2);


}

void command_cd(string path)
{
	/*
	cd - 현재 작업 디렉토리를 바꾼다.
	cd a - 현재 디렉토리에 있는 디렉토리 a로 이동
	cd /a/b - 루트 밑에 a밑에 있는 디렉토리 b로 이동 (절대 경로)
	cd .. - 상위 디렉토리로 이동
	cd ../a - 상위 디렉토리에 있는 디렉토리 a로 이동 (상대 경로)
	*/

	//현재 디렉토리의 하위 디렉토리
	//초기 pwd는 4(home)
	//FW_READ로 inode blocks를 버퍼에 저장 후 현재inode의 block값을 탐색 후 temp에 저장
	//FS_READ로 temp data block을 버퍼에 저장 
	//temp의 엔트리 목록에서 이동을 원하는 디렉토리명을 비교 후 해당 inode를 dest에 저장 -> 없으면 에러메세지
	//stack에 추가,currentInodeNumber 변경

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

				if (tmpName == ".") // .
				{
					tmpName = "";

					continue;
				}
				else if (tmpName == "..") // ..
				{
					if (tmpIndex == 1)
					{
						cout << "잘못된 경로입니다." << endl;
						return;
					}

					tmpIndex -= 1;

					tmpName = "";
					continue;
				}

				char name[255] = { 0, };

				stringToChar(tmpName, name);

				int inode = getInodeByName(tmp[tmpIndex].path_Inode, name);

				if (!checkDirectory(inode)) //false - Not D
				{
					cout << "Error : 존재하지않는 디렉토리 명 포함" << endl;

					return;
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
				char name[255];

				if (tmpName == ".") // .
				{
					tmpName = "";

					continue;
				}
				else if (tmpName == "..") // ..
				{
					if (tmpIndex == 1)
					{
						cout << "잘못된 경로입니다." << endl;
						return;
					}

					tmpIndex -= 1;

					tmpName = "";
					continue;
				}

				stringToChar(tmpName, name);

				int inode = getInodeByName(tmp[tmpIndex].path_Inode, name);

				if (!checkDirectory(inode)) //false - Not D
				{
					cout << "Error : 존재하지않는 디렉토리 명 포함" << endl;

					return;
				}

				tmpIndex += 1;

				tmp[tmpIndex].path_Inode = inode;
				tmp[tmpIndex].path_Name = tmpName;

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

				if (tmpName == ".") // .
				{
					tmpName = "";

					continue;
				}
				else if (tmpName == "..") // ..
				{
					if (tmpIndex == 1)
					{
						cout << "잘못된 경로입니다." << endl;
						return;
					}

					tmpIndex -= 1;

					tmpName = "";

					int inode_t = tmp[tmpIndex].path_Inode;

					int block = getInodeBlock(inode_t); //inode count in 1 block is 32

					inode_t = inode_t - (32 * (block - INODE_BLOCKS));

					FS_Read(block);

					getTime(INODE_SIZE*(inode_t)+14);

					FS_Write(block);
					continue;
				}


				char name[255] = { 0, };

				stringToChar(tmpName, name);

				int inode = getInodeByName(tmp[tmpIndex].path_Inode, name);

				if (!checkDirectory(inode)) //false - Not D
				{
					cout << "Error : 존재하지않는 디렉토리 명 포함" << endl;

					return;
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

					continue;
				}
				else if (tmpName == "..") // ..
				{
					if (tmpIndex == 1)
					{
						cout << "잘못된 경로입니다." << endl;
						return;
					}

					tmpIndex -= 1;

					tmpName = "";

					continue;
				}

				char name[255];

				stringToChar(tmpName, name);

				int inode = getInodeByName(tmp[tmpIndex].path_Inode, name);

				if (inode == -1)
				{
					cout << "Error : 잘못된 경로 입력" << endl;

					return;
				}

				if (!checkDirectory(inode)) //false - Not D
				{
					cout << "Error : 존재하지않는 디렉토리 명 포함" << endl;

					return;
				}

				tmpIndex += 1;

				tmp[tmpIndex].path_Inode = inode;
				tmp[tmpIndex].path_Name = tmpName;
			}
		}
	}

	////현재경로 -> data/etc
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
	//         tmpName = popBack(tmpName);

	//         if (tmpName == ".") // .
	//         {
	//            tmpName = "";

	//            continue;
	//         }
	//         else if (tmpName == "..") // ..
	//         {
	//            tmpIndex -= 1;

	//            tmpName = "";
	//            continue;
	//         }

	//         char name[255] = { 0, };

	//         stringToChar(tmp[tmpIndex].path_Name, name);

	//         int inode = getInodeByName(tmp[tmpIndex].path_Inode, name);

	//         if (inode == -1)
	//         {
	//            cout << "Error : 잘못된 경로 입력" << endl;

	//            return;
	//         }

	//         if (!checkDirectory(inode)) //false - Not D
	//         {
	//            cout << "Error : 존재하지않는 디렉토리 명 포함" << endl;

	//            return;
	//         }

	//         tmpIndex += 1;

	//         tmp[tmpIndex].path_Inode = inode;
	//         tmp[tmpIndex].path_Name = tmpName;



	//         tmpName = "";

	//      }
	//      else if (i + 1 == len)
	//      {
	//         char name[255];

	//         stringToChar(tmpName, name);

	//         int inode = getInodeByName(tmp[tmpIndex].path_Inode, name);

	//         if (inode == -1)
	//         {
	//            cout << "Error : 잘못된 경로 입력" << endl;

	//            return;
	//         }

	//         if (!checkDirectory(inode)) //false - Not D
	//         {
	//            cout << "Error : 존재하지않는 디렉토리 명 포함" << endl;

	//            return;
	//         }

	//         tmpIndex += 1;

	//         tmp[tmpIndex].path_Inode = inode;
	//         tmp[tmpIndex].path_Name = tmpName;
	//      }
	//   }
	//}

	//현재디렉토리 과정을 모듈화 -> 절대경로도 시작을 루트(/)로 하는 상대경로 방식 적용
	//                     -> 스택을 비우고 top 0, currentInodeNumber 0

	for (int i = 0; i <= tmpIndex; ++i)
		stack[i] = tmp[i];

	pathIndex = tmpIndex;
}

void command_split(string path)
{
	/*
	split - 큰 파일을 절반 크기의 두 개 파일로 분할한다. 분할된 파일 이름은 xaa와 xab이다.
	split a - 현재 디렉토리에 있는 파일 a를 xaa와 xab로 분할하여 저장한다.
	*/

	if (path == "")
		return;

	char file[255] = { 0, };
	char cPath[255] = { 0, };
	stringToChar(path, cPath);
	int workspace_inode = getWorkspaceInodeByPath(cPath, file);
	if (workspace_inode == -1)
		return;
	char splitFile1[] = "xaa";
	char splitFile2[] = "xab";

	//split 할 file open
	int fd1 = File_Open(workspace_inode, file);
	if (fd1 == -1)
		return;

	//xaa 파일 
	int check = File_Create(splitFile1, workspace_inode);
	while ( check  != 1)  // True면 존재하지않는것, False면 존재하는것
	{
		if (check == -99)
		{
			return;
		}
		check = File_Create(splitFile1, workspace_inode);
	}
	int fd2 = File_Open(workspace_inode, splitFile1);

	if (fd2 == -1)
		return;

	//xab 파일 생성
	check = File_Create(splitFile2, workspace_inode);
	while ( check != 1) // True면 존재하지않는것, False면 존재하는것
	{
		if (check == -99)
		{
			return;
		}
		check = File_Create(splitFile2, workspace_inode);
	}
	int fd3 = File_Open(workspace_inode, splitFile2);

	if (fd3 == -1)
		return;


	//file의 inodenumber를 가져옴
	int inodeFile1 = getFdByInodeNumber(fd1);
	int inodeFile2 = getFdByInodeNumber(fd2);
	int inodeFile3 = getFdByInodeNumber(fd3);

	int block = getInodeBlock(inodeFile1);

	inodeFile1 = inodeFile1 - 32 * (block - INODE_BLOCKS);

	//file1의 inodeBlock을 읽어 데이터 쓸준비
	FS_Read(block);

	int maxSize1 = charToInt(10 + INODE_SIZE*inodeFile1);
	int firstSize = maxSize1 / 2;
	int blocks1 = charToInt(33 + INODE_SIZE*inodeFile1);

	int s = 0;

	for (int i = 0; i < blocks1; ++i)
	{
		FS_Read(block);
		//파일2의 데이터를 읽어서 파일1에 이어쓰기
		//cout << charToInt(37 + s + INODE_SIZE*inodeFile1) << '\n';
		FS_Read(charToInt(37 + s + INODE_SIZE*inodeFile1));
		int j = 0;
		int k = 0;
		

		for (; j < 4096; ++j)
		{
			My_Buffer[k++] = FS_Buffer[j];
			maxSize1--;

			if (maxSize1 == 0)
			{
				break;
			} 
			if (maxSize1 == firstSize)
			{
				File_Write(fd2, k, 1);
				FS_Read(block);
				FS_Read(charToInt(37 + s + INODE_SIZE*inodeFile1));
				k = 0;
			}
		}

		//a의 내용을 b에 write
		if ( maxSize1 >= firstSize)
			File_Write(fd2, k, 1);
		else
			File_Write(fd3, k, 1);

		s += 4;
	}

	File_Seek(fd1, -1);
	//close 의도적인 주석처리
	//File_Close(fd1);
	//File_Close(fd2);

}

void command_paste(string fileName1, string fileName2)
{
	/*
	paste - 입력된 두 개의 파일에 대해 첫 번째 파일의 끝에 두 번째 파일의 내용을 추가한다.
	*/

	if (fileName1 == "" || fileName2 == "")
		return;

	//file1에 디렉토리의inodeNumber 구해오기
	char file1[255] = { 0, };
	char cPath1[255] = { 0, };
	stringToChar(fileName1, cPath1);
	int workspace_inode1 = getWorkspaceInodeByPath(cPath1, file1);
	if (workspace_inode1 == -1)
		return;

	//file2에 디렉토리의inodeNumber 구해오기
	char file2[255] = { 0, };
	char cPath2[255] = { 0, };
	stringToChar(fileName2, cPath2);
	int workspace_inode2 = getWorkspaceInodeByPath(cPath2, file2);
	if (workspace_inode2 == -1)
		return;
	//fileName1이 오픈중인지 체크
	int fd1 = File_Open(workspace_inode1, file1);
	if (fd1 == -1)
	{
		return;
	}
	//fileName2이 오픈중인지 체크
	int fd2 = File_Open(workspace_inode2, file2);
	if (fd2 == -1)
	{
		return;
	}

	//file의 inodenumber를 가져옴
	int inodeFile1 = getFdByInodeNumber(fd1);
	int inodeFile2 = getFdByInodeNumber(fd2);

	int block = getInodeBlock(inodeFile2);

	inodeFile2 = inodeFile2 - 32 * (block - INODE_BLOCKS);

	//file1의 inodeBlock을 읽어 데이터 쓸준비
	FS_Read(block);

	//file1의 filePointer를 끝으로 설정
	File_Seek(fd1, -1);

	int maxSize2 = charToInt(10 + INODE_SIZE*inodeFile2);
	int blocks2 = charToInt(33 + INODE_SIZE*inodeFile2);
	int s = 0;
	for (int i = 0; i < blocks2; ++i)
	{
		FS_Read(block);

		//파일2의 데이터를 읽어서 파일1에 이어쓰기
		FS_Read(charToInt(37 + s + INODE_SIZE*inodeFile2));
		int j = 0;
		s += 4;
		for (; j < 4096; ++j)
		{
			My_Buffer[j] = FS_Buffer[j];
			maxSize2--;

			if (maxSize2 == 0)
			{
				j++;
				break;

			}
		}

		//a의 내용을 b에 write
		File_Write(fd1, j, 1);
	}

	File_Seek(fd2, -1);
	//close 의도적인 주석처리
	//File_Close(fd1);
	//File_Close(fd2);
}

void filecopy(string fileName ,int fd)
{
	ifstream fin;

	fin.open(fileName.c_str());

	if (fin.fail())
	{
		cout << "file open fail" << '\n';
		return;
	}


	const int MAX = 4096 * 100;
	char tempBuffer[MAX];
	int index = 0;

	char next;
	fin.get(next);

	while (!fin.eof())
	{
		
		tempBuffer[index++] = next;

		fin.get(next);
	}
	tempBuffer[index] = '\0';

	int len = strlen(tempBuffer);
	int size = len;
	int pointer = 0;

	for (int i = 0; i <= len / 4096; ++i)
	{
		if (size > 4096)
		{
			for (int j = 0; j < 4096; ++j)
			{
				My_Buffer[j] = tempBuffer[pointer++];
			}

			File_Write(fd, 4096, 1);
			size -= 4096;
		}
		else
		{
			for (int j = 0; j < size; ++j)
			{
				My_Buffer[j] = tempBuffer[pointer++];
			}
			File_Write(fd, size, 1);
		}
	}

	fin.close();

}

void command_filecopy(string fileName, string path)
{
	/*
	filecopy a b - 이 명령어는 리눅스에는 없지만 테스트를 위해 만든 별도의 명령어이다. 
	실제 하드디스크에서 a라는 파일을 읽은 후에 구현한 파일 시스템에 새로운 파일 b를 만든다. 
	a와 b의 파일 내용은 동일하다.
	*/

	if (path == "")
		return;
	char file[255] = { 0, };

	char cPath[255] = { 0, };

	stringToChar(path, cPath);

	int workspace_inode = getWorkspaceInodeByPath(cPath, file);
	if (workspace_inode == -1)
		return;

	int dirCheck = getInodeByName(workspace_inode, file);

	if (checkDirectory(dirCheck))
	{
		cout << "filecopy: cannot overwrite directory \n";
		return;
	}

	int check = File_Create(file, workspace_inode);

	while (check != 1) // True면 존재하지않는것, False면 존재하는것
	{
		if (check == -99)
		{
			return;
		}
		check = File_Create(file, workspace_inode);
	}
	int fd = File_Open(workspace_inode, file);

	if (fd == -1)
		return;

	filecopy(fileName, fd);


}

void command_close(int fd)
{
	/*
	close fd - 리눅스에는 없는 명령어지만 파일 디스크립터 번호가 fd인 파일을 닫기 위해 사용된다. File_Close() 함수를 호출하여 동작한다.
	주의: File_Open() 함수로 호출된 모든 파일은 close 명령어가 실행되기 전에는 절대 File_Close() 함수를 호출하지 않는다.
	나중을 위해 File_Close() 함수를 호출하는 곳에 코멘트 처리한다.
	예로 cat a하면 파일 a 내용을 확인하기 위해 File_Open() 함수를 호출하고 모든 내용을 확인한 후에는 File_Close() 함수를 호출하여 종료하는 것이 정상적인 절차이다.
	그러나 여기에서는 디스플레이를 통해 테이블 정보를 확인하기 위해 종료하지 않는다.
	*/

	File_Close(fd);


	if (fd == -1)
	{
		for (int i = 3; i < 20; ++i)
		{
			File_Close(i);
		}
	}
}

void command_display(char a)
{
	/*
	display - 리눅스에는 없는 명령어지만 디스플레이 기능을 위해 만든 별도의 명령어이다.
	display t: 디스플레이의 "테이블 정보 출력" 기능 수행
	display n: 디스플레이의 "블록 정보 출력" 기능 수행 - 블록 번호 n의 정보를 출력한다.
	*/


	/*
	t & 4 번은 예제 데이터 없음 / for문 수정 - 현재 돌아가게만 해놓음 / 구분자 나중에 수정
	*/
	string resultDisplay;
	int inodeCount = 0;
	char mode[11];
	int size = 0;  //10
	char time[6];// = {} //14
	char ctime[6]; //19
	char mtime[6]; //24
	//int links_count;; //29
	int blocks ; //33
	//int block[22];// = blockIndex; //37
	int s = 0;
	int index = 0;

	unsigned char temp[4096];


	switch (a)
	{
	case 't':
		cout << "<< File Descriptor Table >>" << endl;
		cout << "Index" << "\t\t" << "SF Index" << endl;

		for (int i = 0; i < 3; i++)
		{
			cout << i << "\t\t" << "-" << endl;
		}

		for (int j = 3; j < NUMBEROFINODES; j++) //NUMBEROFINODES
		{
			if (FileDescriptorTable[j] != -1)
				cout << j << "\t\t" << FileDescriptorTable[j] << endl;
		}

		cout << endl << "<< System File Table >>" << endl;
		cout << "Index" << "\t\t" << "Inode index" << "\t\t" << "File Pointer" << endl;

		for (int k = 0; k < NUMBEROFINODES; k++)//NUMBEROFINODES
		{
			if (SystemFileTable[k].inode_number != -1)
				cout << k << "\t\t" << SystemFileTable[k].inode_number << "\t\t\t" << SystemFileTable[k].file_pointer << endl;
		}

		cout << endl << "<< Inode Tabel >>" << endl;
		cout << "Index" << "\t\t" << "inode Number" << endl;

		for (int w = 0; w < NUMBEROFINODES; w++) //NUMBEROFINODES
		{
			if (inodeTable[w].inode_number != -1)
				cout << w << "\t\t" << inodeTable[w].inode_number << endl;
		}

		break;

	case '0':
		FS_Read(0);

		charToChar(FS_Buffer, temp, 0, 8);
		
		cout << "Magic Number : " << temp << endl; // 0 - 7 FS_Buffer[] 
		
		charToChar(FS_Buffer, temp, 8, 3);
		
		cout << "Block Size : " << temp << endl; // 8 -10
		

		charToChar(FS_Buffer, temp, 11, 3);
		cout << "Blocks Per Group : " << temp << endl; // 100 - 11-13
		

		charToChar(FS_Buffer, temp, 14, 2);
		cout << "Number of Inodes : " << temp << endl; // 64 - 14-15

		charToChar(FS_Buffer, temp, 16, 3);

		cout << "Inode Size : " << temp << endl; //128 - 16-18

		charToChar(FS_Buffer, temp, 19, 16);
		cout << "날짜 및 시간 : " << temp << endl; // 0000-00-00-00-00 19-34

		charToChar(FS_Buffer, temp, 35, 1);
		cout << "첫 번째 Data Block 번호 : " << temp << endl; // 35

		charToChar(FS_Buffer, temp, 36, 1);
		cout << "첫 번째 Inode : " << temp << endl; // 36

		break;

	case '1':
		FS_Read(1);

		cout << "Block Bitmap : " << charToInt(0) << endl;
		resultDisplay.erase();

		cout << "Inode Bitmap : " << charToInt(4) << endl;
		resultDisplay.erase();

		cout << "첫 번째 Inode Table : " << charToInt(8) << endl;
		resultDisplay.erase();

		///////////////////////////////////////

		cout << "미 할당된 블록 수 : " << charToInt(12) << endl; // 12
		cout << "미 할당된 Inode 수 : " << charToInt(16) << endl;

		break;

	case '2':

		FS_Read(2);

		for (int i = 0; i < NUM_BLOCKS; i++)
		{
			
			cout << FS_Buffer[i];
			if ((i+1)%10 == 0)
			{
				cout << "\n";
			}
		}
		cout << '\n';

		break;

	case '3':

		FS_Read(3);

		for (int i = 0; i < NUMBEROFINODES; i++)
		{
			cout << FS_Buffer[i];

			if ((i + 1) % 10 == 0)
			{
				cout <<  "\n";
			}
		}

		cout << '\n';

		break;

	case '4':

		FS_Read(1);
		
		inodeCount = 64 - charToInt(16);

		cout << "< 사용중인 Inode 수 : " << inodeCount << " >" << endl << endl;

		FS_Read(4);

		for (int i = 0; i < inodeCount; i++)
		{
			cout << "inode : " << i << endl;
			
			for (int j = 0 + (i * 128); j < 10 + (i * 128); ++j)
			{
				mode[index++] = FS_Buffer[j];
			}
			mode[index] = '\0';
			index = 0;

			cout << "mode : " << mode << endl;
			
			cout << "size : " << charToInt(10 + (i * 128)) << endl;

			for (int j = 14 + (i * 128); j < 19 + (i * 128); ++j)
			{
				time[index++] = FS_Buffer[j];
			}
			time[index] = '\0';
			index = 0;

			cout << "time : " << time << endl;

			for (int j = 19 + (i * 128); j < 24 + (i * 128); ++j)
			{
				ctime[index++] = FS_Buffer[j];
			}
			ctime[index] = '\0';
			index = 0;

			cout << "ctime : " << ctime << endl;

			for (int j = 24 + (i * 128); j < 29 + (i * 128); ++j)
			{
				mtime[index++] = FS_Buffer[j];
			}
			mtime[index] = '\0';
			index = 0;

			cout << "mtime : " << mtime << endl;

			cout << "links_count : " << charToInt(29 + (i * 128)) << endl;
			
			blocks = charToInt(33 + (i * 128));
			cout << "blocks : " << blocks << endl;

			s = 0;
			cout << "block[] : ";
			for (int j = 0; j < blocks; ++j)
			{
				cout << charToInt(37 + s + (i * 128)) << " " ;
				s += 4;
			}
			cout << endl;
			
			if (i == 32)
			{
				FS_Read(5);
			}

			cout << '\n';
		}

		
		cout << endl;
		break;

	default:
		cout << "알 수 없는 명령어 입니다.";
		break;
	}
}

#endif
