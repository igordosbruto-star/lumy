/**
 * LayerPanel - Implementação do painel de gerenciamento de layers
 */

#include "layer_panel.h"
#include "map.h"
#include <wx/artprov.h>
#include <wx/textdlg.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>

// ============================================================================
// LayerListCtrl Implementation
// ============================================================================

wxBEGIN_EVENT_TABLE(LayerListCtrl, wxListCtrl)
    EVT_LIST_ITEM_SELECTED(wxID_ANY, LayerListCtrl::OnItemSelected)
    EVT_LIST_ITEM_RIGHT_CLICK(wxID_ANY, LayerListCtrl::OnItemRightClick)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY, LayerListCtrl::OnItemActivated)
    EVT_LIST_BEGIN_DRAG(wxID_ANY, LayerListCtrl::OnBeginDrag)
wxEND_EVENT_TABLE()

LayerListCtrl::LayerListCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos,
                             const wxSize& size, long style)
    : wxListCtrl(parent, id, pos, size, style)
    , m_layerManager(nullptr)
{
    // Configure columns
    AppendColumn("", wxLIST_FORMAT_CENTRE, 24); // Visibility icon
    AppendColumn("", wxLIST_FORMAT_CENTRE, 24); // Lock icon  
    AppendColumn("Nome", wxLIST_FORMAT_LEFT, 120);
    AppendColumn("Tipo", wxLIST_FORMAT_LEFT, 80);
}

void LayerListCtrl::OnItemSelected(wxListEvent& event)
{
    // Forward to parent (LayerPanel)
    GetParent()->GetEventHandler()->ProcessEvent(event);
}

void LayerListCtrl::OnItemRightClick(wxListEvent& event)
{
    // Forward to parent
    GetParent()->GetEventHandler()->ProcessEvent(event);
}

void LayerListCtrl::OnItemActivated(wxListEvent& event)
{
    // Forward to parent for rename operation
    GetParent()->GetEventHandler()->ProcessEvent(event);
}

void LayerListCtrl::OnBeginDrag(wxListEvent& event)
{
    // TODO: Implement drag & drop reordering
    // For now, just forward the event
    GetParent()->GetEventHandler()->ProcessEvent(event);
}

// ============================================================================
// LayerPanel Implementation
// ============================================================================

wxBEGIN_EVENT_TABLE(LayerPanel, wxPanel)
    // Toolbar events
    EVT_TOOL(ID_LAYER_ADD, LayerPanel::OnAddLayer)
    EVT_TOOL(ID_LAYER_REMOVE, LayerPanel::OnRemoveLayer)
    EVT_TOOL(ID_LAYER_DUPLICATE, LayerPanel::OnDuplicateLayer)
    EVT_TOOL(ID_LAYER_MOVE_UP, LayerPanel::OnMoveLayerUp)
    EVT_TOOL(ID_LAYER_MOVE_DOWN, LayerPanel::OnMoveLayerDown)
    
    // List events
    EVT_LIST_ITEM_SELECTED(ID_LAYER_LIST, LayerPanel::OnLayerSelected)
    EVT_LIST_ITEM_RIGHT_CLICK(ID_LAYER_LIST, LayerPanel::OnLayerRightClick)
    EVT_LIST_ITEM_ACTIVATED(ID_LAYER_LIST, LayerPanel::OnLayerDoubleClick)
    
    // Mouse events for clicking on visibility/lock icons
    EVT_LEFT_UP(LayerPanel::OnToggleVisibility)
    
    // Context menu
    EVT_MENU(ID_CONTEXT_NEW_LAYER, LayerPanel::OnContextMenuNewLayer)
    EVT_MENU(ID_CONTEXT_DELETE_LAYER, LayerPanel::OnContextMenuDeleteLayer)
    EVT_MENU(ID_CONTEXT_DUPLICATE_LAYER, LayerPanel::OnContextMenuDuplicateLayer)
    EVT_MENU(ID_CONTEXT_RENAME_LAYER, LayerPanel::OnContextMenuRenameLayer)
    EVT_MENU(ID_CONTEXT_LAYER_PROPERTIES, LayerPanel::OnContextMenuLayerProperties)
