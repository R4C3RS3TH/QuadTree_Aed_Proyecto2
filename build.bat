@echo off
set "SFML_INCLUDE=.\SFML\include"
set "SFML_LIB=.\SFML\lib"

echo Compilando QuadTreeSim...
g++ -std=c++17 -O2 -Wall -Wextra -Iinclude -I"%SFML_INCLUDE%" -L"%SFML_LIB%" -o quadtree_sim.exe src/main.cpp src/QuadTree.cpp src/Simulation.cpp src/App.cpp -lsfml-graphics -lsfml-window -lsfml-system
if %errorlevel% neq 0 (
    echo Error de compilacion! 
    exit /b %errorlevel%
)
echo Compilacion exitosa. Para ejecutar, escriba: ./quadtree_sim.exe
