@echo off
chcp 65001 >nul
setlocal EnableDelayedExpansion

set "INPUT_ARG=%~1"
set "CONFIG_FILE=config.json"

:: Случай 1: Пользователь передал аргумент (имя файла или путь)
if not "!INPUT_ARG!"=="" (
    if exist "!INPUT_ARG!" (
        set "TARGET_FILE=!INPUT_ARG!"
    ) else if exist "tests\!INPUT_ARG!" (
        set "TARGET_FILE=tests\!INPUT_ARG!"
    ) else (
        echo [ОШИБКА]: Указанный файл "!INPUT_ARG!" не найден ни в текущей директории, ни в папке tests\!
        goto error
    )
    
    set "TARGET_FILE_JSON=!TARGET_FILE:\=/%"
    
    findstr /C:"repository_mode" "!TARGET_FILE!" >nul
    if !errorlevel! equ 0 (
        set "CONFIG_FILE=!TARGET_FILE!"
        echo [+] Используется переданный файл конфигурации: !CONFIG_FILE!
    ) else (
        echo [+] Обнаружен тестовый файл репозитория: !TARGET_FILE!
        echo [+] Создание временной конфигурации для тестирования...
        
        echo { > temp_auto_config.json
        echo   "package_name": "A", >> temp_auto_config.json
        echo   "repository_source": "!TARGET_FILE_JSON!", >> temp_auto_config.json
        echo   "repository_mode": "test_file", >> temp_auto_config.json
        echo   "ascii_tree_mode": true >> temp_auto_config.json
        echo } >> temp_auto_config.json
        
        set "CONFIG_FILE=temp_auto_config.json"
    )
) else (
    :: Случай 2: Аргумент не передан, используется config.json по умолчанию
    if not exist "config.json" (
        echo [ОШИБКА]: Файл конфигурации по умолчанию "config.json" не найден!
        goto error
    )
    echo [+] Используется конфигурация по умолчанию: config.json

    :: Автоматически проверяем, не лежит ли файл из config.json в папке tests\
    for %%f in (test_repo_cyclic.json test_repo_linear.json test_repo_branch.json) do (
        findstr /C:"%%f" "config.json" >nul
        if !errorlevel! equ 0 (
            if not exist "%%f" if exist "tests\%%f" (
                echo [+] Автоматически определено: файл "%%f" найден в папке tests\
                echo [+] Создание временной конфигурации...
                
                echo { > temp_auto_config.json
                echo   "package_name": "A", >> temp_auto_config.json
                echo   "repository_source": "tests/%%f", >> temp_auto_config.json
                echo   "repository_mode": "test_file", >> temp_auto_config.json
                echo   "ascii_tree_mode": true >> temp_auto_config.json
                echo } >> temp_auto_config.json
                
                set "CONFIG_FILE=temp_auto_config.json"
            )
        )
    )
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

if "!EXE_PATH!"=="" (
    echo [ОШИБКА]: Исполняемый файл dep_visualizer.exe не найден ни в одной из стандартных папок!
    echo Убедитесь, что проект успешно собран.
    goto error
)

echo [+] Найден исполняемый файл: !EXE_PATH!
echo [+] Используется конфигурация: !CONFIG_FILE!
echo ---------------------------------------------------------------
echo [+] Запуск приложения...
echo.

"!EXE_PATH!" "!CONFIG_FILE!"
set "EXIT_CODE=!errorlevel!"

if exist "temp_auto_config.json" del "temp_auto_config.json"

if !EXIT_CODE! neq 0 (
    echo.
    echo [ОШИБКА]: Приложение завершилось с кодом !EXIT_CODE!.
    goto error
)

goto end

:error
if exist "temp_auto_config.json" del "temp_auto_config.json"
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