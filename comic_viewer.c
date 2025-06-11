/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>

#define PANEL_COUNT 6
#define CLEAR_SCREEN "\033[2J\033[H"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define CYAN "\033[36m"
#define RED "\033[31m"
#define RESET "\033[0m"
#define BOLD "\033[1m"

const char* panels[PANEL_COUNT] = {
    // Panel 1
    "\n"
    "╔══════════════════════════════════════════════════════════════╗\n"
    "║  " BOLD YELLOW "Panel 1: The Dreaded Segfault" RESET "                               ║\n"
    "╠══════════════════════════════════════════════════════════════╣\n"
    "║                                                              ║\n"
    "║    Developer: \"Oh no! What if someone                       ║\n"
    "║               passes an invalid ID?\"                         ║\n"
    "║                                                              ║\n"
    "║    " RED "// UNSAFE CODE" RESET "                                            ║\n"
    "║    managed_aircraft_t* get_aircraft(                         ║\n"
    "║        aircraft_manager_t* mgr, int id) {                    ║\n"
    "║        return &mgr->aircraft[id]; " RED "// BOOM! 💥" RESET "                ║\n"
    "║    }                                                         ║\n"
    "║                                                              ║\n"
    "║    " RED "Segfault Bug: \"I'm coming for you!\" 🐛" RESET "                    ║\n"
    "║                                                              ║\n"
    "╚══════════════════════════════════════════════════════════════╝\n",
    
    // Panel 2
    "\n"
    "╔══════════════════════════════════════════════════════════════╗\n"
    "║  " BOLD YELLOW "Panel 2: Truth Bucket to the Rescue!" RESET "                        ║\n"
    "╠══════════════════════════════════════════════════════════════╣\n"
    "║                                                              ║\n"
    "║    Developer: \"We'll use formal verification!\"               ║\n"
    "║                                                              ║\n"
    "║    " CYAN "🛡️  TRUTH BUCKET SHIELD 🛡️" RESET "                                 ║\n"
    "║                                                              ║\n"
    "║    " GREEN "// VERIFIED SAFE" RESET "                                          ║\n"
    "║    TRUTH_REQUIRE(truth,                                      ║\n"
    "║        \"manager is not null\",                                ║\n"
    "║        manager != NULL);                                     ║\n"
    "║    TRUTH_REQUIRE(truth,                                      ║\n"
    "║        \"id is in bounds\",                                    ║\n"
    "║        id >= 0 && id < mgr->count);                         ║\n"
    "║                                                              ║\n"
    "╚══════════════════════════════════════════════════════════════╝\n",
    
    // Panel 3
    "\n"
    "╔══════════════════════════════════════════════════════════════╗\n"
    "║  " BOLD YELLOW "Panel 3: Every Access is Bounds-Checked" RESET "                     ║\n"
    "╠══════════════════════════════════════════════════════════════╣\n"
    "║                                                              ║\n"
    "║    Aircraft Array [16 slots total, 5 used]:                 ║\n"
    "║    " GREEN "[✓][✓][✓][✓][✓]" RESET "[ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ]         ║\n"
    "║     0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15         ║\n"
    "║                                                              ║\n"
    "║    " GREEN "// Runtime verification" RESET "                                   ║\n"
    "║    static bool verify_aircraft_count_bounds(                 ║\n"
    "║        const truth_bucket_t* bucket,                         ║\n"
    "║        void* context) {                                      ║\n"
    "║        aircraft_manager_t* mgr = context;                    ║\n"
    "║        return mgr != NULL &&                                 ║\n"
    "║               mgr->aircraft_count >= 0 &&                    ║\n"
    "║               mgr->aircraft_count <= MAX_AIRCRAFT;           ║\n"
    "║    }                                                         ║\n"
    "║                                                              ║\n"
    "╚══════════════════════════════════════════════════════════════╝\n",
    
    // Panel 4
    "\n"
    "╔══════════════════════════════════════════════════════════════╗\n"
    "║  " BOLD YELLOW "Panel 4: The Safe Wrapper Function" RESET "                          ║\n"
    "╠══════════════════════════════════════════════════════════════╣\n"
    "║                                                              ║\n"
    "║    " RED "Unsafe Request" RESET " → " CYAN "Truth Verification" RESET " → " GREEN "Safe Access" RESET "         ║\n"
    "║                                                              ║\n"
    "║    managed_aircraft_t* aircraft_manager_get_safe(            ║\n"
    "║        aircraft_manager_t* manager, int id,                  ║\n"
    "║        truth_bucket_t* truth) {                              ║\n"
    "║                                                              ║\n"
    "║        TRUTH_REQUIRE(truth, \"valid manager\",                 ║\n"
    "║                      manager != NULL);                       ║\n"
    "║        TRUTH_REQUIRE(truth, \"valid id\",                      ║\n"
    "║                      id >= 0 && id < manager->count);        ║\n"
    "║                                                              ║\n"
    "║        return &manager->aircraft[id]; " GREEN "// SAFE! ✓" RESET "             ║\n"
    "║    }                                                         ║\n"
    "║                                                              ║\n"
    "╚══════════════════════════════════════════════════════════════╝\n",
    
    // Panel 5
    "\n"
    "╔══════════════════════════════════════════════════════════════╗\n"
    "║  " BOLD YELLOW "Panel 5: Mathematical Proof with F*" RESET "                         ║\n"
    "╠══════════════════════════════════════════════════════════════╣\n"
    "║                                                              ║\n"
    "║    Mathematician: \"Proven at compile time!\" 🎓                ║\n"
    "║                                                              ║\n"
    "║    " CYAN "// F* Type System Proof" RESET "                                   ║\n"
    "║    type valid_count = n:nat{n <= 16}                         ║\n"
    "║    type valid_id (count:valid_count) =                      ║\n"
    "║        n:nat{n < count}                                      ║\n"
    "║                                                              ║\n"
    "║    let get_aircraft (mgr:aircraft_manager)                   ║\n"
    "║        (id:valid_id mgr.count) =                            ║\n"
    "║        mgr.aircraft.(id) " GREEN "(* Type-safe! *)" RESET "                   ║\n"
    "║                                                              ║\n"
    "║    " GREEN "// Compiler rejects invalid access!" RESET "                       ║\n"
    "║                                                              ║\n"
    "╚══════════════════════════════════════════════════════════════╝\n",
    
    // Panel 6
    "\n"
    "╔══════════════════════════════════════════════════════════════╗\n"
    "║  " BOLD YELLOW "Panel 6: Segfault Defeated!" RESET "                                 ║\n"
    "╠══════════════════════════════════════════════════════════════╣\n"
    "║                                                              ║\n"
    "║    Developer: \"No more segfaults! 🎉\"                        ║\n"
    "║               \"Every access is verified safe!\"               ║\n"
    "║                                                              ║\n"
    "║    Segfault Bug: ☠️  " RED "RIP" RESET "                                      ║\n"
    "║                                                              ║\n"
    "║    " GREEN "✓ Null checks on every access" RESET "                            ║\n"
    "║    " GREEN "✓ Bounds verified before array access" RESET "                    ║\n"
    "║    " GREEN "✓ Type system prevents invalid states" RESET "                    ║\n"
    "║    " GREEN "✓ Truth buckets document invariants" RESET "                      ║\n"
    "║                                                              ║\n"
    "║    " BOLD CYAN "The End - Sleep well knowing your code is SAFE!" RESET "          ║\n"
    "║                                                              ║\n"
    "╚══════════════════════════════════════════════════════════════╝\n"
};

