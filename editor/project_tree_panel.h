/**
 * Project Tree Panel - Árvore do projeto com estrutura hierárquica
 */

#pragma execution_character_set("utf-8")

#pragma once

#include <wx/wx.h>
#include <wx/treectrl.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/ffile.h>
#include "editor_events.h"

class ProjectTreePanel : public wxPanel
{
public:
    ProjectTreePanel(wxWindow* parent);
    virtual ~ProjectTreePanel() = default;
    
    // Métodos públicos
    void RefreshMapList();
    void SetProjectPath(const wxString& path);

private:
    void CreateControls();
    void PopulateTree();
    
    // Event handlers
    void OnTreeSelChanged(wxTreeEvent& event);
    void OnTreeItemActivated(wxTreeEvent& event);
    void OnTreeRightClick(wxTreeEvent& event);
    
    // Communication event handlers
    void OnSelectionChanged(SelectionChangeEvent& event);
    void OnProjectChanged(ProjectChangeEvent& event);
    
    // Helper methods
    SelectionType GetItemType(const wxTreeItemId& item) const;
    void NotifySelection(const wxTreeItemId& item);
    void ScanForMaps();
    void RefreshMaps();
    void ScanDirectoryForMaps(const wxString& directory);
    bool IsMapFile(const wxString& filePath);
    
    // Métodos auxiliares para gerenciamento de mapas
    bool IsMapInMapsSection(const wxTreeItemId& item) const;
    void RequestMapChange(const wxString& mapPath);
    
    // Controles
    wxTreeCtrl* m_treeCtrl;
    
    // IDs dos nós principais
    wxTreeItemId m_rootId;
    wxTreeItemId m_mapsId;
    wxTreeItemId m_dataId;
    wxTreeItemId m_assetsId;
    
    // Caminho do projeto atual
    wxString m_projectPath;

    wxDECLARE_EVENT_TABLE();
};
