# ROADMAP.md — Lumy

> Este roadmap é um **documento vivo**.  
> **Revisão:** quinzenal ou por release.  
> **Última atualização:** 05-10-2025.
> **Status:** `Planned` · `In Progress` · `Done` · `Parked` · `Dropped`.

## Histórico de alterações
- 21-08-2025 — Criação do roadmap inicial (M0–M3).
- 22-08-2025 — README com descrição da engine e guia de CMakePresets.
- 31-08-2025 — ✅ **M0 "Faísca" concluído** com sucesso! Todas as entregas implementadas e testadas.
- 05-10-2025 — ✅ **M0 oficialmente fechado** com v0.1.1. 🎆 **M1 "Brilho" iniciado** - foco no editor wxWidgets.


## Visão geral de marcos (releases)

### M0 — **Faísca** *(Done)* ✅
**Meta:** engine carrega mapas, executa eventos básicos e salva/fecha em segurança.  
**Entregas:**
- Loop + **SceneStack** (Boot → Title → Map).  
- Carregar/desenhar **TMX** (Tiled) e tileset.  
- **Sistema de Eventos** com 10 comandos:  
  `ShowText`, `SetSwitch`, `SetVar`, `If/Else`, `Wait`, `TransferPlayer`, `PlayBGM`, `PlaySE`, `ShowPicture`, `ErasePicture`.  
- **Switch/Var** globais e **locais**; **Save/Load** (JSON).  
- Documentação mínima: executar exemplo `examples/hello-town`.

**DoD (Definition of Done):**
- Build limpa (Win/Linux), 60 FPS estáveis em mapa simples.
- Testes básicos passando (eventos/variáveis/save).
- `examples/hello-town` jogável por 2–3 minutos.

---

### M1 — **Brilho** *(In Progress - 85%)* 🔥
**Meta:** editor wx funcional com edição de mapas/eventos e hot-reload.  
**Entregas:**
- ✅ Editor: **wxAUI** (docks), **Árvore do Projeto**, **PropertyGrid**, **Viewport**.
- ✅ Sistema completo de **Layers** com opacidade
- ✅ **4 Ferramentas de Pintura** (Pincel, Balde, Seleção, Borracha)
- ✅ **Gerenciamento de Tilesets** com propriedades
- 🔄 Sistema **Undo/Redo** (header completo, implementação pendente)
- 🔄 **Hot-reload** de mapas (FileWatcher OK, integração com runtime pendente)
- ❌ Tutoriais de 10 minutos (planejado pós-release)

**DoD:**
- ✅ Editar mapas visualmente com múltiplas layers
- 🔄 Salvar e abrir projetos completos  
- ❌ Ver mudanças no runtime sem reiniciar (pending)
- ✅ Projeto de exemplo editável no editor

---

### M2 — **Chama** *(Planned)*
**Meta:** batalha por **turnos** completa + importador inicial de RM.  
**Entregas:**
- Batalha por turnos: fila por Velocidade, estados/resistências, itens/habilidades, UI clara, partículas.
- Template **“RPG Clássico Lumy”**.
- Ferramenta externa **`rpgx-import`** (MV/MZ): assets, database base, mapas e eventos básicos → relatório de TODO.

**DoD:**
- Combates estáveis com 5–6 habilidades e 4–6 estados.
- Import de um projeto MV/MZ simples funcionando (assets/database/mapas).

---

### M3 — **Aurora** *(Planned)*
**Meta:** grid **hex** + **módulo colecionáveis** (cartas/runas/criaturas).  
**Entregas:**
- Navegação hex (projeção axial), custo, terreno e **altura** influenciando defesa/precisão.
- IA tática básica (prioridades por objetivo/terreno).
- Módulo **Colecionáveis**: deckbuilder no editor, raridades e sinergias.
- Templates **“Tático/Hex”** e **“Cartas”**.

