import os, textwrap, zipfile, pathlib, json, re, shutil, datetime

base = pathlib.Path("/mnt/data/cbi_simulator_activity3")
if base.exists():
    shutil.rmtree(base)
base.mkdir(parents=True)

# File contents
agents = textwrap.dedent("""\
# AGENTS.md — Instrucciones para Codex (Extensión IDE)

## Objetivo
Construir un **simulador del ciclo básico de instrucción** (fetch → decode → execute) en **C++** usando **Programación Orientada a Objetos (POO)**, siguiendo `SPEC.md`.

## Reglas de trabajo (muy importantes)
1) **Lee `SPEC.md` completo antes de modificar código.**
2) Trabaja por incrementos pequeños (Parser → Memory/Registers → CPU → Instrucciones → Programas de prueba).
3) **Sin variables globales**. Todo encapsulado en clases.
4) **Cada registro de CPU es independiente** (instancias separadas de `Register`).
5) Manejo de errores: archivo no existe, instrucción inválida, operandos inválidos, dirección `Dn` inválida.
6) Mantén el proyecto compilable en todo momento.

## Estándar de código
- C++20 (o C++17 mínimo).
- Nombres claros: `Cpu`, `Memory`, `Register`, `Instruction`, `Parser`, `Simulator`.
- `#include` organizados.
- `-Wall -Wextra -pedantic` sin warnings (ideal).

## Cómo compilar y correr
Ver `README.md`.

## Definition of Done
- Compila y ejecuta.
- Corre **un programa a la vez** (archivo `.txt`).
- Implementa instrucciones: `SET, LDR, ADD, INC, DEC, STR, SHW, PAUSE, END`.
- Incluye 3 programas de ejemplo en `programs/` y reproduce la salida del ejemplo del enunciado (61).
""")

