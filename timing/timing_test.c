#include <stdio.h>
#include <stddef.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

double now(){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + (double)tv.tv_usec / 1000000;
}

double timeit(void (*func_ptr)(size_t thread_count, void* args), size_t thread_count, void* args){
    double start_at = now();
    (*func_ptr)(thread_count, args);
    double finish_at = now();
    return finish_at - start_at;
}

double doublemul(size_t thread_count, void* args){
    double sum = 0;
    for (double i = 0; i < 100000000; i++){
        sum += i * i;
    }
    return sum;
}

size_t ulongmul(size_t thread_count, void* args){
    size_t sum = 0;
    for (size_t i = 0; i < 100000000; i++){
        sum += i * i;
    }
    return sum;
}

void exe_strtod(size_t thread_count, void* args){
    char str[] = "123.456";
    for (size_t i = 0; i < 10000000; i++){
        double num = strtod(str, NULL);
    }
}

void exe_strtol(size_t thread_count, void* args){
    char str[] = "123.456";
    for (size_t i = 0; i < 10000000; i++){
        long num = strtol(str, NULL, 10) * 1000 + strtol(str + 4, NULL, 10);
    }
}

void exe_fast_strtod(){
    char str[] = "123.456";
    for (size_t i = 0; i < 10000000; i++){
        double num = (str[0] - '0') * 100 + (str[1] - '0') * 10 + (str[2] - '0')
                   + (str[4] - '0') * 0.1 + (str[5] - '0') * 0.01 + (str[6] - '0') * 0.001;
        // assert(num == 123.456);
    }
}

void exe_fast_strtol(){
    char str[] = "123.456";
    for (size_t i = 0; i < 10000000; i++){
        long num = (str[0] - '0') * 100000 + (str[1] - '0') * 10000 + (str[2] - '0') * 1000
                 + (str[4] - '0') * 100 + (str[5] - '0') * 10 + (str[6] - 10);
    }
}


void exe_strtok(){
    for (size_t i = 0; i < 10000000; i++){
        char* a;
        char str[] = "123.456,123.456,123.456";
        strtok(str, ",");
        // printf("%p\n", str);
        // fflush(NULL);
        while((a = strtok(NULL, ",")));
    }
}

void exe_mystrtok(){
    size_t len = strlen("123.456,123.456,123.456");

    for (size_t i = 0; i < 10000000; i++){
        char* a;
        char* str = "123.456,123.456,123.456";

        for (size_t i = 0; i < len; i += 7){
            a = str + i;
        }
    }
}

void exe_malloc(){
    for (size_t i = 0; i < 10000000; i++){
        malloc(1000);
    }
}


void exe_strtok_strtol(){
    for (size_t i = 0; i < 10000000; i++){
        char str[] = "10000000,10000000\n";
        long a = strtol(strtok(str, ","), NULL, 10);
        long b = strtol(strtok(NULL, ","), NULL, 10);
        // printf("%ld, %ld\n", a, b);
    }
}

void exe_mystrtok_strtol(){
    for (size_t i = 0; i < 10000000; i++){
        char str[] = "10000000,10000000\n";
        char* cur = str;
        long row = 0;
        long col = 0;

        for (; *cur != ','; cur++) {
            row *= 10;
            row += *cur - '0';
        }
        cur++;
        for (; *cur != '\n'; cur++) {
            col *= 10;
            col += *cur - '0';
        }
        // printf("%ld, %ld\n", row, col);
    }
}

void exec_sprintf_long(){
    char* buffer = malloc(20);

    for (size_t i = 0; i < 10000000; i++){
        long num = 56033485267832;
        sprintf(buffer, "%ld.", num / 1000000);
        sprintf(buffer, "%ld", num % 1000000);
    }
}


void exec_fast_sprintf_long(){
    char* buffer = malloc(20);

    for (size_t i = 0; i < 10000000; i++){
        char* cur = buffer + 20 - 1;
        long num = 56033485267832;

        for (int i = 0; i < 6; i++){
            *cur = (num % 10) + '0';
            num /= 10;
            cur--;
        }

        *cur = '.';
        cur--;

        while (num) {
            *cur = (num % 10) + '0';
            num /= 10;
            cur--;
        }
    }
}


void exec_printf_double(){
    char* buffer = malloc(20);

    for (size_t i = 0; i < 10000000; i++){
        double num = 56033485.267832;
        sprintf(buffer, "%lf", num);
    }
}

void exec_myprintf_double(){
    char* buffer = malloc(20);

    for (size_t t = 0; t < 10000000; t++){
        double num = 56033485.267832;

        char* dot = buffer + 25 - 6 - 1;
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
}

// void exec_myprintf_memcpy(){
//     char* temp = malloc(20);
//     char* buffer = malloc(20);


// }



int main(){
    double t;
    // t = timeit(doublemul, 1, 1);
 //    printf("doublemul = %lf\n", t);
 //    t = timeit(ulongmul, 1, 1);
 //    printf("ulongmul = %lf\n", t);
 //    t = timeit(exe_strtod, 1, 1);
    // printf("strtod = %lf\n", t);
 //    t = timeit(exe_fast_strtod, 1, 1);
 //    printf("fast_strtod = %lf\n", t);

    // t = timeit(exe_strtol, 1, 1);
    // printf("strtol = %lf\n", t);
    // t = timeit(exe_fast_strtol, 1, 1);
    // printf("fast_strtol = %lf\n", t);
    // t = timeit(exe_strtok, 1, 1);
    // printf("strtok = %lf\n", t);
    // t = timeit(exe_mystrtok, 1, 1);
    // printf("mystrtok = %lf\n", t);
    // t = timeit(exe_malloc, 1, 1);
    // printf("malloc = %lf\n", t);
    // t = timeit(exe_strtok_strtol, 1, 1);
    // printf("strtok_strtol = %lf\n", t);
    // t = timeit(exe_mystrtok_strtol, 1, 1);
    // printf("mystrtok_strtol = %lf\n", t);


    t = timeit(exec_sprintf_long, 1, NULL);
    printf("sprintf_long = %lf\n", t);
    t = timeit(exec_fast_sprintf_long, 1, NULL);
    printf("fast_sprintf_long = %lf\n", t);
    
    // t = timeit(exec_printf_double, 1, NULL);
    // printf("printf_double = %lf\n", t);
    // t = timeit(exec_myprintf_double, 1, NULL);
    // printf("myprintf_double = %lf\n", t);
    return 0;
}