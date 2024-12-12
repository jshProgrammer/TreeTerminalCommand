#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <getopt.h>

//TODO: booleans nutzen

//TODO: Braum Fragen => sollen wir Segmentierung nutzen von Queue zu Teilqueues bei den Threads, damit diese nicht die komplette Queue blockieren
// oder sollten wir hier darauf verzichten, weil enqueue/dequeue ja eh nicht lange dauert im Vergleich zu Auslesen des Dateisystems

#define MAX_PATH 1024
#define MAX_INDENT 128
#define MAX_THREADS 16  // Anzahl der Threads

// Globale Optionen
int option_show_full_path = 0;
int option_max_depth = -1;
int option_follow_symlinks = 0;
int option_show_hidden = 0;
int option_show_file_sizes = 0;
int option_show_summary = 1;

// as ursprüngliches tree Kommando zählt auch das aktuelle Verzeichnis, daher beginnt diese Variable mit 1
// Synchronized counters
pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;
int total_files = 0, total_dirs = 1;


typedef struct QueueNode {
    char path[MAX_PATH];
    int level;
    struct QueueNode *next;
} QueueNode;

typedef struct {
    QueueNode *front;
    QueueNode *rear;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    int is_finished;  // Flag to indicate queue processing is complete
} Queue;

void queue_init(Queue *q) {
    q->front = q->rear = NULL;
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->cond, NULL);
}

void enqueue(Queue *q, const char *path, int level) {
    QueueNode *node = malloc(sizeof(QueueNode));

    if (!node) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    strncpy(node->path, path, MAX_PATH-1);
    node->path[MAX_PATH - 1] = '\0';  // Ensure null-termination
    node->level = level;
    node->next = NULL;

    pthread_mutex_lock(&q->lock);
    if (q->rear) {
        q->rear->next = node;
    } else {
        q->front = node;
    }
    q->rear = node;
    pthread_cond_signal(&q->cond);
    pthread_mutex_unlock(&q->lock);
}

int dequeue(Queue *q, char *path, int *level) {
    pthread_mutex_lock(&q->lock);

    while (q->front == NULL && !q->is_finished) {
        pthread_cond_wait(&q->cond, &q->lock);
    }

    // Exit if queue is empty and finished
    if (q->front == NULL && q->is_finished) {
        pthread_mutex_unlock(&q->lock);
        return 0;
    }

    QueueNode *node = q->front;
    strncpy(path, node->path, MAX_PATH);
    *level = node->level;

    q->front = node->next;
    if (q->front == NULL) {
        q->rear = NULL;
    }

    free(node);
    pthread_mutex_unlock(&q->lock);
    return 1; // Erfolgreich ein Element entnommen
}

void queue_destroy(Queue *q) {
    pthread_mutex_lock(&q->lock);
    QueueNode *current = q->front;
    while (current) {
        QueueNode *next = current->next;
        free(current);
        current = next;
    }
    // Explicitly reset queue state
    q->front = q->rear = NULL;
    pthread_mutex_unlock(&q->lock);

    pthread_mutex_destroy(&q->lock);
    pthread_cond_destroy(&q->cond);
}

void print_usage(const char *program_name) {
    printf("Usage: %s [options] [directory]\n", program_name);
    printf("Options:\n");
    printf("  -f            Show full path\n");
    printf("  -L <n>        Limit the depth of the tree to <n>\n");
    printf("  -l            Follow symbolic links\n");
    printf("  -a            Show hidden files\n");
    printf("  -s            Show file sizes\n");
    printf("  --noSum       Show no summary :)\n");
    printf("  -h            Show this help message\n");
}

void print_indentation(int level) {
    for (int i = 0; i < level; ++i) {
        printf("|   ");
    }
}

void print_file_info(const char *path, const struct stat *statbuf) {
    if (option_show_file_sizes) {
        printf(" [%ld bytes]", statbuf->st_size);
    }
    printf("\n");
}

// Thread-safe counter increment
void increment_counters(bool is_dir) {
    pthread_mutex_lock(&counter_mutex);
    if (is_dir) {
        total_dirs++;
    } else {
        total_files++;
    }
    pthread_mutex_unlock(&counter_mutex);
}

void process_directory(const char *path, int level) {
    if (option_max_depth != -1 && level > option_max_depth) {
        return;
    }

    DIR *dir = opendir(path);
    if (!dir) {
        fprintf(stderr, "Cannot open directory '%s': %s\n", path, strerror(errno));
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        if (!option_show_hidden && entry->d_name[0] == '.') {
            continue;
        }

        char full_path[MAX_PATH];
        if (snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name) >= MAX_PATH) {
            fprintf(stderr, "Path too long: %s/%s\n", path, entry->d_name);
            continue;
        }

        struct stat statbuf;
        if ((option_follow_symlinks ? stat : lstat)(full_path, &statbuf) == -1) {
            fprintf(stderr, "Cannot stat '%s': %s\n", full_path, strerror(errno));
            continue;
        }

        print_indentation(level);
        printf("|-- %s", option_show_full_path ? full_path : entry->d_name);

        print_file_info(full_path, &statbuf);

        if (S_ISDIR(statbuf.st_mode)) {
            increment_counters(true);
            process_directory(full_path, level + 1);
        } else {
            increment_counters(false);
        }
    }

    closedir(dir);
}

void *worker(void *arg) {
    Queue *path_queue = (Queue *)arg;
    char path[MAX_PATH];
    int level;

    // Jeder Thread bearbeitet seine eigene Queue
    while (dequeue(path_queue, path, &level)) {
        process_directory(path, level);
    }

    return NULL;
}

void print_directory_summary() {
    if (option_show_summary) {
        printf("\nSummary:\n");
        printf("Total directories: %d\n", total_dirs);
        printf("Total files: %d\n", total_files);
    }
}
int main(int argc, char *argv[]) {
    int opt;

    // Optionstruktur für getopt_long
    static struct option long_options[] = {
        {"noSum", no_argument, NULL, 'n'},  // Long-Option für --noSum
        {0, 0,  0, 0}  // Null-Terminator
    };

    while ((opt = getopt_long(argc, argv, "fL:lasmh", long_options, NULL)) != -1) {
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
            case 'n':  // Wenn --noSum erkannt wird
                option_show_summary = 0;
                break;
            case 'h':
                print_usage(argv[0]);
                return EXIT_SUCCESS;
            default:
                print_usage(argv[0]);
                return EXIT_FAILURE;
        }
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

    return EXIT_SUCCESS;
}