spec = textwrap.dedent("""\
# SPEC.md — Actividad 3: Simulador del Ciclo Básico de Instrucción (CBI)

Este documento aterriza el enunciado para implementar el simulador en C++ con POO.

## 1) Requisitos (del enunciado)
- El simulador admite el conjunto de instrucciones: `SET, LDR, ADD, INC, DEC, STR, SHW, PAUSE, END`.
- Debe leer **archivos de texto plano** con instrucciones ejecutables.
- Debe **cargar el programa en memoria**, y luego **obtener (fetch), decodificar (decode), ejecutar (execute)**.
- Solo se ejecuta **un programa a la vez**.
- Implementar la **memoria principal como un arreglo**.
- Implementar **cada registro de la CPU de manera independiente**.

(Referencia: enunciado PDF, páginas 1–2.) 

## 2) Formato del archivo de programa (entrada)
Cada línea contiene una instrucción con tokens separados por espacios. Se aceptan 3 o 4 operandos:

- Formato general recomendado (tolerante):
  - `OPCODE OP1 OP2 OP3 OP4`
  - Si faltan operandos, se completan como `NULL`.
  - `NULL` significa “no aplica”.

Se ignoran líneas vacías. (Opcional) Puedes permitir comentarios con `#` o `//` al inicio de la línea.

### Direcciones `Dn`
- Una dirección de memoria se escribe como `D` seguido de un entero positivo: `D1, D2, D3, ...`.
- El simulador mapea `Dn` a un índice dentro del arreglo de memoria.
- Recomendación: memoria de tamaño 256 y mapear `D1..D255` a índices `1..255` (se deja `0` sin usar).  
  (También es válido mapear `D1→0`, siempre que sea consistente y documentado.)

### Valores inmediatos (X)
Para `SET`, el valor `X` puede ser:
- Un entero (ej. `12`, `-5`).
- (Opcional) Un `Dn` si quieres soportar “direct value”: `SET D1 D2 ...` para copiar el valor de `D2` a `D1`.

## 3) Semántica de instrucciones
La memoria es un arreglo de celdas numéricas (se recomienda `long long`).

### SET — store in memory (sin afectar ACC)
`SET D1 X NULL NULL`
- Guarda `X` en la dirección `D1`.
- Nota del enunciado: al leer `SET`, el valor se almacena en memoria “sin ejecución del procesador”; en esta implementación, se modela como una instrucción que **solo escribe memoria** y **no altera ACC**.

### LDR — load to accumulator
`LDR D3 NULL NULL`
- Lee `MEM[D3]` y lo carga en **ACC**.

### ADD — addition (3 formas)
1) `ADD D1 NULL NULL`
   - `ACC = ACC + MEM[D1]`
2) `ADD D1 D3 NULL`
   - `ACC = MEM[D1] + MEM[D3]`
3) `ADD D1 D3 D4`
   - `ACC = MEM[D1] + MEM[D3]`
   - `MEM[D4] = ACC` (resultado también se guarda en D4)

### INC — increment memory
`INC D3 NULL NULL`
- `MEM[D3] = MEM[D3] + 1`

### DEC — decrement memory
`DEC D3 NULL NULL`
- **Supuesto razonable:** `MEM[D3] = MEM[D3] - 1`  
  (En el PDF aparece “adds 1” también para DEC, lo cual parece un typo; el nombre DECREMENT sugiere restar 1.)

### STR — store accumulator to memory
`STR D3 NULL NULL`
- `MEM[D3] = ACC`

### SHW — show values
- `SHW D2 ...` → imprime `MEM[D2]`
- `SHW ACC` → imprime `ACC`
- `SHW ICR` → imprime Instruction Counter Register (índice de la próxima instrucción)
- `SHW MAR` → imprime Memory Address Register (última dirección accedida)
- `SHW MDR` → imprime Memory Data Register (último dato transferido desde/hacia memoria)
- `SHW UC`  → imprime el estado textual de la Unidad de Control (ej. `FETCH/DECODE/EXECUTE/PAUSED/HALT`)

**Formato de salida recomendado:** imprimir solo el valor (o estado) en una línea.  
El enunciado muestra que el ejemplo imprime `61`.

### PAUSE — pause execution
`PAUSE NULL NULL NULL`
- Detiene el ciclo hasta que el usuario presione Enter.

### END — finish
`END ...`
- Termina la ejecución del programa.

## 4) Registros y ciclo fetch/decode/execute (modelo)
Registros mínimos:
- `ACC`: acumulador
- `ICR`: instruction counter (próxima instrucción)
- `MAR`: última dirección de memoria usada (para datos)
- `MDR`: último dato leído/escrito en memoria (para datos)
- `UC`: estado de la unidad de control

Ciclo:
1) UC = FETCH: leer instrucción `program[ICR]`, luego `ICR++`
2) UC = DECODE: validar opcode/operandos
3) UC = EXECUTE: ejecutar semántica y actualizar MAR/MDR cuando aplique
4) Si `PAUSE`: UC = PAUSED hasta Enter
5) Si `END`: UC = HALT y finalizar

## 5) Casos de error (mínimo)
- Archivo no existe / no se puede leer.
- `OPCODE` no reconocido.
- `Dn` inválido (sin número, <=0, o fuera de rango de memoria).
- Falta de operandos críticos (ej. `LDR` sin `Dn`).

## 6) Archivos de ejemplo (programs/)
Ver carpeta `programs/`.

## 7) Criterios de aceptación
- Compila en Windows (MinGW g++) o Linux.
- `programs/programa1.txt` produce salida final **61** al ejecutar `SHW D2`.
""")

readme = textwrap.dedent("""\
# Simulador CBI — Actividad 3 (C++ / POO)

Este proyecto implementa un simulador del ciclo básico de instrucción:
**fetch → decode → execute**, leyendo un archivo `.txt` con instrucciones.

Requisitos oficiales: ver `SPEC.md` (basado en el enunciado PDF).

---

## Estructura
- `include/` headers
- `src/` implementación
- `programs/` programas de prueba
- `docs/` material para Codex / prompts

---

## Compilar

### Opción A) g++ (recomendado)
En la raíz del proyecto:

**Linux/macOS**
```bash
g++ -std=c++20 -O2 -Wall -Wextra -pedantic -Iinclude src/*.cpp main.cpp -o sim