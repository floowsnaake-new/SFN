#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

// Function to attach, write data, and detach
int write_memory(pid_t pid, unsigned long long address, void *data, size_t size) {
    // Attach to process
    if (ptrace(PTRACE_ATTACH, pid, NULL, NULL) == -1) {
        perror("ptrace attach");
        return -1;
    }

    // Wait for the process to stop
    waitpid(pid, NULL, 0);

    // Open /proc/<pid>/mem
    char mem_path[64];
    snprintf(mem_path, sizeof(mem_path), "/proc/%d/mem", pid);
    int mem_fd = open(mem_path, O_RDWR);
    if (mem_fd == -1) {
        perror("open mem");
        ptrace(PTRACE_DETACH, pid, NULL, NULL);
        return -1;
    }

    // Seek to target address
    if (lseek(mem_fd, address, SEEK_SET) == -1) {
        perror("lseek");
        close(mem_fd);
        ptrace(PTRACE_DETACH, pid, NULL, NULL);
        return -1;
    }

    // Write data
    size_t bytes_written = write(mem_fd, data, size);
    if (bytes_written != size) {
        perror("write");
        close(mem_fd);
        ptrace(PTRACE_DETACH, pid, NULL, NULL);
        return -1;
    }

    printf("Wrote %zd bytes to address 0x%llx in process %d\n", bytes_written, address, pid);

    close(mem_fd);

    // Detach from process
    if (ptrace(PTRACE_DETACH, pid, NULL, NULL) == -1) {
        perror("ptrace detach");
        return -1;
    }

    return 0;
}

int main() {
    pid_t target_pid = 14067;

    // Write the single byte 0x01 to address 0x5575815833A2
    unsigned long long addr1 = 0x5575815833A2ULL;
    unsigned char byte_value = 0x01; // single byte
    if (write_memory(target_pid, addr1, &byte_value, 1) != 0) {
        fprintf(stderr, "Failed to write 1 byte to address 0x%llx\n", addr1);
        return 1;
    }

    // Write the array of bytes to address 0x5575815885D8
    unsigned long long addr2 = 0x5575815885D8ULL;
    unsigned char bytes[] = {0xEB, 0xFF, 0xDF, 0x40, 0x00, 0x00, 0x80, 0x3F};
    size_t bytes_size = sizeof(bytes);

    if (write_memory(target_pid, addr2, bytes, bytes_size) != 0) {
        fprintf(stderr, "Failed to write bytes to address 0x%llx\n", addr2);
        return 1;
    }

    printf("Memory modifications completed successfully.\n");
    printf("Door should be Open!\n");
    return 0;
}
