/**
 * Implementação do GridRenderer - Grid otimizado e adaptativo
 */

#include "grid_renderer.h"
#include <wx/log.h>
#include <cmath>
#include <algorithm>

// ============================================================================
// GridRenderer
// ============================================================================

GridRenderer::GridRenderer()
    : m_gridWidth(25)
    , m_gridHeight(15)
    , m_zoom(1.0f)
    , m_needsRebuild(true)
    , m_cachedZoom(0.0f)
    , m_cachedSpacing(0)
{
}

GridRenderer::~GridRenderer()
{
    Clear();
}

void GridRenderer::SetGridSize(int width, int height)
{
    if (m_gridWidth != width || m_gridHeight != height) {
        m_gridWidth = width;
        m_gridHeight = height;
        m_needsRebuild = true;
    }
}

void GridRenderer::SetConfig(const GridConfig& config)
{
    m_config = config;
    m_needsRebuild = true;
}

void GridRenderer::SetZoom(float zoom)
{
    // Clampar zoom a valores razoáveis
    zoom = std::max(0.1f, std::min(10.0f, zoom));
    
    if (std::abs(m_zoom - zoom) > 0.01f) { // Threshold para evitar rebuilds desnecessários
        m_zoom = zoom;
        
        // Se o espaçamento mudou, marcar para rebuild
        if (m_config.adaptive) {
            int newSpacing = GetGridSpacing();
            if (newSpacing != m_cachedSpacing) {
                m_needsRebuild = true;
            }
        }
    }
}

int GridRenderer::GetGridSpacing() const
{
    if (!m_config.adaptive) {
        return 1; // Grid normal: linha a cada tile
    }
    
    // Grid adaptativo baseado no zoom
    // Zoom < 0.5x: grid a cada 4 tiles
    // Zoom < 1.0x: grid a cada 2 tiles
    // Zoom >= 1.0x: grid normal (cada tile)
    
    if (m_zoom < 0.5f) {
        return 4;
    }
    else if (m_zoom < 1.0f) {
        return 2;
    }
    else {
        return 1;
    }
}

float GridRenderer::GetLineWidthForZoom() const
{
    // Aumentar espessura em zoom in, reduzir em zoom out
    float width = m_config.lineWidth;
    
    if (m_zoom > 2.0f) {
        width *= 1.5f; // Linhas mais grossas em zoom alto
    }
    else if (m_zoom < 0.5f) {
        width *= 0.75f; // Linhas mais finas em zoom baixo
    }
    
    return std::max(0.5f, std::min(3.0f, width));
}

void GridRenderer::SetAdaptive(bool adaptive)
{
    if (m_config.adaptive != adaptive) {
        m_config.adaptive = adaptive;
        m_needsRebuild = true;
    }
}

void GridRenderer::RebuildGrid()
{
    m_vertices.clear();
    
    if (m_gridWidth <= 0 || m_gridHeight <= 0) {
        return;
    }
    
    int spacing = GetGridSpacing();
    const float tileSize = m_config.tileSize;
    
    // Calcular limites do grid
    const float maxX = m_gridWidth * tileSize;
    const float maxY = m_gridHeight * tileSize;
    
    // Reservar espaço (estimativa: 2 vértices por linha)
    int estimatedLines = (m_gridWidth / spacing + 1) + (m_gridHeight / spacing + 1);
    m_vertices.reserve(estimatedLines * 2);
    
    // Linhas verticais
    for (int x = 0; x <= m_gridWidth; x += spacing) {
        float xPos = x * tileSize;
        AddLine(xPos, 0.0f, xPos, maxY);
    }
    
    // Linhas horizontais
    for (int y = 0; y <= m_gridHeight; y += spacing) {
        float yPos = y * tileSize;
        AddLine(0.0f, yPos, maxX, yPos);
    }
    
    // Upload para GPU
    if (!m_vertices.empty()) {
        // Criar estrutura compatível com GLBuffer
        // GridVertex tem layout: x, y, z, r, g, b, a (7 floats = 28 bytes)
        m_vbo.Upload(m_vertices.data(), 
                     m_vertices.size() * sizeof(GridVertex), 
                     BufferUsage::Static);
    }
    
    // Atualizar estatísticas
    m_stats.vertexCount = static_cast<int>(m_vertices.size());
    m_stats.lineCount = m_stats.vertexCount / 2;
    m_stats.spacing = spacing;
    m_stats.needsRebuild = false;
    
    // Cachear valores
    m_cachedZoom = m_zoom;
    m_cachedSpacing = spacing;
    m_needsRebuild = false;
    
    wxLogMessage("GridRenderer: Grid reconstruído - %d linhas, espaçamento: %d tiles, zoom: %.2f",
                 m_stats.lineCount, spacing, m_zoom);
}

void GridRenderer::AddLine(float x0, float y0, float x1, float y1)
{
    const float z = 0.0f; // Grid no plano z=0
    
    const float r = m_config.color[0];
    const float g = m_config.color[1];
    const float b = m_config.color[2];
    const float a = m_config.color[3];
    
    // Adicionar 2 vértices (início e fim da linha)
    m_vertices.emplace_back(x0, y0, z, r, g, b, a);
    m_vertices.emplace_back(x1, y1, z, r, g, b, a);
}

void GridRenderer::Render()
{
    if (!m_config.enabled) {
        return;
    }
    
    // Reconstruir se necessário
    if (m_needsRebuild) {
        RebuildGrid();
    }
    
    if (m_vertices.empty() || !m_vbo.IsValid()) {
        return;
    }
    
    // Configurar OpenGL para linhas
    float lineWidth = GetLineWidthForZoom();
    glLineWidth(lineWidth);
    
    // Habilitar anti-aliasing para linhas (opcional, pode ter custo de performance)
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    
    // Bind do VBO
    m_vbo.Bind();
    
    // Configurar atributos de vértice
    // Layout: posição (x,y,z) + cor (r,g,b,a)
    const size_t stride = sizeof(GridVertex);
    
    // Location 0: posição (x, y, z)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    
    // Location 1: cor (r, g, b, a)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    
    // Renderizar linhas
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_vertices.size()));
    
    // Cleanup
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    m_vbo.Unbind();
    
    glDisable(GL_LINE_SMOOTH);
    glLineWidth(1.0f); // Reset
}

void GridRenderer::Clear()
{
    m_vertices.clear();
    m_vbo.Destroy();
    m_stats = Stats();
    m_needsRebuild = true;
}
