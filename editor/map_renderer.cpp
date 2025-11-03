/**
 * Implementação de MapRenderer - Renderização otimizada de mapas
 */

#include "map_renderer.h"
#include "map.h"
#include "layer.h"
#include <wx/log.h>
#include <chrono>
#include <algorithm>

// ============================================================================
// ViewportCamera
// ============================================================================

void ViewportCamera::GetVisibleBounds(int& minX, int& minY, int& maxX, int& maxY) const
{
    // Calcular bounds visíveis com margem para evitar pop-in
    const float margin = 2.0f; // Margem de 2 tiles
    
    minX = static_cast<int>(x - margin);
    minY = static_cast<int>(y - margin);
    maxX = static_cast<int>(x + width / zoom + margin);
    maxY = static_cast<int>(y + height / zoom + margin);
    
    // Garantir que não sejam negativos
    minX = std::max(0, minX);
    minY = std::max(0, minY);
}

// ============================================================================
// MapRenderer
// ============================================================================

MapRenderer::MapRenderer()
    : m_map(nullptr)
    , m_atlas(nullptr)
    , m_useFrustumCulling(true)
{
}

MapRenderer::~MapRenderer()
{
    Clear();
}

void MapRenderer::SetMap(Map* map)
{
    if (m_map != map) {
        m_map = map;
        
        if (m_map) {
            // Redimensionar vetor de meshes para corresponder ao número de layers
            int layerCount = m_map->GetLayerCount();
            m_layerMeshes.resize(layerCount);
            
            // Marcar todas as meshes como sujas
            for (auto& mesh : m_layerMeshes) {
                mesh.dirty = true;
            }
        }
        else {
            m_layerMeshes.clear();
        }
    }
}

void MapRenderer::SetTilesetAtlas(TextureAtlas* atlas)
{
    if (m_atlas != atlas) {
        m_atlas = atlas;
        
        // Marcar todas as meshes como sujas quando o atlas muda
        for (auto& mesh : m_layerMeshes) {
            mesh.dirty = true;
        }
    }
}

void MapRenderer::SetCamera(const ViewportCamera& camera)
{
    m_camera = camera;
    
    // Se frustum culling está ativo, marcar meshes como sujas
    // pois os tiles visíveis mudaram
    if (m_useFrustumCulling) {
        for (auto& mesh : m_layerMeshes) {
            mesh.dirty = true;
        }
    }
}

void MapRenderer::BuildLayerMesh(int layerIndex, bool useCamera)
{
    BuildLayerMeshInternal(layerIndex, useCamera || m_useFrustumCulling);
}

void MapRenderer::RebuildAllLayers(bool useCamera)
{
    if (!m_map) return;
    
    bool useCulling = useCamera || m_useFrustumCulling;
    
    for (int i = 0; i < m_map->GetLayerCount(); ++i) {
        BuildLayerMeshInternal(i, useCulling);
    }
}

