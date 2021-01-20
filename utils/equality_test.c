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


ssize_t read_output(FILE* fp, matrix_s* M){
    if (!fp) {
        fprintf(stderr, "File does not exist\n");
        return -1;
    }

    int rc;
    char* line = NULL;
    size_t len = 0;

    double (*Mm)[M->cols] = malloc(sizeof(double[M->rows][M->cols]));
    M->matrix = (double**)Mm;


    for(int r = 0; r < M->rows; r++){
        rc = getline(&line, &len, fp);
        assert(rc != -1);

        int c = 0;

        char* a = strtok(line, ",");
        Mm[r][c++] = strtod(a, NULL);

        while((a = strtok(NULL, ","))){
            Mm[r][c++] = strtod(a, NULL);
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
    int dimension = strtol(argv[1], NULL, 10);
    char* matrix_file1 = argv[2];
    char* matrix_file2 = argv[3];

    FILE* fp1 = fopen(matrix_file1, "r");
    FILE* fp2 = fopen(matrix_file2, "r");

    matrix_s A;
    matrix_s B;

    int rc;


    A.rows = dimension;
    A.cols = dimension;
    B.rows = dimension;
    B.cols = dimension;

    rc = read_output(fp1, &A);
    rc = read_output(fp2, &B);

    assert_equal(&A, &B);
}