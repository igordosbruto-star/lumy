/**
 * Map - Classe para representar um mapa do Lumy Editor
 */

#pragma once

#include <wx/wx.h>
#include <vector>
#include <string>
#include <memory>
#include "layer_manager.h"

struct MapMetadata
{
    wxString name;
    wxString author;
    wxString description;
    wxString version;
    int width;
    int height;
    int tileSize;
    wxString createdDate;
    wxString modifiedDate;
    
    MapMetadata()
        : name("Novo Mapa")
        , author("Usuário")
        , description("")
        , version("1.0")
        , width(25)
        , height(15)
        , tileSize(32)
    {
        wxDateTime now = wxDateTime::Now();
        createdDate = now.Format("%Y-%m-%d %H:%M:%S");
        modifiedDate = createdDate;
    }
};

class Map
{
public:
    Map();
    Map(int width, int height, int tileSize = 32);
    Map(const Map& other);
    Map& operator=(const Map& other);
    ~Map() = default;
    
    // Getters
    const MapMetadata& GetMetadata() const { return m_metadata; }
    int GetWidth() const { return m_metadata.width; }
    int GetHeight() const { return m_metadata.height; }
    int GetTileSize() const { return m_metadata.tileSize; }
    int GetTile(int x, int y) const;  // Delega ao layer ativo
    
    // Setters
    void SetMetadata(const MapMetadata& metadata);
    void SetName(const wxString& name);
    void SetTile(int x, int y, int tileId);  // Delega ao layer ativo
    void SetSize(int width, int height);
    
    // Gerenciamento de layers
    LayerManager* GetLayerManager() { return m_layerManager.get(); }
    const LayerManager* GetLayerManager() const { return m_layerManager.get(); }
    
    // Atalhos para operações de layer comum
    int GetLayerCount() const;
    Layer* GetActiveLayer();
    const Layer* GetActiveLayer() const;
    bool SetActiveLayer(int index);
    Layer* CreateLayer(const LayerProperties& properties = LayerProperties());
    bool RemoveLayer(int index);
    
    // Operações em layers específicos
    int GetTileFromLayer(int layerIndex, int x, int y) const;
    void SetTileInLayer(int layerIndex, int x, int y, int tileId);
    
    // Operações
    void Clear();  // Limpar layer ativo
    void Fill(int tileId);  // Preencher layer ativo
    void ClearAllLayers();  // Limpar todos os layers
    void FillLayer(int layerIndex, int tileId);  // Preencher layer específico
    void Resize(int newWidth, int newHeight, int defaultTile = 0);
    bool IsValidPosition(int x, int y) const;
    
    // Operações de área
    void FillRect(int x, int y, int width, int height, int tileId);
    void CopyRect(int srcX, int srcY, int width, int height, int srcLayer, int destLayer, int destX, int destY);
    
    // Estado
    bool IsModified() const;  // Verifica se algum layer foi modificado
    void SetModified(bool modified = true);
    void MarkSaved();
    
    // Serialização
    bool LoadFromJson(const wxString& jsonData);
    wxString SaveToJson() const;
    
    // Métodos de arquivo (para compatibilidade com MapManager)
    bool LoadFromFile(const std::string& filepath);
    bool SaveToFile(const std::string& filepath) const;
    
    // Debug
    void PrintDebugInfo() const;

private:
    void InitializeDefaultLayers();
    void UpdateModifiedTime();
    bool LoadFromSimpleJson(const wxString& jsonData);
    void MigrateFromTileArray(const std::vector<std::vector<int>>& tiles);
    
    MapMetadata m_metadata;
    std::unique_ptr<LayerManager> m_layerManager;
    bool m_modified;
};
