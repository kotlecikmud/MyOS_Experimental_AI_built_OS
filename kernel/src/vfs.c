#include "vfs.h"
#include "console.h"

typedef struct {
    char name[32];
    vfs_node_type_t type;
    int parent_index; // index of parent directory (-1 for root)
} vfs_node_t;

static vfs_node_t nodes[VFS_MAX_NODES];
static int node_count = 0;
static int current_dir_index = 0; // Root is index 0

static int find_child(int parent_idx, const char* name) {
    for (int i = 0; i < node_count; i++) {
        if (nodes[i].parent_index == parent_idx) {
            // Compare name
            const char* a = nodes[i].name;
            const char* b = name;
            int match = 1;
            while (*a && *b) {
                if (*a != *b) {
                    match = 0;
                    break;
                }
                a++;
                b++;
            }
            if (match && *a == '\0' && *b == '\0') {
                return i;
            }
        }
    }
    return -1;
}

static int add_node(const char* name, vfs_node_type_t type, int parent_idx) {
    if (node_count >= VFS_MAX_NODES) return -1;
    int idx = node_count++;
    int i = 0;
    while (name[i] && i < 31) {
        nodes[idx].name[i] = name[i];
        i++;
    }
    nodes[idx].name[i] = '\0';
    nodes[idx].type = type;
    nodes[idx].parent_index = parent_idx;
    return idx;
}

int vfs_find_node_by_path(const char* path) {
    if (path[0] == '/' && path[1] == '\0') return 0;
    
    // Simple implementation: only supports root-level files for now
    if (path[0] == '/') {
        return find_child(0, path + 1);
    }
    return find_child(0, path);
}

void vfs_init(void) {
    node_count = 0;
    // Create root "/"
    current_dir_index = add_node("/", VFS_NODE_DIR, -1);
    
    // Create standard directories and files
    int home = add_node("home", VFS_NODE_DIR, 0);
    int admin = add_node("admin", VFS_NODE_DIR, home);
    add_node("readme.txt", VFS_NODE_FILE, admin);
    
    int etc = add_node("etc", VFS_NODE_DIR, 0);
    add_node("motd", VFS_NODE_FILE, etc);
    
    int bin = add_node("bin", VFS_NODE_DIR, 0);
    add_node("shell", VFS_NODE_FILE, bin);
    
    add_node("tmp", VFS_NODE_DIR, 0);
    
    console_print("[VFS] Virtual Filesystem initialized with default hierarchy.\n");
}

const char* vfs_get_current_path(void) {
    static char path_buf[VFS_MAX_PATH];
    if (current_dir_index == 0) {
        path_buf[0] = '/';
        path_buf[1] = '\0';
        return path_buf;
    }
    
    // Traverse upwards to build path
    int indices[16];
    int depth = 0;
    int curr = current_dir_index;
    while (curr > 0 && depth < 16) {
        indices[depth++] = curr;
        curr = nodes[curr].parent_index;
    }
    
    int pos = 0;
    path_buf[pos++] = '/';
    for (int i = depth - 1; i >= 0; i--) {
        const char* name = nodes[indices[i]].name;
        int j = 0;
        while (name[j] && pos < VFS_MAX_PATH - 2) {
            path_buf[pos++] = name[j++];
        }
        if (i > 0) {
            path_buf[pos++] = '/';
        }
    }
    path_buf[pos] = '\0';
    return path_buf;
}

void vfs_list_dir(void) {
    console_print("Contents of ");
    console_print(vfs_get_current_path());
    console_print(":\n");
    
    int found = 0;
    for (int i = 0; i < node_count; i++) {
        if (nodes[i].parent_index == current_dir_index) {
            found = 1;
            if (nodes[i].type == VFS_NODE_DIR) {
                console_print("  [DIR]  ");
            } else {
                console_print("  [FILE] ");
            }
            console_print(nodes[i].name);
            console_print("\n");
        }
    }
    if (!found) {
        console_print("  (empty directory)\n");
    }
}

int vfs_change_dir(const char* path) {
    if (!path || path[0] == '\0') {
        return -1;
    }
    
    // Handle root "/"
    if (path[0] == '/' && path[1] == '\0') {
        current_dir_index = 0;
        return 0;
    }
    
    // Copy path for tokenization / parsing
    int target_idx = current_dir_index;
    if (path[0] == '/') {
        target_idx = 0; // Absolute path starts at root
    }
    
    // Simple segment parser
    const char* p = path;
    if (*p == '/') p++;
    
    while (*p) {
        char segment[32];
        int seg_len = 0;
        while (*p && *p != '/' && seg_len < 31) {
            segment[seg_len++] = *p++;
        }
        segment[seg_len] = '\0';
        
        if (*p == '/') p++;
        
        if (segment[0] == '\0') {
            continue;
        }
        
        if (segment[0] == '.' && segment[1] == '\0') {
            // Current directory '.' -> do nothing
            continue;
        }
        
        if (segment[0] == '.' && segment[1] == '.' && segment[2] == '\0') {
            // Parent directory '..'
            if (target_idx > 0) {
                target_idx = nodes[target_idx].parent_index;
            }
            continue;
        }
        
        // Find child node
        int child = find_child(target_idx, segment);
        if (child == -1) {
            console_print("cd: no such file or directory: ");
            console_print(segment);
            console_print("\n");
            return -1;
        }
        
        if (nodes[child].type != VFS_NODE_DIR) {
            console_print("cd: not a directory: ");
            console_print(segment);
            console_print("\n");
            return -1;
        }
        
        target_idx = child;
    }
    
    current_dir_index = target_idx;
    return 0;
}
