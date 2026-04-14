#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_NT     10
#define MAX_T      20
#define MAX_PROD   5
#define MAX_LEN    50
#define STACK_SIZE 100

/* ── Grammar ── */
char  nonterm[MAX_NT];         /* list of non-terminals          */
char  term[MAX_T][4];          /* list of terminals  (incl. "$") */
int   nt_cnt = 0, t_cnt = 0;

/* Productions per non-terminal: prods[A][0..k] = "aB" etc. */
char  prods[MAX_NT][MAX_PROD][MAX_LEN];
int   prod_cnt[MAX_NT];

/* FIRST / FOLLOW */
char  First[MAX_NT][MAX_T][4];
int   first_cnt[MAX_NT];
char  Follow[MAX_NT][MAX_T][4];
int   follow_cnt[MAX_NT];

/* Parsing table: table[NT][T] = production string, "" = error */
char  table[MAX_NT][MAX_T][MAX_LEN];

/* ── helpers ── */
int nt_idx(char c) {
    for (int i = 0; i < nt_cnt; i++) if (nonterm[i] == c) return i;
    return -1;
}
int t_idx(const char *s) {
    for (int i = 0; i < t_cnt; i++) if (!strcmp(term[i], s)) return i;
    return -1;
}
int in_set(char set[][4], int cnt, const char *s) {
    for (int i = 0; i < cnt; i++) if (!strcmp(set[i], s)) return 1;
    return 0;
}
void add_to_set(char set[][4], int *cnt, const char *s) {
    if (!in_set(set, *cnt, s)) strcpy(set[(*cnt)++], s);
}

/* ── input ── */
void read_grammar() {
    int n;
    printf("Number of productions: "); scanf("%d", &n); getchar();
    printf("Enter each as  A=aB|c  (use # for epsilon):\n");

    for (int i = 0; i < n; i++) {
        char line[100]; fgets(line, sizeof line, stdin);
        line[strcspn(line, "\n")] = 0;
        char A = line[0];

        /* register non-terminal */
        if (nt_idx(A) < 0) nonterm[nt_cnt++] = A;
        int ai = nt_idx(A);

        char *rhs = strchr(line, '=') + 1;
        char tmp[100]; strcpy(tmp, rhs);
        for (char *tok = strtok(tmp, "|"); tok; tok = strtok(NULL, "|"))
            strcpy(prods[ai][prod_cnt[ai]++], tok);
    }

    /* collect terminals */
    strcpy(term[t_cnt++], "$");
    for (int i = 0; i < nt_cnt; i++)
        for (int j = 0; j < prod_cnt[i]; j++)
            for (char *p = prods[i][j]; *p; p++)
                if (!isupper(*p) && *p != '#') {
                    char ts[2] = {*p, 0};
                    if (t_idx(ts) < 0) strcpy(term[t_cnt++], ts);
                }
}

/* ── FIRST ── */
void compute_first() {
    int changed;
    do {
        changed = 0;
        for (int i = 0; i < nt_cnt; i++)
            for (int j = 0; j < prod_cnt[i]; j++) {
                char *rhs = prods[i][j];
                if (!rhs[0] || rhs[0] == '#') {
                    if (!in_set(First[i], first_cnt[i], "#"))
                    { add_to_set(First[i], &first_cnt[i], "#"); changed = 1; }
                    continue;
                }
                for (int k = 0; rhs[k]; k++) {
                    if (!isupper(rhs[k])) {           /* terminal */
                        char ts[2] = {rhs[k], 0};
                        if (!in_set(First[i], first_cnt[i], ts))
                        { add_to_set(First[i], &first_cnt[i], ts); changed = 1; }
                        break;
                    }
                    int xi = nt_idx(rhs[k]);           /* non-terminal */
                    for (int m = 0; m < first_cnt[xi]; m++)
                        if (strcmp(First[xi][m], "#"))
                            if (!in_set(First[i], first_cnt[i], First[xi][m]))
                            { add_to_set(First[i], &first_cnt[i], First[xi][m]); changed = 1; }
                    if (!in_set(First[xi], first_cnt[xi], "#")) break;
                    if (!rhs[k+1])   /* entire rhs derives eps */
                        if (!in_set(First[i], first_cnt[i], "#"))
                        { add_to_set(First[i], &first_cnt[i], "#"); changed = 1; }
                }
            }
    } while (changed);
}

/* ── FOLLOW ── */
void compute_follow(char start) {
    add_to_set(Follow[nt_idx(start)], &follow_cnt[nt_idx(start)], "$");
    int changed;
    do {
        changed = 0;
        for (int i = 0; i < nt_cnt; i++)
            for (int j = 0; j < prod_cnt[i]; j++) {
                char *rhs = prods[i][j];
                for (int k = 0; rhs[k]; k++) {
                    if (!isupper(rhs[k])) continue;
                    int bi = nt_idx(rhs[k]);
                    /* add FIRST(beta) - {eps} to FOLLOW(B) */
                    int eps_possible = 1;
                    for (int m = k+1; rhs[m] && eps_possible; m++) {
                        if (!isupper(rhs[m])) {
                            char ts[2] = {rhs[m], 0};
                            if (!in_set(Follow[bi], follow_cnt[bi], ts))
                            { add_to_set(Follow[bi], &follow_cnt[bi], ts); changed = 1; }
                            eps_possible = 0;
                        } else {
                            int xi = nt_idx(rhs[m]);
                            for (int q = 0; q < first_cnt[xi]; q++)
                                if (strcmp(First[xi][q], "#"))
                                    if (!in_set(Follow[bi], follow_cnt[bi], First[xi][q]))
                                    { add_to_set(Follow[bi], &follow_cnt[bi], First[xi][q]); changed = 1; }
                            if (!in_set(First[xi], first_cnt[xi], "#")) eps_possible = 0;
                        }
                    }
                    /* if beta =>* eps, add FOLLOW(A) */
                    if (eps_possible)
                        for (int m = 0; m < follow_cnt[i]; m++)
                            if (!in_set(Follow[bi], follow_cnt[bi], Follow[i][m]))
                            { add_to_set(Follow[bi], &follow_cnt[bi], Follow[i][m]); changed = 1; }
                }
            }
    } while (changed);
}

