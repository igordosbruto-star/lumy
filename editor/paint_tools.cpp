/**
 * PaintTools - Implementação das ferramentas de pintura
 */

#include "paint_tools.h"
#include "map.h"
#include <queue>
#include <set>

// ============================================================================
// BrushTool - Ferramenta Pincel
// ============================================================================

void BrushTool::OnMouseDown(const TilePosition& pos, int tileId, Map* map)
{
    if (!map) return;
    
    m_isPainting = true;
    m_lastPos = pos;
    
    // Pintar tile inicial
    if (map->IsValidPosition(pos.x, pos.y))
    {
        map->SetTile(pos.x, pos.y, tileId);
        map->SetModified(true);
        wxLogMessage("Pincel: Pintando tile (%d, %d) com ID %d", pos.x, pos.y, tileId);
    }
}

void BrushTool::OnMouseMove(const TilePosition& pos, int tileId, Map* map)
{
    if (!map || !m_isPainting) return;
    
    // Pintar apenas se a posição mudou
    if (pos != m_lastPos && map->IsValidPosition(pos.x, pos.y))
    {
        map->SetTile(pos.x, pos.y, tileId);
        map->SetModified(true);
        m_lastPos = pos;
    }
}

void BrushTool::OnMouseUp(const TilePosition& pos, int tileId, Map* map)
{
    m_isPainting = false;
    
    if (map)
    {
        wxLogMessage("Pincel: Finalizado");
    }
}

// ============================================================================
// BucketTool - Ferramenta Balde (Preenchimento)
// ============================================================================

void BucketTool::OnMouseDown(const TilePosition& pos, int tileId, Map* map)
{
    if (!map || !map->IsValidPosition(pos.x, pos.y)) return;
    
    int oldTileId = map->GetTile(pos.x, pos.y);
    
    // Não fazer nada se o tile já é o mesmo
    if (oldTileId == tileId)
    {
        wxLogMessage("Preenchimento: Tile já é o mesmo, nada a fazer");
        return;
    }
    
    wxLogMessage("Preenchimento: Iniciando flood fill em (%d, %d)", pos.x, pos.y);
    FloodFill(pos, oldTileId, tileId, map);
    map->SetModified(true);
    wxLogMessage("Preenchimento: Concluído");
}

void BucketTool::OnMouseMove(const TilePosition& pos, int tileId, Map* map)
{
    // Bucket tool não faz nada no move
}

void BucketTool::OnMouseUp(const TilePosition& pos, int tileId, Map* map)
{
    // Bucket tool não faz nada no up
}

void BucketTool::FloodFill(const TilePosition& start, int oldTile, int newTile, Map* map)
{
    if (!map) return;
    
    // Algoritmo de flood fill usando queue (BFS)
    std::queue<TilePosition> toVisit;
    std::set<std::pair<int, int>> visited;
    
    toVisit.push(start);
    visited.insert({start.x, start.y});
    
    int tilesChanged = 0;
    const int MAX_TILES = 10000; // Limite de segurança
    
    while (!toVisit.empty() && tilesChanged < MAX_TILES)
    {
        TilePosition current = toVisit.front();
        toVisit.pop();
        
        // Verificar se é válido e tem o tile antigo
        if (!map->IsValidPosition(current.x, current.y))
            continue;
            
        if (map->GetTile(current.x, current.y) != oldTile)
            continue;
        
        // Pintar o tile
        map->SetTile(current.x, current.y, newTile);
        tilesChanged++;
        
        // Adicionar vizinhos (cima, baixo, esquerda, direita)
        TilePosition neighbors[] = {
            TilePosition(current.x, current.y - 1), // Cima
            TilePosition(current.x, current.y + 1), // Baixo
            TilePosition(current.x - 1, current.y), // Esquerda
            TilePosition(current.x + 1, current.y)  // Direita
        };
        
        for (const auto& neighbor : neighbors)
        {
            auto key = std::make_pair(neighbor.x, neighbor.y);
            if (visited.find(key) == visited.end())
            {
                visited.insert(key);
                toVisit.push(neighbor);
            }
        }
    }
    
    wxLogMessage("Preenchimento: %d tiles alterados", tilesChanged);
}

// ============================================================================
// SelectionTool - Ferramenta Seleção
// ============================================================================

void SelectionTool::OnMouseDown(const TilePosition& pos, int tileId, Map* map)
{
    if (!map) return;
    
    m_isSelecting = true;
    m_selection.start = pos;
    m_selection.end = pos;
    m_selection.active = true;
    
    wxLogMessage("Seleção: Iniciando em (%d, %d)", pos.x, pos.y);
}

void SelectionTool::OnMouseMove(const TilePosition& pos, int tileId, Map* map)
{
    if (!map || !m_isSelecting) return;
    
    m_selection.end = pos;
}

void SelectionTool::OnMouseUp(const TilePosition& pos, int tileId, Map* map)
{
    if (!map) return;
    
    m_isSelecting = false;
    m_selection.end = pos;
    
    TilePosition topLeft = m_selection.GetTopLeft();
    TilePosition bottomRight = m_selection.GetBottomRight();
    
    wxLogMessage("Seleção: Área selecionada de (%d, %d) a (%d, %d) - %dx%d tiles",
                topLeft.x, topLeft.y, bottomRight.x, bottomRight.y,
                m_selection.GetWidth(), m_selection.GetHeight());
}

// ============================================================================
// EraserTool - Ferramenta Borracha
// ============================================================================

void EraserTool::OnMouseDown(const TilePosition& pos, int tileId, Map* map)
{
    if (!map) return;
    
    m_isErasing = true;
    
    // Apagar tile inicial (usar -1 ou 0 como tile vazio)
    if (map->IsValidPosition(pos.x, pos.y))
    {
        map->SetTile(pos.x, pos.y, -1);
        map->SetModified(true);
        wxLogMessage("Borracha: Apagando tile em (%d, %d)", pos.x, pos.y);
    }
}

void EraserTool::OnMouseMove(const TilePosition& pos, int tileId, Map* map)
{
    if (!map || !m_isErasing) return;
    
    if (map->IsValidPosition(pos.x, pos.y))
    {
        map->SetTile(pos.x, pos.y, -1);
        map->SetModified(true);
    }
}

void EraserTool::OnMouseUp(const TilePosition& pos, int tileId, Map* map)
{
    m_isErasing = false;
    
    if (map)
    {
        wxLogMessage("Borracha: Finalizado");
    }
}