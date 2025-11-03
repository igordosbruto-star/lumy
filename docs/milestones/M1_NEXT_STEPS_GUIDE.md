# 🎯 M1 "Brilho" - Guia para Completar 100%

**Status Atual:** 88% Completo  
**Meta:** 100% Completo + Testes Passando  
**Tempo Estimado:** 15-20 horas

---

## 📋 Checklist de Tarefas (Por Prioridade)

### ✅ **FASE 1: Undo/Redo System** (5-6 horas) - CRÍTICO

#### Tarefa 1.1: Implementar command.cpp
```powershell
# Criar arquivo
New-Item editor/command.cpp -ItemType File
```

**Conteúdo a implementar:**
```cpp
// 1. CommandHistory (gerenciador)
CommandHistory::CommandHistory(size_t maxHistorySize)
    : m_currentIndex(0), m_maxHistorySize(maxHistorySize) {}

bool CommandHistory::ExecuteCommand(std::unique_ptr<ICommand> command) {
    // Executar comando
    if (!command->Execute()) return false;
    
    // Remover comandos após currentIndex (invalida redo)
    m_commands.erase(m_commands.begin() + m_currentIndex, m_commands.end());
    
    // Adicionar novo comando
    m_commands.push_back(std::move(command));
    m_currentIndex = m_commands.size();
    
    // Limitar tamanho
    TrimHistory();
    return true;
}

bool CommandHistory::Undo() {
    if (!CanUndo()) return false;
    m_currentIndex--;
    return m_commands[m_currentIndex]->Undo();
}

bool CommandHistory::Redo() {
    if (!CanRedo()) return false;
    bool result = m_commands[m_currentIndex]->Redo();
    if (result) m_currentIndex++;
    return result;
}

// 2. PaintTileCommand
PaintTileCommand::PaintTileCommand(Map* map, int layerIndex, int x, int y, int newTileId)
    : m_map(map), m_layerIndex(layerIndex), m_x(x), m_y(y), m_newTileId(newTileId) {
    m_oldTileId = map->GetTileFromLayer(layerIndex, x, y);
}

bool PaintTileCommand::Execute() {
    m_map->SetTileInLayer(m_layerIndex, m_x, m_y, m_newTileId);
    return true;
}

bool PaintTileCommand::Undo() {
    m_map->SetTileInLayer(m_layerIndex, m_x, m_y, m_oldTileId);
    return true;
}

// 3-7. Implementar outros comandos...
```

#### Tarefa 1.2: Adicionar ao CMakeLists.txt
```cmake
# Em editor/CMakeLists.txt, adicionar:
set(EDITOR_SOURCES
    ...
    editor/command.cpp  # <- ADICIONAR ESTA LINHA
)
```

#### Tarefa 1.3: Integrar no EditorFrame
```cpp
// editor_frame.h
#include "command.h"

class EditorFrame {
private:
    std::unique_ptr<CommandHistory> m_commandHistory;
};

// editor_frame.cpp
EditorFrame::EditorFrame() {
    m_commandHistory = std::make_unique<CommandHistory>(100);
}

// Adicionar menu Edit
void EditorFrame::CreateMenuBar() {
    wxMenu* editMenu = new wxMenu;
    editMenu->Append(ID_EDIT_UNDO, "&Desfazer\tCtrl+Z");
    editMenu->Append(ID_EDIT_REDO, "&Refazer\tCtrl+Y");
    menuBar->Append(editMenu, "&Editar");
}

void EditorFrame::OnUndo(wxCommandEvent& event) {
    if (m_commandHistory->CanUndo()) {
        m_commandHistory->Undo();
        if (m_viewport) m_viewport->Refresh();
    }
}
```

