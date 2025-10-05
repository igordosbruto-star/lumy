/**
 * Tileset Panel - Painel para seleção visual de tiles
 */

#pragma once

#include <wx/wx.h>
#include <wx/scrolwin.h>

class TilesetPanel : public wxPanel
{
public:
    TilesetPanel(wxWindow* parent);
    virtual ~TilesetPanel() = default;
    
    // Interface pública
    int GetSelectedTile() const { return m_selectedTile; }
    void SetSelectedTile(int tileId);
    
    // Eventos - declaração será feita externamente

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
        
    private:
        void DrawTile(wxDC& dc, int tileId, int x, int y, bool selected = false);
        wxPoint TileIdToGridPos(int tileId) const;
        int GridPosToTileId(const wxPoint& gridPos) const;
        
        TilesetPanel* m_parent;
        int m_selectedTile;
        int m_tileSize;
        int m_tilesPerRow;
        
        // Definição dos tipos de tile disponíveis
        static const int TILE_COUNT = 10;
        
        wxDECLARE_EVENT_TABLE();
    };
    
    // Controles
    TileGrid* m_tileGrid;
    wxStaticText* m_infoLabel;
    
    // Estado
    int m_selectedTile;
    
    wxDECLARE_EVENT_TABLE();
};

// Event declarations
wxDECLARE_EVENT(TILESET_SELECTION_CHANGED, wxCommandEvent);
