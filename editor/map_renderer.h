/**
 * MapRenderer - Sistema de renderização otimizada de mapas com OpenGL
 * Usa batch rendering para desenhar layers inteiras em um único draw call
 */

#pragma once

#include "gl_buffer.h"
#include "texture_atlas.h"
#include <vector>
#include <memory>

// Forward declarations
class Map;
class Layer;

/**
 * Informações de renderização de uma layer
 */
struct LayerRenderInfo {
    int layerIndex;          // Índice da layer no mapa
    float opacity;           // Opacidade (0.0 - 1.0)
    bool visible;            // Se a layer está visível
    int zOrder;              // Ordem Z para sorting
    
    LayerRenderInfo()
        : layerIndex(0)
        , opacity(1.0f)
        , visible(true)
        , zOrder(0)
    {}
};

/**
 * Viewport/Câmera para frustum culling
 */
struct ViewportCamera {
    float x, y;              // Posição da câmera no mundo (em tiles)
    float width, height;     // Tamanho do viewport (em tiles)
    float zoom;              // Nível de zoom (1.0 = 100%)
    
    ViewportCamera()
        : x(0), y(0)
        , width(20), height(15)
        , zoom(1.0f)
    {}
    
    /**
     * Retorna os bounds visíveis em coordenadas de tile
     */
    void GetVisibleBounds(int& minX, int& minY, int& maxX, int& maxY) const;
};

/**
 * MapRenderer - Renderizador otimizado de mapas
 * 
 * Funcionalidades:
 * - Batch rendering: desenha layer inteira em um draw call
 * - Frustum culling: só renderiza tiles visíveis
 * - Suporte a múltiplas layers com opacidades diferentes
 * - Cache de mesh: reconstrói apenas quando necessário
 * - Otimizado para mapas grandes (100x100+)
 */
class MapRenderer {
public:
    /**
     * Construtor
     */
    MapRenderer();
    
    /**
     * Destrutor
     */
    ~MapRenderer();
    
    // Desabilitar cópia
    MapRenderer(const MapRenderer&) = delete;
    MapRenderer& operator=(const MapRenderer&) = delete;
    
    /**
     * Define o mapa a ser renderizado
     * 
     * @param map Ponteiro para o mapa (não toma ownership)
     */
    void SetMap(Map* map);
    
    /**
     * Define o tileset (TextureAtlas) a ser usado
     * 
     * @param atlas Ponteiro para o atlas (não toma ownership)
     */
    void SetTilesetAtlas(TextureAtlas* atlas);
    
    /**
     * Constrói mesh de renderização para uma layer específica
     * Deve ser chamado quando a layer é modificada
     * 
     * @param layerIndex Índice da layer
     * @param useCamera Se true, aplica frustum culling
     */
    void BuildLayerMesh(int layerIndex, bool useCamera = false);
    
    /**
     * Reconstrói meshes de todas as layers
     * 
     * @param useCamera Se true, aplica frustum culling
     */
    void RebuildAllLayers(bool useCamera = false);
    
    /**
     * Renderiza uma layer específica
     * 
     * @param layerIndex Índice da layer
     */
    void RenderLayer(int layerIndex);
    
    /**
     * Renderiza todas as layers visíveis
     * Usa a ordem Z para sorting
     */
    void RenderAllLayers();
    
    /**
     * Define a câmera/viewport para frustum culling
     */
    void SetCamera(const ViewportCamera& camera);
    
    /**
     * Retorna a câmera atual
     */
    const ViewportCamera& GetCamera() const { return m_camera; }
    
    /**
     * Limpa todos os recursos
     */
    void Clear();
    
    /**
     * Retorna estatísticas de renderização
     */
    struct Stats {
        int totalTiles;        // Total de tiles no mapa
        int visibleTiles;      // Tiles visíveis (após culling)
        int drawnVertices;     // Vértices desenhados
        int drawCalls;         // Número de draw calls
        float lastBuildTime;   // Tempo da última construção (ms)
        
        Stats() : totalTiles(0), visibleTiles(0), drawnVertices(0), 
                  drawCalls(0), lastBuildTime(0.0f) {}
    };
    
    const Stats& GetStats() const { return m_stats; }
    
    /**
     * Habilita/desabilita frustum culling
     */
    void SetFrustumCullingEnabled(bool enabled) { m_useFrustumCulling = enabled; }
    bool IsFrustumCullingEnabled() const { return m_useFrustumCulling; }

private:
    /**
     * Informações de uma layer para renderização
     */
    struct LayerMesh {
        GLBuffer vbo;                    // Vertex Buffer Object
        std::vector<TileVertex> vertices; // Vértices da mesh
        bool dirty;                       // Se precisa rebuild
        
        LayerMesh() : dirty(true) {}
    };
    
    Map* m_map;                          // Mapa atual (não possui)
    TextureAtlas* m_atlas;               // Atlas de texturas (não possui)
    ViewportCamera m_camera;             // Câmera para culling
    bool m_useFrustumCulling;            // Se frustum culling está ativo
    
    std::vector<LayerMesh> m_layerMeshes; // Meshes por layer
    Stats m_stats;                        // Estatísticas
    
    /**
     * Constrói mesh de uma layer com frustum culling opcional
     */
    void BuildLayerMeshInternal(int layerIndex, bool useCamera);
    
    /**
     * Adiciona um quad (2 triângulos) para um tile
     * 
     * @param vertices Vetor de vértices
     * @param x Posição X do tile
     * @param y Posição Y do tile
     * @param tileId ID do tile
     * @param opacity Opacidade
     */
    void AddTileQuad(std::vector<TileVertex>& vertices, 
                     int x, int y, int tileId, float opacity);
    
    /**
     * Verifica se um tile está visível na câmera
     */
    bool IsTileVisible(int x, int y) const;
};
