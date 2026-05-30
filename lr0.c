#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_PROD   20
#define MAX_ITEMS  100
#define MAX_STATES 50
#define MAX_SYM    20
#define MAX_LEN    20

// ── Grammar ──────────────────────────────────────────────────────────
char lhs[MAX_PROD];          // lhs[i]    = LHS of production i
char rhs[MAX_PROD][MAX_LEN]; // rhs[i]    = RHS of production i
int  num_prod = 0;

char nt[MAX_SYM];            // non-terminals
int  nt_count = 0;

// ── Items: (production index, dot position) ──────────────────────────
// state_items[s][k] = k-th item of state s
int  item_prod [MAX_STATES][MAX_ITEMS]; // which production
int  item_dot  [MAX_STATES][MAX_ITEMS]; // dot position
int  item_count[MAX_STATES];            // how many items in state s
int  num_states = 0;

// ── Helper: is char a non-terminal? ─────────────────────────────────
int is_nt(char c) {
    for (int i = 0; i < nt_count; i++)
        if (nt[i] == c) return 1;
    return 0;
}

// ── Helper: add item to state, return 1 if newly added ───────────────
int add_item(int s, int prod, int dot) {
    for (int k = 0; k < item_count[s]; k++)
        if (item_prod[s][k] == prod && item_dot[s][k] == dot)
            return 0;
    item_prod [s][item_count[s]] = prod;
    item_dot  [s][item_count[s]] = dot;
    item_count[s]++;
    return 1;
}

// ── CLOSURE: expand [A -> α.Bβ] by adding [B -> .γ] ─────────────────
void closure(int s) {
    int changed = 1;
    while (changed) {
        changed = 0;
        for (int k = 0; k < item_count[s]; k++) {
            int  p   = item_prod[s][k];
            int  dot = item_dot [s][k];
            int  len = strlen(rhs[p]);
            if (dot >= len) continue;          // dot at end

            char next = rhs[p][dot];
            if (!is_nt(next)) continue;

            // add all productions of 'next' with dot at 0
            for (int p2 = 0; p2 < num_prod; p2++)
                if (lhs[p2] == next)
                    if (add_item(s, p2, 0))
                        changed = 1;
        }
    }
}

// ── GOTO(s, sym): build new state, return its index ──────────────────
// Returns -1 if empty
int compute_goto(int s, char sym) {
    // Build a temporary state at index num_states
    int t = num_states;
    item_count[t] = 0;

    for (int k = 0; k < item_count[s]; k++) {
        int  p   = item_prod[s][k];
        int  dot = item_dot [s][k];
        int  len = strlen(rhs[p]);
        if (dot < len && rhs[p][dot] == sym)
            add_item(t, p, dot + 1);
    }

    if (item_count[t] == 0) return -1;

    closure(t);

    // Check if this state already exists
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
        if (match) return i;   // already exists
    }

    // New state
    num_states++;
    return t;
}

// ── Collect all grammar symbols ───────────────────────────────────────
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

// ── Print one item ────────────────────────────────────────────────────
void print_item(int p, int dot) {
    int len = strlen(rhs[p]);
    printf("  [ %c -> ", lhs[p]);
    for (int k = 0; k < len; k++) {
        if (k == dot) printf(". ");
        printf("%c ", rhs[p][k]);
    }
    if (dot == len) printf(". ");
    printf("]");
    if (dot == len) printf("  (reduce)");
    printf("\n");
}

// ── BUILD LR(0) ───────────────────────────────────────────────────────
void build_lr0() {
    // I0: closure of [prod0, dot=0]
    item_count[0] = 0;
    add_item(0, 0, 0);
    closure(0);
    num_states = 1;

    char sym[MAX_SYM];
    int  sym_count = 0;
    get_symbols(sym, &sym_count);

    // Work through each state
    for (int s = 0; s < num_states; s++) {
        printf("\nState I%d:\n", s);
        for (int k = 0; k < item_count[s]; k++)
            print_item(item_prod[s][k], item_dot[s][k]);

        printf("  GOTO:\n");
        int any = 0;
        for (int si = 0; si < sym_count; si++) {
            int t = compute_goto(s, sym[si]);
            if (t >= 0) {
                printf("    GOTO ( I%d, %c ) = I%d\n", s, sym[si], t);
                any = 1;
            }
        }
        if (!any) printf("    (none)\n");
    }
}

// ── MAIN ──────────────────────────────────────────────────────────────
int main() {
    int n;
    printf("Enter number of productions: ");
    scanf("%d", &n);

    // Read productions like: S=Ab  or  S=#
    // First production must be the augmented rule e.g. Z=S
    for (int i = 0; i < n; i++) {
        char buf[MAX_LEN];
        printf("Enter production %d (X=abc): ", i + 1);
        scanf("%s", buf);

        lhs[num_prod] = buf[0];      // LHS
        strcpy(rhs[num_prod], buf + 2); // skip 'X='

        // Register non-terminal
        int found = 0;
        for (int j = 0; j < nt_count; j++)
            if (nt[j] == lhs[num_prod]) { found = 1; break; }
        if (!found) nt[nt_count++] = lhs[num_prod];

        num_prod++;
    }

    printf("\n--- Productions ---\n");
    for (int i = 0; i < num_prod; i++)
        printf("  P%d: %c = %s\n", i, lhs[i], rhs[i]);

    printf("\n--- LR(0) Canonical Collection ---\n");
    build_lr0();

    return 0;
}