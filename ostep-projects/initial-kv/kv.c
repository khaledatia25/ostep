#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_DATABASE_ENTRIES 100
#define DATABASE_FILE "./database.txt"

// In-memory storage for keys and values
char *values[MAX_DATABASE_ENTRIES];
int   keys[MAX_DATABASE_ENTRIES];
int   total = 0; // current number of entries


int splitByComma(char *str, char *splitted[], int max_parts) {
    int count = 0;
    for (int i = 0; i < max_parts && (splitted[i] = strsep(&str, ",")) != NULL; i++)
        count++;
    return count;
}


FILE *ensureDatabaseFile() {
    FILE *fptr = fopen(DATABASE_FILE, "a+");
    if (fptr == NULL) {
        fprintf(stderr, "error: could not open database file\n");
        exit(1);
    }
    rewind(fptr);
    return fptr;
}

// Read all key-value pairs from database.txt into the in-memory arrays
// Each line in the file is formatted as: key,value
void readFile(FILE *fptr) {
    char *line = NULL;
    size_t line_buf_len = 0;
    ssize_t read_size;

    while ((read_size = getline(&line, &line_buf_len, fptr)) != -1) {
        if (read_size > 0 && line[read_size - 1] == '\n')
            line[read_size - 1] = '\0';

        if (strlen(line) == 0)
            continue;

        char *parts[2];
        int parsCount = splitByComma(line, parts, 2);
        if (parsCount != 2)
            continue;

        int key = atoi(parts[0]);
        keys[total] = key;
        values[total] = strdup(parts[1]);
        total++;
    }
    free(line);
}

// Look up a key. Returns a malloc'd string "key,value" if found, NULL if not.
char *get(int key) {
    for (int i = 0; i < total; i++) {
        if (key == keys[i]) {
            int needed = snprintf(NULL, 0, "%d,%s", key, values[i]) + 1;
            char *ret = malloc(needed);
            if (ret == NULL) {
                fprintf(stderr, "error: malloc failed\n");
                exit(1);
            }
            sprintf(ret, "%d,%s", key, values[i]);
            return ret;
        }
    }
    return NULL;
}


void put(int key, char *value) {
    for (int i = 0; i < total; i++) {
        if (key == keys[i]) {
            values[i] = strdup(value);
            return;
        }
    }
    if (total >= MAX_DATABASE_ENTRIES) {
        fprintf(stderr, "error: database full\n");
        exit(1);
    }
    keys[total] = key;
    values[total] = strdup(value);
    total++;
}

int delete(int key) {
    for (int i = 0; i < total; i++) {
        if (key == keys[i]) {
            free(values[i]);
            for (int j = i; j < total - 1; j++) {
                keys[j] = keys[j + 1];
                values[j] = values[j + 1];
            }
            total--;
            return 1;
        }
    }
    return 0;
}

void clear() {
    for (int i = 0; i < total; i++) {
        free(values[i]);
    }
    total = 0;
}


void printAll() {
    for (int i = 0; i < total; i++) {
        printf("%d,%s\n", keys[i], values[i]);
    }
}


void writeFile() {
    FILE *fptr = fopen(DATABASE_FILE, "w");
    if (fptr == NULL) {
        fprintf(stderr, "error: could not write database file\n");
        exit(1);
    }
    for (int i = 0; i < total; i++) {
        fprintf(fptr, "%d,%s\n", keys[i], values[i]);
    }
    fclose(fptr);
}

int main(int argc, char *argv[]) {
    // Step 1: Load data
    FILE *fptr = ensureDatabaseFile();
    readFile(fptr);
    fclose(fptr);

    // Step 2: Process each command-line argument
    for (int i = 1; i < argc; i++) {
        char *parts[4];
        char *arg_copy = strdup(argv[i]);
        int partsCount = splitByComma(arg_copy, parts, 4);

        if (partsCount == 2 && strcmp(parts[0], "g") == 0) {
            int key = atoi(parts[1]);
            char *result = get(key);
            if (result != NULL) {
                printf("%s\n", result);
                free(result);
            } else {
                printf("%d not found\n", key);
            }
        } else if (partsCount == 3 && strcmp(parts[0], "p") == 0) {
            int key = atoi(parts[1]);
            put(key, parts[2]);
        } else if (partsCount == 2 && strcmp(parts[0], "d") == 0) {
            int key = atoi(parts[1]);
            if (!delete(key)) {
                printf("%d not found\n", key);
            }
        } else if (partsCount == 1 && strcmp(parts[0], "c") == 0) {
            clear();
        } else if (partsCount == 1 && strcmp(parts[0], "a") == 0) {
            printAll();
        } else {
            printf("bad command\n");
        }
        free(arg_copy);
    }

    // Step 3: Store data
    writeFile();

    return 0;
}
