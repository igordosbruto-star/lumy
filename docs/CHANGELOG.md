# CHANGELOG — Lumy

Todas as mudanças relevantes deste projeto serão documentadas aqui.
Segue SemVer: MAJOR.MINOR.PATCH (ex.: 0.2.1).

## [Unreleased]
### Added

### Changed

### Fixed

### Docs


## [0.1.1] — 2025-10-05 🎯
**M0 Fechamento - Consolidação do Marco Zero**

### Added
- **Sistema de Eventos completo**: 10 comandos básicos implementados (`ShowText`, `SetSwitch`, `SetVariable`, `Wait`, `ConditionalBranch`, `EndConditional`, `TransferPlayer`, `ChangeGold`, `PlaySound`, `ShowImage`).
- **Sistema de Save/Load**: salvamento em múltiplos slots (JSON) com dados de posição, switches, variáveis, party e inventário.
- **Esquemas JSON de dados**: estruturas para `actors.json`, `items.json`, `states.json`, `skills.json`, `enemies.json` e `system.json` em `game/data/`.
- **Eventos de exemplo**: NPCs demonstrando comandos básicos e condicionais em `hello-town`.
- **Atalhos de teclado para saves**: AltGr+número (salvar), Ctrl+AltGr+número (carregar), Shift+AltGr+número (deletar).
- **Movimento WASD**: controle do personagem com teclas W/A/S/D com limitação aos bounds do mapa.
- **UI de debug**: display de posição, controles e estado de switches/variáveis na tela.
- **Headers necessários**: `<algorithm>` e `<cmath>` para funções matemáticas.
- `CMakePresets.json` com preset **msvc-vcpkg** (Debug/Release) e`VCPKG_TARGET_TRIPLET=x64-windows`.
- `src/main.cpp` (demo SFML 3) com janela, loop de eventos e animação.
- `.vscode/launch.json` (Run/Debug pelo F5 usando CMake Tools).
- `.vscode/settings.json` (usar presets, build antes do Run).
- `.gitignore` para CMake/VSCode/VS/vcpkg.
- Regra de **cópia de assets** do `examples/hello-town/` para `bin/<Config>/game/`.
- Mapa `hello-town.tmx` e tileset simples em `examples/hello-town`.
- Teste básico de inicialização com GoogleTest.
- `src/scene.hpp` e `src/scene.cpp`: classe `Scene` com `update(deltaTime)`.
- `src/scene_stack.hpp` e `src/scene_stack.cpp`: pilha de cenas com push/pop/switch.
- Testes para `SceneStack`.
- `src/map_scene.hpp` e `src/map_scene.cpp`: classe `MapScene` com lógica do herói.
- `src/map.hpp`/`src/map.cpp`: classe `Map` para carregar e desenhar TMX.
- `src/boot_scene.hpp`/`src/boot_scene.cpp`: cena de boot que carrega recursos e vai para `TitleScene`.
- `src/title_scene.hpp`/`src/title_scene.cpp`: menu de título com opção Start que abre `MapScene`.
- Teste de fluxo de cenas Boot → Title → Map.
- Teste de fluxo de cenas via clique do mouse em Start.
- Teste para encerrar loop ao fechar a janela sem chamar `update`.
- `src/delta_time.hpp`/`src/delta_time.cpp`: função `clampDeltaTime` limita `deltaTime` e registra quedas de frame.
- Teste unitário garantindo limitação de `deltaTime` para `maxDeltaTime`.
- `tests/title_scene.cpp`: testa exceção ao faltar `game/font.ttf`.
- `game/font.ttf`: fonte padrão para `TitleScene`.
- `game/assets/maps/hello.tmx` e tileset correspondente.
- `DartConfiguration.tcl` com configuração básica para envios ao CDash.
- `src/texture_manager.hpp`/`src/texture_manager.cpp`: cache simples de texturas.
- Cache de texturas de tileset via `TextureManager`, evitando carregamentos duplicados.
- `src/map.hpp`/`src/map.cpp`: métodos `getTileID` e `setTileID` com atualização do `VertexArray`.
- `Map` (`src/map.hpp`/`src/map.cpp`): funções `drawLayer` e `drawRange` para desenhar camadas específicas.
- `map_loader.py`: função `load_hello_map()` para parsear `hello.tmx` com tratamento de erros.
- `src/map_scene.cpp`: posiciona o herói usando objeto `player`/`spawn` da camada de objetos do TMX com fallback.
- `src/map.cpp`/`src/map_scene.cpp`: suporte a tiles colidíveis com bloqueio de movimento.
- **Sistema de saves funcional**: arquivo `save1.json` com dados persistentes do jogo.
- **Mapa expandido**: `hello.tmx` atualizado com dimensões 25x15 e estrutura de bordas.