void MapRenderer::BuildLayerMeshInternal(int layerIndex, bool useCamera)
{
    if (!m_map || !m_atlas) {
        wxLogWarning("MapRenderer: Não é possível construir mesh - mapa ou atlas não definido");
        return;
    }
    
    if (layerIndex < 0 || layerIndex >= static_cast<int>(m_layerMeshes.size())) {
        wxLogWarning("MapRenderer: Índice de layer inválido: %d", layerIndex);
        return;
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    const LayerManager* layerMgr = m_map->GetLayerManager();
    if (!layerMgr || layerIndex >= layerMgr->GetLayerCount()) {
        return;
    }
    
    const Layer* layer = layerMgr->GetLayer(layerIndex);
    if (!layer) {
        return;
    }
    
    LayerMesh& mesh = m_layerMeshes[layerIndex];
    mesh.vertices.clear();
    
    // Obter bounds de renderização
    int minX = 0, minY = 0;
    int maxX = m_map->GetWidth();
    int maxY = m_map->GetHeight();
    
    if (useCamera) {
        m_camera.GetVisibleBounds(minX, minY, maxX, maxY);
        maxX = std::min(maxX, m_map->GetWidth());
        maxY = std::min(maxY, m_map->GetHeight());
    }
    
    // Contar tiles e reservar espaço
    int tileCount = 0;
    for (int y = minY; y < maxY; ++y) {
        for (int x = minX; x < maxX; ++x) {
            int tileId = layer->GetTile(x, y);
            if (tileId > 0) { // Tile 0 ou negativo = vazio
                tileCount++;
            }
        }
    }
    
    // Cada tile = 6 vértices (2 triângulos)
    mesh.vertices.reserve(tileCount * 6);
    
    // Construir mesh
    float opacity = layer->GetOpacity();
    int visibleTiles = 0;
    
    for (int y = minY; y < maxY; ++y) {
        for (int x = minX; x < maxX; ++x) {
            int tileId = layer->GetTile(x, y);
            
            if (tileId > 0) {
                AddTileQuad(mesh.vertices, x, y, tileId, opacity);
                visibleTiles++;
            }
        }
    }
    
    // Upload para GPU
    if (!mesh.vertices.empty()) {
        mesh.vbo.UploadVertices(mesh.vertices, BufferUsage::Static);
    }
    
    mesh.dirty = false;
    
    // Atualizar estatísticas
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    m_stats.lastBuildTime = duration.count() / 1000.0f; // Converter para ms
    m_stats.visibleTiles = visibleTiles;
    m_stats.totalTiles = m_map->GetWidth() * m_map->GetHeight();
    
    wxLogMessage("MapRenderer: Layer %d reconstruída - %d tiles visíveis em %.2f ms",
                 layerIndex, visibleTiles, m_stats.lastBuildTime);
}

void MapRenderer::AddTileQuad(std::vector<TileVertex>& vertices, 
                               int x, int y, int tileId, float opacity)
{
    if (!m_atlas) return;
    
    // Obter UVs do atlas
    TileUV uv = m_atlas->GetTileUV(tileId);
    
    // Tamanho do tile em unidades do mundo
    const float tileSize = static_cast<float>(m_map->GetTileSize());
    
    // Posição do quad no mundo
    float x0 = x * tileSize;
    float y0 = y * tileSize;
    float x1 = x0 + tileSize;
    float y1 = y0 + tileSize;
    
    // Z = 0 por enquanto (pode ser usado para layering futuro)
    const float z = 0.0f;
    
    // Cor com opacidade
    const float r = 1.0f;
    const float g = 1.0f;
    const float b = 1.0f;
    const float a = opacity;
    
    // Criar 2 triângulos (6 vértices)
    // Triângulo 1: top-left, bottom-left, bottom-right
    vertices.emplace_back(x0, y0, z, uv.u0, uv.v0, r, g, b, a); // Top-left
    vertices.emplace_back(x0, y1, z, uv.u0, uv.v1, r, g, b, a); // Bottom-left
    vertices.emplace_back(x1, y1, z, uv.u1, uv.v1, r, g, b, a); // Bottom-right
    
    // Triângulo 2: top-left, bottom-right, top-right
    vertices.emplace_back(x0, y0, z, uv.u0, uv.v0, r, g, b, a); // Top-left
    vertices.emplace_back(x1, y1, z, uv.u1, uv.v1, r, g, b, a); // Bottom-right
    vertices.emplace_back(x1, y0, z, uv.u1, uv.v0, r, g, b, a); // Top-right
}

void MapRenderer::RenderLayer(int layerIndex)
{
    if (!m_map || !m_atlas) return;
    
    if (layerIndex < 0 || layerIndex >= static_cast<int>(m_layerMeshes.size())) {
        return;
    }
    
    const LayerManager* layerMgr = m_map->GetLayerManager();
    if (!layerMgr) return;
    
    const Layer* layer = layerMgr->GetLayer(layerIndex);
    if (!layer || !layer->IsVisible()) {
        return;
    }
    
    LayerMesh& mesh = m_layerMeshes[layerIndex];
    
    // Reconstruir se necessário
    if (mesh.dirty) {
        BuildLayerMesh(layerIndex);
    }
    
    // Renderizar se há vértices
    if (!mesh.vertices.empty() && mesh.vbo.IsValid()) {
        // Bind da textura
        m_atlas->Bind();
        
        // Bind do VBO
        mesh.vbo.Bind();
        
        // Configurar atributos de vértice
        GLBuffer::SetupVertexAttributes();
        
        // Draw call
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(mesh.vbo.GetVertexCount()));
        
        // Unbind
        mesh.vbo.Unbind();
        m_atlas->Unbind();
        
        m_stats.drawCalls++;
        m_stats.drawnVertices += static_cast<int>(mesh.vertices.size());
    }
}

void MapRenderer::RenderAllLayers()
{
    if (!m_map) return;
    
    // Reset de estatísticas
    m_stats.drawCalls = 0;
    m_stats.drawnVertices = 0;
    
    const LayerManager* layerMgr = m_map->GetLayerManager();
    if (!layerMgr) return;
    
    // Coletar informações de layers visíveis
    std::vector<LayerRenderInfo> renderInfos;
    for (int i = 0; i < layerMgr->GetLayerCount(); ++i) {
        const Layer* layer = layerMgr->GetLayer(i);
        if (layer && layer->IsVisible()) {
            LayerRenderInfo info;
            info.layerIndex = i;
            info.opacity = layer->GetOpacity();
            info.visible = layer->IsVisible();
            info.zOrder = layer->GetZOrder();
            renderInfos.push_back(info);
        }
    }
    
    // Ordenar por Z-order (menor primeiro = mais atrás)
    std::sort(renderInfos.begin(), renderInfos.end(),
              [](const LayerRenderInfo& a, const LayerRenderInfo& b) {
                  return a.zOrder < b.zOrder;
              });
    
    // Renderizar layers na ordem
    for (const auto& info : renderInfos) {
        RenderLayer(info.layerIndex);
    }
}

bool MapRenderer::IsTileVisible(int x, int y) const
{
    if (!m_useFrustumCulling) {
        return true;
    }
    
    int minX, minY, maxX, maxY;
    m_camera.GetVisibleBounds(minX, minY, maxX, maxY);
    
    return x >= minX && x < maxX && y >= minY && y < maxY;
}

void MapRenderer::Clear()
{
    m_layerMeshes.clear();
    m_map = nullptr;
    m_atlas = nullptr;
    m_stats = Stats();
}