#### Tarefa 1.4: Integrar no ViewportPanel
```cpp
// viewport_panel.cpp
void ViewportPanel::OnMouseDown(wxMouseEvent& event) {
    if (m_currentTool == PaintTool::BRUSH) {
        auto cmd = std::make_unique<PaintTileCommand>(
            m_currentMap, m_activeLayerIndex, tileX, tileY, m_selectedTileId
        );
        GetParent()->GetCommandHistory()->ExecuteCommand(std::move(cmd));
    }
}
```

**Tempo:** 5-6 horas  
**Resultado:** Ctrl+Z / Ctrl+Y funcionando!

---

### ✅ **FASE 2: Testes Unitários** (4-5 horas) - CRÍTICO

#### Tarefa 2.1: Criar estrutura de testes do editor
```powershell
# Criar diretório
New-Item tests/editor -ItemType Directory

# Criar arquivos
New-Item tests/editor/layer_manager_test.cpp -ItemType File
New-Item tests/editor/command_test.cpp -ItemType File
New-Item tests/editor/map_test.cpp -ItemType File
```

#### Tarefa 2.2: Implementar layer_manager_test.cpp
```cpp
#include <gtest/gtest.h>
#include "layer_manager.h"
#include "layer.h"

TEST(LayerManager, CreateLayer) {
    LayerManager manager;
    
    LayerProperties props;
    props.name = "Test Layer";
    props.type = LayerType::TILE_LAYER;
    
    Layer* layer = manager.CreateLayer(props);
    
    ASSERT_NE(layer, nullptr);
    EXPECT_EQ(layer->GetName(), "Test Layer");
    EXPECT_EQ(manager.GetLayerCount(), 1);
}

TEST(LayerManager, RemoveLayer) {
    LayerManager manager;
    manager.CreateLayer();
    manager.CreateLayer();
    
    EXPECT_EQ(manager.GetLayerCount(), 2);
    
    manager.RemoveLayer(0);
    EXPECT_EQ(manager.GetLayerCount(), 1);
}

TEST(LayerManager, MoveLayerUpDown) {
    LayerManager manager;
    Layer* layer1 = manager.CreateLayer();
    Layer* layer2 = manager.CreateLayer();
    
    layer1->SetName("Layer1");
    layer2->SetName("Layer2");
    
    // Layer2 está no índice 1
    EXPECT_EQ(manager.GetLayer(1)->GetName(), "Layer2");
    
    // Mover para baixo (índice 0)
    manager.MoveLayerDown(1);
    EXPECT_EQ(manager.GetLayer(0)->GetName(), "Layer2");
}

// +10 testes mais...
```

#### Tarefa 2.3: Implementar command_test.cpp
```cpp
#include <gtest/gtest.h>
#include "command.h"
#include "map.h"

TEST(CommandHistory, ExecuteAndUndo) {
    CommandHistory history;
    Map map(10, 10);
    
    auto cmd = std::make_unique<PaintTileCommand>(&map, 0, 5, 5, 42);
    history.ExecuteCommand(std::move(cmd));
    
    EXPECT_TRUE(history.CanUndo());
    EXPECT_FALSE(history.CanRedo());
    
    history.Undo();
    
    EXPECT_FALSE(history.CanUndo());
    EXPECT_TRUE(history.CanRedo());
}

TEST(CommandHistory, Redo) {
    CommandHistory history;
    Map map(10, 10);
    
    auto cmd = std::make_unique<PaintTileCommand>(&map, 0, 5, 5, 42);
    history.ExecuteCommand(std::move(cmd));
    history.Undo();
    
    EXPECT_TRUE(history.Redo());
    EXPECT_TRUE(history.CanUndo());
}

// +8 testes mais...
```

#### Tarefa 2.4: Atualizar CMakeLists.txt
```cmake
# Adicionar ao lumy-tests
add_executable(lumy-editor-tests
  tests/editor/layer_manager_test.cpp
  tests/editor/command_test.cpp
  tests/editor/map_test.cpp
  editor/layer.cpp
  editor/layer_manager.cpp
  editor/map.cpp
  editor/command.cpp
)

target_link_libraries(lumy-editor-tests PRIVATE
  GTest::gtest_main
  wx::core
  nlohmann_json::nlohmann_json
)
```

