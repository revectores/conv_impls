#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/time.h>


const char* input_filename = "input.txt";
const char* output_filename = "output.txt";

const size_t COL_LEN = 3 * 2 + 1;
const size_t OCOL_LEN = 19;
const size_t COMMA_LEN = 1;
const size_t NEWLINE_LEN = 1;

const char ascii_map[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};


typedef struct __matrix_s {
    long** matrix;
    int rows;
    int cols;
} matrix_s;

typedef struct __loc_s {
    int r;
    int c;
} loc_s;

// static inline double now(){
//     struct timeval tv;
//     gettimeofday(&tv, NULL);
//     return tv.tv_sec + (double)tv.tv_usec / 1000000;
// }

static inline int min(const int a, const int b){
    return a < b ? a : b;
}

static inline int max(const int a, const int b){
    return a > b ? a : b;
}

static inline void swap(long* restrict a, long* restrict b){
    long temp = *a;
    *a = *b;
    *b = temp;
}

static inline size_t col2linelen(const size_t col, const size_t sizelines_len){
    return col * COL_LEN + (col - 1) * COMMA_LEN + NEWLINE_LEN + 1;
}

static inline size_t ocol2linelen(const size_t col){
    return col * OCOL_LEN + (col - 1) * COMMA_LEN + NEWLINE_LEN;
}

// static inline long fast_strtod(const char* str){
//     return (str[0] - '0') * 100 + (str[1] - '0') * 10 + (str[2] - '0')
//          + (str[4] - '0') * 0.1 + (str[5] - '0') * 0.01 + (str[6] - '0') * 0.001;
// }

static inline long fast_strtol(const char* restrict str){
    return (str[0] - '0') * 100000 + (str[1] - '0') * 10000 + (str[2] - '0') * 1000
         + (str[4] - '0') * 100 + (str[5] - '0') * 10 + (str[6] - '0');
}

static inline ssize_t read_input(matrix_s* restrict A, matrix_s* restrict C){
    FILE* fp = fopen(input_filename, "r");

    size_t A_sizeline_len = 0;
    size_t C_sizeline_len = 0;
    size_t sizelines_len = 0;
    char* line = NULL;
    size_t len = 0;


    if ((A_sizeline_len = getline(&line, &len, fp)) != -1){
        A->rows = strtol(strtok(line, ","), NULL, 10);
        A->cols = strtol(strtok(NULL, ","), NULL, 10);
    } else {
        return -1;
    }

    if ((C_sizeline_len = getline(&line, &len, fp)) != -1){
        C->rows = strtol(strtok(line, ","), NULL, 10);
        C->cols = strtol(strtok(NULL, ","), NULL, 10);
    } else {
        return -1;
    }


    sizelines_len = A_sizeline_len + C_sizeline_len;

    // printf("A: (%d, %d)\n", A->rows, A->cols);
    // printf("C: (%d, %d)\n", C->rows, C->cols);

    long (*Am)[A->cols] = malloc(sizeof(long[A->rows][A->cols]));
    long (*Cm)[C->cols] = malloc(sizeof(long[C->rows][C->cols]));
    A->matrix = (long**)Am;
    C->matrix = (long**)Cm;

    size_t A_linelen = col2linelen(A->cols, sizelines_len);
    size_t C_linelen = col2linelen(C->cols, sizelines_len);

    char* A_line_buffers = malloc(A->rows * A_linelen);
    char* C_line_buffers = malloc(C->rows * C_linelen);

#   pragma omp parallel num_threads(768)
#   pragma omp for
    for (int r = 0; r < A->rows; r++){
        char* line_buffer = A_line_buffers + r * A_linelen;
        size_t offset = sizelines_len + A_linelen * r;
        pread(fileno(fp), line_buffer, A_linelen, offset);

        for (size_t c = 0; c < A->cols; c++){
            Am[r][c] = fast_strtol(line_buffer + c * (COL_LEN + COMMA_LEN));
        }
    }

#   pragma omp for
    for (int r = 0; r < C->rows; r++){
        // char* line_buffer = malloc(C_linelen);
        char* line_buffer = C_line_buffers + r * C_linelen;

        size_t offset = sizelines_len + A_linelen * A->rows + C_linelen * r;
        pread(fileno(fp), line_buffer, C_linelen, offset);

        for (size_t c = 0; c < C->cols; c++){
            Cm[r][c] = fast_strtol(line_buffer + c * (COL_LEN + COMMA_LEN));
        }
    }

    // long finish_at = now();
    // printf("read info cost: %lf\n", finish_at - start_at);
    return 0;
}

static inline void fast_sprintf(char* restrict buffer, long num){
    char* cur = buffer + OCOL_LEN - 1;

    for (int i = 0; i < 6; i++){
        *cur = ascii_map[num % 10];
        num /= 10;
        cur--;
    }

    *cur = '.';
    cur--;

    while (num) {
        *cur = ascii_map[num % 10];
        num /= 10;
        cur--;
    }

    while (cur >= buffer) {
        *cur = ' ';
        cur--;
    }
}

