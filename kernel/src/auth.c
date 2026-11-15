#include "auth.h"
#include "console.h"
#include "vfs.h"

// Simple pseudo-random salt generator using internal state / timer
static void generate_salt(char* salt_buf, int len) {
    // 16 bytes hex-encoded salt
    static uint32_t seed = 0x12345678;
    for (int i = 0; i < len - 1; i++) {
        seed = seed * 1103515245 + 12345;
        char hex[] = "0123456789abcdef";
        salt_buf[i] = hex[(seed >> (i % 4)) & 0x0F];
    }
    salt_buf[len - 1] = '\0';
}

// Simple iterative salted hash (PBKDF2-style secure derivation simulation for freestanding kernel)
static void hash_password(const char* password, const char* salt, char* hash_out) {
    uint32_t h = 0x811c9dc5;
    // Mix salt
    for (int i = 0; salt[i]; i++) {
        h ^= (unsigned char)salt[i];
        h *= 0x01000193;
    }
    // Mix password over 1000 rounds
    for (int r = 0; r < 1000; r++) {
        for (int i = 0; password[i]; i++) {
            h ^= (unsigned char)password[i] + r;
            h *= 0x01000193;
        }
    }
    // Format hash as hex string
    char hex[] = "0123456789abcdef";
    for (int i = 0; i < 8; i++) {
        hash_out[i] = hex[(h >> (4 * (7 - i))) & 0x0F];
    }
    hash_out[8] = '\0';
}

// In-memory shadow storage representation for the kernel VFS environment
static char stored_salt[32] = "";
static char stored_hash[32] = "";
static int account_exists = 0;

void auth_init(void) {
    console_print("[Auth] Initializing authentication and security subsystem...\n");
}

int auth_authenticate(void) {
    // Check if we should auto-login
    if (vfs_find_node_by_path("/run.myos") != -1) {
        console_print("[Auth] Auto-login enabled via /run.myos file.\n");
        return 1;
    }

    console_print("\n==================================================\n");
    console_print(" MyOS Security & Authentication (Milestone 6)     \n");
    console_print("==================================================\n\n");

    if (!account_exists) {
        console_print("[First Boot] No administrator account found.\n");
        console_print("Please create your administrator account.\n\n");

        char username[32];
        char password[64];
        char confirm[64];

        for (;;) {
            console_print("Enter administrator username [default: admin]: ");
            int ulen = console_getline(username, sizeof(username));
            if (ulen <= 0) {
                // Default to admin
                username[0] = 'a'; username[1] = 'd'; username[2] = 'm'; username[3] = 'i'; username[4] = 'n'; username[5] = '\0';
            }
            break;
        }

        for (;;) {
            console_print("Enter administrator password: ");
            int plen = console_getline(password, sizeof(password));
            if (plen < 4) {
                console_print("Password must be at least 4 characters long. Try again.\n");
                continue;
            }

            console_print("Confirm administrator password: ");
            int clen = console_getline(confirm, sizeof(confirm));
            
            // Compare password and confirm
            int match = 1;
            for (int i = 0; i <= plen || i <= clen; i++) {
                if (password[i] != confirm[i]) {
                    match = 0;
                    break;
                }
            }

            if (!match) {
                console_print("Passwords do not match. Please try again.\n");
                continue;
            }
            break;
        }

        generate_salt(stored_salt, sizeof(stored_salt));
        hash_password(password, stored_salt, stored_hash);
        account_exists = 1;

        console_print("\n[Auth] Administrator account successfully created and secured with salted hash!\n\n");
    }

    // Login prompt
    for (int attempt = 1; attempt <= 3; attempt++) {
        console_print("Login required.\n");
        char user_input[32];
        char pass_input[64];

        console_print("Username: ");
        console_getline(user_input, sizeof(user_input));

        console_print("Password: ");
        console_getline(pass_input, sizeof(pass_input));

        // Verify username ("admin")
        int user_ok = (user_input[0] == 'a' && user_input[1] == 'd' && user_input[2] == 'm' && user_input[3] == 'i' && user_input[4] == 'n' && user_input[5] == '\0');

        // Verify password hash
        char computed_hash[32];
        hash_password(pass_input, stored_salt, computed_hash);

        int hash_ok = 1;
        for (int i = 0; i < 8; i++) {
            if (computed_hash[i] != stored_hash[i]) {
                hash_ok = 0;
                break;
            }
        }

        if (user_ok && hash_ok) {
            console_print("\n[Auth] Authentication successful. Welcome, administrator!\n");
            return 1;
        } else {
            console_print("\n[Auth] Authentication failed: invalid username or password.\n");
            console_print("Attempts remaining: ");
            console_putchar('0' + (3 - attempt));
            console_print("\n\n");
        }
    }

    console_print("[Auth] Maximum authentication attempts exceeded. System locked.\n");
    return 0;
}
