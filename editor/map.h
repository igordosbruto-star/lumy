/**
 * Map - Classe para representar um mapa do Lumy Editor
 */

#pragma once

#include <wx/wx.h>
#include <vector>
#include <string>

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
    int GetTile(int x, int y) const;
    
    // Setters
    void SetMetadata(const MapMetadata& metadata);
    void SetName(const wxString& name);
    void SetTile(int x, int y, int tileId);
    void SetSize(int width, int height);
    
    // Operações
    void Clear();
    void Fill(int tileId);
    void Resize(int newWidth, int newHeight, int defaultTile = 0);
    bool IsValidPosition(int x, int y) const;
    
    // Estado
    bool IsModified() const { return m_modified; }
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
    void InitializeDefaultMap();
    void UpdateModifiedTime();
    bool LoadFromSimpleJson(const wxString& jsonData);
    
    MapMetadata m_metadata;
    std::vector<std::vector<int>> m_tiles;
    bool m_modified;
};
