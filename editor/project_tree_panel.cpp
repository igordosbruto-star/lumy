/**
 * Implementação do Project Tree Panel
 */

#pragma execution_character_set("utf-8")

#include "project_tree_panel.h"
#include "editor_frame.h"
#include "utf8_strings.h"
#include <wx/imaglist.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/ffile.h>

// Classe para armazenar dados do item de mapa
class MapItemData : public wxTreeItemData
{
public:
    MapItemData(const wxString& path) : filePath(path) {}
    wxString GetFilePath() const { return filePath; }
    
private:
    wxString filePath;
};

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
    
    // Escanear e adicionar mapas reais
    ScanForMaps();
    
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
    // Apenas seleção simples - não executar ações de carregamento
    // O carregamento agora é feito apenas no duplo clique
    event.Skip();
}

void ProjectTreePanel::OnTreeItemActivated(wxTreeEvent& event)
{
    wxTreeItemId item = event.GetItem();
    if (!item.IsOk()) return;
    
    wxString itemText = m_treeCtrl->GetItemText(item);
    
    // Verificar se é um mapa JSON (nossos mapas editáveis)
    if (itemText.EndsWith(".json") && GetItemType(item) == SelectionType::DATA_FILE) {
        // Obter caminho do arquivo
        wxTreeItemData* itemData = m_treeCtrl->GetItemData(item);
        MapItemData* mapData = dynamic_cast<MapItemData*>(itemData);
        
        if (mapData && IsMapInMapsSection(item)) {
            // Notificar o EditorFrame para iniciar processo de troca de mapa
            RequestMapChange(mapData->GetFilePath());
        }
    }
    else if (itemText.EndsWith(".tmx"))
    {
        wxMessageBox(wxString::Format("Abrindo mapa TMX: %s\n(Formato TMX ainda não suportado)", itemText),
            "M1 - Viewport", wxOK | wxICON_INFORMATION);
    }
    else if (itemText.EndsWith(".json") && GetItemType(item) == SelectionType::DATA_FILE)
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
    
    // Obter caminho completo do arquivo se disponível
    wxTreeItemData* itemData = m_treeCtrl->GetItemData(item);
    if (itemData) {
        MapItemData* mapData = dynamic_cast<MapItemData*>(itemData);
        if (mapData) {
            info.filePath = mapData->GetFilePath();
        } else {
            info.filePath = info.displayName;
        }
    } else {
        info.filePath = info.displayName;
    }
    
    // Criar e enviar evento
    SelectionChangeEvent selectionEvent(EVT_SELECTION_CHANGED);
    selectionEvent.SetSelectionInfo(info);
    
    // Enviar para o parent (EditorFrame) que redistribuirá
    wxWindow* parent = GetParent();
    if (parent) {
        parent->GetEventHandler()->ProcessEvent(selectionEvent);
    }
}

void ProjectTreePanel::ScanForMaps()
{
    // Adicionar mapa exemplo TMX
    m_treeCtrl->AppendItem(m_mapsId, "hello.tmx", -1, -1, nullptr);
    
    // Usar diretório do projeto atual ou diretório de trabalho
    wxString projectDir = m_projectPath.IsEmpty() ? wxGetCwd() : m_projectPath;
    
    // Lista de diretórios para escanear (relativos ao projeto)
    wxArrayString scanDirs;
    scanDirs.Add(projectDir);
    scanDirs.Add(projectDir + "/maps");
    scanDirs.Add(projectDir + "/data/maps");
    scanDirs.Add(projectDir + "/game/maps");
    
    for (const wxString& dir : scanDirs) {
        if (wxDirExists(dir)) {
            ScanDirectoryForMaps(dir);
        }
    }
    
    wxLogMessage("Escaneamento de mapas concluído para projeto: %s", projectDir);
}

