# ChronoGraph 3D

Visualizador matematico interactivo desarrollado en C++17 con WinAPI y GDI.  
Permite graficar funciones 2D y superficies 3D en tiempo real, incluyendo animacion por tiempo y modo anaglifo rojo/cian.

## Caracteristicas

- Graficado en 4 modos: cartesiano 2D, polar 2D, cartesiano 3D y polar 3D.
- Parser matematico integrado para evaluar expresiones personalizadas.
- 15 presets listos para explorar ondas, superficies y curvas.
- Interaccion en vivo: rotacion con raton, zoom con rueda, sliders de ejes y animacion temporal.
- Interfaz nativa Windows con controles estandar y renderizado en un lienzo dedicado.

## Modos de grafica

| Modo | Ecuacion |
|------|----------|
| Cartesiano 2D | `y = f(x, T)` |
| Polar 2D | `r = f(theta, T)` |
| Cartesiano 3D | `y = f(x, z, T)` |
| Polar 3D | `y = f(r, theta, T)` |

## Sintaxis soportada

- **Operadores:** `+ - * / ^`
- **Funciones:** `sin cos tan asin acos atan atan2 sqrt abs log log2 log10 exp floor ceil round sinh cosh tanh sign min max mod pow`
- **Constantes:** `pi`, `e`
- **Variables:** `x`, `y`, `z`, `T`

## Estructura del proyecto

```text
ChronoGraph3D/
├── CMakeLists.txt
├── ChronoGraph3D.manifest
├── ChronoGraph.h
├── main.cpp
├── WndProc.cpp
├── Renderer.cpp
└── Parser.cpp
```

## Requisitos

- Windows 10/11
- CMake 3.20 o superior
- CLion (o cualquier entorno con toolchain C++)
- Toolchain MSVC (recomendado) o MinGW-w64

## Compilacion y ejecucion

### Opcion 1: CLion (recomendada)

1. Abre la carpeta del proyecto en CLion.
2. Verifica el toolchain en `Settings > Build, Execution, Deployment > Toolchains`.
3. Compila con el perfil de CMake que prefieras.
4. Ejecuta `ChronoGraph3D`.

### Opcion 2: CMake desde terminal (MSVC)

```powershell
cmake -S . -B build
cmake --build build --config Debug
```

El ejecutable se genera en la carpeta de salida de CMake.

## Uso rapido

1. Escribe una formula en el campo superior.
2. Pulsa **Graficar** o la tecla **Enter**.
3. Cambia entre modos con los radio buttons.
4. Activa **Animar Tiempo (T)** para ver evolucion temporal.
5. En 3D, arrastra con clic izquierdo para rotar y usa la rueda para zoom.

## Arquitectura

- `main.cpp`: punto de entrada y creacion de la ventana principal.
- `WndProc.cpp`: manejo de mensajes WinAPI, controles UI y eventos.
- `Renderer.cpp`: pipeline de dibujo 2D/3D y proyeccion.
- `Parser.cpp`: parser recursivo y evaluador de expresiones.
- `ChronoGraph.h`: tipos compartidos, estado global e IDs de controles.

## Notas

- El proyecto usa `WIN32` (sin consola).
- El `manifest` habilita controles visuales modernos de Windows.
- Para subir a GitHub, se ignoran archivos de CLion y carpetas de build con `.gitignore`.