static inline ssize_t write_output(matrix_s* restrict M){
    FILE* fp = fopen(output_filename, "w");

    long (*Mm)[M->cols] = (void*) M->matrix;

    size_t linelen = ocol2linelen(M->cols);
    char* line_buffers = malloc(linelen * M->rows);
    // memset(line_buffers, ' ', linelen * M->rows);

    // double start_at = now();

#   pragma omp parallel for num_threads(768)
    for (int r = 0; r < M->rows; r++){
        char* line_buffer = line_buffers + r * linelen;

        for (int c = 0; c < M->cols; c++){
            char* col_buffer = line_buffer + c * (OCOL_LEN + COMMA_LEN);
            fast_sprintf(col_buffer, Mm[r][c]);
            *(col_buffer + OCOL_LEN) = ',';
        }
        line_buffer[linelen - 1] = '\n';

        size_t offset = linelen * r;
        pwrite(fileno(fp), line_buffer, linelen, offset);
    }

    // double finish_at = now();
    // printf("iter cost: %lf\n", finish_at - start_at);


    return 0;
}


static inline ssize_t rotate(matrix_s* restrict C){
    long (*Cm)[C->cols] = (void*) C->matrix;

#   pragma omp parallel for num_threads(768) collapse(2)
    for (int r = 0; r < C->rows / 2; r++){
        for (int c = 0; c < C->cols; c++){
            swap(&Cm[r][c], &Cm[C->rows - 1 - r][C->cols - 1 - c]);
        }
    }

    if (C->rows % 2 == 1){
        for (int c = 0; c < C->cols / 2; c++){
            swap(&Cm[C->rows / 2][c], &Cm[C->rows / 2][C->cols - 1 - c]);
        }
    }

    return 0;
}


// static inline loc_s get_matrix_center(const matrix_s* restrict C){
//     loc_s center = {(C->rows - 1) / 2, (C->cols - 1) / 2};
//     return center;
// }


static inline long pointwise_sum(const matrix_s* restrict A, const matrix_s* restrict C, const loc_s loc){
    long (*Am)[A->cols] = (void*) A->matrix;
    long (*Cm)[C->cols] = (void*) C->matrix;

    // loc_s center = get_matrix_center(C);
    int center_r = (C->rows - 1) / 2;
    int center_c = (C->cols - 1) / 2;
    int loc_r = loc.r;
    int loc_c = loc.c;

    long sum = 0;
    
    // for (int r = -center.r; r < C->rows - center.r; r++){
    //     for (int c = -center.c; c < C->cols - center.c; c++){
    //         if ((loc.r + r < 0) || (loc.c + c < 0) || (loc.r + r >= A->rows) || (loc.c + c >= A->cols)){
    //             continue;
    //         }
    //         sum += Cm[center.r + r][center.c + c] * Am[loc.r + r][loc.c + c];
    //     }
    // }

    const int r_low  = max(-center_r, -loc_r);
    const int r_high = min(C->rows - center_r, A->rows - loc_r);
    const int c_low  = max(-center_c, -loc_c);
    const int c_high = min(C->cols - center_c, A->cols - loc_c);

    for (int r = r_low; r < r_high; r++){
        for (int c = c_low; c < c_high; c++){
            sum += Cm[center_r + r][center_c + c] * Am[loc_r + r][loc_c + c];
        }
    }

    return sum;
}


static inline ssize_t conv(matrix_s* restrict A, matrix_s* restrict C, matrix_s* restrict M){
    // long (*Am)[A->cols] = (void*) A->matrix;
    // long (*Cm)[C->cols] = (void*) C->matrix;

    rotate(C);

    M->rows = A->rows;
    M->cols = A->cols;

    long (*Mm)[M->cols] = malloc(sizeof(long[M->rows][M->cols]));
    M->matrix = (long**)Mm;

#   pragma omp parallel for num_threads(768) collapse(2)
    for (int r = 0; r < A->rows; r++){
        for (int c = 0; c < A->cols; c++){
            Mm[r][c] = pointwise_sum(A, C, (loc_s){r, c});
        }
    }

    return 0;
}


int main(int argc, char* argv[]){
    matrix_s A;
    matrix_s C;
    matrix_s M;

    // double read_start_at = now();
    read_input(&A, &C);
    // double read_finish_at = now();
 //     printf("read cost: %lf\n", read_finish_at - read_start_at);

    // double start_at = now();
    conv(&A, &C, &M);
    // double finish_at = now();
    // printf("computation cost: %lf\n", finish_at - start_at);

    // double write_start_at = now();
    write_output(&M);
    // double write_finish_at = now();
 //     printf("write cost: %lf\n", write_finish_at - write_start_at);

    return 0;
}
