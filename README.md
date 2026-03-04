# Simulador-CBI

## Como ejecutar
Desde la carpeta "Simulador CBI"
1. Compilar:
g++ -std=c++17 -Wall -Wextra -pedantic -Iinclude main.cpp src/instruction.cpp src/parser.cpp src/memory.cpp src/memory_address.cpp src/register.cpp src/cpu.cpp src/simulator.cpp -o sim.exe

3. Ejecutar:
   .\sim.exe programs\NombreArchivo.txt
