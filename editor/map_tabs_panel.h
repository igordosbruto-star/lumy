/**
 * MapTabsPanel - Painel com abas para múltiplos mapas abertos
 * Permite alternar entre diferentes mapas usando abas
 */

#pragma once

#include <wx/wx.h>
#include <wx/aui/auibook.h>
#include <memory>
#include <vector>
#include <map>

// Forward declarations
class ViewportPanel;
class Map;

struct MapTabInfo
{
    wxString filePath;
    wxString displayName;
    std::shared_ptr<Map> map;
    bool isModified;
    ViewportPanel* viewport;
    
    MapTabInfo() : isModified(false), viewport(nullptr) {}
};

class MapTabsPanel : public wxPanel
{
public:
    MapTabsPanel(wxWindow* parent, wxWindowID id = wxID_ANY);
    virtual ~MapTabsPanel();
    
    // Map management
    int AddMap(std::shared_ptr<Map> map, const wxString& filePath = wxEmptyString);
    bool CloseMap(int tabIndex);
    bool CloseAllMaps();
    void SetActiveMap(int tabIndex);
    
    // Current map access
    std::shared_ptr<Map> GetCurrentMap();
    ViewportPanel* GetCurrentViewport();
    int GetCurrentMapIndex() const;
    
    // Map info
    bool HasModifiedMaps() const;
    std::vector<wxString> GetModifiedMapPaths() const;
    int GetMapCount() const { return static_cast<int>(m_mapTabs.size()); }
    
    // File operations
    void SetMapPath(int tabIndex, const wxString& filePath);
    void SetMapModified(int tabIndex, bool modified = true);
    wxString GetMapPath(int tabIndex) const;
    bool IsMapModified(int tabIndex) const;
    
    // UI updates
    void UpdateTabTitle(int tabIndex);
    void UpdateAllTabTitles();
    
private:
    void CreateControls();
    void CreateDefaultTab();
    
    // Event handlers
    void OnTabChanged(wxAuiNotebookEvent& event);
    void OnTabClose(wxAuiNotebookEvent& event);
    void OnTabRightClick(wxAuiNotebookEvent& event);
    
    // Helper methods
    wxString GetDisplayName(const wxString& filePath) const;
    wxString GetTabTitle(const MapTabInfo& tabInfo) const;
    ViewportPanel* CreateViewportForMap(std::shared_ptr<Map> map);
    
    // Context menu
    void ShowTabContextMenu(const wxPoint& pos, int tabIndex);
    void OnContextMenuClose(wxCommandEvent& event);
    void OnContextMenuCloseOthers(wxCommandEvent& event);
    void OnContextMenuCloseAll(wxCommandEvent& event);
    void OnContextMenuSave(wxCommandEvent& event);
    void OnContextMenuSaveAs(wxCommandEvent& event);
    
    // UI Controls
    wxAuiNotebook* m_notebook;
    
    // Data
    std::vector<MapTabInfo> m_mapTabs;
    int m_currentTabIndex;
    int m_nextUntitledNumber;
    
    // Context menu
    wxMenu* m_contextMenu;
    int m_contextTabIndex;
    
    wxDECLARE_EVENT_TABLE();
};

// Event IDs
enum MapTabsEventIds
{
    ID_MAP_TABS_NOTEBOOK = wxID_HIGHEST + 300,
    
    // Context menu
    ID_CONTEXT_TAB_CLOSE,
    ID_CONTEXT_TAB_CLOSE_OTHERS,
    ID_CONTEXT_TAB_CLOSE_ALL,
    ID_CONTEXT_TAB_SAVE,
    ID_CONTEXT_TAB_SAVE_AS
};