/**
 * Editor Frame - Janela principal do editor Lumy
 * Implementa wxAUI com panes para árvore do projeto, property grid e viewport GL
 */

#pragma once

#include <wx/wx.h>
#include <wx/aui/aui.h>
#include <wx/treectrl.h>
#include <wx/propgrid/propgrid.h>
#include <wx/glcanvas.h>
#include <memory>
#include "file_watcher.h"
#include "editor_events.h"
#include "project_manager.h"
#include "map_manager.h"
#include "map.h"

// Forward declarations
class ProjectTreePanel;
class PropertyGridPanel;  
class ViewportPanel;
class TilesetPanel;

class EditorFrame : public wxFrame
{
public:
    EditorFrame();
    virtual ~EditorFrame();

private:
    // UI Setup
    void CreateMenuBar();
    void CreateStatusBar();
    void CreateAuiPanes();
    void LoadAuiPerspective();
    
    // Event handlers
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);
    void OnNewProject(wxCommandEvent& event);
    void OnOpenProject(wxCommandEvent& event);
    void OnSaveProject(wxCommandEvent& event);
    void OnTilesetSelectionChanged(wxCommandEvent& event);
    
    // Event handlers para mapas
    void OnNewMap(wxCommandEvent& event);
    void OnOpenMap(wxCommandEvent& event);
    void OnSaveMap(wxCommandEvent& event);
    void OnSaveMapAs(wxCommandEvent& event);
    
    // AUI Manager
    wxAuiManager m_auiManager;
    
    // Panes principais
    std::unique_ptr<ProjectTreePanel> m_projectTree;
    std::unique_ptr<PropertyGridPanel> m_propertyGrid;
    std::unique_ptr<ViewportPanel> m_viewport;
    std::unique_ptr<TilesetPanel> m_tilesetPanel;
    
    // Hot-reload system
    std::unique_ptr<FileWatcher> m_fileWatcher;
    wxString m_currentProjectPath;
    
    // Project management system
    std::unique_ptr<ProjectManager> m_projectManager;
    
    // Map management system
    std::unique_ptr<MapManager> m_mapManager;
    wxString m_currentMapPath;
    
    // Hot-reload callbacks
    void OnMapFileChanged(const wxString& path, const wxString& filename);
    void OnDataFileChanged(const wxString& path, const wxString& filename);
    
    // Project management
    bool LoadProject(const wxString& projectPath);
    void SetupHotReload();
    
    // Communication between panes
    void OnSelectionChanged(SelectionChangeEvent& event);
    void OnPropertyChanged(PropertyChangeEvent& event);
    void OnProjectChanged(ProjectChangeEvent& event);
    
    // Broadcasting methods
    void BroadcastSelectionChange(const SelectionInfo& info);
    void BroadcastPropertyChange(const wxString& propertyName, const wxVariant& value);
    void BroadcastProjectChange(const wxString& projectPath, bool loaded);
    
    // Métodos auxiliares para mapas
    bool IsMapInMapsFolder(const wxString& filePath);
    void LoadMapFromPath(const wxString& filePath);

    wxDECLARE_EVENT_TABLE();
};

// Event IDs
enum
{
    ID_NewProject = wxID_HIGHEST + 1,
    ID_OpenProject,
    ID_SaveProject,
    // IDs para mapas
    ID_NewMap,
    ID_OpenMap,
    ID_SaveMap,
    ID_SaveMapAs
};