**DoD:**
- Skirmish tático funcional (mapa hex com 2–3 tipos de terreno).
- Deck simples com 20–30 cartas demonstrando sinergias.

---

## Tarefas por marco (checklist de acompanhamento)

### M0 — Faísca
- [x] Definir esquemas **JSON** (actors/items/states/skills/enemies/system). *(Done)*
- [x] Tabela de **comandos de evento** (10 básicos) com argumentos. *(Done)*
- [x] Carregar **TMX** via tmxlite; desenhar layers e flags de colisão. *(Done)*
- [x] Implementar **SceneStack** e ciclo de jogo. *(Done)*
- [x] **Save/Load** de switches/variáveis/posição. *(Done)*
- [x] Exemplo `hello-town` + README. *(Done)*

### M1 — Brilho (85% Completo)
- [x] Editor wx: panes, árvore, property grid. *(Done)*
- [x] Sistema de Layers completo com UI. *(Done)*
- [x] 4 Ferramentas de pintura implementadas. *(Done)*
- [ ] Viewport GL com grade, seleção e pincéis. *(In Progress - 60%)*
- [x] Gerenciamento de tilesets com propriedades. *(Done)*
- [ ] Sistema Undo/Redo. *(In Progress - 50%)*
- [ ] Ferramenta de colisão/regiões. *(Planned - visualização pendente)*
- [ ] Hot-reload de mapas/eventos. *(In Progress - FileWatcher OK)*
- [ ] Tutoriais curtos (vídeo/gif ou passo-a-passo). *(Planned)*

### M2 — Chama
- [ ] Loop de batalha por turnos + fila por Velocidade. *(Planned)*
- [ ] Estados/resistências/itens/habilidades. *(Planned)*
- [ ] UI de combate + partículas. *(Planned)*
- [ ] Importador **MV/MZ** (F0–F3) + relatório de TODO. *(Planned)*
- [ ] Template “RPG Clássico Lumy”. *(Planned)*

### M3 — Aurora
- [ ] Grid hex (axial), custo e terreno/altura. *(Planned)*
- [ ] IA tática básica. *(Planned)*
- [ ] Módulo colecionáveis + deckbuilder. *(Planned)*
- [ ] Templates “Tático/Hex” e “Cartas”. *(Planned)*

---

## Dependências & escolhas técnicas (baseline)
- **C++20 + CMake**
- **SFML** (runtime) — render/áudio/input.
- **wxWidgets** (editor) — wxAUI, PropertyGrid, wxGLCanvas.
- **tmxlite** (TMX), **nlohmann/json** (JSON).
- **Lua 5.4 + sol2** (scripts/comandos).
- Licença prevista: **MIT** (engine).

---

## Riscos & Mitigações
- **Complexidade do editor:** começar mínimo (M1) e evoluir por panes.
- **Importação RM:** priorizar **MV/MZ** (JSON). VX Ace/XP/2k3 ficam após M2.
- **Hex + IA:** escopo controlado (terreno/altura/alcance) antes de heurísticas avançadas.
- **Performance:** dados simples, perfis periódicos, evitar micro-otimização precoce.

---

## Métricas de sucesso (primeira fase)
- **Tempo até “primeiro mapa jogável”:** ≤ 10 min seguindo o guia.
- **FPS em mapa padrão:** ≥ 60 FPS.
- **Import MV/MZ simples:** sucesso com ≥ 80% dos dados básicos.
- **Adoção:** ≥ 3 projetos de exemplo da comunidade até M3.

---

## Backlog (nice-to-have)
- Export Android; marketplace de plugins/recursos; shaders extras; câmera “cinemática”; recortes automáticos de sprites; editor de diálogos com preview; localizador de recursos (paths quebrados).

---

## Governança & contribuição
- **Owner do roadmap:** *(preencher)*  
- **Processo:** PR com rótulo `roadmap` + descrição breve do ajuste; revisão quinzenal.  
- **Guia:** ver `CONTRIBUTING.md`.

