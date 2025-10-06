$program = ".\os.exe"
$logFile = ".\log.txt"

# Clear previous log
Remove-Item $logFile -ErrorAction SilentlyContinue

$successCount = 0
$failureCount = 0

for ($i = 1; $i -le 100; $i++) {
    Write-Host "Running iteration $i..."

    # Header
    Add-Content $logFile "`n=============================="
    Add-Content $logFile "Iteration $i - $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')"
    Add-Content $logFile "==============================`n"

    # Capture all output (stdout + stderr)
    $output = & $program 2>&1

    # Append the captured output
    Add-Content $logFile $output

    # Check exit code
    if ($LASTEXITCODE -eq 0) {
        $successCount++
        Add-Content $logFile "`nResult: SUCCESS`n"
    } else {
        $failureCount++
        Add-Content $logFile "`nResult: FAILURE (exit code: $LASTEXITCODE)`n"
    }
}

Write-Host "`n=== Summary ==="
Write-Host "Total Successes: $successCount"
Write-Host "Total Failures : $failureCount"

# Append summary to log
Add-Content $logFile "`n=== Summary ==="
Add-Content $logFile "Total Successes: $successCount"
Add-Content $logFile "Total Failures : $failureCount"
