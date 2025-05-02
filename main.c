#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <time.h>
#include <limits.h>
#include "config.h"
#include "utils.h"
#include "watcher.h"

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <source_dir> <target_dir>\n", argv[0]);
        return 1;
    }

    realpath(argv[1], source_dir);
    realpath(argv[2], target_dir);

    sync_directories(source_dir, target_dir);

    int fd = inotify_init1(IN_NONBLOCK);
    if (fd < 0) {
        perror("inotify_init1");
        return 1;
    }

    add_watch_recursive(fd, source_dir);
    printf("🛡️  Monitorando alterações em: %s\n", source_dir);

    char buffer[EVENT_BUF_LEN];

    while (1) {
        int length = read(fd, buffer, EVENT_BUF_LEN);
        if (length <= 0) {
            usleep(100000); // 100ms
            continue;
        }

        int i = 0;
        while (i < length) {
            struct inotify_event *event = (struct inotify_event *) &buffer[i];

            const char *watched_path = get_watch_path(event->wd);
            if (!watched_path || !event->len) {
                i += EVENT_SIZE + event->len;
                continue;
            }

            char full_src[PATH_MAX], full_dst[PATH_MAX];
            snprintf(full_src, sizeof(full_src), "%s/%s", watched_path, event->name);
            snprintf(full_dst, sizeof(full_dst), "%s/%s", target_dir, full_src + strlen(source_dir) + 1);

            struct stat st;
            int is_dir = (stat(full_src, &st) == 0 && S_ISDIR(st.st_mode));

            if (event->mask & (IN_CREATE | IN_MOVED_TO)) {
                if (is_dir) {
                    add_watch_recursive(fd, full_src);
                    sync_directories(full_src, full_dst);
                    log_event("📁 Nova pasta", full_src, full_dst);
                } else {
                    copy_file(full_src, full_dst);
                    log_event("📝 Arquivo criado", full_src, full_dst);
                }
            } else if (event->mask & IN_MODIFY && !is_dir) {
                copy_file(full_src, full_dst);
                log_event("✏️ Modificado", full_src, full_dst);
            } else if (event->mask & (IN_DELETE | IN_MOVED_FROM)) {
                remove(full_dst);
                log_event("🗑️ Deletado", full_src, full_dst);
            }

            i += EVENT_SIZE + event->len;
        }
    }

    close(fd);
    return 0;
}
