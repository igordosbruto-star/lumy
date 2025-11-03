/**
 * Viewport Panel - Painel principal de visualização e edição com OpenGL
 */

#pragma once

#include <GL/glew.h>  // GLEW deve vir antes de qualquer header OpenGL
#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <memory>
#include "smooth_transform.h"
#include "collision_overlay.h"

// Forward declarations
class MapManager;
class EditorFrame;
class CommandHistory;
class ShaderProgram;
class MapRenderer;
class TextureAtlas;

class ViewportPanel : public wxPanel
{
public:
    ViewportPanel(wxWindow* parent);
    virtual ~ViewportPanel() = default;
    
    // Interface pública
    void SetSelectedTile(int tileId);
    void SetMapManager(MapManager* mapManager);
    void SetTilesetManager(class TilesetManager* tilesetManager);
    void SetCurrentMap(class Map* map); // Novo: usar Map diretamente
    void RefreshMapDisplay();
    void NotifyMapModified(); // Notificar EditorFrame sobre modificações no mapa
    
    // Undo/Redo
    CommandHistory* GetCommandHistory() { return m_commandHistory.get(); }
    bool CanUndo() const;
    bool CanRedo() const;
    bool Undo();
    bool Redo();

private:
    void CreateControls();
    void CreateToolbox();  // Renomeado de CreateToolbar
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
        void OnKeyUp(wxKeyEvent& event);
        
        // Helper functions
        wxPoint WorldToTile(const wxPoint& worldPos);
        wxPoint TileToWorld(const wxPoint& tilePos);
        void PaintTile(int tileX, int tileY);
        void EraseTile(int tileX, int tileY);
        void ToggleCollision(int tileX, int tileY);
        void UpdateViewportBounds(); // Atualizar limites dinâmicos baseados no mapa
        
    private:
        void InitGL();
        void Render();
        void DrawGrid();
        void DrawMap();
        void DrawSelection();
        
        wxGLContext* m_glContext;
        bool m_glInitialized;
        
        // Estado da visualização
        SmoothTransform m_smoothTransform;
        CollisionOverlay m_collisionOverlay;
        std::unique_ptr<ShaderProgram> m_overlayShader;
        std::unique_ptr<MapRenderer> m_mapRenderer;
        std::unique_ptr<TextureAtlas> m_tileAtlas;
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
        bool m_isTemporaryPanning; // Pan temporário com tecla Espaço
        wxPoint m_lastMousePos;
        
        // Ferramentas de edição
        enum Tool {
            TOOL_SELECT,
            TOOL_PAINT,
            TOOL_BUCKET,       // Balde de tinta
            TOOL_ERASE,
            TOOL_COLLISION,
            TOOL_SELECT_RECT,  // Seleção retangular
            TOOL_SELECT_CIRCLE // Seleção circular
        } m_currentTool;
        
        wxDECLARE_EVENT_TABLE();
    };
    
    // Controles
    wxToolBar* m_toolbox;  // Renomeado de m_toolbar para m_toolbox
    GLCanvas* m_glCanvas;
    
    // Event handlers para toolbox
    void OnToolSelect(wxCommandEvent& event);
    void OnToolPaint(wxCommandEvent& event);
    void OnToolBucket(wxCommandEvent& event);      // Novo: Balde
    void OnToolErase(wxCommandEvent& event);
    void OnToolCollision(wxCommandEvent& event);
    void OnToolSelectRect(wxCommandEvent& event);  // Novo: Seleção retangular
    void OnToolSelectCircle(wxCommandEvent& event); // Novo: Seleção circular
    void OnZoomIn(wxCommandEvent& event);
    void OnZoomOut(wxCommandEvent& event);
    void OnResetView(wxCommandEvent& event);
    
    // Map Manager, TilesetManager e Map direto
    MapManager* m_mapManager;
    class TilesetManager* m_tilesetManager;
    class Map* m_currentMap;
    
    // Command History para Undo/Redo
    std::unique_ptr<CommandHistory> m_commandHistory;

    wxDECLARE_EVENT_TABLE();
};

// IDs dos tools do Viewport
enum ViewportToolIds
{
    ID_VP_TOOL_SELECT = wxID_HIGHEST + 100,
    ID_VP_TOOL_PAINT,
    ID_VP_TOOL_BUCKET,        // Novo: Balde
    ID_VP_TOOL_ERASE,
    ID_VP_TOOL_COLLISION,
    ID_VP_TOOL_SELECT_RECT,   // Novo: Seleção retangular
    ID_VP_TOOL_SELECT_CIRCLE, // Novo: Seleção circular
    ID_VP_ZOOM_IN,
    ID_VP_ZOOM_OUT,
    ID_VP_RESET_VIEW
};
