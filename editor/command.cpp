/**
 * Command - Implementação do Sistema de Undo/Redo
 */

#pragma execution_character_set("utf-8")

#include "command.h"
#include "map.h"
#include "layer.h"
#include "layer_manager.h"
#include <queue>
#include <set>

// ============================================================================
// PaintTileCommand - Pintar um tile individual
// ============================================================================

PaintTileCommand::PaintTileCommand(Map* map, int layerIndex, int x, int y, int newTileId)
    : m_map(map)
    , m_layerIndex(layerIndex)
    , m_x(x)
    , m_y(y)
    , m_newTileId(newTileId)
    , m_oldTileId(-1)
{
    // Capturar tile atual para undo
    if (m_map) {
        m_oldTileId = m_map->GetTileFromLayer(m_layerIndex, m_x, m_y);
    }
}

bool PaintTileCommand::Execute()
{
    if (!m_map || !m_map->IsValidPosition(m_x, m_y)) {
        return false;
    }
    
    m_map->SetTileInLayer(m_layerIndex, m_x, m_y, m_newTileId);
    return true;
}

bool PaintTileCommand::Undo()
{
    if (!m_map || !m_map->IsValidPosition(m_x, m_y)) {
        return false;
    }
    
    m_map->SetTileInLayer(m_layerIndex, m_x, m_y, m_oldTileId);
    return true;
}

bool PaintTileCommand::CanMergeWith(const ICommand* other) const
{
    // Pode mesclar se for pintura contínua no mesmo layer
    const PaintTileCommand* otherPaint = dynamic_cast<const PaintTileCommand*>(other);
    if (!otherPaint) {
        return false;
    }
    
    // Mesmo layer e tile ID
    return m_layerIndex == otherPaint->m_layerIndex && 
           m_newTileId == otherPaint->m_newTileId;
}

void PaintTileCommand::MergeWith(ICommand* other)
{
    // Mesclagem não altera o estado deste comando
    // O histórico mantém ambos os comandos, mas pode otimizar futuramente
}

// ============================================================================
// FillAreaCommand - Flood fill
// ============================================================================

FillAreaCommand::FillAreaCommand(Map* map, int layerIndex, int startX, int startY, int newTileId)
    : m_map(map)
    , m_layerIndex(layerIndex)
    , m_startX(startX)
    , m_startY(startY)
    , m_newTileId(newTileId)
{
}

bool FillAreaCommand::Execute()
{
    if (!m_map || !m_map->IsValidPosition(m_startX, m_startY)) {
        return false;
    }
    
    int targetTileId = m_map->GetTileFromLayer(m_layerIndex, m_startX, m_startY);
    
    // Não fazer nada se já é o tile desejado
    if (targetTileId == m_newTileId) {
        return true;
    }
    
    // Coletar todas as mudanças antes de aplicar
    m_changes.clear();
    GatherChanges(m_startX, m_startY, targetTileId);
    
    // Aplicar mudanças
    for (const auto& change : m_changes) {
        m_map->SetTileInLayer(m_layerIndex, change.x, change.y, m_newTileId);
    }
    
    return !m_changes.empty();
}

bool FillAreaCommand::Undo()
{
    if (!m_map) {
        return false;
    }
    
    // Restaurar tiles originais
    for (const auto& change : m_changes) {
        m_map->SetTileInLayer(m_layerIndex, change.x, change.y, change.oldTileId);
    }
    
    return true;
}

void FillAreaCommand::GatherChanges(int x, int y, int targetTileId)
{
    if (!m_map || !m_map->IsValidPosition(x, y)) {
        return;
    }
    
    // BFS flood fill para coletar mudanças
    std::queue<std::pair<int, int>> toVisit;
    std::set<std::pair<int, int>> visited;
    
    toVisit.push({x, y});
    visited.insert({x, y});
    
    const int MAX_TILES = 10000; // Limite de segurança
    
    while (!toVisit.empty() && m_changes.size() < MAX_TILES) {
        auto [cx, cy] = toVisit.front();
        toVisit.pop();
        
        if (!m_map->IsValidPosition(cx, cy)) {
            continue;
        }
        
        int currentTile = m_map->GetTileFromLayer(m_layerIndex, cx, cy);
        
        // Só processar se for o tile alvo
        if (currentTile != targetTileId) {
            continue;
        }
        
        // Registrar mudança
        m_changes.push_back({cx, cy, currentTile});
        
        // Adicionar vizinhos (4 direções)
        std::pair<int, int> neighbors[] = {
            {cx, cy - 1}, // Cima
            {cx, cy + 1}, // Baixo
            {cx - 1, cy}, // Esquerda
            {cx + 1, cy}  // Direita
        };
        
        for (const auto& neighbor : neighbors) {
            if (visited.find(neighbor) == visited.end()) {
                visited.insert(neighbor);
                toVisit.push(neighbor);
            }
        }
    }
}

