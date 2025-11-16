@echo off
REM Build with MSVC x64, C++17, dynamic runtime for SimpleBLE
cl /std:c++17 /EHsc /MD /I server\include server\%1.cpp server\lib\simpleble.lib /Fe:%1.exe