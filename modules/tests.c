#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <assert.h>
#include "h-Files/queue.h"
#include "h-Files/globals.h"
#include "h-Files/logic.h"
#include "h-Files/printConsole.h"

void test_queue_basic() {
    Queue queue;
    queue_init(&queue);

    enqueue(&queue, "/test/path", 1);

    char path[max_path];
    int level;
    int result = dequeue(&queue, path, &level);

    assert(result == 1);
    assert(strcmp(path, "/test/path") == 0);
    assert(level == 1);

    queue_destroy(&queue);
    printf("Test queue_basic passed.\n");
}

void test_queue_empty() {
    Queue queue;
    queue_init(&queue);

    char path[max_path];
    int level;
    int result = dequeue(&queue, path, &level);

    assert(result == 0);
    queue_destroy(&queue);

    printf("Test queue_empty passed.\n");
}

void test_option_no_summary() {
    option_show_summary = 0;

    total_files = 10;
    total_dirs = 5;

    printf("Testing --noSum option:\n");
    print_directory_summary();
    printf("Test option_no_summary passed.\n");
}


void test_process_directory() {
    total_files = 0;
    total_dirs = 1;

    mkdir("test_dir", 0777);
    mkdir("test_dir/subdir", 0777);
    FILE *file = fopen("test_dir/file.txt", "w");
    fclose(file);

    process_directory("test_dir", 0, NULL);

    assert(total_dirs > 1);
    assert(total_files > 0);

    unlink("test_dir/file.txt");
    rmdir("test_dir/subdir");
    rmdir("test_dir");

    printf("Test process_directory passed.\n");
}

void test_multithread_processing() {
    Queue queue;
    queue_init(&queue);

    mkdir("test_dir", 0777);
    mkdir("test_dir/subdir", 0777);
    FILE *file = fopen("test_dir/file.txt", "w");
    fclose(file);

    enqueue(&queue, "test_dir", 0);

    pthread_t threads[4];
    for (int i = 0; i < 4; i++) {
        if (pthread_create(&threads[i], NULL, worker, &queue) != 0) {
            perror("Failed to create thread");
            exit(EXIT_FAILURE);
        }
    }

    pthread_mutex_lock(&queue.lock);
    queue.is_finished = 1;
    pthread_cond_broadcast(&queue.cond);
    pthread_mutex_unlock(&queue.lock);

    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }

    assert(total_dirs > 1);
    assert(total_files > 0);

    unlink("test_dir/file.txt");
    rmdir("test_dir/subdir");
    rmdir("test_dir");

    queue_destroy(&queue);

    printf("Test multithread_processing passed.\n");
}

void test_invalid_directory() {
    struct stat statbuf;
    const char *invalid_path = "/invalid/path";

    int result = stat(invalid_path, &statbuf);
    assert(result == -1);

    printf("Test invalid_directory passed.\n");
}

void test_prune_directory() {
    mkdir("test_prune_dir", 0777);
    mkdir("test_prune_dir/subdir1", 0777);
    mkdir("test_prune_dir/subdir2", 0777);
    FILE *file = fopen("test_prune_dir/file.txt", "w");
    fclose(file);

    pruned_directories[0] = strdup("test_prune_dir/subdir1");
    pruned_dir_count = 1;

    total_files = 0;
    total_dirs = 1;
    process_directory("test_prune_dir/subdir1", 0, NULL);
    assert(total_dirs == 2);
    assert(total_files == 1);

    unlink("test_prune_dir/file.txt");
    rmdir("test_prune_dir/subdir1");
    rmdir("test_prune_dir/subdir2");
    rmdir("test_prune_dir");

    free(pruned_directories[0]);
    pruned_dir_count = 0;

    printf("Test prune_directory passed.\n");
}

void run_tests() {
    test_queue_basic();
    test_queue_empty();
    test_option_no_summary();
    test_process_directory();
    test_multithread_processing();
    test_invalid_directory();
    test_prune_directory();
}
