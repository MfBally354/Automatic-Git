#!/usr/bin/env node

const { exec } = require('child_process');
const util = require('util');

// Promisify exec untuk async/await
const execAsync = util.promisify(exec);

// ====== WARNA ======
const Colors = {
    BLUE: '\x1b[1;34m',      // BIRU
    GREEN: '\x1b[1;32m',     // HIJAU
    PINK: '\x1b[1;35m',      // MERAH MUDA
    DARK_RED: '\x1b[1;31m',  // MERAH GELAP
    NC: '\x1b[0m'            // TANPA WARNA
};

function printColor(text, color) {
    console.log(`${color}${text}${Colors.NC}`);
}

async function runCommand(command) {
    try {
        const { stdout, stderr } = await execAsync(command);
        return { success: true, output: stdout || stderr };
    } catch (error) {
        return { 
            success: false, 
            output: error.stderr || error.message 
        };
    }
}

async function main() {
    const MAX_RETRY = 3;
    let count = 1;
    
    try {
        printColor("Menambahkan perubahan...", Colors.BLUE);
        const addResult = await runCommand("git add .");
        
        if (!addResult.success) {
            printColor(`Error saat git add: ${addResult.output}`, Colors.DARK_RED);
            process.exit(1);
        }
        
        printColor("Melakukan commit...", Colors.BLUE);
        const commitResult = await runCommand('git commit -m "Auto commit"');
        
        if (!commitResult.success) {
            if (commitResult.output.includes('nothing to commit')) {
                printColor("Tidak ada perubahan untuk di-commit", Colors.PINK);
            }
        }
        
        while (count <= MAX_RETRY) {
            printColor(`Percobaan push ke-${count}...`, Colors.BLUE);
            
            const pushResult = await runCommand("git push");
            
            if (pushResult.success) {
                printColor("✅ Push berhasil!", Colors.GREEN);
                break;
            } else {
                printColor("❌ Push gagal, mencoba pull --rebase...", Colors.PINK);
                await runCommand("git pull --rebase");
            }
            
            count++;
        }
        
        if (count > MAX_RETRY) {
            printColor(`🚨 Push gagal setelah ${MAX_RETRY} kali percobaan.`, Colors.DARK_RED);
        }
        
        // Menampilkan status git
        console.log("\n=== Status Git ===");
        await runCommand("git status");
        
    } catch (error) {
        console.error("Terjadi error:", error);
        process.exit(1);
    }
}

main();
