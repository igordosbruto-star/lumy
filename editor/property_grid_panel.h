/**
 * Property Grid Panel - Grade de propriedades para edição de objetos
 */

#pragma once

#include <wx/wx.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

class Map;
class Layer;

class PropertyGridPanel : public wxPanel
{
public:
    PropertyGridPanel(wxWindow* parent);
    virtual ~PropertyGridPanel() = default;
    
    // Public methods to load different object properties
    void LoadMapProperties(Map* map);
    void LoadLayerProperties(Layer* layer);
    void LoadTileProperties(int x, int y, int tileId, bool hasCollision = false);
    void ClearProperties();

private:
    void CreateControls();
    void PopulateDefaultProperties();
    void PopulateMapProperties();
    void PopulateEventProperties();  // Now loads Layer properties
    
    // Event handlers
    void OnPropertyChanged(wxPropertyGridEvent& event);
    void OnPropertySelected(wxPropertyGridEvent& event);
    
    // Controles
    wxPropertyGrid* m_propertyGrid;
    
    // Current object references
    Map* m_currentMap = nullptr;
    Layer* m_currentLayer = nullptr;
    bool m_hasTileSelection = false;
    int m_selectedTileX = 0;
    int m_selectedTileY = 0;

    wxDECLARE_EVENT_TABLE();
};
