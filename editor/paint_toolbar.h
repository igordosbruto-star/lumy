/**
 * PaintToolbar - Barra de ferramentas de pintura
 * Exibe botões para selecionar ferramenta ativa (Pincel, Balde, Seleção, Borracha)
 */

#pragma once

#include <wx/wx.h>
#include <wx/toolbar.h>
#include "paint_tools.h"
#include <memory>
#include <map>

class PaintToolbar : public wxPanel
{
public:
    PaintToolbar(wxWindow* parent, wxWindowID id = wxID_ANY);
    virtual ~PaintToolbar();
    
    // Obter ferramenta ativa
    IPaintTool* GetActiveTool();
    PaintTool GetActiveToolType() const { return m_activeTool; }
    
    // Definir ferramenta ativa
    void SetActiveTool(PaintTool tool);
    
    // Obter instância específica de ferramenta
    BrushTool* GetBrushTool() { return m_brushTool.get(); }
    BucketTool* GetBucketTool() { return m_bucketTool.get(); }
    SelectionTool* GetSelectionTool() { return m_selectionTool.get(); }
    EraserTool* GetEraserTool() { return m_eraserTool.get(); }
    
private:
    void CreateControls();
    void UpdateToolbarState();
    
    // Event handlers
    void OnToolSelected(wxCommandEvent& event);
    
    // UI
    wxToolBar* m_toolbar;
    
    // Ferramentas
    std::unique_ptr<BrushTool> m_brushTool;
    std::unique_ptr<BucketTool> m_bucketTool;
    std::unique_ptr<SelectionTool> m_selectionTool;
    std::unique_ptr<EraserTool> m_eraserTool;
    
    PaintTool m_activeTool;
    
    wxDECLARE_EVENT_TABLE();
};

// Event IDs
enum PaintToolbarEventIds
{
    ID_TOOL_BRUSH = wxID_HIGHEST + 400,
    ID_TOOL_BUCKET,
    ID_TOOL_SELECTION,
    ID_TOOL_ERASER
};

// Evento customizado para notificar mudança de ferramenta
wxDECLARE_EVENT(EVT_PAINT_TOOL_CHANGED, wxCommandEvent);
