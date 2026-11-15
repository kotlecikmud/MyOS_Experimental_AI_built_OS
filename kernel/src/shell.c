#include "shell.h"
#include "console.h"
#include "vfs.h"

void shell_init(void) {
    console_print("[Shell] Initializing MyOS interactive command-line shell...\n");
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
        if (len <= 0) {
            continue;
        }

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
        else if (cmd[0] == 'h' && cmd[1] == 'e' && cmd[2] == 'l' && cmd[3] == 'p' && cmd[4] == '\0') {
            console_print("Available commands:\n");
            console_print("  list         - Display contents of current directory\n");
            console_print("  cd <path>    - Change current working directory\n");
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
}
