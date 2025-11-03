/**
 * PropertiesTabsPanel - Painel com abas para Propriedades e Camadas
 * Combina PropertyGridPanel e LayerPanel em um só controle com abas
 */

#pragma once

#include <wx/wx.h>
#include <wx/notebook.h>
#include <memory>
#include "property_grid_panel.h"
#include "layer_panel.h"

// Forward declarations
class Map;

class PropertiesTabsPanel : public wxPanel
{
public:
    PropertiesTabsPanel(wxWindow* parent, wxWindowID id = wxID_ANY);
    virtual ~PropertiesTabsPanel();
    
    // Access to child panels
    PropertyGridPanel* GetPropertyGrid() { return m_propertyGrid.get(); }
    LayerPanel* GetLayerPanel() { return m_layerPanel.get(); }
    
    // Map integration
    void SetMap(Map* map);
    
    // Tab management
    void SelectPropertiesTab();
    void SelectLayersTab();
    int GetSelectedTab() const;
    
private:
    void CreateControls();
    void OnTabChanged(wxBookCtrlEvent& event);
    
    // UI Controls
    wxNotebook* m_notebook;
    
    // Child panels
    std::unique_ptr<PropertyGridPanel> m_propertyGrid;
    std::unique_ptr<LayerPanel> m_layerPanel;
    
    wxDECLARE_EVENT_TABLE();
};

// Tab indices
enum TabIndices
{
    TAB_PROPERTIES = 0,
    TAB_LAYERS = 1
};