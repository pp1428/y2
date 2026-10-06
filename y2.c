#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_LINE 4096
#define MAX_TERMS 8
#define MAX_TERM_LEN 64

char line[MAX_LINE];

char letters[26];
int  nletters;

int  digits[26];
int  sol_digits[26];

char terms[MAX_TERMS][MAX_TERM_LEN];
int  nterms;
char result[MAX_TERM_LEN];

int solved;

// ОПТИМИЗАЦИЯ 1: массив занятых цифр для проверки за O(1)
static int used[10];

// ОПТИМИЗАЦИЯ 2: признак того, что буква стоит первой в каком-либо слагаемом/результате
static int is_leading[26];

int find_letter(char c) {
    for (int i = 0; i < nletters; i++)
        if (letters[i] == c) return i;
    return -1;
}

long long str_to_num(char *s) {
    long long n = 0;
    int len = (int)strlen(s);
    for (int i = 0; i < len; i++) {
        int idx = find_letter(s[i]);
        n = n * 10 + digits[idx];
    }
    return n;
}

int leading_ok(void) {
    for (int i = 0; i < nletters; i++) {
        if (digits[i] != 0) continue;
        char c = letters[i];
        for (int t = 0; t < nterms; t++)
            if (terms[t][0] == c) return 0;
        if (result[0] == c) return 0;
    }
    return 1;
}

int check(void) {
    if (!leading_ok()) return 0;
    long long sum = 0;
    for (int i = 0; i < nterms; i++)
        sum += str_to_num(terms[i]);
    return sum == str_to_num(result);
}

// ОПТИМИЗАЦИЯ 1: ранний выход из рекурсии после нахождения решения
void permute(int pos) {
    if (solved) return;
    if (pos == nletters) {
        if (check()) {
            solved = 1;
            for (int i = 0; i < nletters; i++)
                sol_digits[i] = digits[i];
        }
        return;
    }
    for (int d = 0; d <= 9; d++) {
        // ОПТИМИЗАЦИЯ 1: проверка занятости цифры через массив used[]
        if (used[d]) continue;
        // ОПТИМИЗАЦИЯ 2: не разрешаем 0 для буквы, стоящей в начале числа
        if (d == 0 && is_leading[pos]) continue;
        used[d] = 1;
        digits[pos] = d;
        permute(pos + 1);
        used[d] = 0;
        // ОПТИМИЗАЦИЯ 1: прерываем перебор, как только решение найдено
        if (solved) return;
    }
}

void parse_line(void) {
    nterms = 0;
    nletters = 0;

    char buf[MAX_TERM_LEN];
    int  len = 0;
    int  phase = 0;

    for (int i = 0; ; i++) {
        char c = line[i];
        if (c == '\0' || c == ' ' || c == '+' || c == '=') {
            if (len > 0) {
                buf[len] = '\0';
                if (phase == 0) {
                    if (nterms < MAX_TERMS)
                        strcpy(terms[nterms++], buf);
                } else {
                    strcpy(result, buf);
                }
                len = 0;
            }
            if (c == '=') phase = 1;
            if (c == '\0') break;
        } else {
            buf[len++] = c;
            int found = 0;
            for (int j = 0; j < nletters; j++)
                if (letters[j] == c) { found = 1; break; }
            if (!found && nletters < 26) letters[nletters++] = c;
        }
    }

    // ОПТИМИЗАЦИЯ 2: отмечаем буквы, которые стоят на первом месте в числах
    memset(is_leading, 0, sizeof(is_leading));
    for (int t = 0; t < nterms; t++)
        is_leading[find_letter(terms[t][0])] = 1;
    is_leading[find_letter(result[0])] = 1;
}

int main(void) {
    printf("Enter cryptarithm (e.g. SEND + MORE = MONEY): ");
    fflush(stdout);

    if (scanf(" %4095[^\n]", line) != 1) {
        printf("Input error.\n");
        return 1;
    }

    clock_t start = clock();

    parse_line();

    if (nletters > 10) {
        printf("Too many unique letters (>10), no solution possible.\n");
        clock_t end = clock();
        double ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
        printf("Elapsed time: %.3f ms\n", ms);
        return 1;
    }

    // ОПТИМИЗАЦИЯ 1: сброс массива занятых цифр перед стартом перебора
    memset(used, 0, sizeof(used));

    solved = 0;
    permute(0);

    if (solved) {
        for (int i = 0; i < nterms; i++) {
            for (int j = 0; terms[i][j]; j++) {
                int idx = find_letter(terms[i][j]);
                printf("%d", sol_digits[idx]);
            }
            if (i < nterms - 1) printf(" + ");
        }
        printf(" = ");
        for (int j = 0; result[j]; j++) {
            int idx = find_letter(result[j]);
            printf("%d", sol_digits[idx]);
        }
        printf("\n");
    } else {
        printf("No solution found.\n");
    }

    clock_t end = clock();
    double ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
    printf("Elapsed time: %.3f ms\n", ms);

    return 0;
}
