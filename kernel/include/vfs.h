#ifndef VFS_H
#ifndef VFS_H
#define VFS_H

#include <stdint.h>
#include <stddef.h>

#define VFS_MAX_PATH 256
#define VFS_MAX_NODES 32

typedef enum {
    VFS_NODE_DIR = 1,
    VFS_NODE_FILE = 2
} vfs_node_type_t;

void vfs_init(void);
int vfs_change_dir(const char* path);
void vfs_list_dir(void);
const char* vfs_get_current_path(void);
int vfs_find_node_by_path(const char* path);

#endif /* VFS_H */
#endif
