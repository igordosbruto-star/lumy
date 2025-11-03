# 🎆 Lumy v0.1.0 - "Faísca" Release Notes

**Data de lançamento:** 31 de agosto de 2025  
**Milestone:** M0 "Faísca" ✅

## 🎉 Primeira Release da Lumy Engine!

Esta é a primeira release oficial da Lumy, uma engine 2D para RPGs top-down em C++20. O M0 "Faísca" estabelece a fundação sólida da engine com todas as funcionalidades essenciais para criar jogos RPG básicos.

## 🚀 Principais Funcionalidades

### 🗺️ **Sistema de Mapas TMX**
- Carregamento completo de mapas criados no Tiled Editor
- Renderização otimizada com múltiplas camadas
- Suporte a tilesets com cache inteligente de texturas
- Sistema de posicionamento automático do personagem via objetos spawn

### 📦 **Sistema de Eventos Robusto**
**10 comandos básicos implementados:**
- **ShowText**: Diálogos e mensagens de texto
- **SetSwitch**: Controle de switches globais (boolean)
- **SetVariable**: Variáveis numéricas globais
- **Wait**: Pausas temporizadas para sequências
- **ConditionalBranch**: Lógica condicional (if/else)
- **EndConditional**: Estruturas de controle
- **TransferPlayer**: Teletransporte de jogador
- **ChangeGold**: Sistema monetário básico
- **PlaySound**: Preparado para áudio (implementação futura)
- **ShowImage**: Preparado para imagens na tela (implementação futura)

### 💾 **Sistema de Save/Load Avançado**
- **9 slots independentes** de save
- **Dados persistidos**: Posição, switches, variáveis, party, inventário
- **Formato JSON** legível e modificável
- **Atalhos intuitivos**:
  - F5/F9: Quick save/load
  - AltGr + número: Salvar
  - Ctrl + AltGr + número: Carregar
  - Shift + AltGr + número: Deletar

### 🎮 **Experiência de Jogo Completa**
- **Movimento fluido** com teclas WASD
- **Sistema de cenas** (Boot → Title → Map)
- **UI de debug** com informações em tempo real
- **Performance otimizada** mantendo 60 FPS

## 🏗️ Arquitetura Técnica

### **Core Systems**
- **SceneStack**: Gerenciamento robusto de cenas com push/pop
- **EventSystem**: Execução de comandos com integração Lua
- **SaveSystem**: Persistência em JSON com validação
- **TextureManager**: Cache inteligente de recursos gráficos

### **Compatibilidade SFML 3**
Todas as mudanças de API do SFML 3 foram implementadas:
- `sf::Text` sem construtor padrão
- `sf::Font::openFromFile` (nova API)
- `setPosition` com `sf::Vector2`
- `TextureManager::acquire` retornando referências

## 🎮 Exemplo: Hello Town

O exemplo `hello-town` demonstra todas as funcionalidades em uma experiência jogável completa:

- **2 NPCs interativos** demonstrando sistema de eventos
- **Lógica condicional** baseada em estado do jogo
- **Sistema de saves** totalmente funcional
- **UI informativa** para desenvolvimento e debug

## 🧪 Qualidade e Testes

- **7 testes unitários** passando com 100% de sucesso
- **Build limpa** sem erros de compilação
- **Performance consistente** de 60 FPS
- **Validação completa** de todos os sistemas

## 📚 Documentação

### **Guias Disponíveis**
- `README.md`: Visão geral e instruções de build
- `docs/hello-town-guide.md`: Guia completo da experiência
- `docs/scene_flow.md`: Detalhes do sistema de cenas
- `ROADMAP.md`: Planejamento de próximos milestones
- `CHANGELOG.md`: Histórico detalhado de mudanças

### **Esquemas de Dados**
Estruturas JSON completas em `game/data/`:
- `actors.json`: Definições de personagens
- `items.json`: Sistema de itens
- `states.json`: Estados e efeitos
- `skills.json`: Habilidades e magias
- `enemies.json`: Inimigos e bestiário
- `system.json`: Configurações globais

## 🛠️ Para Desenvolvedores

### **Instalação Rápida**
```bash
# Clonar repositório
git clone <seu-repo>
cd lumy

# Configurar build
cmake --preset msvc-vcpkg

# Compilar
cmake --build build --config Debug

# Executar exemplo
build/bin/Debug/hello-town.exe

# Rodar testes
ctest -C Debug
```

### **Estrutura do Projeto**
```
lumy/
├── src/                    # Código fonte da engine
├── game/                   # Assets e dados do jogo
├── docs/                   # Documentação
├── tests/                  # Testes unitários
└── build/                  # Build outputs
```

## 🎯 Próximos Passos

Com M0 concluído, o próximo milestone é **M1 "Brilho"**:
- Editor visual com wxWidgets
- Hot-reload de mapas e eventos
- Interface gráfica para edição

## 🙏 Agradecimentos

Obrigado por acompanhar o desenvolvimento da Lumy Engine. Esta primeira release representa a base sólida para um futuro brilhante de criação de RPGs!

---

**Links úteis:**
- [Roadmap completo](ROADMAP.md)
- [Como contribuir](CONTRIBUTING.md)
- [Histórico de mudanças](CHANGELOG.md)
