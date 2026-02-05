@echo off
REM ===============================================
REM Auto Git Sync with Cached File Cleanup
REM ===============================================

:: --------- Set repository folder ---------
set REPO_DIR=C:\Users\2240705\Desktop\Winter-Project

:: --------- Change to repo directory ---------
cd /d "%REPO_DIR%" || (
    echo Failed to change directory to %REPO_DIR%
    pause
    exit /b 1
)

:: --------- Check Git version ---------
git --version >nul 2>&1 || (
    echo Git not found in PATH!
    pause
    exit /b 1
)

:: --------- Detect current branch ---------
for /f %%i in ('git rev-parse --abbrev-ref HEAD') do set BRANCH=%%i
if "%BRANCH%"=="" (
    echo Could not detect current branch.
    pause
    exit /b 1
)
echo Current branch: [%BRANCH%]

:: --------- Pull latest changes ---------
echo Pulling latest changes from remote...
git pull --no-edit origin "%BRANCH%" >nul 2>&1 || (
    echo Pull failed or nothing to merge
)
echo.

:: --------- Cached file cleanup ---------
:: Untracking files or folders that should be ignored based on .gitignore
echo Cleaning up cached files...
git rm -r --cached .vs >nul 2>&1
git rm --cached browse.VC.db >nul 2>&1
echo Cached file cleanup complete.
echo.

:: --------- Stage all changes ---------
echo Staging all changes...
git add . >nul 2>&1 || (
    echo Failed to add files
    pause
    exit /b 1
)
echo.

:: --------- Commit changes ---------
for /f "tokens=1-4 delims=/ " %%a in ("%DATE%") do set MYDATE=%%c-%%a-%%b
for /f "tokens=1-2 delims=:." %%a in ("%TIME%") do set MYTIME=%%a-%%b
set DATETIME=%MYDATE%_%MYTIME%
git commit -m "Auto commit %DATETIME%" >nul 2>&1 || (
    echo No changes to commit
)
echo.

:: --------- Push to remote ---------
echo Pushing changes to remote...
git push origin "%BRANCH%" >nul 2>&1 || (
    echo Push failed
)
echo.

echo Auto Git sync completed successfully!
pause
