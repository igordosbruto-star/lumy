/**
 * MapTabsPanel - Implementação do painel de abas para múltiplos mapas
 */

#pragma execution_character_set("utf-8")

#include "map_tabs_panel.h"
#include "viewport_panel.h"
#include "map.h"
#include "utf8_strings.h"
#include <wx/filename.h>
#include <wx/msgdlg.h>

// ============================================================================
// Event Table
// ============================================================================

wxBEGIN_EVENT_TABLE(MapTabsPanel, wxPanel)
    EVT_AUINOTEBOOK_PAGE_CHANGED(ID_MAP_TABS_NOTEBOOK, MapTabsPanel::OnTabChanged)
    EVT_AUINOTEBOOK_PAGE_CLOSE(ID_MAP_TABS_NOTEBOOK, MapTabsPanel::OnTabClose)
    EVT_AUINOTEBOOK_TAB_RIGHT_DOWN(ID_MAP_TABS_NOTEBOOK, MapTabsPanel::OnTabRightClick)
    
    // Context menu events
    EVT_MENU(ID_CONTEXT_TAB_CLOSE, MapTabsPanel::OnContextMenuClose)
    EVT_MENU(ID_CONTEXT_TAB_CLOSE_OTHERS, MapTabsPanel::OnContextMenuCloseOthers)
    EVT_MENU(ID_CONTEXT_TAB_CLOSE_ALL, MapTabsPanel::OnContextMenuCloseAll)
    EVT_MENU(ID_CONTEXT_TAB_SAVE, MapTabsPanel::OnContextMenuSave)
    EVT_MENU(ID_CONTEXT_TAB_SAVE_AS, MapTabsPanel::OnContextMenuSaveAs)
wxEND_EVENT_TABLE()

// ============================================================================
// Constructor / Destructor
// ============================================================================

MapTabsPanel::MapTabsPanel(wxWindow* parent, wxWindowID id)
    : wxPanel(parent, id)
    , m_notebook(nullptr)
    , m_currentTabIndex(-1)
    , m_nextUntitledNumber(1)
    , m_contextMenu(nullptr)
    , m_contextTabIndex(-1)
{
    CreateControls();
    CreateDefaultTab();
}

MapTabsPanel::~MapTabsPanel()
{
    if (m_contextMenu) {
        delete m_contextMenu;
    }
}

// ============================================================================
// UI Creation
// ============================================================================

void MapTabsPanel::CreateControls()
{
    // Create AUI Notebook with style for closeable tabs
    long style = wxAUI_NB_TOP | 
                 wxAUI_NB_TAB_SPLIT | 
                 wxAUI_NB_TAB_MOVE | 
                 wxAUI_NB_SCROLL_BUTTONS |
                 wxAUI_NB_CLOSE_ON_ACTIVE_TAB |
                 wxAUI_NB_MIDDLE_CLICK_CLOSE;
    
    m_notebook = new wxAuiNotebook(this, ID_MAP_TABS_NOTEBOOK, 
                                   wxDefaultPosition, wxDefaultSize, style);
    
    // Layout
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(m_notebook, 1, wxEXPAND);
    SetSizer(sizer);
    
    // Create context menu
    m_contextMenu = new wxMenu();
    m_contextMenu->Append(ID_CONTEXT_TAB_CLOSE, UTF8("&Fechar\tCtrl+W"));
    m_contextMenu->Append(ID_CONTEXT_TAB_CLOSE_OTHERS, UTF8("Fechar &Outras"));
    m_contextMenu->Append(ID_CONTEXT_TAB_CLOSE_ALL, UTF8("Fechar &Todas"));
    m_contextMenu->AppendSeparator();
    m_contextMenu->Append(ID_CONTEXT_TAB_SAVE, UTF8("&Salvar\tCtrl+S"));
    m_contextMenu->Append(ID_CONTEXT_TAB_SAVE_AS, UTF8("Salvar &Como...\tCtrl+Shift+S"));
}

void MapTabsPanel::CreateDefaultTab()
{
    // Create a default untitled map
    auto defaultMap = std::make_shared<Map>(25, 15, 32);
    AddMap(defaultMap, wxEmptyString);
}

