import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

public class GitAutoPush {
    
    // ====== WARNA ======
    private static final String BLUE = "\033[1;34m";
    private static final String GREEN = "\033[1;32m";
    private static final String PINK = "\033[1;35m";
    private static final String DARK_RED = "\033[1;31m";
    private static final String NC = "\033[0m";
    
    private static void printColor(String text, String color) {
        System.out.println(color + text + NC);
    }
    
    private static CommandResult runCommand(String command) {
        try {
            Process process = Runtime.getRuntime().exec(command);
            
            // Membaca output
            BufferedReader reader = new BufferedReader(
                new InputStreamReader(process.getInputStream())
            );
            BufferedReader errorReader = new BufferedReader(
                new InputStreamReader(process.getErrorStream())
            );
            
            StringBuilder output = new StringBuilder();
            StringBuilder errorOutput = new StringBuilder();
            
            String line;
            while ((line = reader.readLine()) != null) {
                output.append(line).append("\n");
            }
            
            while ((line = errorReader.readLine()) != null) {
                errorOutput.append(line).append("\n");
            }
            
            int exitCode = process.waitFor();
            
            return new CommandResult(
                exitCode == 0,
                output.toString(),
                errorOutput.toString()
            );
            
        } catch (IOException | InterruptedException e) {
            return new CommandResult(
                false,
                "",
                e.getMessage()
            );
        }
    }
    
    static class CommandResult {
        boolean success;
        String output;
        String error;
        
        CommandResult(boolean success, String output, String error) {
            this.success = success;
            this.output = output;
            this.error = error;
        }
    }
    
    public static void main(String[] args) {
        final int MAX_RETRY = 3;
        int count = 1;
        
        printColor("Menambahkan perubahan...", BLUE);
        CommandResult addResult = runCommand("git add .");
        
        if (!addResult.success) {
            printColor("Error saat git add: " + addResult.error, DARK_RED);
            System.exit(1);
        }
        
        printColor("Melakukan commit...", BLUE);
        CommandResult commitResult = runCommand("git commit -m \"Auto commit\"");
        
        if (!commitResult.success) {
            String fullOutput = commitResult.output + commitResult.error;
            if (fullOutput.contains("nothing to commit") || 
                fullOutput.contains("no changes added to commit")) {
                printColor("Tidak ada perubahan untuk di-commit", PINK);
            }
        }
        
        boolean pushSuccess = false;
        while (count <= MAX_RETRY && !pushSuccess) {
            printColor("Percobaan push ke-" + count + "...", BLUE);
            
            CommandResult pushResult = runCommand("git push");
            
            if (pushResult.success) {
                printColor("✅ Push berhasil!", GREEN);
                pushSuccess = true;
            } else {
                printColor("❌ Push gagal, mencoba pull --rebase...", PINK);
                runCommand("git pull --rebase");
            }
            
            count++;
        }
        
        if (!pushSuccess) {
            printColor("🚨 Push gagal setelah " + MAX_RETRY + " kali percobaan.", DARK_RED);
        }
        
        // Menampilkan status git
        System.out.println("\n=== Status Git ===");
        CommandResult statusResult = runCommand("git status");
        if (statusResult.success) {
            System.out.println(statusResult.output);
        }
    }
}