**Tempo:** 4-5 horas  
**Resultado:** `ctest -C Debug` passando!

---

### ✅ **FASE 3: Polish e Melhorias** (3-4 horas) - IMPORTANTE

#### Tarefa 3.1: PropertyGrid Dinâmico
```cpp
// property_grid_panel.cpp
void PropertyGridPanel::UpdateForLayer(Layer* layer) {
    m_propertyGrid->Clear();
    
    if (!layer) return;
    
    m_propertyGrid->Append(new wxStringProperty("Nome", "name", layer->GetName()));
    m_propertyGrid->Append(new wxFloatProperty("Opacidade", "opacity", layer->GetOpacity()));
    m_propertyGrid->Append(new wxBoolProperty("Visível", "visible", layer->IsVisible()));
    m_propertyGrid->Append(new wxBoolProperty("Bloqueada", "locked", layer->IsLocked()));
    m_propertyGrid->Append(new wxIntProperty("Z-Order", "zorder", layer->GetZOrder()));
}

void PropertyGridPanel::UpdateForTile(int tileId, TileProperties* props) {
    m_propertyGrid->Clear();
    
    if (!props) return;
    
    m_propertyGrid->Append(new wxIntProperty("Tile ID", "id", tileId));
    m_propertyGrid->Append(new wxBoolProperty("Colisão", "collision", props->hasCollision));
    m_propertyGrid->Append(new wxBoolProperty("Animado", "animated", props->isAnimated));
    m_propertyGrid->Append(new wxStringProperty("Som", "sound", props->soundEffect));
}
```

#### Tarefa 3.2: Zoom e Pan no Viewport
```cpp
// viewport_panel.h
class ViewportPanel {
private:
    float m_zoom;  // 0.25 - 4.0
    wxPoint m_panOffset;
    bool m_isPanning;
    wxPoint m_panStart;
};

// viewport_panel.cpp
void ViewportPanel::OnMouseWheel(wxMouseEvent& event) {
    float delta = event.GetWheelRotation() > 0 ? 0.1f : -0.1f;
    m_zoom = std::clamp(m_zoom + delta, 0.25f, 4.0f);
    Refresh();
}

void ViewportPanel::OnMiddleMouseDown(wxMouseEvent& event) {
    m_isPanning = true;
    m_panStart = event.GetPosition();
}

void ViewportPanel::OnMiddleMouseUp(wxMouseEvent& event) {
    m_isPanning = false;
}

void ViewportPanel::OnMouseMove(wxMouseEvent& event) {
    if (m_isPanning) {
        wxPoint current = event.GetPosition();
        m_panOffset += current - m_panStart;
        m_panStart = current;
        Refresh();
    }
}
```

#### Tarefa 3.3: Collision Overlay
```cpp
// viewport_panel.cpp
void ViewportPanel::RenderMap(wxDC& dc) {
    // ... renderizar tiles normalmente ...
    
    // Overlay de colisão (se ativado)
    if (m_showCollision && m_tilesetManager) {
        dc.SetBrush(wxBrush(wxColour(255, 0, 0, 100)));
        dc.SetPen(*wxTRANSPARENT_PEN);
        
        for (int y = 0; y < mapHeight; ++y) {
            for (int x = 0; x < mapWidth; ++x) {
                int tileId = map->GetTile(x, y);
                if (m_tilesetManager->HasCollision(tileId)) {
                    dc.DrawRectangle(x * tileSize * m_zoom, 
                                    y * tileSize * m_zoom,
                                    tileSize * m_zoom, 
                                    tileSize * m_zoom);
                }
            }
        }
    }
}
```

**Tempo:** 3-4 horas  
**Resultado:** UX melhorada significativamente!

---

### 🔄 **FASE 4: OpenGL (Opcional)** (6-8 horas) - PERFORMANCE