// ============================================================================
// Map Management
// ============================================================================

int MapTabsPanel::AddMap(std::shared_ptr<Map> map, const wxString& filePath)
{
    if (!map) {
        wxLogError("Tentativa de adicionar mapa nulo");
        return -1;
    }
    
    // Create tab info
    MapTabInfo tabInfo;
    tabInfo.map = map;
    tabInfo.filePath = filePath;
    tabInfo.isModified = false;
    
    // Determine display name
    if (filePath.IsEmpty()) {
        tabInfo.displayName = wxString::Format("Sem título %d", m_nextUntitledNumber++);
    } else {
        tabInfo.displayName = GetDisplayName(filePath);
    }
    
    // Create viewport for this map
    tabInfo.viewport = CreateViewportForMap(map);
    
    // Add to notebook
    wxString tabTitle = GetTabTitle(tabInfo);
    m_notebook->AddPage(tabInfo.viewport, tabTitle, true);
    
    // Add to our data structure
    m_mapTabs.push_back(tabInfo);
    int newIndex = static_cast<int>(m_mapTabs.size()) - 1;
    
    // Set as current
    m_currentTabIndex = newIndex;
    m_notebook->SetSelection(newIndex);
    
    wxLogMessage("Mapa adicionado: %s (aba %d)", tabInfo.displayName, newIndex);
    return newIndex;
}

bool MapTabsPanel::CloseMap(int tabIndex)
{
    if (tabIndex < 0 || tabIndex >= static_cast<int>(m_mapTabs.size())) {
        return false;
    }
    
    const MapTabInfo& tabInfo = m_mapTabs[tabIndex];
    
    // Check if modified
    if (tabInfo.isModified) {
        wxString message = wxString::Format(
            UTF8("O mapa '%s' possui alterações não salvas.\nDeseja salvar antes de fechar?"),
            tabInfo.displayName
        );
        
        int result = wxMessageBox(message, UTF8("Salvar alterações?"),
                                 wxYES_NO | wxCANCEL | wxICON_QUESTION, this);
        
        if (result == wxCANCEL) {
            return false; // User cancelled
        }
        
        if (result == wxYES) {
            // TODO: Implement save logic
            wxLogMessage("TODO: Salvar mapa antes de fechar");
        }
    }
    
    // Se esta é a última aba, criar uma nova ANTES de remover
    bool isLastTab = (m_mapTabs.size() == 1);
    if (isLastTab) {
        // Criar nova aba padrão primeiro
        CreateDefaultTab();
        // Agora temos 2 abas, remover a antiga (que agora está no índice 0)
        tabIndex = 0;
    }
    
    // Remove from notebook
    m_notebook->DeletePage(tabIndex);
    
    // Remove from data structure
    m_mapTabs.erase(m_mapTabs.begin() + tabIndex);
    
    // Update current index
    if (m_currentTabIndex >= tabIndex && m_currentTabIndex > 0) {
        m_currentTabIndex--;
    } else if (isLastTab) {
        // Se fechamos a última aba, o índice atual é 0 (a nova aba)
        m_currentTabIndex = 0;
    }
    
    wxLogMessage("Mapa fechado (aba %d)", tabIndex);
    return true;
}

bool MapTabsPanel::CloseAllMaps()
{
    // Check for modified maps
    std::vector<int> modifiedIndices;
    for (size_t i = 0; i < m_mapTabs.size(); ++i) {
        if (m_mapTabs[i].isModified) {
            modifiedIndices.push_back(static_cast<int>(i));
        }
    }
    
    // Ask user about modified maps
    if (!modifiedIndices.empty()) {
        wxString message = wxString::Format(
            UTF8("Há %d mapa(s) com alterações não salvas.\nDeseja salvar antes de fechar todos?"),
            static_cast<int>(modifiedIndices.size())
        );
        
        int result = wxMessageBox(message, UTF8("Salvar alterações?"),
                                 wxYES_NO | wxCANCEL | wxICON_QUESTION, this);
        
        if (result == wxCANCEL) {
            return false;
        }
        
        if (result == wxYES) {
            // TODO: Implement save all logic
            wxLogMessage("TODO: Salvar todos os mapas modificados");
        }
    }
    
    // Close all tabs
    m_notebook->DeleteAllPages();
    m_mapTabs.clear();
    m_currentTabIndex = -1;
    
    // Create default tab
    CreateDefaultTab();
    
    wxLogMessage("Todos os mapas fechados");
    return true;
}

