#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <limits.h>
#include "utils.h"
#include "config.h"
#include <fnmatch.h>

void copy_file(const char *src_path, const char *dst_path) {
    FILE *src = fopen(src_path, "rb");
    FILE *dst = fopen(dst_path, "wb");
    if (!src || !dst) {
        perror("copy_file fopen");
        if (src) fclose(src);
        if (dst) fclose(dst);
        return;
    }

    char buf[4096];
    size_t size;
    while ((size = fread(buf, 1, sizeof(buf), src)) > 0) {
        fwrite(buf, 1, size, dst);
    }

    fclose(src);
    fclose(dst);
}

void log_event(const char *message, const char *src, const char *dst) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char ts[32];
    strftime(ts, sizeof(ts), "[%Y-%m-%d %H:%M:%S]", t);
    printf("%s %s: %s → %s\n", ts, message, src, dst);
}

void mkdir_p(const char *path) {
    char tmp[8192];
    snprintf(tmp, sizeof(tmp), "%s", path);
    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            mkdir(tmp, 0755);
            *p = '/';
        }
    }
    mkdir(tmp, 0755);
}

void sync_directories(const char *src, const char *dst) {
    DIR *dir = opendir(src);
    if (!dir) return;

    mkdir_p(dst);
    struct dirent *entry;
    while ((entry = readdir(dir))) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 || blacklist(entry->d_name) || is_temporary_file(entry->d_name)) continue;

        char src_path[PATH_MAX], dst_path[PATH_MAX];
        snprintf(src_path, sizeof(src_path), "%s/%s", src, entry->d_name);
        snprintf(dst_path, sizeof(dst_path), "%s/%s", dst, entry->d_name);

        struct stat st;
        if (stat(src_path, &st) < 0) continue;

        if (S_ISDIR(st.st_mode)) {
            sync_directories(src_path, dst_path);
        } else if (S_ISREG(st.st_mode)) {
            copy_file(src_path, dst_path);
        }
    }

    closedir(dir);
}

void remove_from_target(const char *relative_path) {
    char dst[8192];
    snprintf(dst, sizeof(dst), "%s/%s", target_dir, relative_path);
    remove(dst);
}

int parse_config_file() {
   
    return 0;
}

int blacklist(const char *black_dir) {
    FILE *file = fopen("/home/nextlevelcode/bin/blacklist.txt", "r");
    if (!file) {
        perror("Cannot open blacklist file");
        return 0;
    }

    char line[MAX_LINE];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\r\n")] = 0;  // Remove newline

        if (strcmp(black_dir, line) == 0) {
            fclose(file);
            return 1;  // Match found
        }
    }

    fclose(file);
    return 0;  // No match
}

int is_temporary_file(const char *filename) {
    return (
        strcmp(filename, "4913") == 0 ||
        fnmatch("*.swp", filename, 0) == 0 ||
        fnmatch("*~", filename, 0) == 0 ||
        fnmatch("*.tmp", filename, 0) == 0 ||
        fnmatch(".#*", filename, 0) == 0
    );
}
