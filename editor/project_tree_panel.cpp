/**
 * Implementação do Project Tree Panel
 */

#pragma execution_character_set("utf-8")

#include "project_tree_panel.h"
#include "utf8_strings.h"
#include <wx/imaglist.h>

wxBEGIN_EVENT_TABLE(ProjectTreePanel, wxPanel)
    EVT_TREE_SEL_CHANGED(wxID_ANY, ProjectTreePanel::OnTreeSelChanged)
    EVT_TREE_ITEM_ACTIVATED(wxID_ANY, ProjectTreePanel::OnTreeItemActivated)
    EVT_TREE_ITEM_RIGHT_CLICK(wxID_ANY, ProjectTreePanel::OnTreeRightClick)
    // Eventos customizados
    EVT_SELECTION_CHANGE(ProjectTreePanel::OnSelectionChanged)
    EVT_PROJECT_CHANGE(ProjectTreePanel::OnProjectChanged)
wxEND_EVENT_TABLE()

ProjectTreePanel::ProjectTreePanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY)
{
    CreateControls();
    PopulateTree();
}

void ProjectTreePanel::CreateControls()
{
    // Criar árvore com estilo adequado
    m_treeCtrl = new wxTreeCtrl(this, wxID_ANY, 
        wxDefaultPosition, wxDefaultSize,
        wxTR_DEFAULT_STYLE | wxTR_EDIT_LABELS | wxTR_SINGLE
    );
    
    // Layout
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    
    // Título
    wxStaticText* title = new wxStaticText(this, wxID_ANY, "Projeto");
    wxFont titleFont = title->GetFont();
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    title->SetFont(titleFont);
    
    sizer->Add(title, 0, wxALL | wxEXPAND, 5);
    sizer->Add(m_treeCtrl, 1, wxALL | wxEXPAND, 5);
    
    SetSizer(sizer);
}

void ProjectTreePanel::PopulateTree()
{
    // Limpar árvore existente
    m_treeCtrl->DeleteAllItems();
    
    // Criar nó raiz
    m_rootId = m_treeCtrl->AddRoot("Projeto Lumy", -1, -1, nullptr);
    
    // Estrutura básica de projeto
    m_mapsId = m_treeCtrl->AppendItem(m_rootId, "Mapas", -1, -1, nullptr);
    m_dataId = m_treeCtrl->AppendItem(m_rootId, "Database", -1, -1, nullptr);
    m_assetsId = m_treeCtrl->AppendItem(m_rootId, "Assets", -1, -1, nullptr);
    
    // Submapas de exemplo (quando há um projeto carregado)
    m_treeCtrl->AppendItem(m_mapsId, "hello.tmx", -1, -1, nullptr);
    
    // Database de exemplo
    m_treeCtrl->AppendItem(m_dataId, "actors.json", -1, -1, nullptr);
    m_treeCtrl->AppendItem(m_dataId, "items.json", -1, -1, nullptr);
    m_treeCtrl->AppendItem(m_dataId, "skills.json", -1, -1, nullptr);
    m_treeCtrl->AppendItem(m_dataId, "states.json", -1, -1, nullptr);
    m_treeCtrl->AppendItem(m_dataId, "enemies.json", -1, -1, nullptr);
    m_treeCtrl->AppendItem(m_dataId, "system.json", -1, -1, nullptr);
    
    // Assets de exemplo
    wxTreeItemId spritesId = m_treeCtrl->AppendItem(m_assetsId, "Sprites", -1, -1, nullptr);
    wxTreeItemId tilesetsId = m_treeCtrl->AppendItem(m_assetsId, "Tilesets", -1, -1, nullptr);
    wxTreeItemId audioId = m_treeCtrl->AppendItem(m_assetsId, "Audio", -1, -1, nullptr);
    
    // Expandir nós principais
    m_treeCtrl->Expand(m_rootId);
    m_treeCtrl->Expand(m_mapsId);
    m_treeCtrl->Expand(m_dataId);
    m_treeCtrl->Expand(m_assetsId);
    
    // Selecionar raiz
    m_treeCtrl->SelectItem(m_rootId);
}

