#include "vfs.h"
#include <arch/i686/vga_text.h>
#include <arch/i686/e9.h>

int VFS_Write(fd_t file, uint8_t* data, size_t size) {
    switch (file) {
        case VFS_FD_STDIN:
            return -1;
        
        case VFS_FD_STDOUT:
            VGA_putn(data, size);
            return size;

        case VFS_FD_STDERR:
            VGA_putn(data, size);
            e9_putn(data, size);
            return size;

        case VFS_FD_DEBUG:
            e9_putn(data, size);
            return size;
            
    default:
        return -1;
    }
}