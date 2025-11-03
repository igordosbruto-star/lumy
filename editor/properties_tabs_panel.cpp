/**
 * PropertiesTabsPanel - Implementação do painel com abas
 */

#include "properties_tabs_panel.h"
#include "property_grid_panel.h"
#include "layer_panel.h"
#include "map.h"

wxBEGIN_EVENT_TABLE(PropertiesTabsPanel, wxPanel)
    EVT_NOTEBOOK_PAGE_CHANGED(wxID_ANY, PropertiesTabsPanel::OnTabChanged)
wxEND_EVENT_TABLE()

PropertiesTabsPanel::PropertiesTabsPanel(wxWindow* parent, wxWindowID id)
    : wxPanel(parent, id)
    , m_notebook(nullptr)
{
    CreateControls();
}

PropertiesTabsPanel::~PropertiesTabsPanel()
{
}

void PropertiesTabsPanel::CreateControls()
{
    // Create notebook control
    m_notebook = new wxNotebook(this, wxID_ANY);
    
    // Create child panels
    m_propertyGrid = std::make_unique<PropertyGridPanel>(m_notebook);
    m_layerPanel = std::make_unique<LayerPanel>(m_notebook);
    
    // Add pages to notebook
    m_notebook->AddPage(m_propertyGrid.get(), "Propriedades", true);  // Select by default
    m_notebook->AddPage(m_layerPanel.get(), "Camadas", false);
    
    // Layout
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(m_notebook, 1, wxEXPAND | wxALL, 0);
    SetSizer(sizer);
}

void PropertiesTabsPanel::SetMap(Map* map)
{
    // Forward map to layer panel
    if (m_layerPanel)
    {
        m_layerPanel->SetMap(map);
    }
    
    // Property grid doesn't need map directly for now, but we could add it later
    // if (m_propertyGrid)
    // {
    //     m_propertyGrid->SetMap(map);
    // }
}

void PropertiesTabsPanel::SelectPropertiesTab()
{
    if (m_notebook)
    {
        m_notebook->SetSelection(TAB_PROPERTIES);
    }
}

void PropertiesTabsPanel::SelectLayersTab()
{
    if (m_notebook)
    {
        m_notebook->SetSelection(TAB_LAYERS);
    }
}

int PropertiesTabsPanel::GetSelectedTab() const
{
    if (m_notebook)
    {
        return m_notebook->GetSelection();
    }
    return -1;
}

void PropertiesTabsPanel::OnTabChanged(wxBookCtrlEvent& event)
{
    int selectedTab = event.GetSelection();
    
    // Log tab change for debugging
    wxLogMessage("Aba alterada para: %s", 
               selectedTab == TAB_PROPERTIES ? "Propriedades" : "Camadas");
    
    // Could add specific logic here when tabs change
    // For example, refreshing content or updating other panels
    
    event.Skip();
}