#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <io.h>
#include <fstream>
#include <direct.h>
#include <time.h>
#include <sys/utime.h>

#pragma warning( disable : 4996)      // scanf ���� ���� ��� ����

typedef struct _finddata_t FILE_SERARCH;  // ���丮 �˻��� ���� _finddata_t �� struct ���. io.h�� ����Ǿ� ����. 

void SearchingDir(char* path, char* backup, char* logpath);  // ���丮 ���� ��� ������ �˻��ϴ� �Լ�.

void Copy(char* src, char* dest, char * path_for_log);  // ������ copy�ϴ� �Լ�

int main() {

	char path_name[200];   
	char backuppath_name[200];
	char logpath_name[200];     // ���� source ���� ���, backup���� ���, log���� ���� ���

	FILE * log;   // �α� ���� ���⸦ ���� file �� ������ ����

	printf("input file path :");
	scanf("%s", path_name);

	printf("\ninput file backup path :");  // �����θ� �Է��� ������, ��� ���丮 �̸��� ���� �־��ش�.
	scanf("%s", backuppath_name);

	strcpy(logpath_name, backuppath_name);  // logpath_name �� ��� ���� ��� ����.
	strcat(logpath_name, "\\mybackuplog.txt");        // �α� ���� ������ ���� �̸� ���� �̸����� �߰�

	if (log = fopen(logpath_name, "w+")) {           // fopen �ɼ��� w+�� �־ ������ ���� ����.
		/*printf("�α׻�������\n");*/ fclose(log);       // ���� ���ǿ� �α� ������ ������Ͽ� ���� ������ ������ ������, 
	}												 // ���� ����� �����ϰ� ���ο� ��ϸ� ��� �ϱ� ���ؼ� w+�ɼ��� ����.
													 // ���� ��ϵ� ����� �ʹٸ� a+�ɼ��� �ָ� �ȴ�.

	SearchingDir(path_name, backuppath_name, logpath_name);  // ���丮 �˻� �Լ��� ������ ��θ� �Ѱ��ش�.

	printf("Done! Check log file!!\n\n");

	return 0;
}


void SearchingDir(char* path, char* backup, char* logpath) {
	// �ҽ���� =  path, ������ = backup, �α����ϰ�� = logpath
	// ���丮 ���� ��� ����, ������丮�� �˻��Ѵ�.
	// ������丮�� ����ϰ� �ϱ����ؼ� backuppath�� �����ذ��鼭 copy �Լ��� �Ѱ��ش�.

	long current_file;
	char search_Path[200];

	FILE_SERARCH file_search;

	mkdir(backup);  // ��������� �����Ѵ�. �̹� ������ �Ѿ��.

	strcpy(search_Path, path);   // serch_path�� �ҽ���� + '/*.*'�� �־��ش�. ���� ÷�ڵ��� ��������� �˻��ϰԲ� ���ش�.
	strcat(search_Path, "\\*.*");

	int count = 0;  // _findfirst �� _findnext �Լ��� i-nodeó�� ������ ������ �о���µ�
	                // ù��°�� �ι�°�� ���� �����ο� ������θ� �������ְ� �ȴ�.
					// ���� �� �ΰ����� ���ܽ�Ű�� ����, ���� count�� 2��° �����̸����� ����ϰԲ� �Ѵ�.


	if ((current_file = _findfirst(search_Path, &file_search)) == -1L) {
		printf("No file in current dir!!\n");
	}

	else {

		do {
			char newpath[200];
			char newbackuppath[200];

			strcpy(newpath, path);
			strcat(newpath, "\\");
			strcat(newpath, file_search.name);

			strcpy(newbackuppath, backup);
			strcat(newbackuppath, "\\");
			strcat(newbackuppath, file_search.name);
			// ���� ���丮�� ��ο� ���� �̸����� �߰��Ͽ� ���ο� ��θ� ����
			// �����ε� ���������� ���� �̸����� �߰��Ͽ� ���ο� ��η� �����Ѵ�.

			count++;  // ������ ������ �� ó�� ���� �� ���� ���� ������� ���� count���� �̴�.
			
			if ((file_search.attrib & _A_SUBDIR) && count>2)  // ���� �� ������ ������丮���� Ȯ���ϰ�
															    // �´ٸ� SearchingDir�� ��������� ȣ���Ͽ� Ž���Ѵ�.
															    // ù��°�� �ι�°�� ���� �� ���� ���丮����̹Ƿ� �����Ѵ�.
			{
				
				SearchingDir(newpath, newbackuppath, logpath);
			}

			else if (count > 2) Copy(newpath, newbackuppath, logpath);  // �Ϲ� ������ ��� copy�Լ��� �� ������ ��ο� ������ backup�� ���, log������ ��θ� �Ѱ��ش�.

			else
				continue;

		} while (_findnext(current_file, &file_search) == 0);   // ���� ������ ������ ���� while�� �ݺ�

		_findclose(current_file);   // Ž�� ���Ḧ ���� �����ִ� ������ �ݾ��ش�.
	}

	return ;
}



