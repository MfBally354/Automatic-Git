#!/usr/bin/env swift

import Foundation

// ====== WARNA ======
enum Color: String {
    case blue = "\u{001B}[1;34m"
    case green = "\u{001B}[1;32m"
    case pink = "\u{001B}[1;35m"
    case red = "\u{001B}[1;31m"
    case reset = "\u{001B}[0m"
    
    var code: String { return self.rawValue }
}

func printColor(_ text: String, color: Color) {
    print("\(color.code)\(text)\(Color.reset.code)")
}

// ====== COMMAND EXECUTION ======
struct CommandResult {
    let success: Bool
    let output: String
    let error: String
}

func runCommand(_ command: String) -> CommandResult {
    let process = Process()
    let outputPipe = Pipe()
    let errorPipe = Pipe()
    
    process.executableURL = URL(fileURLWithPath: "/bin/bash")
    process.arguments = ["-c", command]
    process.standardOutput = outputPipe
    process.standardError = errorPipe
    
    do {
        try process.run()
        process.waitUntilExit()
        
        let outputData = outputPipe.fileHandleForReading.readDataToEndOfFile()
        let errorData = errorPipe.fileHandleForReading.readDataToEndOfFile()
        
        let output = String(data: outputData, encoding: .utf8) ?? ""
        let error = String(data: errorData, encoding: .utf8) ?? ""
        
        return CommandResult(
            success: process.terminationStatus == 0,
            output: output,
            error: error
        )
    } catch {
        return CommandResult(
            success: false,
            output: "",
            error: error.localizedDescription
        )
    }
}

func runGitCommand(_ args: String...) -> CommandResult {
    let command = "git " + args.joined(separator: " ")
    return runCommand(command)
}

// ====== MAIN PROGRAM ======
func main() {
    let MAX_RETRY = 3
    var count = 1
    
    // Git Add
    printColor("Menambahkan perubahan...", color: .blue)
    let addResult = runGitCommand("add", ".")
    
    if !addResult.success {
        printColor("Error saat git add: \(addResult.error)", color: .red)
        exit(1)
    }
    
    // Git Commit
    printColor("Melakukan commit...", color: .blue)
    let commitResult = runGitCommand("commit", "-m", "Auto commit")
    
    if !commitResult.success {
        let fullOutput = commitResult.output + commitResult.error
        if fullOutput.contains("nothing to commit") || 
           fullOutput.contains("no changes added") {
            printColor("Tidak ada perubahan untuk di-commit", color: .pink)
        }
    }
    
    // Git Push dengan retry
    var pushSuccess = false
    while count <= MAX_RETRY && !pushSuccess {
        printColor("Percobaan push ke-\(count)...", color: .blue)
        
        let pushResult = runGitCommand("push")
        
        if pushResult.success {
            printColor("✅ Push berhasil!", color: .green)
            pushSuccess = true
        } else {
            printColor("❌ Push gagal, mencoba pull --rebase...", color: .pink)
            _ = runGitCommand("pull", "--rebase")
        }
        
        count += 1
    }
    
    if !pushSuccess {
        printColor("🚨 Push gagal setelah \(MAX_RETRY) kali percobaan.", color: .red)
    }
    
    // Git Status
    print("\n=== Status Git ===")
    let statusResult = runGitCommand("status")
    print(statusResult.output)
}

// Run the program
main()
