/**
 * Viewport Panel - Painel principal de visualização e edição com OpenGL
 */

#pragma once

#include <wx/wx.h>
#include <wx/glcanvas.h>

// Forward declarations
class MapManager;
class EditorFrame;

class ViewportPanel : public wxPanel
{
public:
    ViewportPanel(wxWindow* parent);
    virtual ~ViewportPanel() = default;
    
    // Interface pública
    void SetSelectedTile(int tileId);
    void SetMapManager(MapManager* mapManager);
    void RefreshMapDisplay();
    void NotifyMapModified(); // Notificar EditorFrame sobre modificações no mapa

private:
    void CreateControls();
    void CreateToolbar();
    wxBitmap CreateSimpleBitmap(const wxColour& color);
    
    // OpenGL Canvas para renderização
    class GLCanvas : public wxGLCanvas
    {
        friend class ViewportPanel; // Permitir acesso aos membros privados
    public:
        GLCanvas(wxWindow* parent);
        virtual ~GLCanvas();
        
        void OnPaint(wxPaintEvent& event);
        void OnSize(wxSizeEvent& event);
        void OnMouseLeftDown(wxMouseEvent& event);
        void OnMouseRightDown(wxMouseEvent& event);
        void OnMouseMove(wxMouseEvent& event);
        void OnMouseWheel(wxMouseEvent& event);
        void OnMouseMiddleDown(wxMouseEvent& event);
        void OnMouseMiddleUp(wxMouseEvent& event);
        void OnKeyDown(wxKeyEvent& event);
        
        // Helper functions
        wxPoint WorldToTile(const wxPoint& worldPos);
        wxPoint TileToWorld(const wxPoint& tilePos);
        void PaintTile(int tileX, int tileY);
        void EraseTile(int tileX, int tileY);
        void ToggleCollision(int tileX, int tileY);
        
    private:
        void InitGL();
        void Render();
        void DrawGrid();
        void DrawMap();
        void DrawSelection();
        
        wxGLContext* m_glContext;
        bool m_glInitialized;
        
        // Estado da visualização
        float m_zoom;
        float m_panX, m_panY;
        bool m_showGrid;
        bool m_showCollision;
        
        // Map data (simulated)
        static const int MAP_WIDTH = 25;
        static const int MAP_HEIGHT = 15;
        static const int TILE_SIZE = 32;
        int m_mapTiles[MAP_HEIGHT][MAP_WIDTH]; // 0=grass, 1=wall, 2=collision
        
        // Editing state
        int m_selectedTile;
        bool m_isPanning;
        wxPoint m_lastMousePos;
        
        // Ferramentas de edição
        enum Tool {
            TOOL_SELECT,
            TOOL_PAINT,
            TOOL_ERASE,
            TOOL_COLLISION
        } m_currentTool;
        
        wxDECLARE_EVENT_TABLE();
    };
    
    // Controles
    wxToolBar* m_toolbar;
    GLCanvas* m_glCanvas;
    
    // Event handlers para toolbar
    void OnToolSelect(wxCommandEvent& event);
    void OnToolPaint(wxCommandEvent& event);
    void OnToolErase(wxCommandEvent& event);
    void OnToolCollision(wxCommandEvent& event);
    void OnZoomIn(wxCommandEvent& event);
    void OnZoomOut(wxCommandEvent& event);
    void OnResetView(wxCommandEvent& event);
    
    // Map Manager
    MapManager* m_mapManager;

    wxDECLARE_EVENT_TABLE();
};

// IDs dos tools
enum
{
    ID_TOOL_SELECT = wxID_HIGHEST + 100,
    ID_TOOL_PAINT,
    ID_TOOL_ERASE,
    ID_TOOL_COLLISION,
    ID_ZOOM_IN,
    ID_ZOOM_OUT,
    ID_RESET_VIEW
};
