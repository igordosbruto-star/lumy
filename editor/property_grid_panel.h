/**
 * Property Grid Panel - Grade de propriedades para edição de objetos
 */

#pragma once

#include <wx/wx.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

class PropertyGridPanel : public wxPanel
{
public:
    PropertyGridPanel(wxWindow* parent);
    virtual ~PropertyGridPanel() = default;

private:
    void CreateControls();
    void PopulateDefaultProperties();
    void PopulateMapProperties();
    void PopulateEventProperties();
    
    // Event handlers
    void OnPropertyChanged(wxPropertyGridEvent& event);
    void OnPropertySelected(wxPropertyGridEvent& event);
    
    // Controles
    wxPropertyGrid* m_propertyGrid;

    wxDECLARE_EVENT_TABLE();
};
