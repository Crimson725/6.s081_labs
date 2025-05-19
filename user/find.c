#include "kernel/types.h"
#include "kernel/fs.h"
#include "kernel/stat.h"
#include "user/user.h"

#define PATH_MAX 512
/**
 * custom function to handle error message gracefully
 * @param msg error mssage
 * @param path related path or filename
 */
void print_error(const char *msg, const char *path)
{
    fprintf(2, "%s %s \n", msg, path);
}

/**
 * is a directory or not
 * @param st file status
 * @return 1 is a directory, 0 a file
 */
int is_directory(struct stat *st)
{
    return st->type == T_DIR;
}

/**
 * open the directory and check status
 * @param path directory pah
 * @param st file status
 * @return file descriptor, if failed, return -1
 */
int open_directory(const char *path, struct stat *st)
{
    int fd = open(path, 0);
    if (fd < 0)
    {
        print_error("find: cannot open", path);
        return -1;
    }
    if (fstat(fd, st) < 0)
    {
        // fail to get status
        print_error("find: cannot fstat", path);
        close(fd);
        return -1;
    }
    if (!is_directory(st))
    {
        // not a directory
        print_error("usage: find <DIRECTORY>", path);
        close(fd);
        return -1;
    }
    return fd;
}
/**
 * build the complete path
 * @param buffer buffer to store the complete path
 * @param path current path
 * @param name file name or directory name
 */
void build_path(char *buffer, const char *path, const char *name)
{
    if (strlen(path) + 1 + DIRSIZ + 1 > PATH_MAX)
    {
        print_error("find: path too long", path);
        return;
    }
    strcpy(buffer, path);
    // to the end of the string
    char *p = buffer + strlen(buffer);
    // add separator
    *p++ = '/';
    // add file name
    memmove(p, name, DIRSIZ);
    // add end sign
    p[DIRSIZ] = 0;
}
/**
 * check if it's a special directory "." or ".."
 * @param name directory name
 * @return 1 is a special directory, 0 not a special directory
 */
int is_special_directory(const char *name)
{
    return strcmp(name, ".") == 0 || strcmp(name, "..") == 0;
}
/**
 * recursively find the target file
 * @param path current directory
 * @param filename the name of the target file
 */
void find(const char *path, const char *filename)
{
    struct dirent de;
    struct stat st;
    // buffer to store the path
    char buffer[PATH_MAX];
    int fd = open_directory(path, &st);
    if (fd < 0)
    {
        // fail to open the directory, return
        return;
    }
    // traverse
    while (read(fd, &de, sizeof(de)) == sizeof(de))
    {
        if (de.inum == 0)
        {
            // invaldi directory
            continue;
        }
        // get the complete path
        build_path(buffer, path, de.name);
        // get file status
        if (stat(buffer, &st) < 0)
        {
            print_error("find: cannot stat", buffer);
            continue;
        }
        // if directory is not "." or "..", recursively find
        if (is_directory(&st) && !is_special_directory(de.name))
        {
            find(buffer, filename);
        }
        // found
        else if (strcmp(de.name, filename) == 0)
        {
            printf("%s\n", buffer);
        }
    }
    close(fd);
}
int main(int argc, const char *argv[])
{
    if (argc != 3)
    {
        fprintf(2, "usage: find <directory> <filename>\n");
        exit(1);
    }
    find(argv[1], argv[2]);
    exit(0);
}
