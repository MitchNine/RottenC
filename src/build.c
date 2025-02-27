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

/*****************************************************************************
* Util Functions
*****************************************************************************/
int exec(const char* command)
{
	char buff[256];
	FILE* fp = popen(command, "r");
	assert(fp && "popen failed");
	while (fgets(buff, sizeof(buff), fp) != NULL) {
		printf("%s", buff);
	}
	return pclose(fp);
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
* Context File
*****************************************************************************/
struct BuildContext {
	long int last_build;
	long int last_modif;
	bool lock;
};
void get_build(struct BuildContext* ctx, char* lockfile)
{
	FILE* build_file_info;
	if ((build_file_info = fopen(lockfile, "r"))) {
		long int mod = 0;
		assert(fread(&ctx->last_build, sizeof(long int), 1, build_file_info) > 0);
		assert(fread(&ctx->last_modif, sizeof(long int), 1, build_file_info) > 0);
		assert(fread(&ctx->lock, sizeof(bool), 1, build_file_info) > 0);
		fclose(build_file_info);
	} else if ((build_file_info = fopen(lockfile, "w"))) {
		fclose(build_file_info);
	}
}
void set_build(struct BuildContext* ctx, char* lockfile)
{
	FILE* build_file_info;
	if ((build_file_info = fopen(lockfile, "w"))) {
		long int mod = 0;
		assert(fwrite(&ctx->last_build, sizeof(long int), 1, build_file_info) > 0);
		assert(fwrite(&ctx->last_modif, sizeof(long int), 1, build_file_info) > 0);
		assert(fwrite(&ctx->lock, sizeof(bool), 1, build_file_info) > 0);
		fclose(build_file_info);
	}
}

/*****************************************************************************
* Compile Files
*****************************************************************************/
int compile_file(const char* path, struct BuildContext* ctx)
{
	char cmd[256];
	char output_dir[256];
	char output_file[256];
	char* out_dir = dirname(strdup(path));
	char* out_file = strtok(basename(strdup(path)), ".");

	bool dep_has_changed = false;

	// Check if file deps have been modified
	// I hate this, but I'm too lazy to implement a better approch
	sprintf(output_file, "%s/%s/%s.d", build_dir, out_dir, out_file);
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
						if (filestat.st_mtim.tv_sec >= ctx->last_build) {
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
		if (filestat.st_mtim.tv_sec < ctx->last_build
				&& !dep_has_changed) {
			return 1;
		}
	}

	// Check if output directory exists, if not, create it
	sprintf(output_dir, "%s/%s", build_dir, out_dir);
	struct stat st = {0};
	if (stat(output_dir, &st) == -1) {
		if (rec_mkdir(output_dir) == -1) {
			printf("Failed %s\n", strerror(errno));
		}
	}

	// Create start of gcc command
	sprintf(cmd, "gcc -c %s -o %s/%s/%s.o ",
				 path, build_dir, out_dir, out_file);

	// Add all the flags to the gcc command
	for (int i = 0; i < (sizeof(c_cflags) / sizeof(c_cflags[0])); i++) {
		sprintf(cmd + strlen(cmd), "%s ", c_cflags[i]);
	}

	printf("[\033[34mBUILD\033[0m] %s\n", path);
	return exec(cmd) == 0 ? 0 : 2;
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
		char* out_dir = dirname(strdup(path));
		char* out_file = strtok(basename(strdup(path)), ".");
		sprintf(cmd + strlen(cmd), "%s/%s/%s.o ", build_dir, out_dir, out_file);
	}

	printf("[\033[34mBUILD\033[0m] %s/%s\n", build_dir, c_exe);
	return (exec(cmd) == 0);
}

/*****************************************************************************
* Main
*****************************************************************************/
int main(int argc, char *argv[])
{
	char build_file_lock[256];
	sprintf(build_file_lock, "%s/build.lock", build_dir);

	// Check if build directory exists, if not, create it
	struct stat st = {0};
	if (stat(build_dir, &st) == -1) {
		if (rec_mkdir(build_dir) == -1) {
			printf("Failed %s\n", strerror(errno));
		}
	}

	struct BuildContext build_ctx = {0};
	get_build(&build_ctx, build_file_lock);

	// Don't run if another build is already running
	if (build_ctx.lock) {
		printf("[\033[31mBUILD\033[0m] Build in progress\n");
		return 1;
	} else {
		build_ctx.lock = true;
		set_build(&build_ctx, build_file_lock);
	}

	// Rebuild the build.c file if it has changed
	struct stat build_filestat;
	assert(stat(build_c, &build_filestat) >= 0 && "Failed to stat build.c");
	if (build_ctx.last_modif < build_filestat.st_mtim.tv_sec) {
		char cmd[256];
		sprintf(cmd, "gcc %s -o %s", build_c, build_exe);
		printf("[\033[35mBUILD\033[0m] %s\n", cmd);
		int ret = exec(cmd);
		if (ret != 0) {
			return ret;
		}
	
		build_ctx.last_modif = time(NULL);
		build_ctx.last_build = time(NULL);
		build_ctx.lock = false;
		set_build(&build_ctx, build_file_lock);

		sprintf(cmd, "%s", build_exe);
		for (int i = 1; i < argc; i++)
			sprintf(cmd, " %s", argv[i]);
		ret = exec(build_exe);
		return ret;
	}
	
	// Compile all the .c files
	bool change = false;
	for (int i = 0; i < (sizeof(c_src) / sizeof(c_src[0])); i++) {
		int ret = compile_file(c_src[i], &build_ctx);
		if (ret == 0) {
			change = true;
		} else if (ret == 2) {
			printf("[\033[31mBUILD\033[0m] Failed to build file\n");
			build_ctx.lock = false;
			set_build(&build_ctx, build_file_lock);
			return 2;
		}
	}
	if (change) {
		if (!compile_exe()) {
			printf("[\033[31mBUILD\033[0m] Failed to build exe\n");
		}
	} else {
		printf("Nothing to recompile\n");
	}
	build_ctx.last_build = time(NULL);
	build_ctx.lock = false;
	set_build(&build_ctx, build_file_lock);

	// Run the executable if -- is the first arg
	// Will also use the rest of the args as the
	// input args of the executable if any are
	// supplyed
	if (argc > 1) {
		if (strstr(argv[1], "--")) {
			char cmd[256];
			sprintf(cmd, "%s/%s", build_dir, c_exe);
			for (int i = 2; i < argc; i++) {
				sprintf(cmd, "%s %s", cmd, argv[i]);
			}
			return exec(cmd);
		}
	}
	return 0;
}
