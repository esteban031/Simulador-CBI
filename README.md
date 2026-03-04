# Simulador-CBI

## Como ejecutar
Desde la carpeta "Simulador CBI"
1. Compilar:
  ** g++ -std=c++17 -Wall -Wextra -pedantic -Iinclude main.cpp src/instruction.cpp src/parser.cpp src/memory.cpp src/register.cpp src/cpu.cpp src/simulator.cpp -o sim.exe **
2. Ejecutar:
   .\sim.exe programs\NombreArchivo.txt
