/**
 * TilesetManager - Classe para gerenciar tilesets carregados de imagens
 */

#pragma once

#include <wx/wx.h>
#include <wx/image.h>
#include <vector>
#include <memory>
#include <map>

struct TileInfo {
    int id;
    wxString name;
    wxString category;
    wxImage image;
    bool hasCollision;
    bool isAnimated;
    wxString soundEffect;
    
    TileInfo() 
        : id(-1), hasCollision(false), isAnimated(false) {}
        
    TileInfo(int tileId, const wxImage& tileImage, const wxString& tileName = "")
        : id(tileId), image(tileImage), name(tileName), hasCollision(false), isAnimated(false) {}
};

struct TilesetInfo {
    wxString name;
    wxString filepath;
    wxSize tileSize;
    int tilesPerRow;
    int totalTiles;
    std::vector<TileInfo> tiles;
    
    TilesetInfo() 
        : tileSize(32, 32), tilesPerRow(0), totalTiles(0) {}
};

class TilesetManager {
public:
    TilesetManager();
    ~TilesetManager() = default;
    
    // Carregamento de tilesets
    bool LoadTilesetFromFile(const wxString& filepath, const wxSize& tileSize = wxSize(32, 32));
    bool LoadDefaultTileset(); // Carregar tileset padrão (cores sólidas)
    
    // Gerenciamento de tilesets
    bool AddTileset(const TilesetInfo& tilesetInfo);
    bool RemoveTileset(const wxString& name);
    void ClearTilesets();
    
    // Acesso aos dados
    const std::vector<TilesetInfo>& GetTilesets() const { return m_tilesets; }
    const TilesetInfo* GetCurrentTileset() const;
    const TilesetInfo* GetTileset(const wxString& name) const;
    TileInfo* GetTileInfo(int tileId);
    const TileInfo* GetTileInfo(int tileId) const;
    
    // Seleção de tileset ativo
    bool SetCurrentTileset(const wxString& name);
    wxString GetCurrentTilesetName() const { return m_currentTilesetName; }
    
    // Utilitários
    int GetTileCount() const;
    wxImage GetTileImage(int tileId) const;
    wxString GetTileName(int tileId) const;
    
    // Configuração de propriedades dos tiles
    void SetTileProperty(int tileId, const wxString& property, const wxVariant& value);
    wxVariant GetTileProperty(int tileId, const wxString& property) const;
    
    // Serialização
    bool SaveToProject(const wxString& projectPath) const;
    bool LoadFromProject(const wxString& projectPath);
    
    // Debug
    void PrintDebugInfo() const;

private:
    // Métodos auxiliares
    bool SplitImageIntoTiles(const wxImage& sourceImage, const wxSize& tileSize, 
                           std::vector<TileInfo>& tiles);
    wxImage CreateDefaultTileImage(int tileId) const;
    wxColour GetDefaultTileColor(int tileId) const;
    
    // Dados
    std::vector<TilesetInfo> m_tilesets;
    wxString m_currentTilesetName;
    
    // Cache para performance
    mutable std::map<int, wxImage> m_tileImageCache;
};