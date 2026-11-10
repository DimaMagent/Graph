@echo off
chcp 65001 >nul

echo [+] Создание директории сборки...
if not exist build mkdir build

echo [+] Конфигурация через CMake...
cmake -B build -S .
if %errorlevel% neq 0 goto error

echo [+] Компиляция проекта...
cmake --build build --config Release
if %errorlevel% neq 0 goto error

set "CONFIG_FILE=%~1"
if "%CONFIG_FILE%"=="" set "CONFIG_FILE=config.json"

:: Предварительная проверка существования конфигурационного файла
if not exist "%CONFIG_FILE%" (
    echo [ОШИБКА]: Файл конфигурации "%CONFIG_FILE%" не найден!
    goto error
)

echo [+] Поиск исполняемого файла dep_visualizer.exe...

set "EXE_PATH="

:: Проверка стандартных путей сборки CMake / Visual Studio / ручной сборки
if exist "out\build\x64-Debug\dep_visualizer.exe" (
    set "EXE_PATH=out\build\x64-Debug\dep_visualizer.exe"
) else if exist "out\build\x64-Release\dep_visualizer.exe" (
    set "EXE_PATH=out\build\x64-Release\dep_visualizer.exe"
) else if exist "out\build\x64-Debug\Release\dep_visualizer.exe" (
    set "EXE_PATH=out\build\x64-Debug\Release\dep_visualizer.exe"
) else if exist "out\build\x64-Debug\Debug\dep_visualizer.exe" (
    set "EXE_PATH=out\build\x64-Debug\Debug\dep_visualizer.exe"
) else if exist "build\Release\dep_visualizer.exe" (
    set "EXE_PATH=build\Release\dep_visualizer.exe"
) else if exist "build\Debug\dep_visualizer.exe" (
    set "EXE_PATH=build\Debug\dep_visualizer.exe"
) else if exist "build\dep_visualizer.exe" (
    set "EXE_PATH=build\dep_visualizer.exe"
)

:: Если бинарник не найден ни в одной из директорий
if "%EXE_PATH%"=="" (
    echo [ОШИБКА]: Исполняемый файл dep_visualizer.exe не найден ни в одной из стандартных папок!
    echo Убедитесь, что проект успешно собран.
    goto error
)

echo [+] Найден исполняемый файл: %EXE_PATH%
echo [+] Используется конфигурация: %CONFIG_FILE%
echo ---------------------------------------------------------------
echo [+] Запуск приложения...
echo.

:: Запуск программы с передачей файла конфигурации
"%EXE_PATH%" "%CONFIG_FILE%"

:: Сохраняем код возврата программы
set "EXIT_CODE=%errorlevel%"

:: Проверяем, завершилась ли программа с ошибкой
if %EXIT_CODE% neq 0 (
    echo.
    echo [ОШИБКА]: Приложение завершилось с кодом %EXIT_CODE%.
    goto error
)

goto end

:error
echo.
echo [ОШИБКА]: Процесс завершился с ошибкой.
pause
exit /b 1

:end
echo.
echo ---------------------------------------------------------------
echo [+] Работа приложения успешно завершена.
pause
exit /b 0