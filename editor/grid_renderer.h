/**
 * GridRenderer - Sistema de renderização otimizada de grid com OpenGL
 * Usa VBOs e line strips para performance, com grid adaptativo baseado em zoom
 */

#pragma once

#include "gl_buffer.h"
#include "shader_program.h"
#include <vector>

/**
 * Configuração do grid
 */
struct GridConfig {
    float tileSize;          // Tamanho do tile em pixels
    float lineWidth;         // Espessura da linha (1.0 = default)
    float color[4];          // Cor RGBA (0.0 - 1.0)
    bool enabled;            // Se o grid está ativo
    bool adaptive;           // Se usa espaçamento adaptativo baseado no zoom
    
    GridConfig()
        : tileSize(32.0f)
        , lineWidth(1.0f)
        , color{0.5f, 0.5f, 0.5f, 0.5f} // Cinza 50% transparente
        , enabled(true)
        , adaptive(true)
    {}
};

/**
 * Vértice simples para linhas do grid
 */
struct GridVertex {
    float x, y, z;     // Posição
    float r, g, b, a;  // Cor
    
    GridVertex() : x(0), y(0), z(0), r(1), g(1), b(1), a(1) {}
    
    GridVertex(float px, float py, float pz, float cr, float cg, float cb, float ca)
        : x(px), y(py), z(pz), r(cr), g(cg), b(cb), a(ca) {}
};

/**
 * GridRenderer - Renderizador otimizado de grid
 * 
 * Funcionalidades:
 * - Usa VBOs para performance
 * - Line strips para reduzir vértices
 * - Cache inteligente: reconstrói apenas quando necessário
 * - Grid adaptativo: ajusta densidade baseado no zoom
 * - Anti-aliasing via GL_LINE_SMOOTH
 */
class GridRenderer {
public:
    /**
     * Construtor
     */
    GridRenderer();
    
    /**
     * Destrutor
     */
    ~GridRenderer();
    
    // Desabilitar cópia
    GridRenderer(const GridRenderer&) = delete;
    GridRenderer& operator=(const GridRenderer&) = delete;
    
    /**
     * Define a área do grid (em tiles)
     * 
     * @param width Largura em tiles
     * @param height Altura em tiles
     */
    void SetGridSize(int width, int height);
    
    /**
     * Define configuração do grid
     */
    void SetConfig(const GridConfig& config);
    
    /**
     * Retorna configuração atual
     */
    const GridConfig& GetConfig() const { return m_config; }
    
    /**
     * Define nível de zoom para grid adaptativo
     * 
     * @param zoom Fator de zoom (1.0 = 100%, 0.5 = 50%, 2.0 = 200%)
     */
    void SetZoom(float zoom);
    
    /**
     * Retorna zoom atual
     */
    float GetZoom() const { return m_zoom; }
    
    /**
     * Calcula espaçamento do grid baseado no zoom
     * Grid adaptativo: reduz densidade em zoom out
     * 
     * @return Número de tiles entre linhas do grid
     */
    int GetGridSpacing() const;
    
    /**
     * Constrói mesh do grid
     * Deve ser chamado quando tamanho, zoom ou config mudam
     */
    void RebuildGrid();
    
    /**
     * Renderiza o grid
     * Requer que o shader esteja ativo
     */
    void Render();
    
    /**
     * Habilita/desabilita o grid
     */
    void SetEnabled(bool enabled) { m_config.enabled = enabled; }
    bool IsEnabled() const { return m_config.enabled; }
    
    /**
     * Habilita/desabilita grid adaptativo
     */
    void SetAdaptive(bool adaptive);
    bool IsAdaptive() const { return m_config.adaptive; }
    
    /**
     * Limpa recursos
     */
    void Clear();
    
    /**
     * Retorna estatísticas
     */
    struct Stats {
        int vertexCount;     // Vértices do grid
        int lineCount;       // Número de linhas
        int spacing;         // Espaçamento atual (em tiles)
        bool needsRebuild;   // Se precisa reconstruir
        
        Stats() : vertexCount(0), lineCount(0), spacing(1), needsRebuild(true) {}
    };
    
    const Stats& GetStats() const { return m_stats; }

private:
    GridConfig m_config;              // Configuração
    int m_gridWidth;                  // Largura em tiles
    int m_gridHeight;                 // Altura em tiles
    float m_zoom;                     // Nível de zoom atual
    
    GLBuffer m_vbo;                   // VBO para linhas
    std::vector<GridVertex> m_vertices; // Vértices do grid
    
    Stats m_stats;                    // Estatísticas
    bool m_needsRebuild;              // Flag de reconstrução
    
    // Cache para evitar rebuilds desnecessários
    float m_cachedZoom;
    int m_cachedSpacing;
    
    /**
     * Calcula espessura da linha baseada no zoom
     */
    float GetLineWidthForZoom() const;
    
    /**
     * Adiciona uma linha ao grid (2 vértices)
     */
    void AddLine(float x0, float y0, float x1, float y1);
};
