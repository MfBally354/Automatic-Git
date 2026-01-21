#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// ====== WARNA ======
#define BLUE    "\033[1;34m"
#define GREEN   "\033[1;32m"
#define PINK    "\033[1;35m"
#define DARK_RED "\033[1;31m"
#define NC      "\033[0m"

#define MAX_RETRY 3
#define BUFFER_SIZE 4096

void print_color(const char *text, const char *color) {
    printf("%s%s%s\n", color, text, NC);
}

int execute_command(const char *command, char *output, size_t output_size) {
    FILE *fp;
    char buffer[BUFFER_SIZE];
    int result = 0;
    
    // Clear output buffer
    if (output && output_size > 0) {
        output[0] = '\0';
    }
    
    // Execute command and capture output
    fp = popen(command, "r");
    if (fp == NULL) {
        perror("popen failed");
        return -1;
    }
    
    // Read output
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (output && output_size > 0) {
            strncat(output, buffer, output_size - strlen(output) - 1);
        }
        printf("%s", buffer);
    }
    
    // Get exit status
    result = pclose(fp);
    
    return WEXITSTATUS(result);
}

int main() {
    int count = 1;
    char output[BUFFER_SIZE];
    int exit_code;
    
    print_color("Menambahkan perubahan...", BLUE);
    exit_code = execute_command("git add .", output, sizeof(output));
    
    if (exit_code != 0) {
        print_color("Error saat git add", DARK_RED);
        exit(1);
    }
    
    print_color("Melakukan commit...", BLUE);
    exit_code = execute_command("git commit -m \"Auto commit\"", output, sizeof(output));
    
    if (exit_code != 0) {
        // Check if it's "nothing to commit" error
        if (strstr(output, "nothing to commit") != NULL || 
            strstr(output, "no changes added to commit") != NULL) {
            print_color("Tidak ada perubahan untuk di-commit", PINK);
        }
    }
    
    int push_success = 0;
    while (count <= MAX_RETRY && !push_success) {
        printf("%sPercobaan push ke-%d...%s\n", BLUE, count, NC);
        
        exit_code = execute_command("git push", output, sizeof(output));
        
        if (exit_code == 0) {
            print_color("✅ Push berhasil!", GREEN);
            push_success = 1;
        } else {
            print_color("❌ Push gagal, mencoba pull --rebase...", PINK);
            execute_command("git pull --rebase", output, sizeof(output));
        }
        
        count++;
    }
    
    if (!push_success) {
        printf("%s🚨 Push gagal setelah %d kali percobaan.%s\n", DARK_RED, MAX_RETRY, NC);
    }
    
    // Menampilkan status git
    printf("\n=== Status Git ===\n");
    execute_command("git status", output, sizeof(output));
    
    return 0;
}
