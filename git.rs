use std::process::{Command, exit};
use std::io::{self, Write};

// ====== WARNA ======
const BLUE: &str = "\x1b[1;34m";
const GREEN: &str = "\x1b[1;32m";
const PINK: &str = "\x1b[1;35m";
const DARK_RED: &str = "\x1b[1;31m";
const NC: &str = "\x1b[0m";

fn print_color(text: &str, color: &str) {
    println!("{}{}{}", color, text, NC);
}

fn run_command(cmd: &str, args: &[&str]) -> Result<String, String> {
    let output = Command::new(cmd)
        .args(args)
        .output()
        .map_err(|e| format!("Failed to execute command: {}", e))?;
    
    let stdout = String::from_utf8_lossy(&output.stdout).to_string();
    let stderr = String::from_utf8_lossy(&output.stderr).to_string();
    
    if output.status.success() {
        Ok(stdout)
    } else {
        Err(stderr)
    }
}

fn run_shell_command(cmd: &str) -> Result<String, String> {
    if cfg!(target_os = "windows") {
        run_command("cmd", &["/C", cmd])
    } else {
        run_command("sh", &["-c", cmd])
    }
}

fn main() {
    const MAX_RETRY: u32 = 3;
    let mut count = 1;
    
    // Git Add
    print_color("Menambahkan perubahan...", BLUE);
    match run_shell_command("git add .") {
        Ok(_) => (),
        Err(e) => {
            print_color(&format!("Error saat git add: {}", e), DARK_RED);
            exit(1);
        }
    }
    
    // Git Commit
    print_color("Melakukan commit...", BLUE);
    match run_shell_command(r#"git commit -m "Auto commit""#) {
        Ok(_) => (),
        Err(e) => {
            if e.contains("nothing to commit") || e.contains("no changes added") {
                print_color("Tidak ada perubahan untuk di-commit", PINK);
            }
        }
    }
    
    // Git Push dengan retry
    let mut push_success = false;
    while count <= MAX_RETRY && !push_success {
        print_color(&format!("Percobaan push ke-{}...", count), BLUE);
        
        match run_shell_command("git push") {
            Ok(_) => {
                print_color("✅ Push berhasil!", GREEN);
                push_success = true;
            }
            Err(_) => {
                print_color("❌ Push gagal, mencoba pull --rebase...", PINK);
                let _ = run_shell_command("git pull --rebase");
            }
        }
        
        count += 1;
    }
    
    if !push_success {
        print_color(&format!("🚨 Push gagal setelah {} kali percobaan.", MAX_RETRY), DARK_RED);
    }
    
    // Git Status
    println!("\n=== Status Git ===");
    match run_shell_command("git status") {
        Ok(output) => println!("{}", output),
        Err(e) => eprintln!("Failed to get git status: {}", e),
    }
}
