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

// Forward declarations
class ProjectTreePanel;
class PropertyGridPanel;  
class ViewportPanel;

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
    
    // AUI Manager
    wxAuiManager m_auiManager;
    
    // Panes principais
    std::unique_ptr<ProjectTreePanel> m_projectTree;
    std::unique_ptr<PropertyGridPanel> m_propertyGrid;
    std::unique_ptr<ViewportPanel> m_viewport;

    wxDECLARE_EVENT_TABLE();
};

// Event IDs
enum
{
    ID_NewProject = wxID_HIGHEST + 1,
    ID_OpenProject,
    ID_SaveProject
};
