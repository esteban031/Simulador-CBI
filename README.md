# Simulador-CBI

Simulador del Ciclo Basico de Instruccion (Fetch -> Decode -> Execute) en C++.

## Caracteristicas
- Lee programas desde archivos `.txt`, linea por linea.
- Soporta instrucciones: `SET`, `LDR`, `ADD`, `INC`, `DEC`, `STR`, `SHW`, `PAUSE`, `END`.
- Memoria principal implementada con `vector` y rango fijo `[0..39]`.
- Direcciones de memoria en formato `D0..D39` (tambien acepta `d0..d39`).
- Parser tolerante.
- Acepta cualquier cantidad de argumentos.
- Solo procesa los primeros 4 operandos.
- Si faltan operandos, completa con `NULL`.
- Opcodes case-insensitive (`pause`, `Pause`, `PAUSE`).
- Genera traza de ejecucion en `outputs/<programa>_trace.txt`.

## Requisitos
- g++ con soporte C++17 o superior.

## Compilacion
Desde la raiz del proyecto:

```bash
g++ -std=c++17 -Wall -Wextra -pedantic -Iinclude main.cpp src/instruction.cpp src/parser.cpp src/memory.cpp src/memory_address.cpp src/register.cpp src/cpu.cpp src/simulator.cpp -o sim.exe
```

O equivalente con wildcard:

```bash
g++ -std=c++17 -Wall -Wextra -pedantic -Iinclude main.cpp src/*.cpp -o sim.exe
```

## Ejecucion
```bash
./sim.exe programs/programa1.txt
```

Modo verbose:

```bash
./sim.exe programs/programa1.txt --verbose
```

Uso general:

```text
sim.exe <program_path> [--verbose|-v]
```

## Formato del archivo de programa
Cada linea tiene:

```text
OPCODE OP1 OP2 OP3 OP4 ...
```

Reglas:
- Se ignoran lineas vacias.
- Se ignoran lineas de comentario que inician con `#` o `//`.
- Solo se usan los primeros 4 operandos.
- Operandos faltantes se rellenan con `NULL`.

Ejemplo valido con argumentos extra:

```text
SET D1 10 NULL NULL EXTRA1 EXTRA2
```

## Instrucciones soportadas
- `SET Dn X ...` : escribe `X` en `MEM[Dn]`.
- `LDR Dn ...` : carga `MEM[Dn]` en `ACC`.
- `ADD Dn ...` : `ACC = ACC + MEM[Dn]`.
- `ADD Dn Dm ...` : `ACC = MEM[Dn] + MEM[Dm]`.
- `ADD Dn Dm Dk ...` : igual al anterior y guarda resultado en `MEM[Dk]`.
- `INC Dn ...` : incrementa `MEM[Dn]`.
- `DEC Dn ...` : decrementa `MEM[Dn]`.
- `STR Dn ...` : guarda `ACC` en `MEM[Dn]`.
- `SHW X ...` : muestra valor de `X`, donde `X` puede ser `Dn`, `ACC`, `ICR`, `MAR`, `MDR`, `UC`.
- `PAUSE ...` : pausa la ejecucion y abre menu interactivo.
- `END ...` : termina el programa.

## Menu PAUSE
Cuando se ejecuta `PAUSE`, se muestran opciones por terminal:

- `1` o `regs` : muestra `ACC`, `ICR`, `MAR`, `MDR`, `UC`.
- `2 Dn` o `mem Dn` : muestra `MEM[Dn]`.
- `3 Dstart Dend` o `range Dstart Dend` : muestra rango de memoria.
- `4` o `continue` : reanuda ejecucion.
- `5` o `help` : vuelve a mostrar menu.

## Programas de ejemplo
En la carpeta `programs/`:
- `programa1.txt`
- `programa2.txt`
- `programa3.txt`
- `programa4.txt`

## Salidas y trazas
- La salida de `SHW` se imprime en consola.
- Se crea un archivo de traza por ejecucion en `outputs/`.
- Ejemplo: `outputs/programa1_trace.txt`.

