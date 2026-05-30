#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_PROD   20
#define MAX_ITEMS  100
#define MAX_STATES 50
#define MAX_SYM    20
#define MAX_LEN    20
#define MAX_STACK  100

// ── Grammar ───────────────────────────────────────────────────────────
char lhs[MAX_PROD];
char rhs[MAX_PROD][MAX_LEN];
int  num_prod = 0;

char nt[MAX_SYM];
int  nt_count = 0;

// ── LR(0) Items ───────────────────────────────────────────────────────
int item_prod [MAX_STATES][MAX_ITEMS];
int item_dot  [MAX_STATES][MAX_ITEMS];
int item_count[MAX_STATES];
int num_states = 0;

// ── FIRST and FOLLOW ──────────────────────────────────────────────────
char first_set [MAX_SYM][MAX_SYM];
char follow_set[MAX_SYM][MAX_SYM];

// ── SLR Table ─────────────────────────────────────────────────────────
// action_table[state][symbol] → "s3"=shift3, "r2"=reduce2, "acc"=accept
// goto_table  [state][nt]     → state number (-1 = empty)
char action[MAX_STATES][MAX_SYM][10];
int  go_to [MAX_STATES][MAX_SYM];

char terminals[MAX_SYM];
int  term_count = 0;

// ─────────────────────────────────────────────────────────────────────
//  SECTION 1 : LR(0) (same as before)
// ─────────────────────────────────────────────────────────────────────

int is_nt(char c) {
    for (int i = 0; i < nt_count; i++)
        if (nt[i] == c) return 1;
    return 0;
}

int add_item(int s, int prod, int dot) {
    for (int k = 0; k < item_count[s]; k++)
        if (item_prod[s][k] == prod && item_dot[s][k] == dot)
            return 0;
    item_prod [s][item_count[s]] = prod;
    item_dot  [s][item_count[s]] = dot;
    item_count[s]++;
    return 1;
}

void closure(int s) {
    int changed = 1;
    while (changed) {
        changed = 0;
        for (int k = 0; k < item_count[s]; k++) {
            int p   = item_prod[s][k];
            int dot = item_dot [s][k];
            if (dot >= (int)strlen(rhs[p])) continue;
            char next = rhs[p][dot];
            if (!is_nt(next)) continue;
            for (int p2 = 0; p2 < num_prod; p2++)
                if (lhs[p2] == next)
                    if (add_item(s, p2, 0))
                        changed = 1;
        }
    }
}

int compute_goto(int s, char sym) {
    int t = num_states;
    item_count[t] = 0;
    for (int k = 0; k < item_count[s]; k++) {
        int p   = item_prod[s][k];
        int dot = item_dot [s][k];
        if (dot < (int)strlen(rhs[p]) && rhs[p][dot] == sym)
            add_item(t, p, dot + 1);
    }
    if (item_count[t] == 0) return -1;
    closure(t);
    for (int i = 0; i < num_states; i++) {
        if (item_count[i] != item_count[t]) continue;
        int match = 1;
        for (int k = 0; k < item_count[t]; k++) {
            int found = 0;
            for (int m = 0; m < item_count[i]; m++)
                if (item_prod[i][m] == item_prod[t][k] &&
                    item_dot [i][m] == item_dot [t][k])
                    { found = 1; break; }
            if (!found) { match = 0; break; }
        }
        if (match) return i;
    }
    num_states++;
    return t;
}

void get_symbols(char *sym, int *sym_count) {
    *sym_count = 0;
    for (int i = 0; i < nt_count; i++)
        sym[(*sym_count)++] = nt[i];
    for (int p = 0; p < num_prod; p++)
        for (int k = 0; rhs[p][k]; k++) {
            char c = rhs[p][k];
            if (c == '#') continue;
            int dup = 0;
            for (int s = 0; s < *sym_count; s++)
                if (sym[s] == c) { dup = 1; break; }
            if (!dup) sym[(*sym_count)++] = c;
        }
}

