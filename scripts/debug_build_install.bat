@echo off  
echo Building project in Debug configuration...  
cmake --build ./build --config Debug  
  
if %ERRORLEVEL% neq 0 (  
    echo Failed to build the project.  
    goto :EOF  
)  
  
echo Installing project...  
cmake --install ./build --config Debug
  
if %ERRORLEVEL% neq 0 (  
    echo Failed to install the project.  
) else (  
    echo Installation successful.  
)