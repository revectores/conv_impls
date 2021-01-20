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


const size_t COL_LEN = 3 * 2 + 1;
const size_t OCOL_LEN = 25;
const size_t COMMA_LEN = 1;
const size_t NEWLINE_LEN = 1;


typedef struct __matrix_s {
    double** matrix;
    int rows;
    int cols;
} matrix_s;

typedef struct __loc_s {
    int r;
    int c;
} loc_s;

static inline double now(){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + (double)tv.tv_usec / 1000000;
}

static inline int min(const int a, const int b){
    return a < b ? a : b;
}

static inline int max(const int a, const int b){
    return a > b ? a : b;
}

static inline void swap(double* restrict a, double* restrict b){
    double temp = *a;
    *a = *b;
    *b = temp;
}

static inline size_t col2linelen(const size_t col, const size_t sizelines_len){
    return col * COL_LEN + (col - 1) * COMMA_LEN + NEWLINE_LEN + 1;
}

static inline size_t ocol2linelen(const size_t col){
    return col * OCOL_LEN + (col - 1) * COMMA_LEN + NEWLINE_LEN;
}

static inline double fast_strtod(const char* restrict str){
    return (str[0] - '0') * 100 + (str[1] - '0') * 10 + (str[2] - '0')
         + (str[4] - '0') * 0.1 + (str[5] - '0') * 0.01 + (str[6] - '0') * 0.001;
}

static inline ssize_t read_input(const char* restrict filename, matrix_s* restrict A, matrix_s* restrict C){
    FILE* fp = fopen(filename, "r");

    if (!fp) {
        fprintf(stderr, "File does not exist\n");
        return -1;
    }

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

    double (*Am)[A->cols] = malloc(sizeof(double[A->rows][A->cols]));
    double (*Cm)[C->cols] = malloc(sizeof(double[C->rows][C->cols]));
    A->matrix = (double**)Am;
    C->matrix = (double**)Cm;

    size_t A_linelen = col2linelen(A->cols, sizelines_len);
    size_t C_linelen = col2linelen(C->cols, sizelines_len);

    // double start_at = now();
    char* A_line_buffers = malloc(A->rows * A_linelen);
    char* C_line_buffers = malloc(C->rows * C_linelen);

#   pragma omp parallel num_threads(768)
#   pragma omp for
    for (int r = 0; r < A->rows; r++){
        // char* line_buffer = malloc(A_linelen);
        char* line_buffer = A_line_buffers + r * A_linelen;
        // printf("%ld\n", A_linelen);

        size_t offset = sizelines_len + A_linelen * r;
        pread(fileno(fp), line_buffer, A_linelen, offset);

        for (size_t c = 0; c < A->cols; c++){
            Am[r][c] = fast_strtod(line_buffer + c * (COL_LEN + COMMA_LEN));
        }
    }

#   pragma omp for
    for (int r = 0; r < C->rows; r++){
        // char* line_buffer = malloc(C_linelen);
        char* line_buffer = C_line_buffers + r * C_linelen;

        size_t offset = sizelines_len + A_linelen * A->rows + C_linelen * r;
        pread(fileno(fp), line_buffer, C_linelen, offset);

        for (size_t c = 0; c < C->cols; c++){
            Cm[r][c] = fast_strtod(line_buffer + c * (COL_LEN + COMMA_LEN));
        }
    }

    // double finish_at = now();
    // printf("read info cost: %lf\n", finish_at - start_at);
    return 0;
}


// static inline ssize_t write_output(const char* filename, matrix_s* M){
//     FILE* fp = fopen(filename, "w");
//     double (*Mm)[M->cols] = (void*) M->matrix;

//     for (int r = 0; r < M->rows; r++){
//         for (int c = 0; c < M->cols; c++){
//             fprintf(fp, "%lf", Mm[r][c]);
//             if (c != M->cols - 1) fprintf(fp, ",");
//         }
//         fprintf(fp, "          ");
//         fprintf(fp, "\n");
//     }

//     return 0;
// }

static inline void fast_sprintf(char* restrict buffer, double num){
    char* dot = buffer + OCOL_LEN - 6 - 1;

    long integer = (long)num;
    double decimal = num - integer;

    *dot = '.';

    for (int i = 1; i <= 6; i++) {
        decimal *= 10;
        *(dot + i) = (int)decimal % 10 + '0';
    }

    char* cur = dot - 1;
    while (integer) {
        *cur = integer % 10 + '0';
        integer /= 10;
        cur--;
    }
}


