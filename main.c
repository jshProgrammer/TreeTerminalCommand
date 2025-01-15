#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>

#include "modules/h-Files/globals.h"
#include "modules/h-Files/queue.h"
#include "modules/h-Files/logic.h"
#include "modules/h-Files/tests.h"
#include "modules/h-Files/tree.h"
#include "modules/h-Files/printConsole.h"
#include "modules/h-Files/printFile.h"

int main(int argc, char *argv[]) {

    if (argc > 1 && strcmp(argv[1], "--test") == 0) {
        printf("Running tests...\n");

        run_tests();

        printf("All tests passed.\n");
        return 0;
    }

    int opt;



    const char *start_path = parse_options(argc, argv);


    if (output_file != NULL) {
        FILE *file = fopen(output_file, "w");
        if (file == NULL) {
            perror("Error creating output file");
            exit(EXIT_FAILURE);
        }
        fclose(file);
    }

    if (optind < argc) {
        start_path = argv[optind];
    } else {
        start_path = ".";
    }

    struct stat statbuf;
    if (stat(start_path, &statbuf) == -1 || !S_ISDIR(statbuf.st_mode)) {
        fprintf(stderr, "Invalid directory: %s\n", start_path);
        return EXIT_FAILURE;
    }

    Queue queue;
    queue_init(&queue);
    enqueue(&queue, start_path, 0);

    pthread_t threads[MAX_THREADS];
    for (int i = 0; i < MAX_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, worker, &queue) != 0) {
            perror("Failed to create thread");
            exit(EXIT_FAILURE);
        }
    }

    pthread_mutex_lock(&queue.lock);
    queue.is_finished = 1;
    pthread_cond_broadcast(&queue.cond);
    pthread_mutex_unlock(&queue.lock);

    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    queue_destroy(&queue);
    pthread_mutex_destroy(&counter_mutex);

    print_directory_summary();

    for (int i = 0; i < pruned_dir_count; i++) {
        free(pruned_directories[i]);
    }

    if (option_output_json) {
        FILE *file = fopen(output_file, "w");
        generate_json_output(file, global_root, 0);
    }
    if (option_output_csv) {
        FILE *file = fopen(output_file, "w");
        generate_csv_output(file, global_root);
    }

    free_tree(global_root);
    pthread_mutex_destroy(&tree_mutex);

    return EXIT_SUCCESS;
}