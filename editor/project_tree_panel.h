/**
 * Project Tree Panel - Árvore do projeto com estrutura hierárquica
 */

#pragma once

#include <wx/wx.h>
#include <wx/treectrl.h>

class ProjectTreePanel : public wxPanel
{
public:
    ProjectTreePanel(wxWindow* parent);
    virtual ~ProjectTreePanel() = default;

private:
    void CreateControls();
    void PopulateTree();
    
    // Event handlers
    void OnTreeSelChanged(wxTreeEvent& event);
    void OnTreeItemActivated(wxTreeEvent& event);
    void OnTreeRightClick(wxTreeEvent& event);
    
    // Controles
    wxTreeCtrl* m_treeCtrl;
    
    // IDs dos nós principais
    wxTreeItemId m_rootId;
    wxTreeItemId m_mapsId;
    wxTreeItemId m_dataId;
    wxTreeItemId m_assetsId;

    wxDECLARE_EVENT_TABLE();
};
