# VISION.md — Lumy

**Tagline:** *Lumy — acenda sua imaginação.*

## 1) Propósito
A **Lumy** é uma engine 2D minimalista e mística para criar RPGs e táticos com a fluidez dos clássicos (SNES/GBA/DS) e o conforto de um workflow moderno. Tudo é simples, transparente e versionável.

## 2) Princípios (Manifesto)
- **Luz sobre a simplicidade:** dados legíveis (JSON/TMX), sem caixas-pretas.
- **Mundo antes de menu:** priorizamos o mapa vivo, a interação e a sensação.
- **Liberdade com trilhos:** defaults excelentes; extensões por Lua/C++ quando quiser ousar.
- **Nostalgia com futuro:** pixel-perfect + efeitos/UX atuais.
- **Comunidade é o sol:** importador de RPG Maker, templates prontos, jams mensais.

## 3) Pilares
**Mundo • Batalha • Estilo**

**Mundo:** grid **quadrado** (Zelda) **e hexagonal** (Wesnoth) nativos; parallax, clima, dia/noite, câmera dinâmica; eventos visuais ao estilo RM (Ação/Toque/Autorun/Paralelo), variáveis e switches locais/globais.

**Batalha:** módulos plugáveis — **Turnos** (padrão), **Ação 2D** (opcional), **Tático/Hex** (terreno/altura, IA básica). Estados, resistências, itens e habilidades.

**Estilo:** suporte natural a **16/32/48/64px**; escala sem borrão; UI skinnável; partículas e shaders simples (água/vento/tilt).

## 4) A Energia “Lumy” (ficção que vira design)
**Lumy** é a substância sutil do mundo e um **recurso unificador** no design:
- Magias gastam Lumy; runas carregam Lumy.
- Cartas/criaturas canalizam Lumy para efeitos especiais.
- Um mesmo “recurso” integra exploração, combate e colecionáveis.

## 5) Arquitetura (clara e modular)
- **Runtime C++ (engine):** cenas, render, áudio, input — independente do editor.
- **Editor (wxWidgets):** docks (wxAUI), árvore do projeto, PropertyGrid, viewport GL (wxGLCanvas).
- **Dados:** `/data/*.json` (database), `/maps/*.tmx` (Tiled), `/assets/...` (sprites/áudio/fontes).
- **Scripts:** **Lua (sol2)** para comandos de evento e lógica custom.
- **Plugins C++:** quando performance/integração exigirem.
- **Ferramenta externa:** `rpgx-import` para projetos **RPG Maker** (MV/MZ → VX Ace → XP/2k3). O motor permanece limpo.

## 6) Sistema de Eventos
- **Blocos visuais**: `ShowText`, `SetSwitch/Var`, `If/Else`, `Wait`, `TransferPlayer`, `PlayBGM/SE`, `Show/ErasePicture`, `CallCommonEvent`.
- Variáveis **locais** do evento + **globais** do projeto.
- **Condições por página** (switch/variável/hora/região).
- **Hot-reload** no editor: salvar e ver acontecer.

## 7) Mapas que respiram
- Camadas com prioridade (andar atrás/à frente), **regiões** e **terrenos** (defesa/precisão no tático).
- Autotiles/brushes modernos: rios, estradas, penhascos prontos.
- **Parallax** com velocidade X/Y, loop e blend por clima.
- **Hex**: vizinhança axial, custo de movimento, altura.

## 8) Batalha & Colecionáveis
- **Turnos (MVP):** fila por Velocidade; estados (veneno/sono), resistências; UI clara; partículas simples.
- **Ação 2D (opcional):** hitbox/hurtbox, i-frames, stamina/Lumy.
- **Tático/Hex:** terreno/altura influenciam defesa/precisão; IA com prioridades.
- **Colecionáveis (módulo):** cartas/runas/criaturas com raridade, sinergias e deckbuilder integrado.

## 9) Comunidade & Ecossistema
- Templates: **RPG Clássico**, **Ação 2D**, **Tático/Hex**, **Cartas**.
- Exemplos comentados; guia de plugins Lua/C++.
- Importador RM como ponte para migrar projetos.
- Jams mensais com vitrine e destaque.

## 10) Vertical Slice de Referência
Vila → Floresta → Chefe (turnos) + um **skirmish** tático em hex + **duelo de cartas** introdutório. Alvo de 15–20 min jogáveis.

## 11) Licença & Qualidade
- Engine sob **MIT** (sujeito a revisão conforme crescer).
- 60 FPS por padrão; 30 FPS aceitável em editor pesado.
- Testes básicos em eventos/variáveis/grid.
- Tutoriais de 10 minutos: “Primeiro mapa”, “Primeiro evento”, “Primeira batalha”.

—  
*Última edição: 21-08-2025*
