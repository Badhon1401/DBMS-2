#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <time.h>

int total_lines1, total_lines2;
char **output_lines;
int output_count = 0;

CRITICAL_SECTION critical_section;

DWORD WINAPI myFun(LPVOID x) {
    int tid = *((int *)x);
    FILE *file1 = fopen("input1.txt", "r");
    FILE *file2 = fopen("input2.txt", "r");
    if (!file1 || !file2) {
        perror("Error opening files");
        ExitThread(0);
    }

    char line1[256], line2[256];
    int i;

    int lines_per_thread = total_lines1 / total_lines2;
    int start_line = tid * lines_per_thread;

    for (i = 0; i < start_line; i++) {
        if (!fgets(line1, sizeof(line1), file1)) break;
    }

    for (i = 0; i < lines_per_thread; i++) {
        if (!fgets(line1, sizeof(line1), file1)) break;
        fseek(file2, 0, SEEK_SET);
        while (fgets(line2, sizeof(line2), file2)) {
            if (line1[0] == line2[0]) {
                EnterCriticalSection(&critical_section);
                if (output_count < total_lines1) {
                    output_lines[output_count] = strdup(line1);
                    output_count++;
                }
                LeaveCriticalSection(&critical_section);
                break;
            }
        }
    }

    fclose(file1);
    fclose(file2);

    ExitThread(0);
}

void perform_no_thread_operations() {
    FILE *file1 = fopen("input1.txt", "r");
    FILE *file2 = fopen("input2.txt", "r");
    if (!file1 || !file2) {
        perror("Error opening files");
        exit(1);
    }

    char line1[256], line2[256];

    while (fgets(line1, sizeof(line1), file1)) {
        fseek(file2, 0, SEEK_SET);
        while (fgets(line2, sizeof(line2), file2)) {
            if (line1[0] == line2[0]) {
                if (output_count < total_lines1) {
                    output_lines[output_count] = strdup(line1);
                    output_count++;
                }
                break;
            }
        }
    }

    fclose(file1);
    fclose(file2);
}