/* ── FIRST of a string (for table building) ── */
void first_of_string(char *s, int src_nt, char out[][4], int *cnt) {
    *cnt = 0;
    if (!s[0] || s[0] == '#') { add_to_set(out, cnt, "#"); return; }
    for (int k = 0; s[k]; k++) {
        if (!isupper(s[k])) {
            char ts[2] = {s[k], 0};
            add_to_set(out, cnt, ts); return;
        }
        int xi = nt_idx(s[k]);
        for (int m = 0; m < first_cnt[xi]; m++)
            if (strcmp(First[xi][m], "#")) add_to_set(out, cnt, First[xi][m]);
        if (!in_set(First[xi], first_cnt[xi], "#")) return;
    }
    add_to_set(out, cnt, "#");
}

/* ── Build parsing table ── */
void build_table() {
    for (int i = 0; i < nt_cnt; i++)
        for (int j = 0; j < t_cnt; j++) table[i][j][0] = 0;

    for (int i = 0; i < nt_cnt; i++)
        for (int j = 0; j < prod_cnt[i]; j++) {
            char *rhs = prods[i][j];
            char fs[MAX_T][4]; int fc = 0;
            first_of_string(rhs, i, fs, &fc);

            for (int m = 0; m < fc; m++) {
                if (!strcmp(fs[m], "#")) continue;
                int ti = t_idx(fs[m]);
                if (ti >= 0) {
                    char entry[MAX_LEN];
                    snprintf(entry, sizeof entry, "%c->%s", nonterm[i], rhs);
                    strcpy(table[i][ti], entry);
                }
            }
            if (in_set(fs, fc, "#"))
                for (int m = 0; m < follow_cnt[i]; m++) {
                    int ti = t_idx(Follow[i][m]);
                    if (ti >= 0) {
                        char entry[MAX_LEN];
                        snprintf(entry, sizeof entry, "%c->%s", nonterm[i], rhs);
                        strcpy(table[i][ti], entry);
                    }
                }
        }
}

/* ── Print table ── */
void print_table() {
    printf("\n=== Parsing Table ===\n%-6s", "");
    for (int j = 0; j < t_cnt; j++) printf("%-14s", term[j]);
    printf("\n");
    for (int i = 0; i < nt_cnt; i++) {
        printf("%-6c", nonterm[i]);
        for (int j = 0; j < t_cnt; j++)
            printf("%-14s", table[i][j][0] ? table[i][j] : "-");
        printf("\n");
    }
}

/* ── Parse an input string ── */
void parse(char *input) {
    /* stack holds chars; top is stack[top-1] */
    char stack[STACK_SIZE]; int top = 0;
    stack[top++] = '$';
    stack[top++] = nonterm[0];   /* push start symbol */

    char buf[100]; snprintf(buf, sizeof buf, "%s$", input);
    int pos = 0;

    printf("\n%-20s %-20s %-20s\n", "Stack", "Input", "Action");
    printf("%-20s %-20s %-20s\n", "-----", "-----", "------");

    while (top > 0) {
        /* print stack */
        char stk_str[STACK_SIZE+1];
        for (int i = top-1, k=0; i >= 0; i--) stk_str[k++] = stack[i], stk_str[k] = 0;
        printf("%-20s %-20s ", stk_str, buf+pos);

        char X = stack[top-1];
        char cur[4] = {buf[pos], 0};   /* current input symbol */

        if (X == '$' && buf[pos] == '$') { printf("Accept\n"); return; }

        if (!isupper(X)) {             /* terminal on stack */
            if (X == buf[pos]) { printf("Match %c\n", X); top--; pos++; }
            else               { printf("Error: expected '%c'\n", X); return; }
        } else {                       /* non-terminal on stack */
            int ni = nt_idx(X), ti = t_idx(cur);
            if (ni < 0 || ti < 0 || !table[ni][ti][0]) {
                printf("Error: no rule for %c on '%s'\n", X, cur); return;
            }
            char *prod = strchr(table[ni][ti], '>') + 1;  /* skip "A->" */
            printf("Output: %c->%s\n", X, prod);
            top--;   /* pop X */
            if (strcmp(prod, "#"))     /* push rhs in reverse */
                for (int k = strlen(prod)-1; k >= 0; k--)
                    stack[top++] = prod[k];
        }
    }
    printf("Error: stack empty before accept.\n");
}

/* ── main ── */
int main() {
    read_grammar();
    compute_first();
    compute_follow(nonterm[0]);

    printf("\n=== FIRST sets ===\n");
    for (int i = 0; i < nt_cnt; i++) {
        printf("FIRST(%c) = { ", nonterm[i]);
        for (int j = 0; j < first_cnt[i]; j++) printf("%s ", First[i][j]);
        printf("}\n");
    }
    printf("\n=== FOLLOW sets ===\n");
    for (int i = 0; i < nt_cnt; i++) {
        printf("FOLLOW(%c) = { ", nonterm[i]);
        for (int j = 0; j < follow_cnt[i]; j++) printf("%s ", Follow[i][j]);
        printf("}\n");
    }

    build_table();
    print_table();

    char input[100];
    printf("\nEnter input string to parse: "); scanf("%s", input);
    parse(input);
    return 0;
}