void build_lr0() {
    item_count[0] = 0;
    add_item(0, 0, 0);
    closure(0);
    num_states = 1;

    char sym[MAX_SYM];
    int  sym_count = 0;
    get_symbols(sym, &sym_count);

    for (int s = 0; s < num_states; s++)
        for (int si = 0; si < sym_count; si++)
            compute_goto(s, sym[si]);
}

// ─────────────────────────────────────────────────────────────────────
//  SECTION 2 : FIRST and FOLLOW
// ─────────────────────────────────────────────────────────────────────

int nt_idx(char c) {
    for (int i = 0; i < nt_count; i++)
        if (nt[i] == c) return i;
    return -1;
}

int contains(char set[], char c) {
    for (int i = 0; set[i]; i++)
        if (set[i] == c) return 1;
    return 0;
}

int add_to(char set[], char c) {
    if (!contains(set, c)) {
        int l = strlen(set);
        set[l] = c; set[l+1] = '\0';
        return 1;
    }
    return 0;
}

void calc_first() {
    // Base: terminal on RHS directly
    for (int i = 0; i < num_prod; i++) {
        char A = lhs[i];
        char X = rhs[i][0];
        if (!isupper(X))
            add_to(first_set[nt_idx(A)], X);
    }
    int changed = 1;
    while (changed) {
        changed = 0;
        for (int i = 0; i < num_prod; i++) {
            char A    = lhs[i];
            int  Aidx = nt_idx(A);
            int  k    = 0;
            int  hasEps = 1;
            while (rhs[i][k] != '\0' && hasEps) {
                char X = rhs[i][k];
                hasEps = 0;
                if (isupper(X)) {
                    int Xidx = nt_idx(X);
                    for (int j = 0; first_set[Xidx][j]; j++) {
                        if (first_set[Xidx][j] == '#') hasEps = 1;
                        else if (add_to(first_set[Aidx], first_set[Xidx][j]))
                            changed = 1;
                    }
                } else {
                    if (X == '#') hasEps = 1;
                    else if (add_to(first_set[Aidx], X)) changed = 1;
                }
                k++;
            }
            if (hasEps)
                if (add_to(first_set[Aidx], '#')) changed = 1;
        }
    }
}