void MapTabsPanel::SetActiveMap(int tabIndex)
{
    if (tabIndex < 0 || tabIndex >= static_cast<int>(m_mapTabs.size())) {
        return;
    }
    
    m_currentTabIndex = tabIndex;
    m_notebook->SetSelection(tabIndex);
}

// ============================================================================
// Current Map Access
// ============================================================================

std::shared_ptr<Map> MapTabsPanel::GetCurrentMap()
{
    if (m_currentTabIndex < 0 || m_currentTabIndex >= static_cast<int>(m_mapTabs.size())) {
        return nullptr;
    }
    
    return m_mapTabs[m_currentTabIndex].map;
}

ViewportPanel* MapTabsPanel::GetCurrentViewport()
{
    if (m_currentTabIndex < 0 || m_currentTabIndex >= static_cast<int>(m_mapTabs.size())) {
        return nullptr;
    }
    
    return m_mapTabs[m_currentTabIndex].viewport;
}

int MapTabsPanel::GetCurrentMapIndex() const
{
    return m_currentTabIndex;
}

// ============================================================================
// Map Info
// ============================================================================

bool MapTabsPanel::HasModifiedMaps() const
{
    for (const auto& tabInfo : m_mapTabs) {
        if (tabInfo.isModified) {
            return true;
        }
    }
    return false;
}

std::vector<wxString> MapTabsPanel::GetModifiedMapPaths() const
{
    std::vector<wxString> paths;
    for (const auto& tabInfo : m_mapTabs) {
        if (tabInfo.isModified && !tabInfo.filePath.IsEmpty()) {
            paths.push_back(tabInfo.filePath);
        }
    }
    return paths;
}

// ============================================================================
// File Operations
// ============================================================================

void MapTabsPanel::SetMapPath(int tabIndex, const wxString& filePath)
{
    if (tabIndex < 0 || tabIndex >= static_cast<int>(m_mapTabs.size())) {
        return;
    }
    
    m_mapTabs[tabIndex].filePath = filePath;
    m_mapTabs[tabIndex].displayName = GetDisplayName(filePath);
    UpdateTabTitle(tabIndex);
}

void MapTabsPanel::SetMapModified(int tabIndex, bool modified)
{
    if (tabIndex < 0 || tabIndex >= static_cast<int>(m_mapTabs.size())) {
        return;
    }
    
    m_mapTabs[tabIndex].isModified = modified;
    UpdateTabTitle(tabIndex);
}

wxString MapTabsPanel::GetMapPath(int tabIndex) const
{
    if (tabIndex < 0 || tabIndex >= static_cast<int>(m_mapTabs.size())) {
        return wxEmptyString;
    }
    
    return m_mapTabs[tabIndex].filePath;
}

bool MapTabsPanel::IsMapModified(int tabIndex) const
{
    if (tabIndex < 0 || tabIndex >= static_cast<int>(m_mapTabs.size())) {
        return false;
    }
    
    return m_mapTabs[tabIndex].isModified;
}

// ============================================================================
// UI Updates
// ============================================================================

void MapTabsPanel::UpdateTabTitle(int tabIndex)
{
    if (tabIndex < 0 || tabIndex >= static_cast<int>(m_mapTabs.size())) {
        return;
    }
    
    wxString title = GetTabTitle(m_mapTabs[tabIndex]);
    m_notebook->SetPageText(tabIndex, title);
}

void MapTabsPanel::UpdateAllTabTitles()
{
    for (size_t i = 0; i < m_mapTabs.size(); ++i) {
        UpdateTabTitle(static_cast<int>(i));
    }
}

// ============================================================================
// Event Handlers
// ============================================================================