void ProjectTreePanel::OnTreeSelChanged(wxTreeEvent& event)
{
    wxTreeItemId item = event.GetItem();
    if (!item.IsOk()) return;
    
    // Notificar seleção para outros panes
    NotifySelection(item);
    
    event.Skip();
}

void ProjectTreePanel::OnTreeItemActivated(wxTreeEvent& event)
{
    wxTreeItemId item = event.GetItem();
    if (!item.IsOk()) return;
    
    wxString itemText = m_treeCtrl->GetItemText(item);
    
    // TODO: Abrir item para edição (mapas, database, etc.)
    // Para M1, apenas mostrar que foi ativado
    if (itemText.EndsWith(".tmx"))
    {
        wxMessageBox(wxString::Format("Abrindo mapa: %s\n(Será implementado no viewport GL)", itemText),
            "M1 - Viewport", wxOK | wxICON_INFORMATION);
    }
    else if (itemText.EndsWith(".json"))
    {
        wxMessageBox(wxString::Format("Editando database: %s\n(Será implementado no property grid)", itemText),
            "M1 - Database", wxOK | wxICON_INFORMATION);
    }
    
    event.Skip();
}

void ProjectTreePanel::OnTreeRightClick(wxTreeEvent& event)
{
    wxTreeItemId item = event.GetItem();
    if (!item.IsOk()) return;
    
    // TODO: Menu contextual com opções adequadas para cada tipo de item
    // Para M1, apenas mostrar um menu básico
    
    wxMenu contextMenu;
    contextMenu.Append(wxID_ANY, "Renomear");
    contextMenu.Append(wxID_ANY, "Duplicar");
    contextMenu.AppendSeparator();
    contextMenu.Append(wxID_ANY, "Excluir");
    
    PopupMenu(&contextMenu);
    
    event.Skip();
}

// Communication event handlers
void ProjectTreePanel::OnSelectionChanged(SelectionChangeEvent& event)
{
    // Reagir a mudanças de seleção de outros panes
    // Para M1, apenas log
    const SelectionInfo& info = event.GetSelectionInfo();
    wxLogMessage("ProjectTree received selection change: %s", info.displayName);
    event.Skip();
}

void ProjectTreePanel::OnProjectChanged(ProjectChangeEvent& event)
{
    // Recarregar árvore quando projeto muda
    if (event.IsLoaded()) {
        wxLogMessage("ProjectTree reloading for project: %s", event.GetProjectPath());
        // TODO: Recarregar estrutura real do projeto
        PopulateTree();
    } else {
        // Limpar árvore
        m_treeCtrl->DeleteAllItems();
    }
    event.Skip();
}

// Helper methods
SelectionType ProjectTreePanel::GetItemType(const wxTreeItemId& item) const
{
    if (!item.IsOk()) return SelectionType::NONE;
    
    wxString itemText = m_treeCtrl->GetItemText(item);
    
    if (itemText.EndsWith(".tmx")) {
        return SelectionType::MAP_FILE;
    } else if (itemText.EndsWith(".json")) {
        return SelectionType::DATA_FILE;
    } else if (itemText.Contains(".")) {
        return SelectionType::ASSET_FILE;
    }
    
    return SelectionType::NONE;
}

void ProjectTreePanel::NotifySelection(const wxTreeItemId& item)
{
    if (!item.IsOk()) return;
    
    SelectionInfo info;
    info.type = GetItemType(item);
    info.displayName = m_treeCtrl->GetItemText(item);
    info.filePath = info.displayName; // Por enquanto, usar nome como path
    
    // Criar e enviar evento
    SelectionChangeEvent selectionEvent(EVT_SELECTION_CHANGED);
    selectionEvent.SetSelectionInfo(info);
    
    // Enviar para o parent (EditorFrame) que redistribuirá
    wxWindow* parent = GetParent();
    if (parent) {
        parent->GetEventHandler()->ProcessEvent(selectionEvent);
    }
}
