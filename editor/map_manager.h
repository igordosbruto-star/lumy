/**
 * MapManager - Gerenciador de mapas do Lumy Editor
 */

#pragma once

#include <wx/wx.h>
#include <memory>
#include "map.h"

class MapManager
{
public:
    MapManager();
    ~MapManager() = default;
    
    // Getters
    std::shared_ptr<Map> GetCurrentMap() const;
    bool HasMap() const { return m_currentMap != nullptr; }
    wxString GetCurrentMapPath() const { return m_currentMapPath; }
    wxString GetCurrentMapName() const;
    bool HasUnsavedChanges() const { return m_hasUnsavedChanges || (m_currentMap && m_currentMap->IsModified()); }
    
    // Setters
    void SetCurrentMap(std::shared_ptr<Map> map);
    
    // Operações de mapa
    bool NewMap(int width = 25, int height = 15, const wxString& name = "Novo Mapa");
    bool LoadMap(const std::string& filePath);
    bool LoadMap(const wxString& filePath);
    bool SaveMap(); // Salvar no caminho atual
    bool SaveMap(const std::string& filePath) const; // Salvar em caminho específico
    bool SaveMapAs(const wxString& filePath);
    bool CloseMap();
    
    // Acesso aos dados do mapa
    int GetTile(int x, int y) const;
    void SetTile(int x, int y, int tileId);
    int GetMapWidth() const;
    int GetMapHeight() const;
    
    // Validação
    bool IsValidPosition(int x, int y) const;
    
    // Estado
    void SetMapModified(bool modified = true);
    
    // Utilidades
    wxString GetMapDisplayTitle() const;
    bool PromptSaveIfModified(wxWindow* parent);
    
    // Debug
    void PrintDebugInfo() const;

private:
    bool LoadMapFromFile(const wxString& filePath);
    bool SaveMapToFile(const wxString& filePath);
    wxString GenerateDefaultName() const;
    
    std::unique_ptr<Map> m_currentMap;
    wxString m_currentMapPath;
    bool m_hasUnsavedChanges;
};
