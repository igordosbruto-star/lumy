# lumy

Lumy — acenda sua imaginação.

Lumy é uma engine 2D em C++20 para RPGs top-down. Ela combina **SFML 3** para renderização, leitura de mapas **TMX** via **tmxlite** e scripts em **Lua 5.4** para eventos.

> Requer Windows + MSVC 2022, CMake ≥ 3.25 e vcpkg (manifest).

## Configuração e execução

Use os presets do `CMakePresets.json` para configurar e rodar o exemplo `hello-town`:

```sh
cmake --preset msvc-vcpkg
cmake --build build/msvc --config Debug
build/msvc/bin/Debug/hello-town.exe
```

No VS Code, selecione os mesmos presets e depure o alvo `hello-town` com `F5`.
