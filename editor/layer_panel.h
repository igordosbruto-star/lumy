/**
 * LayerPanel - Painel de gerenciamento de layers estilo Photoshop
 * Permite visualizar, selecionar, ocultar e gerenciar layers do mapa
 */

#pragma once

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/imaglist.h>
#include <wx/toolbar.h>
#include <wx/button.h>
#include <wx/statbmp.h>
#include <memory>
#include "layer_manager.h"
#include "editor_events.h"

// Forward declarations
class LayerManager;
class Map;

// Custom list control para drag & drop de layers
class LayerListCtrl : public wxListCtrl
{
public:
    LayerListCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition,
                  const wxSize& size = wxDefaultSize, long style = wxLC_REPORT | wxLC_SINGLE_SEL);

    void SetLayerManager(LayerManager* manager) { m_layerManager = manager; }
    
private:
    void OnItemSelected(wxListEvent& event);
    void OnItemRightClick(wxListEvent& event);
    void OnItemActivated(wxListEvent& event); // Double-click para renomear
    void OnBeginDrag(wxListEvent& event);
    
    LayerManager* m_layerManager;
    
    wxDECLARE_EVENT_TABLE();
};

class LayerPanel : public wxPanel
{
public:
    LayerPanel(wxWindow* parent, wxWindowID id = wxID_ANY);
    virtual ~LayerPanel();
    
    // Integração com Map/LayerManager
    void SetMap(Map* map);
    void RefreshLayerList();
    void SelectLayer(int index);
    
    // Atualização de estado
    void OnLayerSelectionChanged(int layerIndex);
    void OnLayerPropertiesChanged(int layerIndex);
    
private:
    // UI Creation
    void CreateControls();
    void CreateToolbar();
    void CreateLayerList();
    void SetupImageList();
    void SetupLayout();
    
    // Event handlers - Toolbar
    void OnAddLayer(wxCommandEvent& event);
    void OnRemoveLayer(wxCommandEvent& event);
    void OnDuplicateLayer(wxCommandEvent& event);
    void OnMoveLayerUp(wxCommandEvent& event);
    void OnMoveLayerDown(wxCommandEvent& event);
    
    // Event handlers - Layer list
    void OnLayerSelected(wxListEvent& event);
    void OnLayerRightClick(wxListEvent& event);
    void OnLayerDoubleClick(wxListEvent& event);
    void OnToggleVisibility(wxMouseEvent& event);
    void OnToggleLock(wxMouseEvent& event);
    void OnOpacityChanged(wxScrollEvent& event);
    
    // Context menu
    void ShowContextMenu(const wxPoint& pos, int layerIndex);
    void OnContextMenuNewLayer(wxCommandEvent& event);
    void OnContextMenuDeleteLayer(wxCommandEvent& event);
    void OnContextMenuDuplicateLayer(wxCommandEvent& event);
    void OnContextMenuRenameLayer(wxCommandEvent& event);
    void OnContextMenuLayerProperties(wxCommandEvent& event);
    
    // Helper methods
    void UpdateLayerItem(int listIndex, Layer* layer);
    void AddLayerToList(Layer* layer, int index);
    void RemoveLayerFromList(int index);
    void UpdateLayerVisualState(int listIndex, Layer* layer);
    wxString GetLayerTypeText(LayerType type);
    int GetLayerTypeIcon(LayerType type);
    
    // Layer operations
    bool CanRemoveLayer() const;
    void CreateNewLayer(LayerType type = LayerType::TILE_LAYER);
    void RenameLayer(int index);
    
    // Communication with other panels
    void BroadcastLayerSelectionChanged(int layerIndex);
    void BroadcastLayerPropertiesChanged(int layerIndex);
    
    // UI Controls
    wxToolBar* m_toolbar;
    LayerListCtrl* m_layerList;
    wxSlider* m_opacitySlider;
    wxStaticText* m_opacityLabel;
    
    // Icons and images
    wxImageList* m_imageList;
    wxBitmap m_visibleIcon;
    wxBitmap m_hiddenIcon;
    wxBitmap m_lockedIcon;
    wxBitmap m_unlockedIcon;
    
    // Data
    Map* m_currentMap;
    LayerManager* m_layerManager;
    int m_selectedLayerIndex;
    
    // Context menu
    wxMenu* m_contextMenu;
    int m_contextLayerIndex; // Layer index for context menu operations
    
    wxDECLARE_EVENT_TABLE();
};

// Event IDs
enum LayerPanelEventIds
{
    ID_LAYER_ADD = wxID_HIGHEST + 200,
    ID_LAYER_REMOVE,
    ID_LAYER_DUPLICATE,
    ID_LAYER_MOVE_UP,
    ID_LAYER_MOVE_DOWN,
    ID_LAYER_LIST,
    ID_OPACITY_SLIDER,
    
    // Context menu
    ID_CONTEXT_NEW_LAYER,
    ID_CONTEXT_DELETE_LAYER,
    ID_CONTEXT_DUPLICATE_LAYER,
    ID_CONTEXT_RENAME_LAYER,
    ID_CONTEXT_LAYER_PROPERTIES
};