wxEND_EVENT_TABLE()

LayerPanel::LayerPanel(wxWindow* parent, wxWindowID id)
    : wxPanel(parent, id)
    , m_toolbar(nullptr)
    , m_layerList(nullptr)
    , m_imageList(nullptr)
    , m_currentMap(nullptr)
    , m_layerManager(nullptr)
    , m_selectedLayerIndex(-1)
    , m_contextMenu(nullptr)
    , m_contextLayerIndex(-1)
{
    CreateControls();
    SetupImageList();
    SetupLayout();
}

LayerPanel::~LayerPanel()
{
    if (m_imageList)
    {
        delete m_imageList;
    }
    
    if (m_contextMenu)
    {
        delete m_contextMenu;
    }
}

void LayerPanel::CreateControls()
{
    CreateToolbar();
    CreateLayerList();
}

void LayerPanel::CreateToolbar()
{
    m_toolbar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                              wxTB_HORIZONTAL | wxTB_FLAT | wxTB_NODIVIDER);
    
    // Add layer tools
    m_toolbar->AddTool(ID_LAYER_ADD, "Adicionar Layer", 
                      wxArtProvider::GetBitmap(wxART_PLUS, wxART_TOOLBAR, wxSize(16, 16)),
                      "Adicionar nova layer");
    
    m_toolbar->AddTool(ID_LAYER_REMOVE, "Remover Layer",
                      wxArtProvider::GetBitmap(wxART_MINUS, wxART_TOOLBAR, wxSize(16, 16)),
                      "Remover layer selecionada");
    
    m_toolbar->AddSeparator();
    
    m_toolbar->AddTool(ID_LAYER_DUPLICATE, "Duplicar Layer",
                      wxArtProvider::GetBitmap(wxART_COPY, wxART_TOOLBAR, wxSize(16, 16)),
                      "Duplicar layer selecionada");
    
    m_toolbar->AddSeparator();
    
    m_toolbar->AddTool(ID_LAYER_MOVE_UP, "Mover para Cima",
                      wxArtProvider::GetBitmap(wxART_GO_UP, wxART_TOOLBAR, wxSize(16, 16)),
                      "Mover layer para cima");
    
    m_toolbar->AddTool(ID_LAYER_MOVE_DOWN, "Mover para Baixo",
                      wxArtProvider::GetBitmap(wxART_GO_DOWN, wxART_TOOLBAR, wxSize(16, 16)),
                      "Mover layer para baixo");
    
    m_toolbar->Realize();
}

void LayerPanel::CreateLayerList()
{
    m_layerList = new LayerListCtrl(this, ID_LAYER_LIST, wxDefaultPosition, wxDefaultSize,
                                   wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_NO_HEADER);
}

void LayerPanel::SetupImageList()
{
    // Create image list for layer icons
    m_imageList = new wxImageList(16, 16, true);
    
    // Load/create better icons for visibility and lock states
    m_visibleIcon = wxArtProvider::GetBitmap(wxART_FILE_OPEN, wxART_MENU, wxSize(16, 16));     // Eye-like: open = visible
    m_hiddenIcon = wxArtProvider::GetBitmap(wxART_MISSING_IMAGE, wxART_MENU, wxSize(16, 16));  // Missing = hidden
    m_lockedIcon = wxArtProvider::GetBitmap(wxART_ERROR, wxART_MENU, wxSize(16, 16));          // Lock icon
    m_unlockedIcon = wxArtProvider::GetBitmap(wxART_TICK_MARK, wxART_MENU, wxSize(16, 16));    // Unlocked
    
    // Add icons to image list
    m_imageList->Add(m_visibleIcon);   // Index 0
    m_imageList->Add(m_hiddenIcon);    // Index 1
    m_imageList->Add(m_lockedIcon);    // Index 2  
    m_imageList->Add(m_unlockedIcon);  // Index 3
    
    // Add layer type icons with better visual representation
    m_imageList->Add(wxArtProvider::GetBitmap(wxART_NEW, wxART_MENU, wxSize(16, 16)));         // Tile layer - Index 4
    m_imageList->Add(wxArtProvider::GetBitmap(wxART_FOLDER, wxART_MENU, wxSize(16, 16)));      // Background - Index 5  
    m_imageList->Add(wxArtProvider::GetBitmap(wxART_WARNING, wxART_MENU, wxSize(16, 16)));     // Collision - Index 6
    m_imageList->Add(wxArtProvider::GetBitmap(wxART_EXECUTABLE_FILE, wxART_MENU, wxSize(16, 16))); // Object - Index 7
    m_imageList->Add(wxArtProvider::GetBitmap(wxART_FIND, wxART_MENU, wxSize(16, 16)));        // Overlay - Index 8
    
    m_layerList->SetImageList(m_imageList, wxIMAGE_LIST_SMALL);
}

