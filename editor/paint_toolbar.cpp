/**
 * PaintToolbar - Implementação da barra de ferramentas de pintura
 */

#include "paint_toolbar.h"
#include <wx/artprov.h>

// Definir o evento customizado
wxDEFINE_EVENT(EVT_PAINT_TOOL_CHANGED, wxCommandEvent);

wxBEGIN_EVENT_TABLE(PaintToolbar, wxPanel)
    EVT_TOOL(ID_TOOL_BRUSH, PaintToolbar::OnToolSelected)
    EVT_TOOL(ID_TOOL_BUCKET, PaintToolbar::OnToolSelected)
    EVT_TOOL(ID_TOOL_SELECTION, PaintToolbar::OnToolSelected)
    EVT_TOOL(ID_TOOL_ERASER, PaintToolbar::OnToolSelected)
wxEND_EVENT_TABLE()

PaintToolbar::PaintToolbar(wxWindow* parent, wxWindowID id)
    : wxPanel(parent, id)
    , m_toolbar(nullptr)
    , m_activeTool(PaintTool::BRUSH)
{
    // Criar instâncias das ferramentas
    m_brushTool = std::make_unique<BrushTool>();
    m_bucketTool = std::make_unique<BucketTool>();
    m_selectionTool = std::make_unique<SelectionTool>();
    m_eraserTool = std::make_unique<EraserTool>();
    
    CreateControls();
    
    // Ativar ferramenta padrão (Pincel)
    SetActiveTool(PaintTool::BRUSH);
}

PaintToolbar::~PaintToolbar()
{
}

void PaintToolbar::CreateControls()
{
    // Criar toolbar horizontal
    m_toolbar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                              wxTB_HORIZONTAL | wxTB_FLAT | wxTB_NODIVIDER);
    
    // Obter ícones do sistema (ou criar básicos)
    wxBitmap brushIcon = wxArtProvider::GetBitmap(wxART_NORMAL_FILE, wxART_TOOLBAR, wxSize(24, 24));
    wxBitmap bucketIcon = wxArtProvider::GetBitmap(wxART_QUESTION, wxART_TOOLBAR, wxSize(24, 24));
    wxBitmap selectionIcon = wxArtProvider::GetBitmap(wxART_FIND, wxART_TOOLBAR, wxSize(24, 24));
    wxBitmap eraserIcon = wxArtProvider::GetBitmap(wxART_DELETE, wxART_TOOLBAR, wxSize(24, 24));
    
    // Adicionar ferramentas como botões toggle radio
    m_toolbar->AddRadioTool(ID_TOOL_BRUSH, "Pincel", brushIcon, wxNullBitmap,
                           "Pincel - Pinta tiles individuais (B)");
    
    m_toolbar->AddRadioTool(ID_TOOL_BUCKET, "Preenchimento", bucketIcon, wxNullBitmap,
                           "Preenchimento - Preenche área com mesmo tile (G)");
    
    m_toolbar->AddRadioTool(ID_TOOL_SELECTION, "Seleção", selectionIcon, wxNullBitmap,
                           "Seleção - Seleciona área retangular (S)");
    
    m_toolbar->AddRadioTool(ID_TOOL_ERASER, "Borracha", eraserIcon, wxNullBitmap,
                           "Borracha - Apaga tiles (E)");
    
    m_toolbar->Realize();
    
    // Layout
    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(m_toolbar, 0, wxEXPAND | wxALL, 0);
    SetSizer(sizer);
}

IPaintTool* PaintToolbar::GetActiveTool()
{
    switch (m_activeTool)
    {
        case PaintTool::BRUSH:
            return m_brushTool.get();
        case PaintTool::BUCKET:
            return m_bucketTool.get();
        case PaintTool::SELECTION:
            return m_selectionTool.get();
        case PaintTool::ERASER:
            return m_eraserTool.get();
        default:
            return m_brushTool.get();
    }
}

void PaintToolbar::SetActiveTool(PaintTool tool)
{
    // Desativar ferramenta anterior
    IPaintTool* oldTool = GetActiveTool();
    if (oldTool)
    {
        oldTool->SetActive(false);
    }
    
    // Ativar nova ferramenta
    m_activeTool = tool;
    IPaintTool* newTool = GetActiveTool();
    if (newTool)
    {
        newTool->SetActive(true);
    }
    
    // Atualizar estado visual da toolbar
    UpdateToolbarState();
    
    // Notificar mudança de ferramenta
    wxCommandEvent event(EVT_PAINT_TOOL_CHANGED, GetId());
    event.SetEventObject(this);
    event.SetInt(static_cast<int>(m_activeTool));
    ProcessEvent(event);
    
    wxLogMessage("Ferramenta ativa alterada para: %s", newTool->GetName());
}

void PaintToolbar::UpdateToolbarState()
{
    if (!m_toolbar) return;
    
    // Atualizar estado visual dos botões
    switch (m_activeTool)
    {
        case PaintTool::BRUSH:
            m_toolbar->ToggleTool(ID_TOOL_BRUSH, true);
            break;
        case PaintTool::BUCKET:
            m_toolbar->ToggleTool(ID_TOOL_BUCKET, true);
            break;
        case PaintTool::SELECTION:
            m_toolbar->ToggleTool(ID_TOOL_SELECTION, true);
            break;
        case PaintTool::ERASER:
            m_toolbar->ToggleTool(ID_TOOL_ERASER, true);
            break;
    }
}

void PaintToolbar::OnToolSelected(wxCommandEvent& event)
{
    int toolId = event.GetId();
    
    switch (toolId)
    {
        case ID_TOOL_BRUSH:
            SetActiveTool(PaintTool::BRUSH);
            break;
        case ID_TOOL_BUCKET:
            SetActiveTool(PaintTool::BUCKET);
            break;
        case ID_TOOL_SELECTION:
            SetActiveTool(PaintTool::SELECTION);
            break;
        case ID_TOOL_ERASER:
            SetActiveTool(PaintTool::ERASER);
            break;
    }
}