void ProjectTreePanel::ScanDirectoryForMaps(const wxString& directory)
{
    wxDir dir(directory);
    if (!dir.IsOpened()) {
        return;
    }
    
    wxString filename;
    bool cont = dir.GetFirst(&filename, "*.json", wxDIR_FILES);
    
    while (cont) {
        wxString fullPath = wxFileName(directory, filename).GetFullPath();
        
        // Verificar se é realmente um mapa (tem estrutura de mapa)
        if (IsMapFile(fullPath)) {
            // Adicionar ao nó de mapas
            wxTreeItemData* itemData = new wxTreeItemData();
            wxTreeItemId mapItem = m_treeCtrl->AppendItem(m_mapsId, filename, -1, -1, itemData);
            
            // Armazenar o caminho completo como dado do item
            m_treeCtrl->SetItemData(mapItem, new MapItemData(fullPath));
            
            wxLogMessage("Mapa encontrado: %s", filename);
        }
        
        cont = dir.GetNext(&filename);
    }
}

bool ProjectTreePanel::IsMapFile(const wxString& filePath)
{
    // Ler e verificar se é um arquivo de mapa válido
    wxFFile file(filePath, "r");
    if (!file.IsOpened()) {
        return false;
    }
    
    wxString content;
    if (!file.ReadAll(&content)) {
        return false;
    }
    
    // Verificação simples: deve conter "width", "height" e "tiles"
    // ou "metadata" (para nosso formato completo)
    return (content.Contains("\"width\"") && content.Contains("\"height\"") && 
            (content.Contains("\"tiles\"") || content.Contains("\"metadata\"")));
}

void ProjectTreePanel::RefreshMaps()
{
    // Limpar mapas existentes (exceto hello.tmx)
    wxTreeItemIdValue cookie;
    wxTreeItemId child = m_treeCtrl->GetFirstChild(m_mapsId, cookie);
    
    // Coletar itens para deletar
    wxArrayTreeItemIds itemsToDelete;
    while (child.IsOk()) {
        wxString itemText = m_treeCtrl->GetItemText(child);
        if (itemText.EndsWith(".json")) {
            itemsToDelete.Add(child);
        }
        child = m_treeCtrl->GetNextChild(m_mapsId, cookie);
    }
    
    // Deletar itens coletados
    for (const wxTreeItemId& item : itemsToDelete) {
        m_treeCtrl->Delete(item);
    }
    
    // Escanear novamente usando diretório do projeto
    wxString projectDir = m_projectPath.IsEmpty() ? wxGetCwd() : m_projectPath;
    wxArrayString scanDirs;
    scanDirs.Add(projectDir);
    scanDirs.Add(projectDir + "/maps");
    scanDirs.Add(projectDir + "/data/maps");
    scanDirs.Add(projectDir + "/game/maps");
    
    for (const wxString& dir : scanDirs) {
        if (wxDirExists(dir)) {
            ScanDirectoryForMaps(dir);
        }
    }
}

void ProjectTreePanel::RefreshMapList()
{
    RefreshMaps();
}

void ProjectTreePanel::SetProjectPath(const wxString& path)
{
    m_projectPath = path;
    wxLogMessage("ProjectTree: Caminho do projeto atualizado para: %s", path);
    
    // Recarregar árvore com novo caminho
    PopulateTree();
}

// Métodos auxiliares para gerenciamento de mapas
bool ProjectTreePanel::IsMapInMapsSection(const wxTreeItemId& item) const
{
    if (!item.IsOk()) return false;
    
    // Verificar se o item é filho (direto ou indireto) do nó "Mapas"
    wxTreeItemId parent = m_treeCtrl->GetItemParent(item);
    while (parent.IsOk()) {
        if (parent == m_mapsId) {
            return true;
        }
        parent = m_treeCtrl->GetItemParent(parent);
    }
    return false;
}

void ProjectTreePanel::RequestMapChange(const wxString& mapPath)
{
    // Usar o método seguro que verifica mudanças não salvas
    EditorFrame* editorFrame = dynamic_cast<EditorFrame*>(GetParent());
    if (editorFrame) {
        editorFrame->SafeLoadMapFromPath(mapPath);
    }
}

