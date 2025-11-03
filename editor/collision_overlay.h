/**
 * CollisionOverlay - Sistema de visualização de tiles com colisão
 * Renderiza overlay transparente sobre tiles colidíveis
 */

#pragma once

#include "gl_buffer.h"
#include "shader_program.h"
#include <vector>
#include <wx/colour.h>

// Forward declarations
class Map;
class TilesetManager;

/**
 * Configuração do overlay de colisão
 */
struct CollisionOverlayConfig {
    wxColour color;          // Cor do overlay (padrão: vermelho)
    float opacity;           // Opacidade (0.0 - 1.0)
    bool enabled;            // Se o overlay está ativo
    bool showOnlyCollision;  // Se mostra apenas colisão (ou também regiões/terreno)
    
    CollisionOverlayConfig()
        : color(255, 0, 0, 128) // Vermelho semi-transparente
        , opacity(0.5f)
        , enabled(false)
        , showOnlyCollision(true)
    {}
};

/**
 * Vértice para renderização de overlay
 */
struct OverlayVertex {
    float x, y, z;     // Posição
    float r, g, b, a;  // Cor RGBA
    
    OverlayVertex() : x(0), y(0), z(0), r(1), g(0), b(0), a(0.5f) {}
    
    OverlayVertex(float px, float py, float pz, float cr, float cg, float cb, float ca)
        : x(px), y(py), z(pz), r(cr), g(cg), b(cb), a(ca) {}
};

/**
 * CollisionOverlay - Renderizador de overlay de colisão
 * 
 * Funcionalidades:
 * - Detecta tiles com propriedade de colisão do TilesetManager
 * - Renderiza quad transparente sobre cada tile colidível
 * - Usa shader de overlay para cor sólida com alpha
 * - Cache: reconstrói apenas quando mapa ou tilesets mudam
 * - Opacidade configurável via slider
 */
class CollisionOverlay {
public:
    /**
     * Construtor
     */
    CollisionOverlay();
    
    /**
     * Destrutor
     */
    ~CollisionOverlay();
    
    // Desabilitar cópia
    CollisionOverlay(const CollisionOverlay&) = delete;
    CollisionOverlay& operator=(const CollisionOverlay&) = delete;
    
    /**
     * Constrói overlay a partir do mapa e tileset manager
     * 
     * @param map Ponteiro para o mapa
     * @param tilesets Ponteiro para o tileset manager (para obter propriedades de colisão)
     */
    void BuildFromMap(const Map* map, const TilesetManager* tilesets);
    
    /**
     * Renderiza o overlay de colisão
     * Requer que o shader esteja ativo
     */
    void Render();
    
    /**
     * Define configuração do overlay
     */
    void SetConfig(const CollisionOverlayConfig& config);
    
    /**
     * Retorna configuração atual
     */
    const CollisionOverlayConfig& GetConfig() const { return m_config; }
    
    /**
     * Define opacidade do overlay (0.0 - 1.0)
     */
    void SetOpacity(float opacity);
    
    /**
     * Retorna opacidade atual
     */
    float GetOpacity() const { return m_config.opacity; }
    
    /**
     * Habilita/desabilita o overlay
     */
    void SetEnabled(bool enabled) { m_config.enabled = enabled; }
    bool IsEnabled() const { return m_config.enabled; }
    
    /**
     * Define cor do overlay
     */
    void SetColor(const wxColour& color);
    
    /**
     * Retorna cor atual
     */
    wxColour GetColor() const { return m_config.color; }
    
    /**
     * Marca overlay como dirty (precisa rebuild)
     */
    void MarkDirty() { m_needsRebuild = true; }
    
    /**
     * Limpa recursos
     */
    void Clear();
    
    /**
     * Retorna estatísticas
     */
    struct Stats {
        int collisionTiles;  // Número de tiles com colisão
        int vertexCount;     // Vértices renderizados
        bool needsRebuild;   // Se precisa reconstruir
        
        Stats() : collisionTiles(0), vertexCount(0), needsRebuild(true) {}
    };
    
    const Stats& GetStats() const { return m_stats; }

private:
    CollisionOverlayConfig m_config;      // Configuração
    
    GLBuffer m_vbo;                       // VBO para quads
    std::vector<OverlayVertex> m_vertices; // Vértices do overlay
    
    Stats m_stats;                        // Estatísticas
    bool m_needsRebuild;                  // Flag de reconstrução
    
    float m_tileSize;                     // Tamanho do tile (cache)
    
    /**
     * Adiciona um quad para um tile com colisão
     * 
     * @param x Posição X do tile (em tiles)
     * @param y Posição Y do tile (em tiles)
     */
    void AddCollisionQuad(int x, int y);
    
    /**
     * Verifica se um tile tem colisão
     * 
     * @param tileId ID do tile
     * @param tilesets Tileset manager para consultar propriedades
     * @return true se o tile tem colisão
     */
    bool HasCollision(int tileId, const TilesetManager* tilesets) const;
};
