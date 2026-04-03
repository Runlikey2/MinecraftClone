@echo off
setlocal

echo Downloading Required Dependenices

set ROOT=%~dp0..
set DEPS=%ROOT%\deps

if not exist "%DEPS%" mkdir "%DEPS%"

REM ---------------------------------------------------------------
REM 1. GLFW
REM ---------------------------------------------------------------
if exist "%DEPS%\glfw\CMakeLists.txt" (
    echo [1/5] GLFW already present - skipping.
) else (
    echo [1/5] Cloning GLFW 3.4 ...
    git clone --depth 1 --branch 3.4 https://github.com/glfw/glfw.git "%DEPS%\glfw"
    if not exist "%DEPS%\glfw\CMakeLists.txt" goto :fail_glfw
)
goto :step2

:fail_glfw
echo ERROR: GLFW clone failed.
pause
exit /b 1

REM ---------------------------------------------------------------
REM 2. GLM
REM ---------------------------------------------------------------
:step2
if exist "%DEPS%\glm\glm\glm.hpp" (
    echo [2/5] GLM already present - skipping.
) else (
    echo [2/5] Cloning GLM 1.0.1 ...
    git clone --depth 1 --branch 1.0.1 https://github.com/g-truc/glm.git "%DEPS%\glm"
    if not exist "%DEPS%\glm\glm\glm.hpp" goto :fail_glm
)
goto :step3

:fail_glm
echo ERROR: GLM clone failed.
pause
exit /b 1

REM ---------------------------------------------------------------
REM 3. GLAD  (OpenGL 4.6 Core loader)
REM ---------------------------------------------------------------
:step3
if exist "%DEPS%\glad\src\glad.c" (
    echo [3/5] GLAD already present - skipping.
    goto :step4
)

echo [3/5] Generating GLAD via Python ...
pip install glad >nul 2>&1
python -m glad --generator c --spec gl --out-path "%DEPS%\glad" --api gl=4.6 --profile core >nul 2>&1

if exist "%DEPS%\glad\src\glad.c" (
    echo       GLAD generated successfully.
    goto :step4
)

echo.
echo  *** GLAD auto-generation failed. ***
echo.
echo  Manual fix (takes 30 seconds):
echo    1. Go to  https://glad.dav1d.de/
echo    2. Set:   Language = C/C++
echo             Specification = OpenGL
echo             API gl = 4.6
echo             Profile = Core
echo    3. Click "Generate"
echo    4. Download the zip
echo    5. Extract so you have:
echo         deps\glad\include\glad\glad.h
echo         deps\glad\include\KHR\khrplatform.h
echo         deps\glad\src\glad.c
echo.
pause
exit /b 1

REM ---------------------------------------------------------------
REM 4. FastNoiseLite
REM ---------------------------------------------------------------
:step4
if exist "%DEPS%\FastNoiseLite.h" (
    echo [4/5] FastNoiseLite already present - skipping.
) else (
    echo [4/5] Downloading FastNoiseLite.h ...
    curl -sL -o "%DEPS%\FastNoiseLite.h" https://raw.githubusercontent.com/Auburn/FastNoiseLite/master/Cpp/FastNoiseLite.h
    if not exist "%DEPS%\FastNoiseLite.h" goto :fail_fnl
)
goto :step5

:fail_fnl
echo ERROR: FastNoiseLite download failed. Check that curl is in PATH.
pause
exit /b 1

REM ---------------------------------------------------------------
REM 5. stb_image
REM ---------------------------------------------------------------
:step5
if exist "%DEPS%\stb_image.h" (
    echo [5/5] stb_image already present - skipping.
) else (
    echo [5/5] Downloading stb_image.h ...
    curl -sL -o "%DEPS%\stb_image.h" https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
    if not exist "%DEPS%\stb_image.h" goto :fail_stb
)
goto :done

:fail_stb
echo ERROR: stb_image download failed. Check that curl is in PATH.
pause
exit /b 1

REM ---------------------------------------------------------------
:done
echo.
echo ============================================================
echo   All dependencies ready!
echo.
echo   Next steps:
echo     cd %ROOT%
echo     cmake -S . -B build -G "Visual Studio 18 2026" -A x64
echo     cmake --build build --config Release
echo ============================================================
pause
exit /b 0