void calc_follow() {
    add_to(follow_set[nt_idx(lhs[0])], '$');
    int changed = 1;
    while (changed) {
        changed = 0;
        for (int i = 0; i < num_prod; i++) {
            char A    = lhs[i];
            int  Aidx = nt_idx(A);
            for (int j = 0; rhs[i][j] != '\0'; j++) {
                char B    = rhs[i][j];
                int  Bidx = nt_idx(B);
                if (!isupper(B)) continue;
                int  k      = j + 1;
                int  hasEps = 1;
                while (rhs[i][k] != '\0' && hasEps) {
                    char X = rhs[i][k];
                    hasEps = 0;
                    if (isupper(X)) {
                        int Xidx = nt_idx(X);
                        for (int l = 0; first_set[Xidx][l]; l++) {
                            if (first_set[Xidx][l] == '#') hasEps = 1;
                            else if (add_to(follow_set[Bidx], first_set[Xidx][l]))
                                changed = 1;
                        }
                    } else {
                        if (X == '#') hasEps = 1;
                        else if (add_to(follow_set[Bidx], X)) changed = 1;
                    }
                    k++;
                }
                if (hasEps)
                    for (int l = 0; follow_set[Aidx][l]; l++)
                        if (add_to(follow_set[Bidx], follow_set[Aidx][l]))
                            changed = 1;
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────────────
//  SECTION 3 : BUILD SLR TABLE
// ─────────────────────────────────────────────────────────────────────

int term_idx(char c) {
    for (int i = 0; i < term_count; i++)
        if (terminals[i] == c) return i;
    return -1;
}

void collect_terminals() {
    // Everything that appears in RHS and is not a NT, excluding #
    for (int p = 0; p < num_prod; p++)
        for (int k = 0; rhs[p][k]; k++) {
            char c = rhs[p][k];
            if (!isupper(c) && c != '#') {
                int dup = 0;
                for (int i = 0; i < term_count; i++)
                    if (terminals[i] == c) { dup = 1; break; }
                if (!dup) terminals[term_count++] = c;
            }
        }
    terminals[term_count++] = '$';  // add end marker
}

void build_slr_table() {
    // Init table with empty strings / -1
    for (int s = 0; s < MAX_STATES; s++) {
        for (int t = 0; t < MAX_SYM; t++) {
            action[s][t][0] = '\0';
            go_to [s][t]    = -1;
        }
    }

    char sym[MAX_SYM];
    int  sym_count = 0;
    get_symbols(sym, &sym_count);

    for (int s = 0; s < num_states; s++) {
        for (int k = 0; k < item_count[s]; k++) {
            int  p   = item_prod[s][k];
            int  dot = item_dot [s][k];
            int  len = strlen(rhs[p]);

            // ── SHIFT / GOTO ──────────────────────────────────────────
            if (dot < len) {
                char sym_after_dot = rhs[p][dot];
                int  t = compute_goto(s, sym_after_dot);
                if (t < 0) continue;

                if (!is_nt(sym_after_dot)) {
                    // SHIFT
                    int ti = term_idx(sym_after_dot);
                    if (ti >= 0) {
                        char entry[10];
                        sprintf(entry, "s%d", t);
                        if (action[s][ti][0] != '\0' &&
                            strcmp(action[s][ti], entry) != 0)
                            printf("  [CONFLICT] action[%d][%c]\n", s, sym_after_dot);
                        else
                            strcpy(action[s][ti], entry);
                    }
                } else {
                    // GOTO
                    int ni = nt_idx(sym_after_dot);
                    if (ni >= 0) go_to[s][ni] = t;
                }
            }

            // ── REDUCE ───────────────────────────────────────────────
            else {
                // Augmented start: ACCEPT
                if (lhs[p] == lhs[0] && p == 0) {
                    int di = term_idx('$');
                    strcpy(action[s][di], "acc");
                    continue;
                }
                // For every terminal in FOLLOW(lhs[p]), add reduce
                int Aidx = nt_idx(lhs[p]);
                for (int fi = 0; follow_set[Aidx][fi]; fi++) {
                    char f  = follow_set[Aidx][fi];
                    int  ti = term_idx(f);
                    if (ti < 0) continue;
                    char entry[10];
                    sprintf(entry, "r%d", p);
                    if (action[s][ti][0] != '\0' &&
                        strcmp(action[s][ti], entry) != 0)
                        printf("  [CONFLICT] action[%d][%c]: was %s, now %s\n",
                               s, f, action[s][ti], entry);
                    else
                        strcpy(action[s][ti], entry);
                }
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────────────
//  SECTION 4 : PRINT SLR TABLE
// ─────────────────────────────────────────────────────────────────────

void print_table() {
    printf("\n╔══ SLR Parsing Table ══════════════════════════════╗\n");
    printf("  State |");
    printf(" ACTION");
    for (int i = 0; i < term_count; i++) printf("  [%c] ", terminals[i]);
    printf("| GOTO");
    for (int i = 0; i < nt_count; i++) printf("  [%c] ", nt[i]);
    printf("\n");
    printf("  ------+");
    for (int i = 0; i < term_count; i++) printf("-------");
    printf("+-----");
    for (int i = 0; i < nt_count; i++) printf("------");
    printf("\n");

    for (int s = 0; s < num_states; s++) {
        printf("    %2d  |", s);
        for (int t = 0; t < term_count; t++) {
            if (action[s][t][0] != '\0')
                printf("  %-5s", action[s][t]);
            else
                printf("  .   ");
        }
        printf("|");
        for (int n = 0; n < nt_count; n++) {
            if (go_to[s][n] >= 0)
                printf("  %-4d ", go_to[s][n]);
            else
                printf("  .    ");
        }
        printf("\n");
    }
    printf("╚═══════════════════════════════════════════════════╝\n");
}

// ─────────────────────────────────────────────────────────────────────
//  SECTION 5 : SLR PARSING (Stack simulation)
// ─────────────────────────────────────────────────────────────────────

void parse_input(char *input) {
    int   state_stack[MAX_STACK];
    char  sym_stack  [MAX_STACK];
    int   top = 0;

    state_stack[0] = 0;
    sym_stack  [0] = '$';  // bottom marker

    int ip = 0;  // input pointer

    printf("\n╔══ Parsing Steps ════════════════════════════════════╗\n");
    printf("  %-25s %-15s %s\n", "Stack", "Input", "Action");
    printf("  ─────────────────────────────────────────────────\n");

    while (1) {
        int  cur  = state_stack[top];
        char sym  = input[ip];
        int  ti   = term_idx(sym);

        // Print current stack
        printf("  ");
        for (int i = 0; i <= top; i++) printf("%d ", state_stack[i]);
        // pad
        int pad = 25 - (top + 1) * 2;
        for (int i = 0; i < pad; i++) printf(" ");

        // Print remaining input
        printf("%-15s", input + ip);

        if (ti < 0) {
            printf("  Error: unknown symbol '%c'\n", sym);
            return;
        }

        char *act = action[cur][ti];

        if (act[0] == '\0') {
            printf("  Error: no action for state %d on '%c'\n", cur, sym);
            return;
        }

        if (strcmp(act, "acc") == 0) {
            printf("  Accept ✓\n");
            return;
        }

        if (act[0] == 's') {
            int next_state = atoi(act + 1);
            printf("  Shift  → go to state %d\n", next_state);
            top++;
            state_stack[top] = next_state;
            sym_stack  [top] = sym;
            ip++;

        } else if (act[0] == 'r') {
            int prod_num = atoi(act + 1);
            int rhs_len  = strlen(rhs[prod_num]);
            // Handle epsilon production
            if (strcmp(rhs[prod_num], "#") == 0) rhs_len = 0;

            printf("  Reduce by P%d: %c = %s\n",
                   prod_num, lhs[prod_num], rhs[prod_num]);

            top -= rhs_len;

            // GOTO
            int ni       = nt_idx(lhs[prod_num]);
            int goto_st  = go_to[state_stack[top]][ni];
            if (goto_st < 0) {
                printf("  Error: no GOTO for state %d on '%c'\n",
                       state_stack[top], lhs[prod_num]);
                return;
            }
            top++;
            state_stack[top] = goto_st;
            sym_stack  [top] = lhs[prod_num];
        }
    }
}

// ─────────────────────────────────────────────────────────────────────
//  MAIN
// ─────────────────────────────────────────────────────────────────────

int main() {
    int n;
    printf("Enter number of productions: ");
    scanf("%d", &n);

    for (int i = 0; i < n; i++) {
        char buf[MAX_LEN];
        printf("Enter production %d (X=abc): ", i + 1);
        scanf("%s", buf);
        lhs[num_prod] = buf[0];
        strcpy(rhs[num_prod], buf + 2);
        int found = 0;
        for (int j = 0; j < nt_count; j++)
            if (nt[j] == lhs[num_prod]) { found = 1; break; }
        if (!found) nt[nt_count++] = lhs[num_prod];
        num_prod++;
    }

    printf("\n--- Productions ---\n");
    for (int i = 0; i < num_prod; i++)
        printf("  P%d: %c = %s\n", i, lhs[i], rhs[i]);

    // Build LR(0)
    build_lr0();

    // FIRST and FOLLOW
    calc_first();
    calc_follow();

    printf("\n--- FIRST sets ---\n");
    for (int i = 0; i < nt_count; i++) {
        printf("  FIRST(%c) = { ", nt[i]);
        for (int j = 0; first_set[i][j]; j++) printf("%c ", first_set[i][j]);
        printf("}\n");
    }
    printf("--- FOLLOW sets ---\n");
    for (int i = 0; i < nt_count; i++) {
        printf("  FOLLOW(%c) = { ", nt[i]);
        for (int j = 0; follow_set[i][j]; j++) printf("%c ", follow_set[i][j]);
        printf("}\n");
    }

    // Collect terminals and build SLR table
    collect_terminals();
    build_slr_table();
    print_table();

    // Parse input string
    char input[MAX_LEN];
    printf("\nEnter input string (end with $): ");
    scanf("%s", input);
    parse_input(input);

    return 0;
}