### Changed
- **`CMakeLists.txt`**: adicionados `event_system.cpp` e `save_system.cpp` ao build; dependência do Lua para integração com scripts.
- **`src/map_scene.hpp` e `src/map_scene.cpp`**: integração completa com `EventSystem` e `SaveSystem`; movimento com limitação de bounds; atalhos de teclado; UI de debug.
- **`vcpkg.json`**: removido `builtin-baseline` fixo para usar versão mais recente do vcpkg.
- **Compatibilidade SFML 3**: correções em `sf::Text` (sem construtor padrão), `sf::Font::openFromFile`, `setPosition` com `sf::Vector2`, e `TextureManager::acquire`.
- `CMakeLists.txt`: alvo **hello-town**; ajustes para **SFML 3** (componentes em maiúsculo e targets `SFML::`).
- Integração **tmxlite** via `pkg-config` (`PkgConfig::TMXLITE`).
- Integração **Lua** via `find_package(Lua)` (include/libs do FindLua nativo).
- Saída dos binários para `build/bin/<Config>`.
- `src/main.cpp`: movimento do “herói” controlado por teclas **W/A/S/D**.
- `src/main.cpp`: logs de dimensões e camadas após carregar TMX.
- `src/main.cpp`: mede tempo entre frames com `sf::Clock`.
- `src/main.cpp`: armazena `elapsed` e `deltaTime` no início de cada frame.
- `src/main.cpp`: uso de `sf::Event` no loop e clique do mouse posiciona o herói.
- `src/main.cpp`: chama `scene.update(deltaTime)` para atualizar o herói.
- `src/main.cpp`: limitador manual de 60 FPS com `sf::sleep` e log de FPS médio.
- `src/main.cpp`: limita `deltaTime` a 30 FPS e registra quedas de frame.
- `src/main.cpp`: encerra o loop quando a janela fecha e libera objetos alocados.
- `src/main.cpp`: loop de eventos usa `pollEvent()` opcional, checa `Closed` com `event->is` e lê tecla Escape com `getIf`, repassando para a cena.
- `src/main.cpp`: condiciona loop à janela aberta e verifica fechamento após eventos.
- `src/main.cpp`: usa `clampDeltaTime` para restringir o intervalo de atualização.
- `src/scene.cpp`: `handleEvent` usa ponteiro retornado por `getIf` para cliques do mouse.
- `src/scene.hpp`/`src/scene.cpp`: `Scene` agora é abstrata e delega lógica à `MapScene`.
- `src/main.cpp`: usa `SceneStack` para gerenciar cenas.
- `CMakeLists.txt`: compila `SceneStack` e adiciona testes.
- `src/scene.cpp`: correção de sf::Mouse::Left para sf::Mouse::Button::Left da SFML3.
- `src/main.cpp`: fluxo de cenas Boot → Title → Map via `SceneStack`.
- `src/main.cpp`: inicia `SceneStack` com `BootScene`.
- `src/title_scene.cpp`: usa `openFromFile`, inicializa `startText` no construtor e verifica cliques com `sf::Vector2f`.
- `CMakeLists.txt`: define diretório de trabalho dos testes e copia assets necessários.
- `src/title_scene.cpp`: verifica carregamento da fonte e lança exceção se falhar.
- `src/title_scene.cpp`: trata tecla Enter usando ponteiro retornado por `getIf`.
- Mapa de exemplo renomeado para `hello-town.tmx` e referências atualizadas.
- `CMakeLists.txt`: copia assets do diretório `game/` para `bin/<Config>/game`.
- `src/main.cpp`, `src/map_scene.hpp` e `src/boot_scene.cpp`: carregam mapa de `game/assets/maps/hello.tmx`.
- `docs/scene_flow.md`: referência ao novo caminho do mapa de exemplo.
- `tests/title_scene.cpp`: define diretório de trabalho relativo ao arquivo de teste.
- `tests/title_scene.cpp`: restaura diretório de trabalho via RAII para segurança em exceções.
- `src/map.hpp`/`src/map.cpp`: `Map` usa `TextureManager` e armazena ponteiros para texturas de tileset.
- `src/map.cpp`: constrói `sf::Vector2u` de tamanho do tile explicitamente para evitar conversão implícita.
- `src/main.cpp`/`src/boot_scene.*`: `TextureManager` global passado por referência ao `Map`.
- `src/map.cpp`: substitui `sf::Quads` por `sf::PrimitiveType::Triangles` para desenhar tiles.
- `src/map.hpp`/`src/map.cpp`: substitui `Layer` por `TileLayer` armazenando IDs de tiles e vértices.
- `src/map.cpp`: gera vértices percorrendo `TileLayer::ids`, move batches para `TileLayer::vertices` e remove `Layer` intermediário.
- `src/map.cpp`: otimiza `Map::draw` para um draw por camada/tileset e comenta possíveis extensões de culling.
- `src/map_scene.hpp`/`src/map_scene.cpp`: carrega TMX via `TextureManager` e posiciona o herói usando `spawn_x`/`spawn_y`.
- `src/map_scene.cpp`: desenho ordenado de camadas `ground_*` antes do herói e demais depois.
- `src/main.cpp`: usa `setFramerateLimit` para 60 FPS e remove `sf::sleep` manual.

