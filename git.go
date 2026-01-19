package main

import (
	"fmt"
	"os"
	"os/exec"
)

// ====== WARNA ======
type Color string

const (
	Blue    Color = "\033[1;34m"
	Green   Color = "\033[1;32m"
	Pink    Color = "\033[1;35m"
	DarkRed Color = "\033[1;31m"
	Reset   Color = "\033[0m"
)

// ====== Git Commander ======
type GitCommander struct{}

func (g *GitCommander) Execute(args ...string) (bool, string) {
	cmd := exec.Command("git", args...)
	output, err := cmd.CombinedOutput()
	return err == nil, string(output)
}

func (g *GitCommander) ExecuteShell(cmd string) (bool, string) {
	execCmd := exec.Command("sh", "-c", cmd)
	output, err := execCmd.CombinedOutput()
	return err == nil, string(output)
}

// ====== Printer ======
type Printer struct{}

func (p *Printer) PrintColored(text string, color Color) {
	fmt.Printf("%s%s%s\n", color, text, Reset)
}

func (p *Printer) PrintError(text string, err string) {
	fmt.Printf("%s%s: %s%s\n", DarkRed, text, err, Reset)
}

// ====== Main Application ======
type AutoPushApp struct {
	git     *GitCommander
	printer *Printer
}

func NewAutoPushApp() *AutoPushApp {
	return &AutoPushApp{
		git:     &GitCommander{},
		printer: &Printer{},
	}
}

func (app *AutoPushApp) Run() {
	const MAX_RETRY = 3
	
	// Stage changes
	app.printer.PrintColored("Menambahkan perubahan...", Blue)
	if success, output := app.git.ExecuteShell("git add ."); !success {
		app.printer.PrintError("Error saat git add", output)
		os.Exit(1)
	}
	
	// Commit changes
	app.printer.PrintColored("Melakukan commit...", Blue)
	if success, output := app.git.Execute("commit", "-m", "Auto commit"); !success {
		app.printer.PrintColored("Tidak ada perubahan untuk di-commit", Pink)
	}
	
	// Push with retry
	count := 1
	for count <= MAX_RETRY {
		app.printer.PrintColored(fmt.Sprintf("Percobaan push ke-%d...", count), Blue)
		
		if success, _ := app.git.Execute("push"); success {
			app.printer.PrintColored("✅ Push berhasil!", Green)
			break
		} else {
			app.printer.PrintColored("❌ Push gagal, mencoba pull --rebase...", Pink)
			app.git.Execute("pull", "--rebase")
		}
		count++
	}
	
	if count > MAX_RETRY {
		app.printer.PrintColored(fmt.Sprintf("🚨 Push gagal setelah %d kali percobaan.", MAX_RETRY), DarkRed)
	}
	
	// Show git status
	fmt.Println("\n=== Status Git ===")
	app.git.ExecuteShell("git status")
}

func main() {
	app := NewAutoPushApp()
	app.Run()
}