Refatorar ViewportPanel para wxGLCanvas **SOMENTE** se houver tempo.

**Motivo:** Melhora performance mas não é blocker para release.

**Alternativa:** Lançar M1 sem GL e adicionar no M1.1 (patch).

---

## 🚀 Ordem de Execução Recomendada

### **Dia 1 (6-8 horas):**
1. ✅ Implementar `command.cpp` completo (4h)
2. ✅ Integrar no EditorFrame e ViewportPanel (2h)
3. ✅ Testar Undo/Redo manualmente (30min)

### **Dia 2 (5-7 horas):**
1. ✅ Criar estrutura de testes (30min)
2. ✅ Implementar layer_manager_test.cpp (2h)
3. ✅ Implementar command_test.cpp (2h)
4. ✅ Implementar map_test.cpp (1h)
5. ✅ Rodar testes e corrigir falhas (1h)

### **Dia 3 (4-5 horas):**
1. ✅ PropertyGrid dinâmico (2h)
2. ✅ Zoom/Pan básico (1.5h)
3. ✅ Collision overlay (1h)
4. ✅ Testes finais e correções (30min)

**Total:** 15-20 horas → **M1 100% Completo!**

---

## ✔️ Critérios de Aceitação (M1 100%)

- [x] Editor wxWidgets funcional com wxAUI
- [x] LayerPanel com todas as operações + opacidade
- [ ] Sistema Undo/Redo funcionando (Ctrl+Z/Y)
- [x] 4 Ferramentas de pintura operacionais
- [x] Gerenciamento de tilesets
- [ ] PropertyGrid atualiza dinamicamente
- [ ] Zoom e Pan no viewport
- [ ] Testes unitários passando (>80% cobertura editor)
- [ ] Documentação completa
- [x] ROADMAP atualizado

**Progresso:** 8/10 ✅

---

## 🐛 Problemas Conhecidos e Soluções

### Problema 1: Undefined reference to command classes
```
Error: undefined reference to `PaintTileCommand::PaintTileCommand'
```
**Solução:** Implementar `command.cpp` OU comentar temporariamente no CMakeLists.txt.

### Problema 2: wxGLCanvas não disponível
```
Error: 'wxGLCanvas' was not declared in this scope
```
**Solução:** Adicionar `find_package(OpenGL REQUIRED)` no CMake.

### Problema 3: Testes não compilam
```
Error: cannot find -lGTest::gtest_main
```
**Solução:** Instalar GoogleTest via vcpkg: `vcpkg install gtest:x64-windows`.

---

## 📊 Verificação Final

Antes de marcar M1 como 100%, verificar:

```powershell
# 1. Compilar sem erros
cmake --preset msvc-vcpkg
cmake --build build/msvc --config Debug

# 2. Testes passando
ctest -C Debug --test-dir build/msvc
# Esperado: 100% passed

# 3. Editor funcional
build/msvc/bin/Debug/lumy-editor.exe
# Criar projeto → Criar mapa → Pintar tiles → Ctrl+Z/Y → OK!

# 4. Runtime funcional
build/msvc/bin/Debug/hello-town.exe
# Mapa carrega → Movimenta → Salva → OK!
```

---

## 🎉 Após M1 100%

1. **Tag de Release:**
   ```bash
   git tag -a v0.2.0 -m "M1 Brilho - Editor Completo"
   git push origin v0.2.0
   ```

2. **Criar RELEASE_NOTES.md para v0.2.0**

3. **Atualizar CHANGELOG.md**

4. **Começar M2 "Chama":**
   - Sistema de batalha por turnos
   - Importador RPG Maker
   - Templates de projeto

---

**Boa sorte! Vocês estão muito perto de completar o M1!** 🚀🎨✨

**Dúvidas?** Consulte:
- `M1_COMPLETION_STATUS.md` - Status detalhado
- `M1_UPDATE_SUMMARY.md` - O que já foi feito
- `ROADMAP.md` - Visão geral dos marcos