int compare_lines(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

int main(int argc, char *argv[]) {
    int num_cores;

    SYSTEM_INFO sys_info;
    GetSystemInfo(&sys_info);
    num_cores = sys_info.dwNumberOfProcessors;

    if (num_cores < 1) {
        printf("Error: Unable to determine the number of CPU cores.\n");
        return 1;
    }

    printf("Number of available CPU cores: %d\n", num_cores);

    printf("Enter number of rows for input1.txt: ");
    scanf("%d", &total_lines1);
    printf("Enter number of rows for input2.txt: ");
    scanf("%d", &total_lines2);

    FILE *file1 = fopen("input1.txt", "w");
    FILE *file2 = fopen("input2.txt", "w");

    if (!file1 || !file2) {
        perror("Error creating files");
        exit(1);
    }

    for (int i = 0; i < total_lines1; ++i) {
        fprintf(file1, "%d%c\n", rand() % 10, 'a');
    }

    for (int i = 0; i < total_lines2; ++i) {
        fprintf(file2, "%d%c\n", rand() % 10, 'b');
    }

    fclose(file1);
    fclose(file2);

    output_lines = malloc(total_lines1 * sizeof(char *));
    if (!output_lines) {
        perror("Error allocating memory");
        exit(1);
    }

    LARGE_INTEGER frequency, start_time, end_time;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start_time);

    HANDLE *threads = malloc(num_cores * sizeof(HANDLE));
    int *thread_args = malloc(num_cores * sizeof(int));

    InitializeCriticalSection(&critical_section);

    for (int i = 0; i < num_cores; ++i) {
        thread_args[i] = i;
        threads[i] = CreateThread(NULL, 0, myFun, &thread_args[i], 0, NULL);
    }

    WaitForMultipleObjects(num_cores, threads, TRUE, INFINITE);

    QueryPerformanceCounter(&end_time);
    printf("Multi-core operation took: %.2f seconds\n", (double)(end_time.QuadPart - start_time.QuadPart) / frequency.QuadPart);

    FILE *fptr = fopen("multi_core_output.txt", "w");
    if (!fptr) {
        perror("Error opening multi-core output file");
        exit(1);#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

int total_lines1, total_lines2;
char **output_lines;
int output_count = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *myFun(void *x) {
    int tid = *((int *)x);
    FILE *file1 = fopen("input1.txt", "r");
    FILE *file2 = fopen("input2.txt", "r");
    if (!file1 || !file2) {
        perror("Error opening files");
        pthread_exit(NULL);
    }

    char line1[256], line2[256];
    int i;

    int lines_per_thread = total_lines1 / total_lines2;
    int start_line = tid * lines_per_thread;

    for (i = 0; i < start_line; i++) {
        if (!fgets(line1, sizeof(line1), file1)) break;
    }

    for (i = 0; i < lines_per_thread; i++) {
        if (!fgets(line1, sizeof(line1), file1)) break;
        fseek(file2, 0, SEEK_SET);
        while (fgets(line2, sizeof(line2), file2)) {
            if (line1[0] == line2[0]) {
                pthread_mutex_lock(&mutex);
                if (output_count < total_lines1) {
                    output_lines[output_count] = strdup(line1);
                    output_count++;
                }
                pthread_mutex_unlock(&mutex);
                break;
            }
        }
    }

    fclose(file1);
    fclose(file2);

    pthread_exit(NULL);
}

void perform_no_thread_operations() {
    FILE *file1 = fopen("input1.txt", "r");
    FILE *file2 = fopen("input2.txt", "r");
    if (!file1 || !file2) {
        perror("Error opening files");
        exit(1);
    }

    char line1[256], line2[256];

    while (fgets(line1, sizeof(line1), file1)) {
        fseek(file2, 0, SEEK_SET);
        while (fgets(line2, sizeof(line2), file2)) {
            if (line1[0] == line2[0]) {
                if (output_count < total_lines1) {
                    output_lines[output_count] = strdup(line1);
                    output_count++;
                }
                break;
            }
        }
    }

    fclose(file1);
    fclose(file2);
}

int compare_lines(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

int main(int argc, char *argv[]) {
    int num_cores;
    
    num_cores = sysconf(_SC_NPROCESSORS_ONLN);

    if (num_cores < 1) {
        printf("Error: Unable to determine the number of CPU cores.\n");
        return 1;
    }

    printf("Number of available CPU cores: %d\n", num_cores);

    printf("Enter number of rows for input1.txt: ");
    scanf("%d", &total_lines1);
    printf("Enter number of rows for input2.txt: ");
    scanf("%d", &total_lines2);

    FILE *file1 = fopen("input1.txt", "w");
    FILE *file2 = fopen("input2.txt", "w");

    if (!file1 || !file2) {
        perror("Error creating files");
        exit(1);
    }

    for (int i = 0; i < total_lines1; ++i) {
        fprintf(file1, "%d%c\n", rand() % 10, 'a');
    }

    for (int i = 0; i < total_lines2; ++i) {
        fprintf(file2, "%d%c\n", rand() % 10, 'b');
    }

    fclose(file1);
    fclose(file2);

    output_lines = malloc(total_lines1 * sizeof(char *));
    if (!output_lines) {
        perror("Error allocating memory");
        exit(1);
    }

    clock_t start_time, end_time;
    
    start_time = clock();

    pthread_t *threads = malloc(num_cores * sizeof(pthread_t));
    int *thread_args = malloc(num_cores * sizeof(int));

    for (int i = 0; i < num_cores; ++i) {
        thread_args[i] = i;
        pthread_create(&threads[i], NULL, myFun, (void *)&thread_args[i]);
    }

    for (int i = 0; i < num_cores; ++i) {
        pthread_join(threads[i], NULL);
    }

    end_time = clock();
    printf("Multi-core operation took: %.2f seconds\n", (double)(end_time - start_time) / CLOCKS_PER_SEC);

    FILE *fptr = fopen("multi_core_output.txt", "w");
    if (!fptr) {
        perror("Error opening multi-core output file");
        exit(1);
    }

    qsort(output_lines, output_count, sizeof(char *), compare_lines);
    for (int i = 0; i < output_count; i++) {
        fprintf(fptr, "%c\n", output_lines[i][0]);
        free(output_lines[i]);
    }

    free(output_lines);
    fclose(fptr);

    output_count = 0;
    output_lines = malloc(total_lines1 * sizeof(char *));
    if (!output_lines) {
        perror("Error reallocating memory");
        exit(1);
    }

    start_time = clock();
    perform_no_thread_operations();
    end_time = clock();
    printf("Single-core operation took: %.2f seconds\n", (double)(end_time - start_time) / CLOCKS_PER_SEC);

    fptr = fopen("single_core_output.txt", "w");
    if (!fptr) {
        perror("Error opening single-core output file");
        exit(1);
    }

    qsort(output_lines, output_count, sizeof(char *), compare_lines);
    for (int i = 0; i < output_count; i++) {
        fprintf(fptr, "%c\n", output_lines[i][0]);
        free(output_lines[i]);
    }

    free(output_lines);
    fclose(fptr);

    free(threads);
    free(thread_args);

    return 0;
}

    }

    qsort(output_lines, output_count, sizeof(char *), compare_lines);
    for (int i = 0; i < output_count; i++) {
        fprintf(fptr, "%c\n", output_lines[i][0]);
        free(output_lines[i]);
    }

    free(output_lines);
    fclose(fptr);

    output_count = 0;
    output_lines = malloc(total_lines1 * sizeof(char *));
    if (!output_lines) {
        perror("Error reallocating memory");
        exit(1);
    }

    QueryPerformanceCounter(&start_time);
    perform_no_thread_operations();
    QueryPerformanceCounter(&end_time);
    printf("Single-core operation took: %.2f seconds\n", (double)(end_time.QuadPart - start_time.QuadPart) / frequency.QuadPart);

    fptr = fopen("single_core_output.txt", "w");
    if (!fptr) {
        perror("Error opening single-core output file");
        exit(1);
    }

    qsort(output_lines, output_count, sizeof(char *), compare_lines);
    for (int i = 0; i < output_count; i++) {
        fprintf(fptr, "%c\n", output_lines[i][0]);
        free(output_lines[i]);
    }

    free(output_lines);
    fclose(fptr);

    DeleteCriticalSection(&critical_section);

    free(threads);
    free(thread_args);

    return 0;
}
