/**
 * Implementação do CollisionOverlay
 */

#include "collision_overlay.h"
#include "map.h"
#include "tileset_manager.h"
#include "layer.h"
#include <wx/log.h>
#include <algorithm>

// ============================================================================
// CollisionOverlay
// ============================================================================

CollisionOverlay::CollisionOverlay()
    : m_needsRebuild(true)
    , m_tileSize(32.0f)
{
}

CollisionOverlay::~CollisionOverlay()
{
    Clear();
}

void CollisionOverlay::SetConfig(const CollisionOverlayConfig& config)
{
    m_config = config;
    m_needsRebuild = true;
}

void CollisionOverlay::SetOpacity(float opacity)
{
    // Clampar entre 0.0 e 1.0
    opacity = std::max(0.0f, std::min(1.0f, opacity));
    
    if (std::abs(m_config.opacity - opacity) > 0.01f) {
        m_config.opacity = opacity;
        m_needsRebuild = true; // Precisa rebuild porque a cor dos vértices muda
    }
}

void CollisionOverlay::SetColor(const wxColour& color)
{
    if (m_config.color != color) {
        m_config.color = color;
        m_needsRebuild = true;
    }
}

void CollisionOverlay::BuildFromMap(const Map* map, const TilesetManager* tilesets)
{
    m_vertices.clear();
    
    if (!map || !tilesets) {
        wxLogWarning("CollisionOverlay: Mapa ou tilesets inválidos");
        return;
    }
    
    m_tileSize = static_cast<float>(map->GetTileSize());
    
    int collisionCount = 0;
    
    // Iterar por todas as layers do mapa
    const LayerManager* layerMgr = map->GetLayerManager();
    if (!layerMgr) {
        return;
    }
    
    for (int layerIdx = 0; layerIdx < layerMgr->GetLayerCount(); ++layerIdx) {
        const Layer* layer = layerMgr->GetLayer(layerIdx);
        if (!layer || !layer->IsVisible()) {
            continue;
        }
        
        // Iterar por todos os tiles da layer
        for (int y = 0; y < map->GetHeight(); ++y) {
            for (int x = 0; x < map->GetWidth(); ++x) {
                int tileId = layer->GetTile(x, y);
                
                // Verificar se tem colisão
                if (tileId > 0 && HasCollision(tileId, tilesets)) {
                    AddCollisionQuad(x, y);
                    collisionCount++;
                }
            }
        }
    }
    
    // Upload para GPU
    if (!m_vertices.empty()) {
        m_vbo.Upload(m_vertices.data(), 
                     m_vertices.size() * sizeof(OverlayVertex), 
                     BufferUsage::Static);
    }
    
    // Atualizar estatísticas
    m_stats.collisionTiles = collisionCount;
    m_stats.vertexCount = static_cast<int>(m_vertices.size());
    m_stats.needsRebuild = false;
    m_needsRebuild = false;
    
    wxLogMessage("CollisionOverlay: %d tiles com colisão detectados", collisionCount);
}

void CollisionOverlay::AddCollisionQuad(int x, int y)
{
    // Calcular posição do quad no mundo
    float x0 = x * m_tileSize;
    float y0 = y * m_tileSize;
    float x1 = x0 + m_tileSize;
    float y1 = y0 + m_tileSize;
    
    const float z = 0.1f; // Ligeiramente acima dos tiles para evitar z-fighting
    
    // Cor normalizada com opacidade
    float r = m_config.color.Red() / 255.0f;
    float g = m_config.color.Green() / 255.0f;
    float b = m_config.color.Blue() / 255.0f;
    float a = m_config.opacity;
    
    // Criar 2 triângulos (6 vértices) para o quad
    // Triângulo 1: top-left, bottom-left, bottom-right
    m_vertices.emplace_back(x0, y0, z, r, g, b, a); // Top-left
    m_vertices.emplace_back(x0, y1, z, r, g, b, a); // Bottom-left
    m_vertices.emplace_back(x1, y1, z, r, g, b, a); // Bottom-right
    
    // Triângulo 2: top-left, bottom-right, top-right
    m_vertices.emplace_back(x0, y0, z, r, g, b, a); // Top-left
    m_vertices.emplace_back(x1, y1, z, r, g, b, a); // Bottom-right
    m_vertices.emplace_back(x1, y0, z, r, g, b, a); // Top-right
}

bool CollisionOverlay::HasCollision(int tileId, const TilesetManager* tilesets) const
{
    if (!tilesets || tileId <= 0) {
        return false;
    }
    
    const TileInfo* tileInfo = tilesets->GetTileInfo(tileId);
    if (tileInfo) {
        return tileInfo->hasCollision;
    }
    
    return false;
}

void CollisionOverlay::Render()
{
    if (!m_config.enabled) {
        return;
    }
    
    // Reconstruir se necessário
    if (m_needsRebuild) {
        // Nota: BuildFromMap precisa ser chamado externamente
        // pois precisamos do Map e TilesetManager
        wxLogWarning("CollisionOverlay: Precisa rebuild mas não pode ser feito automaticamente");
        return;
    }
    
    if (m_vertices.empty() || !m_vbo.IsValid()) {
        return;
    }
    
    // Habilitar blending para transparência
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Bind do VBO
    m_vbo.Bind();
    
    // Configurar atributos de vértice
    // Layout: posição (x,y,z) + cor (r,g,b,a)
    const size_t stride = sizeof(OverlayVertex);
    
    // Location 0: posição (x, y, z)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    
    // Location 1: cor (r, g, b, a)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    
    // Renderizar triângulos
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_vertices.size()));
    
    // Cleanup
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    m_vbo.Unbind();
}

void CollisionOverlay::Clear()
{
    m_vertices.clear();
    m_vbo.Destroy();
    m_stats = Stats();
    m_needsRebuild = true;
}
