/**
 * LeftSidePanel - Implementação do painel composto lateral
 */

#include "left_side_panel.h"
#include "project_tree_panel.h"
#include "tileset_panel.h"

wxBEGIN_EVENT_TABLE(LeftSidePanel, wxPanel)
    EVT_SPLITTER_SASH_POS_CHANGED(wxID_ANY, LeftSidePanel::OnSplitterChanged)
wxEND_EVENT_TABLE()

LeftSidePanel::LeftSidePanel(wxWindow* parent, wxWindowID id)
    : wxPanel(parent, id)
    , m_splitter(nullptr)
{
    CreateControls();
}

LeftSidePanel::~LeftSidePanel()
{
}

void LeftSidePanel::CreateControls()
{
    // Create splitter window for vertical split with enhanced resize capabilities
    m_splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                     wxSP_3D | wxSP_LIVE_UPDATE | wxSP_BORDER);
    
    // Set splitter properties for better user experience
    m_splitter->SetMinimumPaneSize(120);  // Minimum size for each pane (reduced for flexibility)
    m_splitter->SetSashGravity(0.4);      // Give slightly more space to project tree by default
    
    // Create child panels
    m_projectTree = std::make_unique<ProjectTreePanel>(m_splitter);
    m_tilesetPanel = std::make_unique<TilesetPanel>(m_splitter);
    
    // Split horizontally (vertical panels) with better initial proportions
    // Position will be set after layout to be proportional
    m_splitter->SplitHorizontally(m_projectTree.get(), m_tilesetPanel.get(), -1);
    
    // Layout
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(m_splitter, 1, wxEXPAND);
    SetSizer(sizer);
    
    // Set initial proportional position after a brief delay to ensure proper sizing
    CallAfter([this]() {
        SetProportionalSplitterPosition();
    });
}

void LeftSidePanel::SetSplitterPosition(int position)
{
    if (m_splitter)
    {
        m_splitter->SetSashPosition(position);
    }
}

int LeftSidePanel::GetSplitterPosition() const
{
    if (m_splitter)
    {
        return m_splitter->GetSashPosition();
    }
    return -1;
}

void LeftSidePanel::SetProportionalSplitterPosition(double proportion)
{
    if (m_splitter && m_splitter->IsSplit())
    {
        wxSize size = m_splitter->GetSize();
        if (size.GetHeight() > 0)
        {
            int position = static_cast<int>(size.GetHeight() * proportion);
            // Ensure position respects minimum pane sizes
            int minSize = m_splitter->GetMinimumPaneSize();
            int maxSize = size.GetHeight() - minSize;
            
            position = wxMax(minSize, wxMin(position, maxSize));
            m_splitter->SetSashPosition(position);
            
            wxLogMessage("Splitter position set to %d (%.1f%% of %d)", 
                        position, proportion * 100, size.GetHeight());
        }
    }
}

void LeftSidePanel::OnSplitterChanged(wxSplitterEvent& event)
{
    int newPosition = event.GetSashPosition();
    wxSize totalSize = m_splitter->GetSize();
    
    if (totalSize.GetHeight() > 0)
    {
        double proportion = static_cast<double>(newPosition) / totalSize.GetHeight();
        wxLogMessage("Splitter ajustado: %d pixels (%.1f%% para árvore, %.1f%% para paleta)", 
                    newPosition, proportion * 100, (1.0 - proportion) * 100);
    }
    else
    {
        wxLogMessage("Splitter ajustado para: %d pixels", newPosition);
    }
    
    event.Skip();
}