### Fixed
- Baseline do vcpkg: `"HEAD"` → SHA real (corrige “builtin-baseline inválido”).
- Erro “Unsupported SFML component: system” (mudança para sintaxe do SFML 3).
- Erro ao configurar por falta de `src/main.cpp`.
- `src/main.cpp`: aplica `stack.applyPending()` logo após os eventos para garantir transições antes do fechamento da janela.
- `CMakeLists.txt`: linka `lumy-tests` com TMXLITE e demais dependências.
- `src/map.cpp`: inicializa IDs de tiles e trata camadas vazias.
- `tests/title_scene.cpp`: envolve inicializador em parênteses no `EXPECT_THROW` para evitar erro de compilação.
- `src/map.cpp`: normaliza caminho das texturas de tileset evitando prefixo duplicado.

### Docs
- Adicionado `VISION.md`.
- Adicionado `ROADMAP.md`.
- Adicionado `CONTRIBUTING.md`.
- Adicionado templates.
- `README.md`: descrição da engine e passos básicos com `CMakePresets.json`.
- `ROADMAP.md`: histórico atualizado após revisão do README.
- `src/main.cpp`: comentários explicando a sequência de renderização (limpar → desenhar → exibir).
- `docs/scene_flow.md` e `README.md`: fluxo Boot → Title → Map e uso do `SceneStack`.
- `docs/scene_flow.md` e `README.md`: documentada exceção ao falhar carregamento da fonte.
- `README.md` e `contributing.md`: instruções para rodar testes com `ctest -C Debug -R basic_startup`.



## [0.1.0] — 2025-08-21
### Added
- Estrutura inicial do repositório **Lumy** (CMake, presets e setup de dependências via vcpkg).
- (exemplo) SceneStack (Boot → Title → Map).
- (exemplo) Sistema de Eventos com 10 comandos básicos.
- (exemplo) Save/Load (JSON).
