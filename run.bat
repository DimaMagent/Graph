@echo off
chcp 65001 >nul

echo [+] Запуск приложения...
if exist out\build\x64-Debug\dep_visualizer.exe (
    out\build\x64-Debug\dep_visualizer.exe config.json
) else if exist out\build\x64-Debug\Release\dep_visualizer.exe (
    out\build\x64-Debug\Release\dep_visualizer.exe config.json
) else if exist build\Release\dep_visualizer.exe (
    build\Release\dep_visualizer.exe config.json
) else if exist build\Debug\dep_visualizer.exe (
    build\Debug\dep_visualizer.exe config.json
) else if exist build\dep_visualizer.exe (
    build\dep_visualizer.exe config.json
) else (
    echo [ОШИБКА]: Исполняемый файл dep_visualizer.exe не найден ни в одной из стандартных папок!
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
pause