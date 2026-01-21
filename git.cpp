#include <iostream>
#include <string>
#include <cstdlib>
#include <memory>
#include <array>
#include <stdexcept>

// ====== WARNA ======
namespace Color {
    constexpr const char* BLUE = "\033[1;34m";
    constexpr const char* GREEN = "\033[1;32m";
    constexpr const char* PINK = "\033[1;35m";
    constexpr const char* DARK_RED = "\033[1;31m";
    constexpr const char* NC = "\033[0m";
}

class GitAutoPusher {
private:
    static constexpr int MAX_RETRY = 3;
    
    // Execute command and return output
    static std::string exec(const std::string& cmd) {
        std::array<char, 128> buffer;
        std::string result;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
        
        if (!pipe) {
            throw std::runtime_error("popen() failed!");
        }
        
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        
        return result;
    }
    
    // Execute command and return exit code
    static int execute(const std::string& cmd) {
        return std::system(cmd.c_str());
    }
    
    static void printColored(const std::string& text, const char* color) {
        std::cout << color << text << Color::NC << std::endl;
    }
    
public:
    void run() {
        int count = 1;
        bool pushSuccess = false;
        
        // Git Add
        printColored("Menambahkan perubahan...", Color::BLUE);
        if (execute("git add .") != 0) {
            printColored("Error saat git add", Color::DARK_RED);
            std::exit(1);
        }
        
        // Git Commit
        printColored("Melakukan commit...", Color::BLUE);
        std::string commitOutput = exec("git commit -m \"Auto commit\" 2>&1");
        
        if (commitOutput.find("nothing to commit") != std::string::npos ||
            commitOutput.find("no changes added") != std::string::npos) {
            printColored("Tidak ada perubahan untuk di-commit", Color::PINK);
        }
        
        // Git Push with retry
        while (count <= MAX_RETRY && !pushSuccess) {
            std::cout << Color::BLUE << "Percobaan push ke-" << count << "..." 
                      << Color::NC << std::endl;
            
            if (execute("git push") == 0) {
                printColored("✅ Push berhasil!", Color::GREEN);
                pushSuccess = true;
            } else {
                printColored("❌ Push gagal, mencoba pull --rebase...", Color::PINK);
                execute("git pull --rebase");
            }
            
            count++;
        }
        
        if (!pushSuccess) {
            printColored("🚨 Push gagal setelah " + std::to_string(MAX_RETRY) + 
                         " kali percobaan.", Color::DARK_RED);
        }
        
        // Git Status
        std::cout << "\n=== Status Git ===" << std::endl;
        std::cout << exec("git status") << std::endl;
    }
};

int main() {
    GitAutoPusher pusher;
    pusher.run();
    return 0;
}
