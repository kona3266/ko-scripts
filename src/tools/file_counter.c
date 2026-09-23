#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>


long countFiles(const char *path) {
    long fileCount = 0;
    DIR *dir;
    struct dirent *entry;
    struct stat statbuf;
    char fullPath[PATH_MAX];

    dir = opendir(path);
    if (dir == NULL) {
        perror("opendir");
        return -1;
    }
	// remove end '/' in path
	int length = strlen(path);
    char *fpath = (char *)malloc((length + 1)*sizeof(char));
	strcpy(fpath, path);
    if (fpath[length-1] == '/') fpath[length-1] = '\0'; 
 
    printf("search %s\n", fpath);
    // array of char* pointers
    char **subdir;
    // allocate 10 area of pointer to char
    int init=10;
    subdir = malloc(sizeof(char *)*init);
    int subsize = 0;

    while ((entry = readdir(dir)) != NULL) {
        snprintf(fullPath, PATH_MAX, "%s/%s", fpath, entry->d_name);
        if (lstat(fullPath, &statbuf) == -1) {
            perror("lstat");
            continue;
        }

        if (S_ISREG(statbuf.st_mode)) {
			printf("    find %s\n", fullPath);
            fileCount++;
        } else if (S_ISDIR(statbuf.st_mode)) {

            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                if (subsize >= init){
                    // need realloc more space for subdir
                    init = init * 2;
                    printf("expand size to %d\n", init);
                    subdir = realloc(subdir, sizeof(char*)*init);
                }
                subdir[subsize] = malloc(sizeof(char)*(strlen(fullPath) + 1));
                strcpy(subdir[subsize], fullPath);
                subsize++;
            }
        }
    }

    for(int i = 0; i < subsize; i++){
        long subDirCount = countFiles(subdir[i]);
        if (subDirCount == -1) {
            closedir(dir);
            return -1;
        }
        fileCount += subDirCount;
    }
	free(fpath);
    for (int i=0; i <subsize; i++){
        free(subdir[i]);
    }
    free(subdir);

    closedir(dir);
    return fileCount;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <directory_path>\n", argv[0]);
        return 1;
    }

    long fileCount = countFiles(argv[1]);
    if (fileCount == -1) {
        return 1;
    }

    printf("Directory '%s' contains %ld files.\n", argv[1], fileCount);
    return 0;
}
