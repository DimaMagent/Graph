@echo off
chcp 65001 >nul
setlocal EnableDelayedExpansion

:: 1. Поиск исполняемого файла dep_visualizer.exe в стандартных путях сборки
set "EXE_PATH="
for %%p in (
    "out\build\x64-Debug\dep_visualizer.exe"
    "out\build\x64-Release\dep_visualizer.exe"
    "out\build\x64-Debug\Release\dep_visualizer.exe"
    "out\build\x64-Debug\Debug\dep_visualizer.exe"
    "build\Release\dep_visualizer.exe"
    "build\Debug\dep_visualizer.exe"
    "build\dep_visualizer.exe"
) do (
    if exist %%p (
        set "EXE_PATH=%%~p"
        goto found_exe
    )
)

:found_exe
if "!EXE_PATH!"=="" (
    echo [ОШИБКА]: Исполняемый файл dep_visualizer.exe не найден ни в одной из папок сборки!
    echo Убедитесь, что проект успешно скомпилирован.
    goto error
)

echo [+] Найден исполняемый файл: !EXE_PATH!

:: 2. Обработка аргументов командной строки
set "INPUT_ARG=%~1"

if not "!INPUT_ARG!"=="" (
    set "CONFIG_FILE=!INPUT_ARG!"
    if not exist "!CONFIG_FILE!" if exist "tests\!INPUT_ARG!" set "CONFIG_FILE=tests\!INPUT_ARG!"
    if not exist "!CONFIG_FILE!" if exist "config_!INPUT_ARG!.json" set "CONFIG_FILE=config_!INPUT_ARG!.json"
    
    if not exist "!CONFIG_FILE!" (
        echo [ОШИБКА]: Указанный конфигурационный файл "!INPUT_ARG!" не найден!
        goto error
    )
    
    echo [+] Запуск с конфигурацией: !CONFIG_FILE!
    echo ---------------------------------------------------------------
    "!EXE_PATH!" "!CONFIG_FILE!"
    goto end
)

:: 3. Если аргумент не передан: ищем и запускаем 3 конфига (в корне или в tests\)
echo [+] Аргумент не передан. Запуск демонстрации для 3 конфигураций...

for %%c in (config_linear.json config_cyclic.json config_diamond.json) do (
    set "TARGET_CFG="
    if exist "%%c" (
        set "TARGET_CFG=%%c"
    ) else if exist "tests\%%c" (
        set "TARGET_CFG=tests\%%c"
    )

    if not "!TARGET_CFG!"=="" (
        echo.
        echo ===============================================================
        echo [+] Демонстрация конфигурации: !TARGET_CFG!
        echo ===============================================================
        "!EXE_PATH!" "!TARGET_CFG!"
    ) else (
        echo [ПРЕДУПРЕЖДЕНИЕ]: Файл %%c не найден ни в корне, ни в папке tests\
    )
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