// ============================================================================
// CreateLayerCommand - Criar layer
// ============================================================================

CreateLayerCommand::CreateLayerCommand(Map* map, const wxString& layerName)
    : m_map(map)
    , m_layerName(layerName)
    , m_layerIndex(-1)
{
}

bool CreateLayerCommand::Execute()
{
    if (!m_map) {
        return false;
    }
    
    LayerProperties props;
    props.name = m_layerName;
    props.type = LayerType::TILE_LAYER;
    
    Layer* layer = m_map->CreateLayer(props);
    
    if (layer) {
        // Encontrar índice do layer criado
        LayerManager* manager = m_map->GetLayerManager();
        if (manager) {
            m_layerIndex = manager->GetLayerCount() - 1;
        }
        return true;
    }
    
    return false;
}

bool CreateLayerCommand::Undo()
{
    if (!m_map || m_layerIndex < 0) {
        return false;
    }
    
    return m_map->RemoveLayer(m_layerIndex);
}

// ============================================================================
// RemoveLayerCommand - Remover layer
// ============================================================================

RemoveLayerCommand::RemoveLayerCommand(Map* map, int layerIndex)
    : m_map(map)
    , m_layerIndex(layerIndex)
    , m_savedLayer(nullptr)
{
}

bool RemoveLayerCommand::Execute()
{
    if (!m_map) {
        return false;
    }
    
    LayerManager* manager = m_map->GetLayerManager();
    if (!manager) {
        return false;
    }
    
    Layer* layer = manager->GetLayer(m_layerIndex);
    if (!layer) {
        return false;
    }
    
    // Salvar cópia do layer antes de remover
    m_savedLayer = std::make_unique<Layer>(*layer);
    
    return m_map->RemoveLayer(m_layerIndex);
}

bool RemoveLayerCommand::Undo()
{
    if (!m_map || !m_savedLayer) {
        return false;
    }
    
    LayerManager* manager = m_map->GetLayerManager();
    if (!manager) {
        return false;
    }
    
    // Criar novo layer a partir do backup
    LayerProperties props = m_savedLayer->GetProperties();
    Layer* restoredLayer = manager->CreateLayer(m_savedLayer->GetWidth(), 
                                                  m_savedLayer->GetHeight(), 
                                                  props);
    
    if (!restoredLayer) {
        return false;
    }
    
    // Restaurar tiles
    for (int y = 0; y < m_savedLayer->GetHeight(); ++y) {
        for (int x = 0; x < m_savedLayer->GetWidth(); ++x) {
            int tileId = m_savedLayer->GetTile(x, y);
            restoredLayer->SetTile(x, y, tileId);
        }
    }
    
    // Mover para posição original
    if (manager->SetLayerOrder(manager->GetLayerCount() - 1, m_layerIndex)) {
        return true;
    }
    
    return true;
}

// ============================================================================
// MoveLayerCommand - Mover layer
// ============================================================================

MoveLayerCommand::MoveLayerCommand(Map* map, int layerIndex, Direction direction)
    : m_map(map)
    , m_layerIndex(layerIndex)
    , m_direction(direction)
{
}

bool MoveLayerCommand::Execute()
{
    if (!m_map) {
        return false;
    }
    
    LayerManager* manager = m_map->GetLayerManager();
    if (!manager) {
        return false;
    }
    
    if (m_direction == UP) {
        return manager->MoveLayerUp(m_layerIndex);
    } else {
        return manager->MoveLayerDown(m_layerIndex);
    }
}

