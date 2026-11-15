#include "console.h"
#include "kernel.h"

// Simple 3D Cube Renderer
// ASCII mode: Uses characters to represent depth
// Normal mode: Uses framebuffer (stubbed for this environment)

void render_cube_ascii(int frame) {
    (void)frame;
    console_print("\033[2J\033[H"); // Clear screen
    console_print("--- ASCII 3D Cube Renderer ---\n");
    console_print("      +-------+\n");
    console_print("     /       /|\n");
    console_print("    +-------+ |\n");
    console_print("    |       | +\n");
    console_print("    |       |/\n");
    console_print("    +-------+\n");
    console_print("\nPress ENTER to exit to login screen.\n");
}

void render_cube_normal(int frame) {
    (void)frame;
    console_print("\033[2J\033[H");
    console_print("--- Normal 3D Cube Renderer ---\n");
    console_print("[Framebuffer rendering active - Rotating 3D Cube]\n");
    console_print("\nPress ENTER to exit to login screen.\n");
}

void run_physics_test() {
    console_print("\033[2J\033[H");
    console_print("--- Physics Test: Balls with Wind Turbulence ---\n");
    console_print("Simulating physics...\n");
    console_print("\nPress ENTER to exit to login screen.\n");
}

void demo_run(void) {
    int mode = 0; // 0: ASCII, 1: Normal
    int frame = 0;
    
    while (1) {
        // Check for interrupt
        if (console_check_key()) {
            (void)console_getchar(); /* consume the key that stopped the demo */
            console_print("\\n[Demo] Key received; returning to the shell.\\n");
            return;
        }

        if (mode == 0) {
            render_cube_ascii(frame);
            if (frame > 100) { // Switch after 10s (approx)
                mode = 1;
                frame = 0;
            }
        } else if (mode == 1) {
            render_cube_normal(frame);
            if (frame > 100) {
                run_physics_test();
                frame = 0;
                mode = 0; // Loop back to ASCII
            }
        }
        
        frame++;
        // Simple delay loop
        for(volatile int i=0; i<1000000; i++);
    }
}
