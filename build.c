#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <libgen.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

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
  "-Wall",
  "-I./src/",
  "-L/usr/lib/",
  "-I/usr/includes/",
};

struct FileInfo {
  char* dir;
  char* file;
};
bool exec(const char* command)
{
  char buff[256];
  FILE* fp = popen(command, "r");
  if (!fp) {
    perror("popen failed");
    return false;
  }
  while (fgets(buff, sizeof(buff), fp) != NULL) {
    printf("%s", buff);
  }
  pclose(fp);
  return true;
}
void get_output_path(const char* path, struct FileInfo* out)
{
  out->dir = dirname(strdup(path));
  out->file = strtok(basename(strdup(path)), ".");
}
int rec_mkdir(const char *dir) {
    char tmp[256];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp),"%s",dir);
    len = strlen(tmp);
    if (tmp[len - 1] == '/')
        tmp[len - 1] = 0;
    for (p = tmp + 1; *p; p++)
        if (*p == '/') {
            *p = 0;
            mkdir(tmp, S_IRWXU);
            *p = '/';
        }
    return mkdir(tmp, S_IRWXU);
}
bool compile_file(const char* path)
{
  char cmd[256];
  char output_dir[256];
  struct FileInfo fileinfo = {};
  get_output_path(path, &fileinfo);

  // Check if output directory exists
  sprintf(output_dir, "build/%s", fileinfo.dir);
  struct stat st = {0};
  if (stat(output_dir, &st) == -1) {
    printf("Making dir; %s\n", output_dir);
    if (rec_mkdir(output_dir) == -1) {
      printf("Failed %s\n", strerror(errno));
    }
  }

  // Create start of gcc command
  sprintf(cmd, "gcc -c %s -o build/%s/%s.o ",
          path, fileinfo.dir, fileinfo.file);

  // Add all the flags to the gcc command
  for (int i = 0; i < (sizeof(c_cflags) / sizeof(c_cflags[0])); i++) {
    sprintf(cmd + strlen(cmd), "%s ", c_cflags[i]);
  }
  
  printf("[\033[31mBUILD\033[0m] %s\n", path);
  exec(cmd);
  
  return true;
}
bool compile_exe()
{
  char cmd[256] = {0};
  sprintf(cmd, "gcc -o build/%s ", c_exe);
  
  // Add all the flags to the gcc command
  for (int i = 0; i < (sizeof(c_cflags) / sizeof(c_cflags[0])); i++) {
    sprintf(cmd + strlen(cmd), "%s ", c_cflags[i]);
  }

  // Add all the obj files to the command
  for (int i = 0; i < (sizeof(c_src) / sizeof(c_src[0])); i++) {
    char* path = c_src[i];
    struct FileInfo fileinfo = {};
    get_output_path(path, &fileinfo);
    sprintf(cmd + strlen(cmd), "build/%s/%s.o ", fileinfo.dir, fileinfo.file);
  }

  printf("[\033[31mBUILD\033[0m] ./build/%s\n", c_exe);
  exec(cmd);

  return true;
}

int main(int argc, char *argv[])
{
  for (int i = 0; i < (sizeof(c_src) / sizeof(c_src[0])); i++) {
    compile_file(c_src[i]);
  }
  compile_exe();
  return 0;
}
