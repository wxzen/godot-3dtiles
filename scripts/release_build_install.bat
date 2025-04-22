@echo off  
echo Building project in Release configuration...  
cmake --build ./build_release --config Release  
  
if %ERRORLEVEL% neq 0 (  
    echo Failed to build the project.  
    goto :EOF  
)  
  
echo Installing project...  
cmake --install ./build_release --config Release
  
if %ERRORLEVEL% neq 0 (  
    echo Failed to install the project.  
) else (  
    echo Installation successful.  
)