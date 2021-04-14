#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

int max(int lhs, int rhs) {
    if (lhs >= rhs) {
        return lhs;
    }
    return rhs;
}

void BuildLCSTable(const char* old_file, size_t old_file_size, const char* new_file, size_t new_file_size, int** lcs_table) {
    for (int i = old_file_size - 1; i >= 0; --i) {
        for (int j = new_file_size - 1; j >= 0; --j) {
            if (old_file[i] == '\0' || new_file[j] == '\0') {
                lcs_table[i][j] = 0;
            } else if (old_file[i] == new_file[j]) {
                lcs_table[i][j] = 1 + lcs_table[i + 1][j + 1];
            } else {
                lcs_table[i][j] = max(lcs_table[i + 1][j], lcs_table[i][j + 1]);
            }
        }
    }
}

void CompareFilesByTable(const char* old_file, int old_file_size, const char* new_file, int new_file_size, int** table,
                        char* diff_buffer) {
    size_t i = 0;
    size_t j = 0;

    int diff_len = 0;

    while (i < old_file_size && j < new_file_size) {
        if (old_file[i] == new_file[j]) {
            diff_buffer[diff_len++] = ' ';
            ++i;
            ++j;
        } else if (i + 1 < old_file_size && j + 1 < new_file_size && table[i + 1][j] >= table[i][j + 1]) {
            diff_buffer[diff_len++] = '-';
            ++i;
        } else {
            diff_buffer[diff_len++] = new_file[j];
            ++j;
        }
    }
}

void FindDiff(const char* first_file, size_t first_file_size, const char* second_file, size_t second_file_size,
              char* diff_buffer) {
    int** lcs_table = (int**)malloc(first_file_size * sizeof(int*));
    for (size_t i = 0; i < first_file_size; ++i)  {
        lcs_table[i] = (int*)malloc(second_file_size * sizeof(int));
    }

    BuildLCSTable(first_file, first_file_size, second_file, second_file_size, lcs_table);
    CompareFilesByTable(first_file, first_file_size, second_file, second_file_size, lcs_table, diff_buffer);

    for (size_t i = 0; i < first_file_size; ++i) {
            free(lcs_table[i]);
    }
}

void RestoreNewFile(const char* old_file_buffer, size_t old_size, const char* diff_file_buffer, size_t diff_size, char* new_file_buffer) {
    size_t old_symb_num = 0;
    size_t diff_symb_num = 0;
    size_t new_file_symb_num = 0;

    while (old_symb_num < old_size && diff_symb_num < diff_size) {
        if (diff_file_buffer[diff_symb_num] == ' ') {
            ++diff_symb_num;
            new_file_buffer[new_file_symb_num++] = old_file_buffer[old_symb_num++];
        } else if (diff_file_buffer[diff_symb_num] == '-') {
            ++diff_symb_num;
            ++old_symb_num;
        } else {
            new_file_buffer[new_file_symb_num++] = diff_file_buffer[diff_symb_num++];
        }
    }

    while (diff_file_buffer[diff_symb_num] != '\0') {
        new_file_buffer[new_file_symb_num] = diff_file_buffer[diff_symb_num];
        ++diff_symb_num;
        ++new_file_symb_num;
    }
}

int main(int argc, char* argv[]) {
    if (strcmp(argv[1], "-d") == 0) {
        FILE* old_file_fd = fopen(argv[2], "rb");
        FILE* new_file_fd = fopen(argv[3], "rb");

        fseek(old_file_fd, 0, SEEK_END);
        size_t old_size = ftell(old_file_fd);
        fseek(old_file_fd, 0, SEEK_SET);

        fseek(new_file_fd, 0, SEEK_END);
        size_t new_size = ftell(new_file_fd);
        fseek(new_file_fd, 0, SEEK_SET);

        char* old_file_buffer = (char*) malloc(old_size * sizeof(char));
        char* new_file_buffer = (char*) malloc(new_size * sizeof(char));

        fgets(old_file_buffer, old_size, old_file_fd);
        fgets(new_file_buffer, new_size, new_file_fd);

        char* diff_buffer = (char*) malloc(2 * (old_size + new_size) * sizeof(char));

        FindDiff(old_file_buffer, old_size, new_file_buffer, new_size, diff_buffer);
        FILE* diff_fd = fopen(argv[4], "wb");

        fputs(diff_buffer, diff_fd);

        free(old_file_buffer);
        free(new_file_buffer);
        free(diff_buffer);

        fclose(diff_fd);
        fclose(new_file_fd);
        fclose(old_file_fd);
    } else if (strcmp(argv[1], "-r") == 0) {
        FILE* old_file_fd = fopen(argv[2], "rb");
        FILE* diff_file_fd = fopen(argv[3], "rb");

        fseek(old_file_fd, 0, SEEK_END);
        size_t old_size = ftell(old_file_fd);
        fseek(old_file_fd, 0, SEEK_SET);

        fseek(diff_file_fd, 0, SEEK_END);
        size_t diff_size = ftell(diff_file_fd);
        fseek(diff_file_fd, 0, SEEK_SET);

        char* old_file_buffer = (char*)malloc(old_size * sizeof(char));
        char* diff_file_buffer = (char*)malloc(diff_size * sizeof(char));

        fgets(old_file_buffer, old_size, old_file_fd);
        fgets(diff_file_buffer, diff_size, diff_file_fd);

        FILE* new_file_fd = fopen(argv[4], "wb");
        char* new_file_buffer = (char*)malloc(diff_size * sizeof(char));

        RestoreNewFile(old_file_buffer, old_size, diff_file_buffer, diff_size, new_file_buffer);

        fputs(new_file_buffer, new_file_fd);

        free(old_file_buffer);
        free(new_file_buffer);
        free(diff_file_buffer);

        fclose(diff_file_fd);
        fclose(new_file_fd);
        fclose(old_file_fd);
    }
}