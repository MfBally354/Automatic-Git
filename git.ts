#!/usr/bin/env ts-node

import { exec } from 'child_process';
import { promisify } from 'util';

const execAsync = promisify(exec);

// ====== WARNA ======
enum Color {
    BLUE = '\x1b[1;34m',
    GREEN = '\x1b[1;32m',
    PINK = '\x1b[1;35m',
    DARK_RED = '\x1b[1;31m',
    NC = '\x1b[0m'
}

interface CommandResult {
    success: boolean;
    output: string;
    error?: string;
}

function printColor(text: string, color: Color): void {
    console.log(`${color}${text}${Color.NC}`);
}

async function runCommand(command: string): Promise<CommandResult> {
    try {
        const { stdout, stderr } = await execAsync(command);
        return {
            success: true,
            output: stdout || stderr || '',
            error: stderr
        };
    } catch (error: any) {
        return {
            success: false,
            output: error.stderr || error.stdout || error.message,
            error: error.message
        };
    }
}

async function main(): Promise<void> {
    const MAX_RETRY = 3;
    let count = 1;
    
    try {
        printColor("Menambahkan perubahan...", Color.BLUE);
        const addResult = await runCommand("git add .");
        
        if (!addResult.success) {
            printColor(`Error saat git add: ${addResult.output}`, Color.DARK_RED);
            process.exit(1);
        }
        
        printColor("Melakukan commit...", Color.BLUE);
        const commitResult = await runCommand('git commit -m "Auto commit"');
        
        if (!commitResult.success) {
            if (commitResult.output.includes('nothing to commit')) {
                printColor("Tidak ada perubahan untuk di-commit", Color.PINK);
            }
        }
        
        let pushSuccess = false;
        while (count <= MAX_RETRY && !pushSuccess) {
            printColor(`Percobaan push ke-${count}...`, Color.BLUE);
            
            const pushResult = await runCommand("git push");
            
            if (pushResult.success) {
                printColor("✅ Push berhasil!", Color.GREEN);
                pushSuccess = true;
            } else {
                printColor("❌ Push gagal, mencoba pull --rebase...", Color.PINK);
                await runCommand("git pull --rebase");
            }
            
            count++;
        }
        
        if (!pushSuccess) {
            printColor(`🚨 Push gagal setelah ${MAX_RETRY} kali percobaan.`, Color.DARK_RED);
        }
        
        // Menampilkan status git
        console.log("\n=== Status Git ===");
        const statusResult = await runCommand("git status");
        console.log(statusResult.output);
        
    } catch (error: any) {
        console.error("Terjadi error:", error);
        process.exit(1);
    }
}

// Jalankan program
if (require.main === module) {
    main().catch(error => {
        console.error('Fatal error:', error);
        process.exit(1);
    });
}
