# lumy

**Lumy — acenda sua imaginação.** ✨

Lumy é uma engine 2D open-source em C++20 projetada especificamente para RPGs top-down. Com foco na simplicidade e produtividade, oferece um sistema robusto de eventos, mapas editados em Tiled, save/load em múltiplos slots e integração com Lua para scripts personalizados.

**Estado atual:** 🎆 **M0 "Faísca" concluído** — engine funcional com exemplo jogável!

## ✨ Funcionalidades (M0 Faísca)

- 🗺️ **Mapas TMX**: Carregamento e renderização de mapas do Tiled com múltiplas camadas
- 📦 **Sistema de Eventos**: 10 comandos básicos para criar interações (diálogos, switches, variáveis, etc.)
- 💾 **Save/Load**: Sistema completo de saves em JSON com múltiplos slots (1-9)
- 🎮 **Controles Intuitivos**: Movimento WASD + atalhos de teclado para saves
- 🔄 **Gerenciamento de Cenas**: Sistema robusto Boot → Title → Map
- ⚡ **Performance**: 60 FPS estáveis com renderização otimizada
- 🧪 **Testes**: Suíte completa de testes automatizados

## 🛠️ Tecnologias

- **C++20** + **CMake** (build system)
- **SFML 3** (renderização, áudio, input)
- **tmxlite** (mapas TMX do Tiled)
- **Lua 5.4 + sol2** (scripting)
- **nlohmann/json** (serialização)
- **GoogleTest** (testes unitários)

> **Requisitos:** Windows + MSVC 2022, CMake ≥ 3.25 e vcpkg

## Configuração e execução

Use os presets do `CMakePresets.json` para configurar e rodar o exemplo `hello-town`:

```sh
cmake --preset msvc-vcpkg
cmake --build build/msvc --config Debug
build/msvc/bin/Debug/hello-town.exe
```

No VS Code, selecione os mesmos presets e depure o alvo `hello-town` com `F5`.


## Testes

Após compilar, execute os testes com:

```sh
ctest -C Debug -R basic_startup
```


## Fluxo de cenas

O exemplo `hello-town` usa `SceneStack` com as cenas Boot → Title → Map. `BootScene` carrega recursos e muda para `TitleScene`, que exibe "Start" (requer `game/font.ttf`; falha no carregamento lança exceção) e ao confirmar abre `MapScene` com um herói movido por W/A/S/D.

Veja [docs/scene_flow.md](docs/scene_flow.md) para detalhes.

## 🎮 Exemplo: Hello Town

O exemplo `hello-town` demonstra todas as funcionalidades do M0 Faísca:

### Controles
- **WASD**: Movimento do personagem
- **Enter/Space**: Interagir com eventos próximos
- **F5**: Quick Save (slot 1)
- **F9**: Quick Load (slot 1)
- **AltGr + Número**: Salvar no slot (1-9)
- **Ctrl + AltGr + Número**: Carregar do slot (1-9)
- **Shift + AltGr + Número**: Deletar save do slot (1-9)

### Experiência de Jogo (2-3 minutos)
1. **Início**: Cena de título com botão "Start"
2. **Mapa**: Personagem aparéce no mapa hello-town
3. **NPC Welcome**: Se aproxime da posição (200, 200) e pressione Enter
   - Recebe mensagem de boas-vindas
   - Switch 1 é ativado, variável 1 recebe valor 100
4. **NPC Conditional**: Vá para (300, 150) e interaja
   - Se já falou com o primeiro NPC, recebe mensagem especial
5. **Sistema de Saves**: Teste os atalhos de save/load
6. **UI Debug**: Observe posição e estado das variáveis na tela

### Estrutura do Projeto
```
game/
├── assets/
│   └── maps/hello.tmx     # Mapa principal
├── data/               # Esquemas JSON dos dados
│   ├── actors.json
│   ├── items.json
│   └── ...
├── saves/              # Saves dos jogadores (criado automaticamente)
└── font.ttf            # Fonte para UI
```

## 🔧 Sistemas Implementados

### Sistema de Eventos
**10 comandos básicos disponíveis:**
- `ShowText`: Exibir diálogos
- `SetSwitch`: Definir switches globais
- `SetVariable`: Definir variáveis numéricas
- `Wait`: Pausas temporizadas
- `ConditionalBranch`: Lógica condicional (if/else)
- `EndConditional`: Fim dos condicionais
- `TransferPlayer`: Mover jogador
- `ChangeGold`: Modificar dinheiro
- `PlaySound`: Áudio (implementação futura)
- `ShowImage`: Imágenes na tela (implementação futura)

### Sistema de Save/Load
- **Múltiplos slots**: 9 slots de save independentes
- **Dados salvos**: Posição do jogador, switches, variáveis, party, inventário
- **Formato JSON**: Fácil para debug e modding
- **Atalhos intuitivos**: AltGr + teclas numéricas

## Gerenciamento de texturas

O `TextureManager` centraliza o carregamento de tilesets e reutiliza texturas já abertas, evitando carregamentos duplicados e reduzindo uso de memória.

## 🛣️ Roadmap

- **M0 "Faísca"** ✅ *(Concluído)* - Engine funcional com eventos e saves
- **M1 "Brilho"** 🔄 *(Próximo)* - Editor visual com wxWidgets + hot-reload
- **M2 "Chama"** 📋 *(Planejado)* - Sistema de batalha + importador RPG Maker
- **M3 "Aurora"** 📋 *(Planejado)* - Grid hexagonal + sistema de cartas

Veja [docs/ROADMAP.md](docs/ROADMAP.md) para detalhes completos.

## 📚 Documentação

- [VISION.md](docs/VISION.md) - Manifesto e princípios do projeto
- [ROADMAP.md](docs/ROADMAP.md) - Plano de desenvolvimento
- [CHANGELOG.md](docs/CHANGELOG.md) - Histórico de versões
- [CONTRIBUTING.md](docs/CONTRIBUTING.md) - Guia de contribuição
- [CODING_STANDARDS.md](docs/CODING_STANDARDS.md) - Padrões de código

## 📄 Licença

MIT License - veja [LICENSE](LICENSE) para detalhes.