void set_raw_mode(struct termios* orig) {
    struct termios raw;
    tcgetattr(STDIN_FILENO, orig);
    raw = *orig;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void show_panel(int panel_num) {
    printf(CLEAR_SCREEN);
    printf(GREEN "═══════════════════════════════════════════════════════════════════\n" RESET);
    printf(BOLD "       Aircraft Manager Safety Comic - How We Prevent Segfaults\n" RESET);
    printf(GREEN "═══════════════════════════════════════════════════════════════════\n" RESET);
    
    printf("%s", panels[panel_num]);
    
    printf("\n" CYAN "Controls: [←/→] Navigate  [a] Show All  [q] Quit  Panel %d/%d\n" RESET, 
           panel_num + 1, PANEL_COUNT);
}

void show_all_panels() {
    printf(CLEAR_SCREEN);
    printf(GREEN "═══════════════════════════════════════════════════════════════════\n" RESET);
    printf(BOLD "       Aircraft Manager Safety Comic - How We Prevent Segfaults\n" RESET);
    printf(GREEN "═══════════════════════════════════════════════════════════════════\n" RESET);
    
    for (int i = 0; i < PANEL_COUNT; i++) {
        printf("%s", panels[i]);
        if (i < PANEL_COUNT - 1) {
            printf("\n");
        }
    }
    
    printf("\n" CYAN "Controls: [←/→] Navigate  [1-6] Jump to Panel  [q] Quit\n" RESET);
}

int main() {
    struct termios orig_termios;
    set_raw_mode(&orig_termios);
    
    int current_panel = 0;
    int show_all = 0;
    
    show_panel(current_panel);
    
    char c;
    while (1) {
        if (read(STDIN_FILENO, &c, 1) == 1) {
            if (c == 'q' || c == 'Q') {
                break;
            } else if (c == '\033') {
                // Arrow key sequence
                char seq[2];
                if (read(STDIN_FILENO, &seq[0], 1) == 1 && 
                    read(STDIN_FILENO, &seq[1], 1) == 1) {
                    if (seq[0] == '[') {
                        if (seq[1] == 'C' && current_panel < PANEL_COUNT - 1) {
                            // Right arrow
                            current_panel++;
                            show_all = 0;
                        } else if (seq[1] == 'D' && current_panel > 0) {
                            // Left arrow
                            current_panel--;
                            show_all = 0;
                        }
                    }
                }
            } else if (c >= '1' && c <= '6') {
                current_panel = c - '1';
                show_all = 0;
            } else if (c == 'a' || c == 'A') {
                show_all = 1;
            }
            
            if (show_all) {
                show_all_panels();
            } else {
                show_panel(current_panel);
            }
        }
    }
    
    // Restore terminal
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    printf(CLEAR_SCREEN);
    
    return 0;
}