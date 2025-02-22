#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <libgen.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <time.h>
#include <utime.h>

/*****************************************************************************
* Settings
*****************************************************************************/
char* c_exe   = "crot";
char* c_src[] =
{
	"src/main.c",
	"src/lexer/lexer.c",
	"src/coredata/allocators/allocator.c",
	"src/coredata/hashmap/hashmap.c"
};
char* c_cflags[] =
{
#ifdef DEBUG
	"-g",
	"-pg",
	"-fsanitize=address",
#else
	"-O3",
#endif
	"-MD",
	"-Wall",
	"-I./src/",
	"-L/usr/lib/",
	"-I/usr/includes/",
};
char* build_dir = "./build";
char* build_c = "./src/build.c";
char* build_exe = "./gcc_build";
struct stat build_filestat;

/*****************************************************************************
* Util Functions
*****************************************************************************/
bool exec(const char* command)
{
	char buff[256];
	FILE* fp = popen(command, "r");
	assert(fp && "popen failed");
	while (fgets(buff, sizeof(buff), fp) != NULL) {
		printf("%s", buff);
	}
	pclose(fp);
	return true;
}
int rec_mkdir(const char *dir) {
	char tmp[256];
	snprintf(tmp, sizeof(tmp),"%s",dir);

	size_t len = strlen(tmp);
	tmp[len - 1] = tmp[len - 1] == '/' ? 0 : tmp[len - 1];
	for (char* p = tmp + 1; *p; p++)
		if (*p == '/') {
			*p = 0;
			mkdir(tmp, S_IRWXU);
			*p = '/';
		}
	return mkdir(tmp, S_IRWXU);
}

/*****************************************************************************
* Get File Info
*****************************************************************************/
struct FileInfo {
	char* dir;
	char* file;
};
void get_output_path(const char* path, struct FileInfo* out)
{
	out->dir = dirname(strdup(path));
	out->file = strtok(basename(strdup(path)), ".");
}

/*****************************************************************************
* Compile Files
*****************************************************************************/
bool compile_file(const char* path)
{
	char cmd[256];
	char output_dir[256];
	char output_file[256];
	struct FileInfo fileinfo = {};
	get_output_path(path, &fileinfo);

	bool dep_has_changed = false;

	// Check if file deps have been modified
	sprintf(output_file, "%s/%s/%s.d", build_dir, fileinfo.dir, fileinfo.file);
	FILE* depfile;
	if ((depfile = fopen(output_file, "r"))) {
		size_t len = 0;
		char* line = NULL;
		while (getline(&line, &len, depfile) != -1) {
			char* linecpy = strdup(line);

			char* tok = strtok(linecpy, " ");
			while (tok != NULL) {
				int strlen = strnlen(tok, 256);
				if (tok[strlen - 1] == '\n') tok[(strlen--) - 1] = 0;

				if ((tok[strlen] == '\0' && tok[strlen - 1] == 'h')) {
					if (strstr(tok, "src")) {
						struct stat filestat;
						assert(stat(tok, &filestat) >= 0 && "Failed to stat file");
						if (filestat.st_mtim.tv_sec >= build_filestat.st_mtim.tv_sec) {
							dep_has_changed = true;
						}
					}
				}
				tok = strtok(NULL, " ");
			}
			
			free(linecpy);
		}

		if (line)
			free(line);
		fclose(depfile);

		// Check if file itself hasn't changed since last build
		struct stat filestat;
		assert(stat(path, &filestat) >= 0 && "Failed to stat file");
		if (filestat.st_mtim.tv_sec < build_filestat.st_mtim.tv_sec
				&& !dep_has_changed) {
			return true;
		}
	}


	// Check if output directory exists
	sprintf(output_dir, "%s/%s", build_dir, fileinfo.dir);
	struct stat st = {0};
	if (stat(output_dir, &st) == -1) {
		if (rec_mkdir(output_dir) == -1) {
			printf("Failed %s\n", strerror(errno));
		}
	}

	// Create start of gcc command
	sprintf(cmd, "gcc -c %s -o %s/%s/%s.o ",
				 path, build_dir, fileinfo.dir, fileinfo.file);

	// Add all the flags to the gcc command
	for (int i = 0; i < (sizeof(c_cflags) / sizeof(c_cflags[0])); i++) {
		sprintf(cmd + strlen(cmd), "%s ", c_cflags[i]);
	}

	printf("[\033[34mBUILD\033[0m] %s\n", path);
	exec(cmd);

	return true;
}
bool compile_exe()
{
	char cmd[256] = {0};
	sprintf(cmd, "gcc -o %s/%s ", build_dir, c_exe);

	// Add all the flags to the gcc command
	for (int i = 0; i < (sizeof(c_cflags) / sizeof(c_cflags[0])); i++) {
		sprintf(cmd + strlen(cmd), "%s ", c_cflags[i]);
	}

	// Add all the obj files to the command
	for (int i = 0; i < (sizeof(c_src) / sizeof(c_src[0])); i++) {
		char* path = c_src[i];
		struct FileInfo fileinfo = {};
		get_output_path(path, &fileinfo);
		sprintf(cmd + strlen(cmd), "%s/%s/%s.o ", build_dir, fileinfo.dir, fileinfo.file);
	}

	printf("[\033[34mBUILD\033[0m] %s/%s\n", build_dir, c_exe);
	exec(cmd);

	return true;
}

/*****************************************************************************
* Main
*****************************************************************************/
int main(int argc, char *argv[])
{
	assert(stat(build_c, &build_filestat) >= 0 && "Failed to stat build.c");


	char build_file_lock[256];
	sprintf(build_file_lock, "%s/build.lock", build_dir);

	bool rebuild = false;
	FILE* build_file_info;
	if ((build_file_info = fopen(build_file_lock, "a+"))) {
		long int mod = 0;
		if (fread(&mod, sizeof(long int), 1, build_file_info) > 0) {
			if (mod < build_filestat.st_mtim.tv_sec) {
				rebuild = true;
			}
		} else {
			rebuild = true;
		}

		fclose(build_file_info);
	} else {
		printf("Build may already be running\n");
		return 2;
	}


	if (rebuild) {
		printf("[\033[34mBUILD\033[0m] %s\n", build_c);
		char cmd[256];
		sprintf(cmd, "gcc %s -o %s", build_c, build_exe);
		
		exec(cmd);


		if ((build_file_info = fopen(build_file_lock, "w+"))) {
			fwrite(&build_filestat.st_mtim.tv_sec, sizeof(long int), 1, build_file_info);
		}
	}

build:

	for (int i = 0; i < (sizeof(c_src) / sizeof(c_src[0])); i++) {
		compile_file(c_src[i]);
	}
	compile_exe();

	//struct utimbuf new_times;
	//new_times.actime = build_filestat.st_atime;
	//new_times.modtime = time(NULL);
	//assert(utime(build_c, &new_times) >= 0 && "Updating build.c mod time failed");

	if (argc > 1) {
		if (strstr(argv[1], "--")) {
			char cmd[256];
			sprintf(cmd, "%s/%s", build_dir, c_exe);
			for (int i = 2; i < argc; i++) {
				sprintf(cmd, "%s %s", cmd, argv[i]);
			}
			exec(cmd);
		}
	}

eof:
	return 0;
}
