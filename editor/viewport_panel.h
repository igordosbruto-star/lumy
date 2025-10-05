/**
 * Viewport Panel - Painel principal de visualização e edição com OpenGL
 */

#pragma once

#include <wx/wx.h>
#include <wx/glcanvas.h>

class ViewportPanel : public wxPanel
{
public:
    ViewportPanel(wxWindow* parent);
    virtual ~ViewportPanel() = default;

private:
    void CreateControls();
    void CreateToolbar();
    
    // OpenGL Canvas para renderização
    class GLCanvas : public wxGLCanvas
    {
    public:
        GLCanvas(wxWindow* parent);
        virtual ~GLCanvas();
        
        void OnPaint(wxPaintEvent& event);
        void OnSize(wxSizeEvent& event);
        void OnMouseLeftDown(wxMouseEvent& event);
        void OnMouseRightDown(wxMouseEvent& event);
        void OnMouseMove(wxMouseEvent& event);
        void OnKeyDown(wxKeyEvent& event);
        
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