void MapTabsPanel::OnTabChanged(wxAuiNotebookEvent& event)
{
    int newSelection = event.GetSelection();
    
    if (newSelection >= 0 && newSelection < static_cast<int>(m_mapTabs.size())) {
        m_currentTabIndex = newSelection;
        
        wxLogMessage("Aba alterada para: %s", m_mapTabs[newSelection].displayName);
        
        // Notify parent that active map changed
        // TODO: Send custom event to parent (EditorFrame)
    }
    
    event.Skip();
}

void MapTabsPanel::OnTabClose(wxAuiNotebookEvent& event)
{
    int tabIndex = event.GetSelection();
    
    // Veto the event to handle closing ourselves
    event.Veto();
    
    // Close the tab (with modified check)
    CloseMap(tabIndex);
}

void MapTabsPanel::OnTabRightClick(wxAuiNotebookEvent& event)
{
    m_contextTabIndex = event.GetSelection();
    
    // Show context menu at mouse position
    wxPoint mousePos = wxGetMousePosition();
    mousePos = ScreenToClient(mousePos);
    
    ShowTabContextMenu(mousePos, m_contextTabIndex);
}

// ============================================================================
// Context Menu
// ============================================================================

void MapTabsPanel::ShowTabContextMenu(const wxPoint& pos, int tabIndex)
{
    if (tabIndex < 0 || tabIndex >= static_cast<int>(m_mapTabs.size())) {
        return;
    }
    
    // Enable/disable menu items based on state
    bool hasOthers = m_mapTabs.size() > 1;
    m_contextMenu->Enable(ID_CONTEXT_TAB_CLOSE_OTHERS, hasOthers);
    
    bool hasPath = !m_mapTabs[tabIndex].filePath.IsEmpty();
    m_contextMenu->Enable(ID_CONTEXT_TAB_SAVE, m_mapTabs[tabIndex].isModified);
    
    PopupMenu(m_contextMenu, pos);
}

void MapTabsPanel::OnContextMenuClose(wxCommandEvent& WXUNUSED(event))
{
    if (m_contextTabIndex >= 0) {
        CloseMap(m_contextTabIndex);
    }
}

void MapTabsPanel::OnContextMenuCloseOthers(wxCommandEvent& WXUNUSED(event))
{
    if (m_contextTabIndex < 0) {
        return;
    }
    
    // Close all except the context tab
    // Work backwards to avoid index issues
    for (int i = static_cast<int>(m_mapTabs.size()) - 1; i >= 0; --i) {
        if (i != m_contextTabIndex) {
            CloseMap(i);
            // Adjust context tab index if necessary
            if (i < m_contextTabIndex) {
                m_contextTabIndex--;
            }
        }
    }
}

void MapTabsPanel::OnContextMenuCloseAll(wxCommandEvent& WXUNUSED(event))
{
    CloseAllMaps();
}

void MapTabsPanel::OnContextMenuSave(wxCommandEvent& WXUNUSED(event))
{
    if (m_contextTabIndex >= 0) {
        // TODO: Implement save logic
        wxLogMessage("TODO: Salvar mapa na aba %d", m_contextTabIndex);
    }
}

void MapTabsPanel::OnContextMenuSaveAs(wxCommandEvent& WXUNUSED(event))
{
    if (m_contextTabIndex >= 0) {
        // TODO: Implement save as logic
        wxLogMessage("TODO: Salvar mapa como... na aba %d", m_contextTabIndex);
    }
}

// ============================================================================
// Helper Methods
// ============================================================================

wxString MapTabsPanel::GetDisplayName(const wxString& filePath) const
{
    if (filePath.IsEmpty()) {
        return wxString::Format("Sem título %d", m_nextUntitledNumber);
    }
    
    wxFileName fileName(filePath);
    return fileName.GetName(); // Just the filename without extension
}

wxString MapTabsPanel::GetTabTitle(const MapTabInfo& tabInfo) const
{
    wxString title = tabInfo.displayName;
    
    // Add asterisk if modified
    if (tabInfo.isModified) {
        title += " *";
    }
    
    return title;
}

ViewportPanel* MapTabsPanel::CreateViewportForMap(std::shared_ptr<Map> map)
{
    // Create viewport panel
    ViewportPanel* viewport = new ViewportPanel(m_notebook);
    
    // Set the map
    viewport->SetCurrentMap(map.get());
    
    return viewport;
}
