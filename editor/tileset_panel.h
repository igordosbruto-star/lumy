/**
 * Tileset Panel - Painel para seleção visual de tiles
 */

#pragma once

#include <wx/wx.h>
#include <wx/scrolwin.h>
#include <wx/choice.h>
#include <wx/button.h>
#include "tileset_manager.h"
#include "tile_properties_dialog.h"

class TilesetPanel : public wxPanel
{
public:
    TilesetPanel(wxWindow* parent);
    virtual ~TilesetPanel() = default;
    
    // Interface pública
    int GetSelectedTile() const { return m_selectedTile; }
    void SetSelectedTile(int tileId);
    
    // Gerenciamento de tilesets
    bool LoadTilesetFromFile(const wxString& filepath, const wxSize& tileSize = wxSize(32, 32));
    void RefreshTilesetList();
    void ApplyFilters();
    TilesetManager* GetTilesetManager() { return m_tilesetManager.get(); }
    
    // Event handlers
    void OnLoadTileset(wxCommandEvent& event);
    void OnTilesetChanged(wxCommandEvent& event);
    void OnTilesetProperties(wxCommandEvent& event);
    void OnSearchChanged(wxCommandEvent& event);
    void OnCategoryFilterChanged(wxCommandEvent& event);

private:
    void CreateControls();
    void CreateTileGrid();
    
    // Classe interna para o grid de tiles
    class TileGrid : public wxScrolledWindow
    {
    public:
        TileGrid(wxWindow* parent, TilesetPanel* tilesetPanel);
        
        void OnPaint(wxPaintEvent& event);
        void OnLeftDown(wxMouseEvent& event);
        void OnSize(wxSizeEvent& event);
        
        int GetSelectedTile() const { return m_selectedTile; }
        void SetSelectedTile(int tileId);
        void UpdateVirtualSize();
        
        // Lista de tiles filtrados (pública para acesso pelo TilesetPanel)
        std::vector<int> m_filteredTiles;
        
    private:
        void DrawTile(wxDC& dc, int tileId, int x, int y, bool selected = false);
        wxPoint TileIdToGridPos(int tileId) const;
        int GridPosToTileId(const wxPoint& gridPos) const;
        
        TilesetPanel* m_parent;
        int m_selectedTile;
        int m_tileSize;
        int m_tilesPerRow;
        
        wxDECLARE_EVENT_TABLE();
    };
    
    // Controles
    wxChoice* m_tilesetChoice;
    wxButton* m_loadButton;
    wxButton* m_propertiesButton;
    wxTextCtrl* m_searchTextCtrl;
    wxChoice* m_categoryFilterChoice;
    TileGrid* m_tileGrid;
    wxStaticText* m_infoLabel;
    
    // Gerenciador de tilesets
    std::unique_ptr<TilesetManager> m_tilesetManager;
    
    // Estado
    int m_selectedTile;
    
    // IDs para eventos
    enum {
        ID_LOAD_TILESET = 2000,
        ID_TILESET_CHOICE,
        ID_TILESET_PROPERTIES,
        ID_SEARCH_TEXT,
        ID_CATEGORY_FILTER
    };
    
    wxDECLARE_EVENT_TABLE();
};

// Event declarations
wxDECLARE_EVENT(TILESET_SELECTION_CHANGED, wxCommandEvent);
