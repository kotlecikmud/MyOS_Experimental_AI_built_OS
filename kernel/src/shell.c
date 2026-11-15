#include "shell.h"
#include "console.h"
#include "vfs.h"
#include "kernel.h"

void shell_init(void) {
    console_print("[Shell] Initializing MyOS interactive command-line shell...\n");
}

void shell_execute(char* cmd_buf) {
    // Parse command and argument
    char* cmd = cmd_buf;
    while (*cmd == ' ') cmd++;
    
    char* arg = cmd;
    while (*arg && *arg != ' ') arg++;
    if (*arg == ' ') {
        *arg = '\0';
        arg++;
        while (*arg == ' ') arg++;
    } else {
        arg = NULL;
    }

    // Compare command
    // 1. "list"
    if (cmd[0] == 'l' && cmd[1] == 'i' && cmd[2] == 's' && cmd[3] == 't' && cmd[4] == '\0') {
        vfs_list_dir();
    }
    // 2. "cd"
    else if (cmd[0] == 'c' && cmd[1] == 'd' && cmd[2] == '\0') {
        if (arg == NULL || arg[0] == '\0') {
            vfs_change_dir("/");
        } else {
            vfs_change_dir(arg);
        }
    }
    // 3. "clear"
    else if (cmd[0] == 'c' && cmd[1] == 'l' && cmd[2] == 'e' && cmd[3] == 'a' && cmd[4] == 'r' && cmd[5] == '\0') {
        console_print("\033[2J\033[H");
    }
    // 4. "info"
    else if (cmd[0] == 'i' && cmd[1] == 'n' && cmd[2] == 'f' && cmd[3] == 'o' && cmd[4] == '\0') {
        console_print("MyOS v0.1 - Milestone 5\n");
    }
    // 5. "reboot"
    else if (cmd[0] == 'r' && cmd[1] == 'e' && cmd[2] == 'b' && cmd[3] == 'o' && cmd[4] == 'o' && cmd[5] == 't' && cmd[6] == '\0') {
        outb(0x64, 0xFE);
    }
    // 6. "version"
    else if (cmd[0] == 'v' && cmd[1] == 'e' && cmd[2] == 'r' && cmd[3] == 's' && cmd[4] == 'i' && cmd[5] == 'o' && cmd[6] == 'n' && cmd[7] == '\0') {
        console_print("MyOS Kernel v0.1.0 (Build: " __DATE__ " " __TIME__ ")\n");
    }
    // 7. "uptime"
    else if (cmd[0] == 'u' && cmd[1] == 'p' && cmd[2] == 't' && cmd[3] == 'i' && cmd[4] == 'm' && cmd[5] == 'e' && cmd[6] == '\0') {
        console_print("System uptime: [Not implemented yet]\n");
    }
    // "demo" - launch the built-in visual demo on demand.
    else if (cmd[0] == 'd' && cmd[1] == 'e' && cmd[2] == 'm' && cmd[3] == 'o' && cmd[4] == '\0') {
        demo_run();
    }
    // 8. "echo"
    else if (cmd[0] == 'e' && cmd[1] == 'c' && cmd[2] == 'h' && cmd[3] == 'o' && cmd[4] == '\0') {
        if (arg != NULL) {
            console_print(arg);
            console_print("\n");
        }
    }
    else if (cmd[0] == 'h' && cmd[1] == 'e' && cmd[2] == 'l' && cmd[3] == 'p' && cmd[4] == '\0') {
        console_print("Available commands:\n");
        console_print("  list         - Display contents of current directory\n");
        console_print("  cd <path>    - Change current working directory\n");
        console_print("  clear        - Clear the screen\n");
        console_print("  info         - Display system information\n");
        console_print("  reboot       - Reboot the system\n");
        console_print("  version      - Display kernel version\n");
        console_print("  uptime       - Display system uptime\n");
        console_print("  echo <text>  - Echo text to console\n");
        console_print("  demo         - Run the built-in visual demo\n");
        console_print("  help         - Display this help message\n");
    }
    else if (cmd[0] == '\0') {
        // Empty command
    }
    else {
        console_print("Unknown command: ");
        console_print(cmd);
        console_print(". Type 'help' for available commands.\n");
    }
}

void shell_run(void) {
    console_print("\n==================================================\n");
    console_print(" Welcome to MyOS Command Shell (Milestone 5)      \n");
    console_print(" Supported commands: list, cd <path>              \n");
    console_print("==================================================\n\n");

    char cmd_buf[128];
    for (;;) {
        console_print("MyOS:");
        console_print(vfs_get_current_path());
        console_print("> ");

        int len = console_getline(cmd_buf, sizeof(cmd_buf));
        if (len > 0) {
            shell_execute(cmd_buf);
        }
    }
}