void Copy(char* src, char* dest, char * path_for_log) {

	FILE* src_ptr, *dest_ptr, *log_ptr;

	struct stat src_stat;
	struct stat dest_stat;      // �ش� ����� ������ ������ �б����� struct stat �� ������ ����

	struct utimbuf timebuf;     // ������ time���� ������ �����ϱ� ���� utimbuf�� ������ ����

	long size_of_file;          // ������ size�� �����ϱ� ���� ����
	char* buffer;               // ������ ������ �����͸� ��� ���� buffer

	int file_status_flag = 0;  // ù ����� ������ ���� ���� �ϱ� ���� ����

	if ((src_ptr = fopen(src, "rb")) == NULL) {
		printf("�б����\n\n");
	}

	if ((dest_ptr = fopen(dest, "r+b")) == NULL) {
		dest_ptr = fopen(dest, "w+b");
		file_status_flag = 1;
		printf("�������. �� ������ �����մϴ�.\n\n");
	};

	if ((log_ptr = fopen(path_for_log, "a+")) == NULL) {
		printf("�α׻������� \n\n");
	}

	stat(src, &src_stat);
	stat(dest, &dest_stat);

	if (src_stat.st_mtime > dest_stat.st_mtime || (dest_stat.st_size == 0 && (file_status_flag == 1))) {
		// ù �����, �����ð��� ������ �ֱٽð��� �Ǿ�����Ƿ�, file_status_flag ������ ������ ���� ���縦 ����Ŵ
		// �ҽ� ������ ũ�Ⱑ 0�� ��쿡�� ���縦 ����Ų��.

		fseek(src_ptr, 0, SEEK_END);      // ������ �� �κ����� �����͸� �ű��.
		size_of_file = ftell(src_ptr);    // ������ ����� ������ �����Ų��.
		rewind(src_ptr);                  // ������ ù �κ����� �����͸� �ٽ� ����ġ�Ѵ�.

		buffer = (char*)malloc(sizeof(char)*size_of_file);      // ������ ũ�⸸ŭ ���ۿ� ���� �Ҵ��Ų��.

		fread(buffer, 1, size_of_file, src_ptr);                // ���� ũ�� ��ŭ ���ۿ� �а�
		fwrite(buffer, 1, size_of_file, dest_ptr);              // ���� ũ�� ��ŭ ���� ���� ���Ͽ� ����.

		if (size_of_file == 0) {                                // �ҽ� ������ ũ�Ⱑ 0�� ��� �˷��ش�.
			printf("�ҽ� ������ ũ�Ⱑ 0�Դϴ�.\n\n");
			fclose(dest_ptr);
			dest_ptr = fopen(dest, "w");
		}

		fputs("������ ���� �̸� : ", log_ptr);
		fputs(strrchr(src, '\\'), log_ptr);
		fputs("\n", log_ptr);
	}

	fclose(src_ptr);
	fclose(dest_ptr);
	fclose(log_ptr);


	timebuf.actime = src_stat.st_atime;            // ������ ������ ���� �ð���, ���� �ð��� timebuf struct�� ��Ƽ� 
	timebuf.modtime = src_stat.st_mtime;

	utime(dest, &timebuf);                         // ���� ���� ������ ���� �ð���, ���� �ð��� �����ش�.

	return;
}