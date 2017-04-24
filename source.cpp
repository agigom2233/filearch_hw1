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

#pragma warning( disable : 4996)      // scanf 보안 관련 경고 무시

typedef struct _finddata_t FILE_SERARCH;  // 디렉토리 검색을 위해 _finddata_t 형 struct 사용. io.h에 선언되어 있음. 

void SearchingDir(char* path, char* backup, char* logpath);  // 디렉토리 내의 모든 파일을 검색하는 함수.

void Copy(char* src, char* dest, char * path_for_log);  // 파일을 copy하는 함수

int main() {

	char path_name[200];   
	char backuppath_name[200];
	char logpath_name[200];     // 각각 source 파일 경로, backup파일 경로, log파일 저장 경로

	FILE * log;   // 로그 파일 쓰기를 위해 file 형 포인터 선언

	printf("input file path :");
	scanf("%s", path_name);

	printf("\ninput file backup path :");  // 백업경로를 입력할 때에는, 백업 디렉토리 이름도 같이 넣어준다.
	scanf("%s", backuppath_name);

	strcpy(logpath_name, backuppath_name);  // logpath_name 에 백업 파일 경로 저장.
	strcat(logpath_name, "\\mybackuplog.txt");        // 로그 파일 생성을 위해 미리 파일 이름까지 추가

	if (log = fopen(logpath_name, "w+")) {           // fopen 옵션을 w+로 주어서 파일을 새로 쓴다.
		/*printf("로그생성성공\n");*/ fclose(log);       // 문제 조건에 로그 파일의 이전기록에 대한 조건이 없었기 때문에, 
	}												 // 이전 기록은 삭제하고 새로운 기록만 담게 하기 위해서 w+옵션을 설정.
													 // 이전 기록도 남기고 싶다면 a+옵션을 주면 된다.

	SearchingDir(path_name, backuppath_name, logpath_name);  // 디렉토리 검색 함수에 각각의 경로를 넘겨준다.

	printf("Done! Check log file!!\n\n");

	return 0;
}


void SearchingDir(char* path, char* backup, char* logpath) {
	// 소스경로 =  path, 백업경로 = backup, 로그파일경로 = logpath
	// 디렉토리 내의 모든 파일, 서브디렉토리를 검색한다.
	// 서브디렉토리도 백업하게 하기위해서 backuppath도 갱신해가면서 copy 함수에 넘겨준다.

	long current_file;
	char search_Path[200];

	FILE_SERARCH file_search;

	mkdir(backup);  // 백업폴더를 생성한다. 이미 있으면 넘어간다.

	strcpy(search_Path, path);   // serch_path에 소스경로 + '/*.*'를 넣어준다. 뒤의 첨자들은 모든파일을 검색하게끔 해준다.
	strcat(search_Path, "\\*.*");

	int count = 0;  // _findfirst 및 _findnext 함수는 i-node처럼 파일의 정보를 읽어오는데
	                // 첫번째와 두번째는 각각 현재경로와 상위경로를 가르쳐주게 된다.
					// 따라서 그 두가지를 제외시키기 위해, 변수 count로 2번째 파일이름부터 허용하게끔 한다.


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
			// 현재 디렉토리의 경로에 파일 이름까지 추가하여 새로운 경로를 갱신
			// 백업경로도 마찬가지로 파일 이름까지 추가하여 새로운 경로로 갱신한다.

			count++;  // 위에서 설명한 것 처럼 현재 및 상위 폴더 미허용을 위한 count변수 이다.
			
			if ((file_search.attrib & _A_SUBDIR) && count>2)  // 현재 이 파일이 서브디렉토리인지 확인하고
															    // 맞다면 SearchingDir을 재귀적으로 호출하여 탐색한다.
															    // 첫번째와 두번째는 현재 및 상위 디렉토리경로이므로 제외한다.
			{
				
				SearchingDir(newpath, newbackuppath, logpath);
			}

			else if (count > 2) Copy(newpath, newbackuppath, logpath);  // 일반 파일일 경우 copy함수로 그 파일의 경로와 생성할 backup의 경로, log파일의 경로를 넘겨준다.

			else
				continue;

		} while (_findnext(current_file, &file_search) == 0);   // 다음 파일이 없을때 까지 while문 반복

		_findclose(current_file);   // 탐색 종료를 위해 열려있는 파일을 닫아준다.
	}

	return ;
}



void Copy(char* src, char* dest, char * path_for_log) {

	FILE* src_ptr, *dest_ptr, *log_ptr;

	struct stat src_stat;
	struct stat dest_stat;      // 해당 경로의 파일의 정보를 읽기위해 struct stat 형 변수를 선언

	struct utimbuf timebuf;     // 파일의 time관련 정보를 수정하기 위해 utimbuf형 변수를 선언

	long size_of_file;          // 파일의 size를 저장하기 위한 변수
	char* buffer;               // 실제로 파일의 데이터를 담기 위한 buffer

	int file_status_flag = 0;  // 첫 백업시 파일을 새로 쓰게 하기 위한 변수

	if ((src_ptr = fopen(src, "rb")) == NULL) {
		printf("읽기실패\n\n");
	}

	if ((dest_ptr = fopen(dest, "r+b")) == NULL) {
		dest_ptr = fopen(dest, "w+b");
		file_status_flag = 1;
		printf("쓰기실패. 새 파일을 생성합니다.\n\n");
	};

	if ((log_ptr = fopen(path_for_log, "a+")) == NULL) {
		printf("로그생성실패 \n\n");
	}

	stat(src, &src_stat);
	stat(dest, &dest_stat);

	if (src_stat.st_mtime > dest_stat.st_mtime || (dest_stat.st_size == 0 && (file_status_flag == 1))) {
		// 첫 백업시, 수정시간은 무조건 최근시간이 되어버리므로, file_status_flag 변수의 유무로 파일 복사를 허용시킴
		// 소스 파일의 크기가 0인 경우에도 복사를 허용시킨다.

		fseek(src_ptr, 0, SEEK_END);      // 파일의 끝 부분으로 포인터를 옮긴다.
		size_of_file = ftell(src_ptr);    // 파일의 사이즈를 변수에 저장시킨다.
		rewind(src_ptr);                  // 파일의 첫 부분으로 포인터를 다시 원위치한다.

		buffer = (char*)malloc(sizeof(char)*size_of_file);      // 파일의 크기만큼 버퍼에 동적 할당시킨다.

		fread(buffer, 1, size_of_file, src_ptr);                // 파일 크기 만큼 버퍼에 읽고
		fwrite(buffer, 1, size_of_file, dest_ptr);              // 파일 크기 만큼 새로 만든 파일에 쓴다.

		if (size_of_file == 0) {                                // 소스 파일의 크기가 0인 경우 알려준다.
			printf("소스 파일의 크기가 0입니다.\n\n");
			fclose(dest_ptr);
			dest_ptr = fopen(dest, "w");
		}

		fputs("수정된 파일 이름 : ", log_ptr);
		fputs(strrchr(src, '\\'), log_ptr);
		fputs("\n", log_ptr);
	}

	fclose(src_ptr);
	fclose(dest_ptr);
	fclose(log_ptr);


	timebuf.actime = src_stat.st_atime;            // 파일의 마지막 접근 시간과, 수정 시간을 timebuf struct에 담아서 
	timebuf.modtime = src_stat.st_mtime;

	utime(dest, &timebuf);                         // 새로 만든 파일의 접근 시간과, 수정 시간을 고쳐준다.

	return;
}