bool MoveLayerCommand::Undo()
{
    if (!m_map) {
        return false;
    }
    
    LayerManager* manager = m_map->GetLayerManager();
    if (!manager) {
        return false;
    }
    
    // Mover na direção oposta
    int targetIndex = m_layerIndex;
    
    if (m_direction == UP) {
        // Se moveu para cima (index-1), mover para baixo
        targetIndex = m_layerIndex - 1;
        if (targetIndex >= 0) {
            return manager->MoveLayerDown(targetIndex);
        }
    } else {
        // Se moveu para baixo (index+1), mover para cima
        targetIndex = m_layerIndex + 1;
        if (targetIndex < manager->GetLayerCount()) {
            return manager->MoveLayerUp(targetIndex);
        }
    }
    
    return false;
}

wxString MoveLayerCommand::GetName() const
{
    return m_direction == UP ? "Mover Layer Para Cima" : "Mover Layer Para Baixo";
}

// ============================================================================
// DuplicateLayerCommand - Duplicar layer
// ============================================================================

DuplicateLayerCommand::DuplicateLayerCommand(Map* map, int layerIndex)
    : m_map(map)
    , m_sourceLayerIndex(layerIndex)
    , m_newLayerIndex(-1)
{
}

bool DuplicateLayerCommand::Execute()
{
    if (!m_map) {
        return false;
    }
    
    LayerManager* manager = m_map->GetLayerManager();
    if (!manager) {
        return false;
    }
    
    Layer* duplicated = manager->DuplicateLayer(m_sourceLayerIndex);
    
    if (duplicated) {
        m_newLayerIndex = manager->GetLayerCount() - 1;
        return true;
    }
    
    return false;
}

bool DuplicateLayerCommand::Undo()
{
    if (!m_map || m_newLayerIndex < 0) {
        return false;
    }
    
    return m_map->RemoveLayer(m_newLayerIndex);
}

// ============================================================================
// CommandHistory - Gerenciador de histórico
// ============================================================================

CommandHistory::CommandHistory(size_t maxHistorySize)
    : m_currentIndex(0)
    , m_maxHistorySize(maxHistorySize)
{
}

CommandHistory::~CommandHistory()
{
    Clear();
}

bool CommandHistory::ExecuteCommand(std::unique_ptr<ICommand> command)
{
    if (!command) {
        return false;
    }
    
    // Executar o comando
    if (!command->Execute()) {
        return false;
    }
    
    // Remover comandos após o índice atual (branch de redo)
    if (m_currentIndex < static_cast<int>(m_commands.size())) {
        m_commands.erase(m_commands.begin() + m_currentIndex, m_commands.end());
    }
    
    // Tentar mesclar com comando anterior
    if (m_currentIndex > 0) {
        ICommand* lastCmd = m_commands[m_currentIndex - 1].get();
        if (lastCmd->CanMergeWith(command.get())) {
            lastCmd->MergeWith(command.get());
            // Adicionar mesmo assim para manter histórico completo
        }
    }
    
    // Adicionar ao histórico
    m_commands.push_back(std::move(command));
    m_currentIndex++;
    
    // Limitar tamanho do histórico
    TrimHistory();
    
    return true;
}

bool CommandHistory::Undo()
{
    if (!CanUndo()) {
        return false;
    }
    
    m_currentIndex--;
    ICommand* cmd = m_commands[m_currentIndex].get();
    
    bool success = cmd->Undo();
    
    if (!success) {
        // Reverter índice se falhou
        m_currentIndex++;
        return false;
    }
    
    wxLogMessage("Undo: %s", cmd->GetName());
    return true;
}

bool CommandHistory::Redo()
{
    if (!CanRedo()) {
        return false;
    }
    
    ICommand* cmd = m_commands[m_currentIndex].get();
    
    bool success = cmd->Redo();
    
    if (success) {
        m_currentIndex++;
        wxLogMessage("Redo: %s", cmd->GetName());
    }
    
    return success;
}

void CommandHistory::Clear()
{
    m_commands.clear();
    m_currentIndex = 0;
}

wxString CommandHistory::GetUndoName() const
{
    if (!CanUndo()) {
        return wxEmptyString;
    }
    
    return m_commands[m_currentIndex - 1]->GetName();
}

wxString CommandHistory::GetRedoName() const
{
    if (!CanRedo()) {
        return wxEmptyString;
    }
    
    return m_commands[m_currentIndex]->GetName();
}

void CommandHistory::TrimHistory()
{
    while (m_commands.size() > m_maxHistorySize) {
        m_commands.erase(m_commands.begin());
        m_currentIndex--;
        if (m_currentIndex < 0) {
            m_currentIndex = 0;
        }
    }
}
