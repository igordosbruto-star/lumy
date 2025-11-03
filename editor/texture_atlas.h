/**
 * TextureAtlas - Sistema de gerenciamento de texturas de tilesets
 * Carrega tilesets como texturas OpenGL e calcula UVs automaticamente
 */

#pragma once

#include <GL/glew.h>
#include <wx/wx.h>
#include <wx/image.h>
#include <unordered_map>
#include <string>
#include <memory>

/**
 * Coordenadas UV normalizadas para um tile específico
 */
struct TileUV {
    float u0, v0; // Canto superior esquerdo
    float u1, v1; // Canto inferior direito
    
    TileUV() : u0(0), v0(0), u1(1), v1(1) {}
    TileUV(float _u0, float _v0, float _u1, float _v1)
        : u0(_u0), v0(_v0), u1(_u1), v1(_v1) {}
};

/**
 * Informações sobre um tileset carregado
 */
struct TilesetInfo {
    wxString filePath;       // Caminho do arquivo de imagem
    int textureWidth;        // Largura da textura em pixels
    int textureHeight;       // Altura da textura em pixels
    int tileWidth;           // Largura de um tile em pixels
    int tileHeight;          // Altura de um tile em pixels
    int tilesPerRow;         // Número de tiles por linha
    int tilesPerColumn;      // Número de tiles por coluna
    int totalTiles;          // Total de tiles no tileset
    
    TilesetInfo()
        : textureWidth(0), textureHeight(0)
        , tileWidth(32), tileHeight(32)
        , tilesPerRow(0), tilesPerColumn(0)
        , totalTiles(0)
    {}
};

/**
 * TextureAtlas - Gerencia texturas de tilesets com cache
 * 
 * Responsável por:
 * - Carregar imagens de tilesets como texturas OpenGL
 * - Calcular coordenadas UV para cada tile automaticamente
 * - Manter cache de texturas para evitar recarregamentos
 * - Gerenciar vida útil das texturas OpenGL (RAII)
 */
class TextureAtlas {
public:
    /**
     * Construtor padrão
     */
    TextureAtlas();
    
    /**
     * Destrutor - libera todas as texturas
     */
    ~TextureAtlas();
    
    // Desabilitar cópia
    TextureAtlas(const TextureAtlas&) = delete;
    TextureAtlas& operator=(const TextureAtlas&) = delete;
    
    // Permitir move
    TextureAtlas(TextureAtlas&& other) noexcept;
    TextureAtlas& operator=(TextureAtlas&& other) noexcept;
    
    /**
     * Carrega um tileset a partir de um arquivo de imagem
     * 
     * @param filePath Caminho para o arquivo de imagem (PNG, BMP, etc.)
     * @param tileWidth Largura de cada tile em pixels
     * @param tileHeight Altura de cada tile em pixels
     * @return true se carregado com sucesso
     */
    bool LoadTileset(const wxString& filePath, int tileWidth = 32, int tileHeight = 32);
    
    /**
     * Descarrega o tileset e libera a textura OpenGL
     */
    void Unload();
    
    /**
     * Verifica se há um tileset carregado
     */
    bool IsLoaded() const { return m_textureId != 0; }
    
    /**
     * Faz bind da textura OpenGL
     */
    void Bind() const;
    
    /**
     * Unbind da textura
     */
    void Unbind() const;
    
    /**
     * Retorna o ID da textura OpenGL
     */
    GLuint GetTextureID() const { return m_textureId; }
    
    /**
     * Retorna as coordenadas UV para um tile específico
     * 
     * @param tileId ID do tile (começando em 0, linha por linha da esquerda para direita)
     * @return Coordenadas UV normalizadas (0.0 - 1.0)
     */
    TileUV GetTileUV(int tileId) const;
    
    /**
     * Retorna as coordenadas UV para um tile em posição (x, y) no grid
     * 
     * @param tileX Posição X no grid de tiles (0 = primeira coluna)
     * @param tileY Posição Y no grid de tiles (0 = primeira linha)
     * @return Coordenadas UV normalizadas
     */
    TileUV GetTileUVByPosition(int tileX, int tileY) const;
    
    /**
     * Retorna informações sobre o tileset carregado
     */
    const TilesetInfo& GetInfo() const { return m_info; }
    
    /**
     * Retorna o número total de tiles no tileset
     */
    int GetTileCount() const { return m_info.totalTiles; }
    
    /**
     * Verifica se um tileId é válido
     */
    bool IsValidTileId(int tileId) const {
        return tileId >= 0 && tileId < m_info.totalTiles;
    }

private:
    GLuint m_textureId;           // ID da textura OpenGL
    TilesetInfo m_info;           // Informações do tileset
    
    // Cache de UVs calculados (opcional, para otimização)
    mutable std::unordered_map<int, TileUV> m_uvCache;
    
    /**
     * Cria textura OpenGL a partir de wxImage
     * 
     * @param image Imagem carregada do disco
     * @return ID da textura criada (0 se falhar)
     */
    GLuint CreateTextureFromImage(const wxImage& image);
    
    /**
     * Calcula coordenadas UV para um tile específico
     * Método interno usado por GetTileUV
     */
    TileUV CalculateTileUV(int tileId) const;
};

/**
 * TextureAtlasManager - Gerencia múltiplos tilesets com cache global
 * 
 * Permite carregar múltiplos tilesets e reutilizá-los entre diferentes
 * instâncias do editor sem recarregar do disco.
 */
class TextureAtlasManager {
public:
    /**
     * Retorna instância singleton
     */
    static TextureAtlasManager& Get();
    
    /**
     * Carrega um tileset (usa cache se já carregado)
     * 
     * @param filePath Caminho do arquivo
     * @param tileWidth Largura do tile
     * @param tileHeight Altura do tile
     * @return Ponteiro para o atlas (nullptr se falhar)
     */
    TextureAtlas* LoadTileset(const wxString& filePath, int tileWidth = 32, int tileHeight = 32);
    
    /**
     * Obtém um tileset do cache
     * 
     * @param filePath Caminho do arquivo
     * @return Ponteiro para o atlas ou nullptr se não estiver carregado
     */
    TextureAtlas* GetTileset(const wxString& filePath);
    
    /**
     * Remove um tileset do cache
     */
    void UnloadTileset(const wxString& filePath);
    
    /**
     * Remove todos os tilesets do cache
     */
    void UnloadAll();
    
    /**
     * Retorna o número de tilesets carregados
     */
    size_t GetLoadedCount() const { return m_atlases.size(); }

private:
    TextureAtlasManager() = default;
    ~TextureAtlasManager();
    
    // Desabilitar cópia e move
    TextureAtlasManager(const TextureAtlasManager&) = delete;
    TextureAtlasManager& operator=(const TextureAtlasManager&) = delete;
    
    // Cache de tilesets: filepath -> atlas
    std::unordered_map<std::string, std::unique_ptr<TextureAtlas>> m_atlases;
};
