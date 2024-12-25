#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <getopt.h>
#include <sys/types.h>

#include "modules/h-Files/globals.h"
#include "modules/h-Files/queue.h"
#include "modules/h-Files/logic.h"
#include "modules/h-Files/tests.h"


//TODO: Braum Fragen => sollen wir Segmentierung nutzen von Queue zu Teilqueues bei den Threads, damit diese nicht die komplette Queue blockieren
// oder sollten wir hier darauf verzichten, weil enqueue/dequeue ja eh nicht lange dauert im Vergleich zu Auslesen des Dateisystems

int main(int argc, char *argv[]) {

  //Tests
    if (argc > 1 && strcmp(argv[1], "--test") == 0) {
        printf("Running tests...\n");

        run_tests();

        printf("All tests passed.\n");
        return 0;
    }

    int opt;

    //TODO: option ignoreCase noch ungenutzt

    //TODO: output-json und output-csv noch ohne Funktion

    //TODO: noch umsetzen: -I | Do not list those files that match the wild-card pattern.


    // Optionstruktur für getopt_long
    static struct option long_options[] = {
        {"noSum", no_argument, &option_show_summary, 0},  // Long-Option für --noSum
        {"dirsfirst", no_argument, &option_dirs_first, 1},
        {"output-json", no_argument, &option_output_json, 1},
        {"output-csv", no_argument, &option_output_csv, 1},
        {"filelimit", required_argument, NULL, 'F'},
        {"prune", no_argument, NULL, 'P'},
        {0, 0, 0, 0}
    };



    while ((opt = getopt_long(argc, argv, "fL:lasdritugpPo:Fh", long_options, NULL)) != -1) {
        if (opt == 0) {
            // Optionen mit Flags wurden gesetzt, überspringe den switch
            continue;
        }
        switch (opt) {
            case 'f':
                option_show_full_path = 1;
                break;
            case 'L':
                option_max_depth = atoi(optarg);
                if (option_max_depth < 0) {
                    fprintf(stderr, "Invalid depth value: %s\n", optarg);
                    return EXIT_FAILURE;
                }
                break;
            case 'l':
                option_follow_symlinks = 1;
                break;
            case 'a':
                option_show_hidden = 1;
                break;
            case 's':
                option_show_file_sizes = 1;
                break;
            case 'd':
                option_dirs_only = 1;
            break;
            case 'r':
                option_reverse_sort = 1;
            break;
            case 'i':
                option_ignore_case = 1;
            break;
            case 't':
                option_sort_time = 1;
            break;
            case 'u':
                option_show_user = 1;
            break;
            case 'g':
                option_show_group = 1;
            break;
            //TODO: add example (./main --prune ./cmake-build-debug) in readme
            case 'p':
            case 'P':  // Long option (--prune)
                if (pruned_dir_count < MAX_PRUNED_DIRS) {
                    // Ensure the path is not NULL and not an empty string
                    if (optarg != NULL && strlen(optarg) > 0) {
                        // Use realpath to resolve relative paths to absolute paths
                        char resolved_path[max_path];
                        if (realpath(optarg, resolved_path) != NULL) {
                            pruned_directories[pruned_dir_count++] = strdup(resolved_path);
                        } else {
                            fprintf(stderr, "Error resolving path: %s\n", optarg);
                        }
                    } else {
                        fprintf(stderr, "Invalid directory path for prune option\n");
                    }
                } else {
                    fprintf(stderr, "Maximum number of pruned directories reached.\n");
                }
                break;
            //TODO: add example (./main -o output.txt .) in readme
            case 'output-json':
                TreeNode *root = build_tree(realpath, 0);
                if (!root) {
                    fprintf(stderr, "Failed to build tree.\n");
                }


                FILE *file = output_file ? fopen(output_file, "w") : stdout;
                if (!file) {
                    perror("Error opening output file");
                    free_tree(root);
                }

                generate_json_output(file, root);

                if (output_file) {
                    fclose(file);
                }

                free_tree(root);

            case 'output-csv':
                if (!root) {
                    fprintf(stderr, "Failed to build tree.\n");
                }

                if (!file) {
                    perror("Error opening output file");
                    free_tree(root);
                }

                generate_csv_output(file, root);

                if (output_file) {
                    fclose(file);
                }

                free_tree(root);

            case 'o':
                if (optarg == NULL) {
                    fprintf(stderr, "Option -o requires an argument\n");
                    exit(EXIT_FAILURE);
                }
            output_file = optarg;
                break;
            case 'F':
                option_file_limit = atoi(optarg);
                break;
            case 'h':
                print_usage(argv[0]);
                return EXIT_SUCCESS;
            default:
                print_usage(argv[0]);
                return EXIT_FAILURE;
        }
    }

    if (output_file != NULL) { // Ausgabedatei erstellen/öffnen
        FILE *file = fopen(output_file, "w");
        if (file == NULL) {
            perror("Error creating output file");
            exit(EXIT_FAILURE);
        }
        fclose(file);
    }

    const char *start_path = (optind < argc) ? argv[optind] : ".";

    struct stat statbuf;
    if (stat(start_path, &statbuf) == -1 || !S_ISDIR(statbuf.st_mode)) {
        fprintf(stderr, "Invalid directory: %s\n", start_path);
        return EXIT_FAILURE;
    }

    Queue queue;
    queue_init(&queue);
    // Start with the initial directory
    enqueue(&queue, start_path, 0);

    // Create thread pool
    pthread_t threads[MAX_THREADS];
    // Explicit error handling for thread creation
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

    // free memory for pruned directories before exit
    for (int i = 0; i < pruned_dir_count; i++) {
        free(pruned_directories[i]);
    }

    return EXIT_SUCCESS;
}



