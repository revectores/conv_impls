#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <assert.h>


typedef struct __matrix_s {
    double** matrix;
    int rows;
    int cols;
} matrix_s;

typedef struct __loc_s {
    int r;
    int c;
} loc_s;


ssize_t read_input(char* filename, matrix_s* A, matrix_s* C){
    FILE* fp = fopen(filename, "r");

    if (!fp) {
        fprintf(stderr, "File does not exist\n");
        return -1;
    }

    int rc;
    char* line = NULL;
    size_t len = 0;

    if ((rc = getline(&line, &len, fp)) != -1){
        A->rows = strtol(strtok(line, ","), NULL, 10);
        A->cols = strtol(strtok(NULL, ","), NULL, 10);
    } else {
        return -1;
    }

    if ((rc = getline(&line, &len, fp)) != -1){
        C->rows = strtol(strtok(line, ","), NULL, 10);
        C->cols = strtol(strtok(NULL, ","), NULL, 10);
    } else {
        return -1;
    }

    // printf("A: (%d, %d)\n", A->rows, A->cols);
    // printf("C: (%d, %d)\n", C->rows, C->cols);

    double (*Am)[A->cols] = malloc(sizeof(double[A->rows][A->cols]));
    double (*Cm)[C->cols] = malloc(sizeof(double[C->rows][C->cols]));
    A->matrix = (double**)Am;
    C->matrix = (double**)Cm;

    for(int r = 0; r < A->rows; r++){
        rc = getline(&line, &len, fp);
        assert(rc != -1);

        int c = 0;

        char* a = strtok(line, ",");
        Am[r][c++] = strtod(a, NULL);

        while((a = strtok(NULL, ","))){
            Am[r][c++] = strtod(a, NULL);
        }
    }

    for(int r = 0; r < C->rows; r++){
        rc = getline(&line, &len, fp);
        assert(rc != -1);

        int c = 0;

        char* a = strtok(line, ",");
        Cm[r][c++] = strtod(a, NULL);

        while((a = strtok(NULL, ","))){
            Cm[r][c++] = strtod(a, NULL);
        }
    }

    return 0;
}



ssize_t write_output(char* filename, matrix_s* M){
    FILE* fp = fopen(filename, "w");

    if (!fp) {
        fprintf(stderr, "File does not exist\n");
        return -1;
    }

    double (*Mm)[M->cols] = (void*) M->matrix;

    for (int r = 0; r < M->rows; r++){
        for (int c = 0; c < M->cols; c++){
            fprintf(fp, "%lf", Mm[r][c]);
            if (c != M->cols - 1) fprintf(fp, ",");
        }
        // if (r != M->rows -1) fprintf(fp, "\n");
        fprintf(fp, "\n");
    }

    return 0;
}


ssize_t print_matrix(matrix_s* M){
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


void swap(double* a, double* b){
    double temp = *a;
    *a = *b;
    *b = temp;
}


ssize_t rotate(matrix_s* C){
    double (*Cm)[C->cols] = (void*) C->matrix;

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


loc_s get_matrix_center(matrix_s* C){
    loc_s center = {(C->rows - 1) / 2, (C->cols - 1) / 2};
    return center;
}


double pointwise_sum(matrix_s* A, matrix_s* C, loc_s loc){
    double (*Am)[A->cols] = (void*) A->matrix;
    double (*Cm)[C->cols] = (void*) C->matrix;

    loc_s center = get_matrix_center(C);

    double sum = 0;
    for (int r = -center.r; r < C->rows - center.r; r++){
        for (int c = -center.c; c < C->cols - center.c; c++){
            if ((loc.r + r < 0) || (loc.c + c < 0) || (loc.r + r >= A->rows) || (loc.c + c >= A->cols)){
                continue;
            }
            sum += Cm[center.r + r][center.c + c] * Am[loc.r + r][loc.c + c];
        }
    }
    return sum;
}


ssize_t conv(matrix_s* A, matrix_s* C, matrix_s* M){
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


void assert_equal(matrix_s* A, matrix_s* B){
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

    ssize_t rc = read_input(input_filename, &A, &C);
    assert(rc == 0);
    conv(&A, &C, &M);
    write_output(output_filename, &M);

    return 0;
}
