/**
 * LeftSidePanel - Painel composto que combina árvore de projetos e paleta de tiles
 * Garante que ambos ficam na mesma largura e empilhados verticalmente
 */

#pragma once

#include <wx/wx.h>
#include <wx/splitter.h>
#include <memory>

// Forward declarations
class ProjectTreePanel;
class TilesetPanel;

class LeftSidePanel : public wxPanel
{
public:
    LeftSidePanel(wxWindow* parent, wxWindowID id = wxID_ANY);
    virtual ~LeftSidePanel();
    
    // Access to child panels
    ProjectTreePanel* GetProjectTree() { return m_projectTree.get(); }
    TilesetPanel* GetTilesetPanel() { return m_tilesetPanel.get(); }
    
    // Layout control
    void SetSplitterPosition(int position);
    int GetSplitterPosition() const;
    void SetProportionalSplitterPosition(double proportion = 0.4); // 40% for project tree by default
    
private:
    void CreateControls();
    void OnSplitterChanged(wxSplitterEvent& event);
    
    // UI Controls
    wxSplitterWindow* m_splitter;
    
    // Child panels
    std::unique_ptr<ProjectTreePanel> m_projectTree;
    std::unique_ptr<TilesetPanel> m_tilesetPanel;
    
    wxDECLARE_EVENT_TABLE();
};