void LayerPanel::SetupLayout()
{
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    
    // Add toolbar
    mainSizer->Add(m_toolbar, 0, wxEXPAND | wxALL, 2);
    
    // Add layer list
    mainSizer->Add(m_layerList, 1, wxEXPAND | wxALL, 2);
    
    SetSizer(mainSizer);
}

void LayerPanel::SetMap(Map* map)
{
    m_currentMap = map;
    m_layerManager = map ? map->GetLayerManager() : nullptr;
    
    if (m_layerList)
    {
        m_layerList->SetLayerManager(m_layerManager);
    }
    
    RefreshLayerList();
}

void LayerPanel::RefreshLayerList()
{
    if (!m_layerList || !m_layerManager)
        return;
    
    // Clear existing items
    m_layerList->DeleteAllItems();
    
    // Add all layers (in reverse order - top layer first in list)
    int layerCount = m_layerManager->GetLayerCount();
    for (int i = layerCount - 1; i >= 0; i--)
    {
        Layer* layer = m_layerManager->GetLayer(i);
        if (layer)
        {
            AddLayerToList(layer, layerCount - 1 - i);
        }
    }
    
    // Select active layer
    int activeIndex = m_layerManager->GetActiveLayerIndex();
    if (activeIndex >= 0 && activeIndex < layerCount)
    {
        int listIndex = layerCount - 1 - activeIndex;
        m_layerList->SetItemState(listIndex, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        m_selectedLayerIndex = activeIndex;
    }
}

void LayerPanel::AddLayerToList(Layer* layer, int listIndex)
{
    if (!layer || !m_layerList)
        return;
    
    // Insert item
    long itemIndex = m_layerList->InsertItem(listIndex, "");
    
    // Set layer name and type
    m_layerList->SetItem(itemIndex, 2, layer->GetName());
    m_layerList->SetItem(itemIndex, 3, GetLayerTypeText(layer->GetType()));
    
    // Update visual state (visibility, lock, icons)
    UpdateLayerVisualState(itemIndex, layer);
}

void LayerPanel::UpdateLayerVisualState(int listIndex, Layer* layer)
{
    if (!layer || !m_layerList)
        return;
    
    // Set visibility icon
    wxListItem visItem;
    visItem.SetId(listIndex);
    visItem.SetColumn(0);
    visItem.SetImage(layer->IsVisible() ? 0 : 1);
    m_layerList->SetItem(visItem);
    
    // Set lock icon
    wxListItem lockItem;
    lockItem.SetId(listIndex);
    lockItem.SetColumn(1);
    lockItem.SetImage(layer->IsLocked() ? 2 : 3);
    m_layerList->SetItem(lockItem);
    
    // Set layer type icon in name column
    wxListItem nameItem;
    nameItem.SetId(listIndex);
    nameItem.SetColumn(2);
    nameItem.SetImage(GetLayerTypeIcon(layer->GetType()));
    m_layerList->SetItem(nameItem);
}

wxString LayerPanel::GetLayerTypeText(LayerType type)
{
    switch (type)
    {
        case LayerType::TILE_LAYER:       return "Tiles";
        case LayerType::BACKGROUND_LAYER: return "Fundo";
        case LayerType::COLLISION_LAYER:  return "Colisão";
        case LayerType::OBJECT_LAYER:     return "Objetos";
        case LayerType::OVERLAY_LAYER:    return "Overlay";
        default:                          return "Desconhecido";
    }
}

int LayerPanel::GetLayerTypeIcon(LayerType type)
{
    switch (type)
    {
        case LayerType::TILE_LAYER:       return 4;
        case LayerType::BACKGROUND_LAYER: return 5;
        case LayerType::COLLISION_LAYER:  return 6;
        case LayerType::OBJECT_LAYER:     return 7;
        case LayerType::OVERLAY_LAYER:    return 8;
        default:                          return 4;
    }
}

void LayerPanel::SelectLayer(int index)
{
    if (!m_layerList || !m_layerManager)
        return;
    
    int layerCount = m_layerManager->GetLayerCount();
    if (index < 0 || index >= layerCount)
        return;
    
    // Convert layer index to list index (reversed)
    int listIndex = layerCount - 1 - index;
    
    m_layerList->SetItemState(listIndex, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    m_selectedLayerIndex = index;
}

// ============================================================================
// Event Handlers
// ============================================================================

void LayerPanel::OnAddLayer(wxCommandEvent& event)
{
    CreateNewLayer();
}

void LayerPanel::OnRemoveLayer(wxCommandEvent& event)
{
    if (!CanRemoveLayer())
        return;
    
    if (m_selectedLayerIndex >= 0)
    {
        wxString layerName = m_layerManager->GetLayer(m_selectedLayerIndex)->GetName();
        
        int result = wxMessageBox(
            wxString::Format("Tem certeza que deseja excluir a layer '%s'?", layerName),
            "Confirmar exclusão",
            wxYES_NO | wxICON_QUESTION,
            this
        );
        
        if (result == wxYES)
        {
            m_layerManager->RemoveLayer(m_selectedLayerIndex);
            RefreshLayerList();
            BroadcastLayerPropertiesChanged(-1); // Notify map changed
        }
    }
}

void LayerPanel::OnDuplicateLayer(wxCommandEvent& event)
{
    if (m_selectedLayerIndex >= 0 && m_layerManager)
    {
        Layer* duplicatedLayer = m_layerManager->DuplicateLayer(m_selectedLayerIndex);
        if (duplicatedLayer)
        {
            RefreshLayerList();
            BroadcastLayerPropertiesChanged(-1);
        }
    }
}

void LayerPanel::OnMoveLayerUp(wxCommandEvent& event)
{
    if (m_selectedLayerIndex >= 0 && m_layerManager)
    {
        if (m_layerManager->MoveLayerUp(m_selectedLayerIndex))
        {
            m_selectedLayerIndex++;
            RefreshLayerList();
            SelectLayer(m_selectedLayerIndex);
            BroadcastLayerPropertiesChanged(-1);
        }
    }
}

void LayerPanel::OnMoveLayerDown(wxCommandEvent& event)
{
    if (m_selectedLayerIndex > 0 && m_layerManager)
    {
        if (m_layerManager->MoveLayerDown(m_selectedLayerIndex))
        {
            m_selectedLayerIndex--;
            RefreshLayerList();
            SelectLayer(m_selectedLayerIndex);
            BroadcastLayerPropertiesChanged(-1);
        }
    }
}

void LayerPanel::OnLayerSelected(wxListEvent& event)
{
    if (!m_layerManager)
        return;
    
    int listIndex = event.GetIndex();
    int layerCount = m_layerManager->GetLayerCount();
    
    // Convert list index to layer index (reversed)
    int layerIndex = layerCount - 1 - listIndex;
    
    if (layerIndex >= 0 && layerIndex < layerCount)
    {
        m_selectedLayerIndex = layerIndex;
        m_layerManager->SetActiveLayer(layerIndex);
        BroadcastLayerSelectionChanged(layerIndex);
    }
}

void LayerPanel::OnLayerRightClick(wxListEvent& event)
{
    int listIndex = event.GetIndex();
    int layerCount = m_layerManager ? m_layerManager->GetLayerCount() : 0;
    
    if (layerCount > 0)
    {
        m_contextLayerIndex = layerCount - 1 - listIndex;
        ShowContextMenu(event.GetPoint(), m_contextLayerIndex);
    }
}

void LayerPanel::OnLayerDoubleClick(wxListEvent& event)
{
    int listIndex = event.GetIndex();
    int layerCount = m_layerManager ? m_layerManager->GetLayerCount() : 0;
    
    if (layerCount > 0)
    {
        int layerIndex = layerCount - 1 - listIndex;
        RenameLayer(layerIndex);
    }
}

void LayerPanel::OnToggleVisibility(wxMouseEvent& event)
{
    if (!m_layerList || !m_layerManager)
    {
        event.Skip();
        return;
    }
    
    // Get position and find which item was clicked
    wxPoint position = event.GetPosition();
    int flags = 0;
    long hitItem = m_layerList->HitTest(position, flags);
    
    if (hitItem == wxNOT_FOUND)
    {
        event.Skip();
        return;
    }
    
    // Check if click was in the visibility column (column 0)
    wxRect itemRect;
    if (!m_layerList->GetItemRect(hitItem, itemRect))
    {
        event.Skip();
        return;
    }
    
    // Check if click was in first 24 pixels (visibility column)
    if (position.x <= 24)
    {
        // Convert list index to layer index (reversed)
        int layerCount = m_layerManager->GetLayerCount();
        int layerIndex = layerCount - 1 - hitItem;
        
        if (layerIndex >= 0 && layerIndex < layerCount)
        {
            Layer* layer = m_layerManager->GetLayer(layerIndex);
            if (layer)
            {
                // Toggle visibility
                layer->SetVisible(!layer->IsVisible());
                
                // Update visual state
                UpdateLayerVisualState(hitItem, layer);
                
                // Broadcast change
                BroadcastLayerPropertiesChanged(layerIndex);
                
                wxLogMessage("Layer '%s' visibilidade alterada para: %s", 
                           layer->GetName(), layer->IsVisible() ? "visível" : "oculta");
                
                return; // Don't skip - we handled it
            }
        }
    }
    // Check if click was in lock column (column 1, between pixels 24-48)
    else if (position.x > 24 && position.x <= 48)
    {
        // Convert list index to layer index (reversed)
        int layerCount = m_layerManager->GetLayerCount();
        int layerIndex = layerCount - 1 - hitItem;
        
        if (layerIndex >= 0 && layerIndex < layerCount)
        {
            Layer* layer = m_layerManager->GetLayer(layerIndex);
            if (layer)
            {
                // Toggle lock state
                layer->SetLocked(!layer->IsLocked());
                
                // Update visual state
                UpdateLayerVisualState(hitItem, layer);
                
                // Broadcast change
                BroadcastLayerPropertiesChanged(layerIndex);
                
                wxLogMessage("Layer '%s' bloqueio alterado para: %s", 
                           layer->GetName(), layer->IsLocked() ? "bloqueada" : "desbloqueada");
                
                return; // Don't skip - we handled it
            }
        }
    }
    
    event.Skip();
}

void LayerPanel::ShowContextMenu(const wxPoint& pos, int layerIndex)
{
    if (!m_contextMenu)
    {
        m_contextMenu = new wxMenu;
        m_contextMenu->Append(ID_CONTEXT_NEW_LAYER, "Nova Layer...");
        m_contextMenu->AppendSeparator();
        m_contextMenu->Append(ID_CONTEXT_DUPLICATE_LAYER, "Duplicar Layer");
        m_contextMenu->Append(ID_CONTEXT_DELETE_LAYER, "Excluir Layer");
        m_contextMenu->AppendSeparator();
        m_contextMenu->Append(ID_CONTEXT_RENAME_LAYER, "Renomear...");
        m_contextMenu->Append(ID_CONTEXT_LAYER_PROPERTIES, "Propriedades...");
    }
    
    // Enable/disable menu items based on context
    bool canDelete = CanRemoveLayer();
    m_contextMenu->Enable(ID_CONTEXT_DELETE_LAYER, canDelete);
    
    PopupMenu(m_contextMenu, pos);
}

void LayerPanel::CreateNewLayer(LayerType type)
{
    if (!m_layerManager)
        return;
    
    LayerProperties props;
    props.name = m_layerManager->GenerateUniqueLayerName("Nova Layer");
    props.type = type;
    
    Layer* newLayer = m_layerManager->CreateLayer(props);
    if (newLayer)
    {
        RefreshLayerList();
        
        // Select the new layer
        int newLayerIndex = m_layerManager->GetLayerCount() - 1;
        SelectLayer(newLayerIndex);
        m_layerManager->SetActiveLayer(newLayerIndex);
        
        BroadcastLayerPropertiesChanged(-1);
        
        // Optionally prompt for rename
        RenameLayer(newLayerIndex);
    }
}

void LayerPanel::RenameLayer(int index)
{
    if (!m_layerManager || index < 0 || index >= m_layerManager->GetLayerCount())
        return;
    
    Layer* layer = m_layerManager->GetLayer(index);
    if (!layer)
        return;
    
    wxString newName = wxGetTextFromUser(
        "Nome da layer:",
        "Renomear Layer",
        layer->GetName(),
        this
    );
    
    if (!newName.IsEmpty() && newName != layer->GetName())
    {
        layer->SetName(newName);
        RefreshLayerList();
        BroadcastLayerPropertiesChanged(index);
    }
}

bool LayerPanel::CanRemoveLayer() const
{
    return m_layerManager && m_layerManager->GetLayerCount() > 1 && m_selectedLayerIndex >= 0;
}

// ============================================================================
// Context Menu Handlers
// ============================================================================

void LayerPanel::OnContextMenuNewLayer(wxCommandEvent& event)
{
    CreateNewLayer();
}

void LayerPanel::OnContextMenuDeleteLayer(wxCommandEvent& event)
{
    if (m_contextLayerIndex >= 0)
    {
        m_selectedLayerIndex = m_contextLayerIndex;
        OnRemoveLayer(event);
    }
}

void LayerPanel::OnContextMenuDuplicateLayer(wxCommandEvent& event)
{
    if (m_contextLayerIndex >= 0)
    {
        m_selectedLayerIndex = m_contextLayerIndex;
        OnDuplicateLayer(event);
    }
}

void LayerPanel::OnContextMenuRenameLayer(wxCommandEvent& event)
{
    if (m_contextLayerIndex >= 0)
    {
        RenameLayer(m_contextLayerIndex);
    }
}

void LayerPanel::OnContextMenuLayerProperties(wxCommandEvent& event)
{
    // TODO: Implement layer properties dialog
    wxMessageBox("Propriedades da layer - Implementação futura", "Info", wxOK | wxICON_INFORMATION, this);
}

// ============================================================================
// Communication Methods
// ============================================================================

void LayerPanel::BroadcastLayerSelectionChanged(int layerIndex)
{
    // TODO: Implement event broadcasting to other panels
    // For now, just trigger a basic event
    wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, wxID_ANY);
    GetParent()->GetEventHandler()->ProcessEvent(evt);
}

void LayerPanel::BroadcastLayerPropertiesChanged(int layerIndex)
{
    // TODO: Implement event broadcasting to other panels
    wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, wxID_ANY);
    GetParent()->GetEventHandler()->ProcessEvent(evt);
}

// ============================================================================
// Public Interface Methods
// ============================================================================

void LayerPanel::OnLayerSelectionChanged(int layerIndex)
{
    SelectLayer(layerIndex);
}

void LayerPanel::OnLayerPropertiesChanged(int layerIndex)
{
    RefreshLayerList();
}