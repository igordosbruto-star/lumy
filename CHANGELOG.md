# CHANGELOG — Lumy

Todas as mudanças relevantes deste projeto serão documentadas aqui.
Segue SemVer: MAJOR.MINOR.PATCH (ex.: 0.2.1).

## [Unreleased]
### Added
- `CMakePresets.json` com preset **msvc-vcpkg** (Debug/Release) e`VCPKG_TARGET_TRIPLET=x64-windows`.
- `src/main.cpp` (demo SFML 3) com janela, loop de eventos e animação.
- `.vscode/launch.json` (Run/Debug pelo F5 usando CMake Tools).
- `.vscode/settings.json` (usar presets, build antes do Run).
- `.gitignore` para CMake/VSCode/VS/vcpkg.
- Regra de **cópia de assets** do `examples/hello-town/` para `bin/<Config>/game/`.
- Mapa `first.tmx` e tileset simples em `examples/hello-town`.
- Teste básico de inicialização com GoogleTest.

### Changed
- `CMakeLists.txt`: alvo **hello-town**; ajustes para **SFML 3** (componentes em maiúsculo e targets `SFML::`).
- Integração **tmxlite** via `pkg-config` (`PkgConfig::TMXLITE`).
- Integração **Lua** via `find_package(Lua)` (include/libs do FindLua nativo).
- Saída dos binários para `build/bin/<Config>`.
- `src/main.cpp`: movimento do “herói” controlado por teclas **W/A/S/D**.
- `src/main.cpp`: logs de dimensões e camadas após carregar TMX.
- `src/main.cpp`: mede tempo entre frames com `sf::Clock`.
- `src/main.cpp`: armazena `elapsed` e `deltaTime` no início de cada frame.
- `src/main.cpp`: uso de `sf::Event` no loop e clique do mouse posiciona o herói.

### Fixed
- Baseline do vcpkg: `"HEAD"` → SHA real (corrige “builtin-baseline inválido”).
- Erro “Unsupported SFML component: system” (mudança para sintaxe do SFML 3).
- Erro ao configurar por falta de `src/main.cpp`.
### Docs
- Adicionado `VISION.md`.
- Adicionado `ROADMAP.md`.
- Adicionado `CONTRIBUTING.md`.
- Adicionado templates.
- `README.md`: descrição da engine e passos básicos com `CMakePresets.json`.
- `ROADMAP.md`: histórico atualizado após revisão do README.



## [0.1.0] — 2025-08-21
### Added
- Estrutura inicial do repositório **Lumy** (CMake, presets e setup de dependências via vcpkg).
- (exemplo) SceneStack (Boot → Title → Map).
- (exemplo) Sistema de Eventos com 10 comandos básicos.
- (exemplo) Save/Load (JSON).