static inline ssize_t write_output(char* restrict filename, matrix_s* restrict M){
    FILE* fp = fopen(filename, "w");
    double (*Mm)[M->cols] = (void*) M->matrix;

    size_t linelen = ocol2linelen(M->cols);
    char* line_buffers = malloc(linelen * M->rows);
    memset(line_buffers, ' ', linelen * M->rows);

#   pragma omp parallel for num_threads(768)
    for (int r = 0; r < M->rows; r++){
        // char* line_buffer = malloc(linelen);
        // memset(line_buffer, ' ', linelen);
        char* line_buffer = line_buffers + r * linelen;
        line_buffer[linelen - 1] = '\n';

        // size_t cur = 0;
        for (int c = 0; c < M->cols; c++){
            char* col_buffer = line_buffer + c * (OCOL_LEN + COMMA_LEN);
            fast_sprintf(col_buffer, Mm[r][c]);
            *(col_buffer + OCOL_LEN) = ',';
            // cur += sprintf(line_buffer + cur, "%19.6lf,", Mm[r][c]);
        }
        line_buffer[linelen - 1] = '\n';
        // line_buffer[cur - 1] = ' ';
        // line_buffer[cur] = ' ';

        size_t offset = linelen * r;
        pwrite(fileno(fp), line_buffer, linelen, offset);

        // if (r != M->rows -1) fprintf(fp, "\n");
    }

    return 0;
}


static inline ssize_t print_matrix(const matrix_s* restrict M){
    double (*Mm)[M->cols] = (void*) M->matrix;

    for (int r = 0; r < M->rows; r++){
        for (int c = 0; c < M->cols; c++){
            printf("%lf ", Mm[r][c]);
        }
        printf("\n");
    }
    printf("\n");

    return 0;
}


static inline ssize_t rotate(matrix_s* restrict C){
    double (*Cm)[C->cols] = (void*) C->matrix;

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


static inline loc_s get_matrix_center(const matrix_s* restrict C){
    loc_s center = {(C->rows - 1) / 2, (C->cols - 1) / 2};
    return center;
}


static inline double pointwise_sum(const matrix_s* restrict A, const matrix_s* restrict C, const loc_s loc){
    double (*Am)[A->cols] = (void*) A->matrix;
    double (*Cm)[C->cols] = (void*) C->matrix;

    loc_s center = get_matrix_center(C);

    double sum = 0;
    // for (int r = -center.r; r < C->rows - center.r; r++){
    //     for (int c = -center.c; c < C->cols - center.c; c++){
    //         if ((loc.r + r < 0) || (loc.c + c < 0) || (loc.r + r >= A->rows) || (loc.c + c >= A->cols)){
    //             continue;
    //         }
    //         sum += Cm[center.r + r][center.c + c] * Am[loc.r + r][loc.c + c];
    //     }
    // }
    const int r_low  = max(-center.r, -loc.r);
    const int r_high = min(C->rows - center.r, A->rows - loc.r);
    const int c_low  = max(-center.c, -loc.c);
    const int c_high = min(C->cols - center.c, A->cols - loc.c);

    for (int r = r_low; r < r_high; r++){
        for (int c = c_low; c < c_high; c++){
            sum += Cm[center.r + r][center.c + c] * Am[loc.r + r][loc.c + c];
        }
    }

    return sum;
}


static inline ssize_t conv(matrix_s* restrict A, matrix_s* restrict C, matrix_s* restrict M){
    // double (*Am)[A->cols] = (void*) A->matrix;
    // double (*Cm)[C->cols] = (void*) C->matrix;

    rotate(C);

    M->rows = A->rows;
    M->cols = A->cols;

    double (*Mm)[M->cols] = malloc(sizeof(double[M->rows][M->cols]));
    M->matrix = (double**)Mm;

#   pragma omp parallel for num_threads(768) collapse(2)
    for (int r = 0; r < A->rows; r++){
        for (int c = 0; c < A->cols; c++){
            Mm[r][c] = pointwise_sum(A, C, (loc_s){r, c});
        }
    }

    return 0;
}


void assert_equal(const matrix_s* restrict A, const matrix_s* restrict B){
    double (*Am)[A->cols] = (void*) A->matrix;
    double (*Bm)[B->cols] = (void*) B->matrix;

    assert(A->rows == B->rows && A->cols == B->cols);
    for (int r = 0; r < A->rows; r++){
        for (int c = 0; c < A->cols; c++){
            // printf("(%d, %d) %lf, %lf\n", r, c, Am[r][c], Bm[r][c]);
            assert(fabs(Am[r][c] - Bm[r][c]) / Bm[r][c] < 1e-9);
        }
    }
}

int main(int argc, char* argv[]){
    char* input_filename = "input.txt";
    char* output_filename = "output.txt";

    matrix_s A;
    matrix_s C;
    matrix_s M;

    // double read_start_at = now();
    ssize_t rc = read_input(input_filename, &A, &C);
    // double read_finish_at = now();
 //    printf("read cost: %lf\n", read_finish_at - read_start_at);

    assert(rc == 0);

    // double start_at = now();
    conv(&A, &C, &M);
    // double finish_at = now();
    // printf("computation cost: %lf\n", finish_at - start_at);

    double write_start_at = now();
    write_output(output_filename, &M);
    double write_finish_at = now();
    printf("write cost: %lf\n", write_finish_at - write_start_at);


    return 0;
}
