#include <iostream>
#include <string.h>
#include <string>
#include <stdio.h>
using namespace std;

#include "FileSystem.h"
#include "Command.h"

void usage(char *prog)
{
	printf("usage: %s <disk image file>\n", prog);
}

void runShell()
{

	//엔터하나 무시하기
	//cin.ignore(1);

	while (1)
	{
		cout << "[ JunHa " << stack[pathIndex].path_Name << " ] # ";
		//현재 디렉토리위치찍기
		char input[100];
		cin.getline(input, 100);

		char *ptr;

		ptr = strtok(input, " ");
		int index = 0;
		string str[10];

		while (ptr != NULL)
		{
			str[index++] = ptr;
			ptr = strtok(NULL, " ");
		}
		
		if (str[0] == "quit")
			return;

		if (str[0] == "ls")
		{
			command_ls(str[1]);
		}
		else if (str[0] == "rmdir")
		{
			if (str[1] == "")
				continue;

			command_rmdir(str[1]);
		}
		else if (str[0] == "cat")
		{
			if (str[1] == "")
				continue;

			if (str[2] == "")
			{
				command_cat(str[1]);
			}
			else
			{
				command_cat(str[1], str[2]);
			}
		}
		else if (str[0] == "mkdir")
		{
			if(str[1] == "")
				continue;

			command_mkdir(str[1]);
		}
		else if (str[0] == "rm")
		{
			if(str[1] == "")
				continue;
			command_rm(str[1]);
		}
		else if (str[0] == "pwd")
		{
			command_pwd();
		}
		else if (str[0] == "chmod")
		{
			if (str[1] == "" || str[2] == "")
				continue;

			command_chmod(str[1], str[2]);
		}
		else if (str[0] == "mv")
		{
			if (str[1] == "" || str[2] == "")
				continue;

			command_mv(str[1], str[2]);
		}
		else if (str[0] == "cd")
		{
			command_cd(str[1]);
		}
		else if (str[0] == "cp")
		{
			if (str[1] == "" || str[2] == "")
				continue;

			command_cp(str[1], str[2]);
		}
		else if (str[0] == "paste")
		{
			if (str[1] == "" || str[2] == "")
				continue;

			command_paste(str[1], str[2]);
		}
		else if (str[0] == "split")
		{
			if (str[1] == "")
				continue;

			command_split(str[1]);
		}
		else if (str[0] == "filecopy")
		{
			if (str[1] == "" || str[2] == "")
				continue;

			command_filecopy(str[1], str[2]);

		}
		else if (str[0] == "close")
		{
			if (str[1] == "")
				continue;

			command_close(atoi(str[1].c_str()));
		}
		else if (str[0] == "display")
		{
			if (str[1] != "")
				command_display(str[1].at(0));
		}
		else
		{
			cout << "잘못된 명령어 입니다.\n";
		}

	}
}

int main(int argc, char *argv[])  // 프로그램 실행 시 FS_File명 같이 입력
{
	if (argc != 2) {
		usage(argv[0]);
		return 0;
	}

	char *fs_file = argv[1];

	FS_Init();

	FS_Load(fs_file);

	runShell();

	FS_Save(fs_file);

	return 0;
}
