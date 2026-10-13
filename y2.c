#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_LINE 4096
#define MAX_TERMS 32
#define MAX_TERM_LEN 64

char line[MAX_LINE];

char letters[26];
int  nletters;

int  digits[26];
int  sol_digits[26];

char term_str[MAX_TERMS][MAX_TERM_LEN];
char term_op[MAX_TERMS];   /* оператор ПЕРЕД term_str[i]; term_op[0] не используется */
int  nterms;
int  eq_index = -1;

int solved;

/* ОПТИМИЗАЦИЯ 1: массив занятых цифр */
static int used[10];

/* ОПТИМИЗАЦИЯ 2: буква стоит первой в каком-либо числе */
static int is_leading[26];

/* ОПТИМИЗАЦИЯ 3: заранее вычисленные индексы букв */
static int term_ids[MAX_TERMS][MAX_TERM_LEN];
static int term_len_arr[MAX_TERMS];

int find_letter(char c) {
    for (int i = 0; i < nletters; i++)
        if (letters[i] == c) return i;
    return -1;
}

long long ids_to_num(const int *ids, int len) {
    long long n = 0;
    for (int i = 0; i < len; i++)
        n = n * 10 + digits[ids[i]];
    return n;
}

int leading_ok(void) {
    for (int t = 0; t < nterms; t++)
        if (term_len_arr[t] > 0 && digits[term_ids[t][0]] == 0) return 0;
    return 1;
}

/* Вычисление одной стороны выражения: термины [start, end).
   Учитывает приоритеты: сначала * и /, потом + и -.
   При делении с остатком или делении на 0 возвращает *ok = 0. */
long long eval_side(int start, int end, int *ok) {
    *ok = 1;
    long long total = 0;
    long long cur = 0;
    int cur_sign = 1;
    int first = 1;

    for (int i = start; i < end; i++) {
        long long val = ids_to_num(term_ids[i], term_len_arr[i]);
        if (first) {
            cur = val;
            first = 0;
        } else {
            char op = term_op[i];
            if (op == '+') {
                total += cur_sign * cur;
                cur_sign = 1;
                cur = val;
            } else if (op == '-') {
                total += cur_sign * cur;
                cur_sign = -1;
                cur = val;
            } else if (op == '*') {
                cur *= val;
            } else if (op == '/') {
                if (val == 0 || cur % val != 0) { *ok = 0; return 0; }
                cur /= val;
            }
        }
    }
    total += cur_sign * cur;
    return total;
}

int check(void) {
    if (!leading_ok()) return 0;
    if (eq_index < 0)  return 0;

    int ok;
    long long left  = eval_side(0, eq_index, &ok);
    if (!ok) return 0;
    long long right = eval_side(eq_index, nterms, &ok);
    if (!ok) return 0;
    return left == right;
}

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
        if (used[d]) continue;
        if (d == 0 && is_leading[pos]) continue;
        used[d] = 1;
        digits[pos] = d;
        permute(pos + 1);
        used[d] = 0;
        if (solved) return;
    }
}

void parse_line(void) {
    nterms = 0;
    nletters = 0;
    eq_index = -1;

    char buf[MAX_TERM_LEN];
    int  len = 0;
    char last_op = 0;

    for (int i = 0; ; i++) {
        char c = line[i];
        int is_op = (c == '+' || c == '-' || c == '*' || c == '/' || c == '=');

        if (c == '\0' || c == ' ' || c == '\t' || is_op) {
            if (len > 0) {
                buf[len] = '\0';
                if (nterms < MAX_TERMS) {
                    strcpy(term_str[nterms], buf);
                    term_op[nterms] = last_op;
                    if (last_op == '=') eq_index = nterms;
                    nterms++;
                }
                len = 0;
            }
            if (is_op) last_op = c;
            if (c == '\0') break;
        } else {
            if (len < MAX_TERM_LEN - 1)
                buf[len++] = c;
            int found = 0;
            for (int j = 0; j < nletters; j++)
                if (letters[j] == c) { found = 1; break; }
            if (!found && nletters < 26) letters[nletters++] = c;
        }
    }

    /* ОПТИМИЗАЦИЯ 3: строим массивы индексов */
    for (int t = 0; t < nterms; t++) {
        term_len_arr[t] = (int)strlen(term_str[t]);
        for (int j = 0; j < term_len_arr[t]; j++)
            term_ids[t][j] = find_letter(term_str[t][j]);
    }

    /* ОПТИМИЗАЦИЯ 2: отмечаем ведущие буквы */
    memset(is_leading, 0, sizeof(is_leading));
    for (int t = 0; t < nterms; t++)
        if (term_len_arr[t] > 0)
            is_leading[term_ids[t][0]] = 1;
}

int main(void) {
    printf("Enter cryptarithm (e.g. SEND + MORE = MONEY or AB * CD = EFGH): ");
    fflush(stdout);

    if (scanf(" %4095[^\n]", line) != 1) {
        printf("Input error.\n");
        return 1;
    }

    clock_t start = clock();

    parse_line();

    if (eq_index < 0) {
        printf("No '=' found in expression.\n");
        return 1;
    }

    if (nletters > 10) {
        printf("Too many unique letters (>10), no solution possible.\n");
        clock_t end = clock();
        double ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
        printf("Elapsed time: %.3f ms\n", ms);
        return 1;
    }

    memset(used, 0, sizeof(used));
    solved = 0;
    permute(0);

    if (solved) {
        for (int i = 0; i < nterms; i++) {
            for (int j = 0; term_str[i][j]; j++) {
                int idx = find_letter(term_str[i][j]);
                printf("%d", sol_digits[idx]);
            }
            if (i < nterms - 1)
                printf(" %c ", term_op[i + 1]);
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
