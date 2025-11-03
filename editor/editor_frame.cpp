/**
 * Implementação da EditorFrame
 */

#pragma execution_character_set("utf-8")

#include "editor_frame.h"
#include "left_side_panel.h"
#include "project_tree_panel.h"
#include "properties_tabs_panel.h"
#include "paint_toolbar.h"
#include "viewport_panel.h"
#include "map_tabs_panel.h"
#include "tileset_panel.h"
#include "new_project_dialog.h"
#include "utf8_strings.h"
#include "i18n.h"
#include <wx/dirdlg.h>
#include <wx/filedlg.h>
#include <wx/filename.h>

// Event table
wxBEGIN_EVENT_TABLE(EditorFrame, wxFrame)
    EVT_MENU(wxID_EXIT, EditorFrame::OnExit)
    EVT_MENU(wxID_ABOUT, EditorFrame::OnAbout)
    EVT_MENU(ID_NewProject, EditorFrame::OnNewProject)
    EVT_MENU(ID_OpenProject, EditorFrame::OnOpenProject)
    EVT_MENU(ID_SaveProject, EditorFrame::OnSaveProject)
    // Event handlers para mapas
    EVT_MENU(ID_NewMap, EditorFrame::OnNewMap)
    EVT_MENU(ID_OpenMap, EditorFrame::OnOpenMap)
    EVT_MENU(ID_SaveMap, EditorFrame::OnSaveMap)
    EVT_MENU(ID_SaveMapAs, EditorFrame::OnSaveMapAs)
    EVT_MENU(ID_SetLanguagePtBr, EditorFrame::OnSetLanguagePtBr)
    EVT_MENU(ID_SetLanguageEnUs, EditorFrame::OnSetLanguageEnUs)
    EVT_CLOSE(EditorFrame::OnClose)
    // Eventos customizados para comunicação entre panes
    EVT_SELECTION_CHANGE(EditorFrame::OnSelectionChanged)
    EVT_PROPERTY_CHANGE(EditorFrame::OnPropertyChanged)
    EVT_PROJECT_CHANGE(EditorFrame::OnProjectChanged)
    // Temporariamente comentado para compilar
    // EVT_MAP_CHANGE_REQUEST(EditorFrame::OnMapChangeRequest)
    // Evento de mudança de tile selecionado
    EVT_COMMAND(wxID_ANY, TILESET_SELECTION_CHANGED, EditorFrame::OnTilesetSelectionChanged)
wxEND_EVENT_TABLE()

EditorFrame::EditorFrame()
    : wxFrame(nullptr, wxID_ANY, "Lumy Editor - M1 Brilho", wxDefaultPosition, wxSize(1200, 800))
{
    // Inicializar sistema de localização
    if (!Localization::Get().Initialize("pt_BR")) {
        wxLogWarning(L_("app.localization_error"));
    }
    
    // Configurar ícone (se houver)
    // SetIcon(wxICON(sample)); // Comentado temporariamente

    // Criar elementos da interface
    CreateMenuBar();
    CreateStatusBar();
    
    // Inicializar AUI Manager
    m_auiManager.SetManagedWindow(this);
    
    // Criar os panes
    CreateAuiPanes();
    
    // Aplicar layout
    m_auiManager.Update();
    
    // Inicializar hot-reload system
    m_fileWatcher = std::make_unique<FileWatcher>();
    
    // Inicializar project manager
    m_projectManager = std::make_unique<ProjectManager>();
    
    // Inicializar map manager
    m_mapManager = std::make_unique<MapManager>();
    
    // Conectar viewport com map manager (via MapTabsPanel)
    if (m_mapTabsPanel) {
        ViewportPanel* currentViewport = m_mapTabsPanel->GetCurrentViewport();
        if (currentViewport) {
            currentViewport->SetMapManager(m_mapManager.get());
        }
    }
    
    // Carregar projeto padrão (diretório atual)
    wxString currentDir = wxGetCwd();
    LoadProject(currentDir);
    
    // Configurar caminho do projeto na árvore
    if (m_leftSidePanel && m_leftSidePanel->GetProjectTree()) {
        m_leftSidePanel->GetProjectTree()->SetProjectPath(currentDir);
    }
    
    // Status inicial
    SetStatusText(L_("status.ready"));
}

EditorFrame::~EditorFrame()
{
    // Destruir AUI manager
    m_auiManager.UnInit();
}

void EditorFrame::CreateMenuBar()
{
    // Menu Arquivo
    wxMenu* fileMenu = new wxMenu;
    fileMenu->Append(ID_NewProject, L_("menu.new") + "\tCtrl+N");
    fileMenu->Append(ID_OpenProject, L_("menu.open") + "\tCtrl+O");
    fileMenu->AppendSeparator();
    fileMenu->Append(ID_SaveProject, L_("menu.save") + "\tCtrl+S");
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_EXIT, L_("menu.exit") + "\tAlt+F4");

    // Menu Mapa
    wxMenu* mapMenu = new wxMenu;
    mapMenu->Append(ID_NewMap, L_("menu.new_map") + "\tCtrl+Shift+N");
    mapMenu->Append(ID_OpenMap, L_("menu.open_map") + "\tCtrl+Shift+O");
    mapMenu->AppendSeparator();
    mapMenu->Append(ID_SaveMap, L_("menu.save_map") + "\tCtrl+Shift+S");
    mapMenu->Append(ID_SaveMapAs, L_("menu.save_map_as"));

    // Menu Ajuda
    wxMenu* helpMenu = new wxMenu;
    
    // Submenu de idiomas
    wxMenu* langMenu = new wxMenu;
    langMenu->Append(ID_SetLanguagePtBr, L_("lang.pt_BR"));
    langMenu->Append(ID_SetLanguageEnUs, L_("lang.en_US"));
    helpMenu->AppendSubMenu(langMenu, L_("menu.language"));
    
    helpMenu->AppendSeparator();
    helpMenu->Append(wxID_ABOUT, L_("menu.about"));

    // Criar barra de menu
    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(fileMenu, L_("menu.file"));
    menuBar->Append(mapMenu, L_("menu.map"));
    menuBar->Append(helpMenu, L_("menu.help"));

    SetMenuBar(menuBar);
}

void EditorFrame::CreateStatusBar()
{
    wxFrame::CreateStatusBar(3);
    SetStatusText(L_("status.ready"), 0);
    SetStatusText(L_("app.version"), 1);
    SetStatusText("", 2);
}

void EditorFrame::CreateAuiPanes()
{
    // Criar panes
    m_leftSidePanel = std::make_unique<LeftSidePanel>(this);
    m_propertiesTabsPanel = std::make_unique<PropertiesTabsPanel>(this);
    // m_paintToolbar removido - toolbar agora é parte do ViewportPanel
    m_mapTabsPanel = std::make_unique<MapTabsPanel>(this);

    // Adicionar panes ao AUI Manager
    
    // Painel lateral esquerdo (Árvore + Paleta empilhados)
    m_auiManager.AddPane(m_leftSidePanel.get(),
        wxAuiPaneInfo()
        .Name("LeftSidePanel")
        .Caption(L_("panels.project_tiles"))
        .Left()
        .MinSize(250, 400)
        .BestSize(280, 600)
        .CloseButton(false)
        .MaximizeButton(false)
    );

    // Painel com abas Propriedades/Camadas (direita)
    m_auiManager.AddPane(m_propertiesTabsPanel.get(),
        wxAuiPaneInfo()
        .Name("PropertiesTabsPanel")
        .Caption(L_("panels.properties_layers"))
        .Right()
        .MinSize(280, 400)
        .BestSize(320, 500)
        .CloseButton(false)
        .MaximizeButton(false)
    );

    // Map Tabs (centro) - Gerenciamento de múltiplas abas de mapa
    m_auiManager.AddPane(m_mapTabsPanel.get(),
        wxAuiPaneInfo()
        .Name("MapTabs")
        .Caption(L_("panels.maps"))
        .Center()
        .CloseButton(false)
        .MaximizeButton(true)
    );
}

// Event Handlers
void EditorFrame::OnExit(wxCommandEvent& WXUNUSED(event))
{
    Close(true);
}

void EditorFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxMessageBox(
        UTF8(
        "Lumy Editor - Marco 1 \"Brilho\"\n"
        "Editor de RPGs 2D com wxWidgets e OpenGL\n\n"
        "Recursos do M1:\n"
        "- Interface wxAUI com docks\n"
        "- Árvore do projeto\n"
        "- Property Grid\n"
        "- Viewport OpenGL\n"
        "- Hot-reload de mapas\n\n"
        "Versão: 0.1.1\n"
        "Engine: C++20 + SFML + wxWidgets"),
        UTF8("Sobre o Lumy Editor"),
        wxOK | wxICON_INFORMATION,
        this
    );
}

void EditorFrame::OnClose(wxCloseEvent& event)
{
    // Verificar se há mudanças não salvas
    if (event.CanVeto())
    {
        int answer = wxMessageBox(
            L_("dialog.confirm_exit"),
            L_("dialog.exit_title"),
            wxYES_NO | wxICON_QUESTION,
            this
        );
        
        if (answer == wxNO)
        {
            event.Veto();
            return;
        }
    }
    
    // Salvar layout do AUI antes de fechar
    // TODO: Implementar salvamento de perspectiva
    
    Destroy();
}

void EditorFrame::OnNewProject(wxCommandEvent& WXUNUSED(event))
{
    SetStatusText(L_("status.creating_project"), 0);
    
    NewProjectDialog dialog(this);
    
    if (dialog.ShowModal() == wxID_OK) {
        wxString projectPath = dialog.GetProjectPath();
        ProjectInfo info = dialog.GetProjectInfo();
        
        if (m_projectManager->CreateNewProject(projectPath, info)) {
            // Projeto criado com sucesso
            m_currentProjectPath = projectPath;
            
            // Atualizar título da janela
            SetTitle(wxString::Format("Lumy Editor - M1 Brilho [%s]", info.projectName));
            
            // Configurar hot-reload
            SetupHotReload();
            
            // Broadcast project change
            BroadcastProjectChange(projectPath, true);
            
            // Atualizar status
            SetStatusText(wxString::Format("Projeto criado: %s", info.projectName), 0);
            SetStatusText(wxString::Format("Projeto: %s", info.projectName), 2);
            
            wxLogMessage("Novo projeto criado com sucesso: %s", projectPath);
        } else {
            SetStatusText(L_("status.error_create_project"), 0);
            wxMessageBox(L_("msg.error_create_project"), L_("dialog.error"), wxOK | wxICON_ERROR);
        }
    } else {
        SetStatusText(L_("status.ready"), 0);
    }
}

void EditorFrame::OnOpenProject(wxCommandEvent& WXUNUSED(event))
{
    SetStatusText(L_("status.opening_project"), 0);
    
    wxDirDialog dirDialog(this, L_("dialog.select_project"), m_currentProjectPath);
    
    if (dirDialog.ShowModal() == wxID_OK) {
        wxString selectedPath = dirDialog.GetPath();
        if (LoadProject(selectedPath)) {
            SetStatusText(wxString::Format(L_("status.project_loaded"), selectedPath), 0);
        } else {
            SetStatusText(L_("status.error_load_project"), 0);
            wxMessageBox(L_("msg.error_load_project"), L_("dialog.error"), wxOK | wxICON_ERROR);
        }
    } else {
        SetStatusText(L_("status.ready"), 0);
    }
}

void EditorFrame::OnSaveProject(wxCommandEvent& WXUNUSED(event))
{
    SetStatusText(L_("status.saving_project"), 0);
    // TODO: Implementar salvamento de projeto
    wxMessageBox(L_("msg.feature_not_implemented"), L_("dialog.wip"), wxOK | wxICON_INFORMATION);
    SetStatusText(L_("status.ready"), 0);
}

// Handlers dos eventos de mapa
void EditorFrame::OnNewMap(wxCommandEvent& WXUNUSED(event))
{
    SetStatusText(L_("status.creating_map"), 0);
    
    // Por enquanto, criar um mapa 20x15 básico
    auto newMap = std::make_shared<Map>(20, 15);
    m_mapManager->SetCurrentMap(newMap);
    
    // Limpar caminho do mapa atual pois é um novo mapa
    m_currentMapPath.Clear();
    
    // Atualizar título da janela
    UpdateWindowTitle();
    
    // Atualizar viewport
    if (m_viewport) {
        m_viewport->RefreshMapDisplay();
    }
    
    // Atualizar painel de layers (via painel de abas)
    if (m_propertiesTabsPanel) {
        m_propertiesTabsPanel->SetMap(newMap.get());
    }
    
    SetStatusText(L_("status.new_map_created"), 0);
    wxLogMessage("Novo mapa criado com dimensões 20x15");
    
    // A árvore será atualizada quando o mapa for salvo
}

void EditorFrame::OnOpenMap(wxCommandEvent& WXUNUSED(event))
{
    SetStatusText(L_("status.opening_map"), 0);
    
    wxFileDialog openDialog(this, L_("dialog.open_map"), "", "",
                           "Arquivos de Mapa (*.json)|*.json",
                           wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    
    if (openDialog.ShowModal() == wxID_OK) {
        wxString filePath = openDialog.GetPath();
        
        if (m_mapManager->LoadMap(filePath.ToStdString())) {
            // Atualizar caminho do mapa atual
            m_currentMapPath = filePath;
            
            // Atualizar título da janela
            UpdateWindowTitle();
            
            // Atualizar viewport
            if (m_viewport) {
                m_viewport->RefreshMapDisplay();
            }
            
            // Atualizar painel de layers (via painel de abas)
            if (m_propertiesTabsPanel) {
                m_propertiesTabsPanel->SetMap(m_mapManager->GetCurrentMap().get());
            }
            
            wxFileName fileName(filePath);
            SetStatusText(wxString::Format(L_("status.map_loaded"), fileName.GetName()), 0);
            wxLogMessage("Mapa carregado com sucesso: %s", filePath);
        } else {
            SetStatusText(L_("status.error_load_map"), 0);
            wxMessageBox(L_("msg.error_load_map"), L_("dialog.error"), wxOK | wxICON_ERROR);
        }
    } else {
        SetStatusText(L_("status.ready"), 0);
    }
}

void EditorFrame::OnSaveMap(wxCommandEvent& WXUNUSED(event))
{
    SetStatusText(L_("status.saving_map"), 0);
    
    if (!m_mapManager->GetCurrentMap()) {
        wxMessageBox(L_("msg.no_map_loaded"), L_("dialog.error"), wxOK | wxICON_WARNING);
        SetStatusText(L_("status.ready"), 0);
        return;
    }
    
    // Se não temos caminho salvo, usar "Salvar Como"
    if (m_currentMapPath.IsEmpty()) {
        wxCommandEvent dummyEvent;
        OnSaveMapAs(dummyEvent);
        return;
    }
    
    if (m_mapManager->SaveMap(m_currentMapPath.ToStdString())) {
        // Atualizar título da janela para remover o asterisco
        UpdateWindowTitle();
        
        SetStatusText(L_("status.map_saved"), 0);
        wxLogMessage("Mapa salvo: %s", m_currentMapPath);
        
        // Atualizar árvore do projeto
        if (m_leftSidePanel && m_leftSidePanel->GetProjectTree()) {
            m_leftSidePanel->GetProjectTree()->RefreshMapList();
        }
    } else {
        SetStatusText(L_("status.error_save_map"), 0);
        wxMessageBox(L_("msg.error_save_map"), L_("dialog.error"), wxOK | wxICON_ERROR);
    }
}

void EditorFrame::OnSaveMapAs(wxCommandEvent& WXUNUSED(event))
{
    SetStatusText(L_("status.saving_map_as"), 0);
    
    if (!m_mapManager->GetCurrentMap()) {
        wxMessageBox(L_("msg.no_map_loaded"), L_("dialog.error"), wxOK | wxICON_WARNING);
        SetStatusText(L_("status.ready"), 0);
        return;
    }
    
    wxFileDialog saveDialog(this, L_("dialog.save_map_as"), "", "",
                           "Arquivos de Mapa (*.json)|*.json",
                           wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    
    if (saveDialog.ShowModal() == wxID_OK) {
        wxString filePath = saveDialog.GetPath();
        
        if (m_mapManager->SaveMapAs(filePath)) {
            m_currentMapPath = filePath;
            
            // Atualizar título da janela
            UpdateWindowTitle();
            
            SetStatusText(wxString::Format(L_("status.map_saved_as"), wxFileName(filePath).GetName()), 0);
            wxLogMessage("Mapa salvo como: %s", filePath);
            
            // Atualizar árvore do projeto
            if (m_leftSidePanel && m_leftSidePanel->GetProjectTree()) {
                m_leftSidePanel->GetProjectTree()->RefreshMapList();
            }
        } else {
            SetStatusText(L_("status.error_save_map"), 0);
            wxMessageBox(L_("msg.error_save_map"), L_("dialog.error"), wxOK | wxICON_ERROR);
        }
    } else {
        SetStatusText(L_("status.ready"), 0);
    }
}

void EditorFrame::OnTilesetSelectionChanged(wxCommandEvent& event)
{
    int selectedTile = event.GetInt();
    
    // Atualizar o tile selecionado no viewport atual
    if (m_mapTabsPanel) {
        ViewportPanel* viewport = m_mapTabsPanel->GetCurrentViewport();
        if (viewport) {
            viewport->SetSelectedTile(selectedTile);
            
            wxLogMessage("Tile selecionado alterado para: %d", selectedTile);
            SetStatusText(wxString::Format(L_("status.tile_selected"), selectedTile), 0);
        }
    }
}

// Hot-reload callbacks
void EditorFrame::OnMapFileChanged(const wxString& path, const wxString& filename)
{
    wxLogMessage("Mapa modificado: %s", filename);
    
    // Recarregar no viewport atual
    if (m_mapTabsPanel) {
        ViewportPanel* viewport = m_mapTabsPanel->GetCurrentViewport();
        if (viewport) {
            // TODO: Implementar recarga real do mapa
            SetStatusText(wxString::Format("Hot-reload: %s", filename), 0);
            
            // For now, just refresh the viewport
            viewport->Refresh();
        }
    }
}

void EditorFrame::OnDataFileChanged(const wxString& path, const wxString& filename)
{
    wxLogMessage("Database modificado: %s", filename);
    
    // Recarregar no property grid (via painel de abas)
    if (m_propertiesTabsPanel && m_propertiesTabsPanel->GetPropertyGrid()) {
        // TODO: Implementar recarga real da database
        SetStatusText(wxString::Format("Hot-reload: %s", filename), 0);
    }
}

// Project management
bool EditorFrame::LoadProject(const wxString& projectPath)
{
    if (!wxDirExists(projectPath)) {
        wxLogError("Diretório do projeto não existe: %s", projectPath);
        return false;
    }
    
    m_currentProjectPath = projectPath;
    
    // Atualizar título da janela
    wxFileName projectDir(projectPath);
    SetTitle(wxString::Format("Lumy Editor - M1 Brilho [%s]", projectDir.GetName()));
    
    // Configurar hot-reload
    SetupHotReload();
    
    // Atualizar árvore do projeto com novo caminho
    if (m_leftSidePanel && m_leftSidePanel->GetProjectTree()) {
        m_leftSidePanel->GetProjectTree()->SetProjectPath(projectPath);
    }
    
    // Atualizar status bar
    SetStatusText(wxString::Format("Projeto: %s", projectDir.GetName()), 2);
    
    wxLogMessage("Projeto carregado: %s", projectPath);
    return true;
}

void EditorFrame::SetupHotReload()
{
    if (!m_fileWatcher || m_currentProjectPath.IsEmpty()) {
        return;
    }
    
    // Parar monitoramento anterior
    m_fileWatcher->StopWatching();
    
    // Registrar callbacks para tipos de arquivo
    m_fileWatcher->RegisterCallback("tmx", [this](const wxString& path, const wxString& filename) {
        // Usar CallAfter para executar na thread principal
        CallAfter([this, path, filename]() {
            OnMapFileChanged(path, filename);
        });
    });
    
    m_fileWatcher->RegisterCallback("json", [this](const wxString& path, const wxString& filename) {
        CallAfter([this, path, filename]() {
            OnDataFileChanged(path, filename);
        });
    });
    
    // Iniciar monitoramento
    if (m_fileWatcher->StartWatching(m_currentProjectPath)) {
        wxLogMessage("Hot-reload configurado para: %s", m_currentProjectPath);
    } else {
        wxLogError("Falha ao configurar hot-reload");
    }
}

// Communication between panes handlers
void EditorFrame::OnSelectionChanged(SelectionChangeEvent& event)
{
    const SelectionInfo& info = event.GetSelectionInfo();
    
    wxLogMessage("Selection changed: type=%d, name=%s", static_cast<int>(info.type), info.displayName);
    
    // Carregar mapa automaticamente se for um arquivo de mapa JSON
    if (info.type == SelectionType::DATA_FILE && info.displayName.EndsWith(".json") && 
        wxFileExists(info.filePath)) {
        
        // Verificar se é um mapa (no contexto da árvore de mapas)
        if (info.filePath.Contains("map") || IsMapInMapsFolder(info.filePath)) {
            LoadMapFromPath(info.filePath);
        }
    }
    
    // Atualizar property grid com base na seleção (via painel de abas)
    if (m_propertiesTabsPanel && m_propertiesTabsPanel->GetPropertyGrid()) {
        // TODO: Implementar atualização dinâmica do property grid
        // Por enquanto, apenas log
        wxLogMessage("Updating property grid for selection: %s", info.displayName);
    }
    
    // Atualizar viewport se necessário
    if (m_mapTabsPanel && info.type == SelectionType::TILE) {
        ViewportPanel* viewport = m_mapTabsPanel->GetCurrentViewport();
        if (viewport) {
            // TODO: Highlight tile no viewport
            wxLogMessage("Highlighting tile at (%d, %d)", info.tilePosition.x, info.tilePosition.y);
        }
    }
    
    // Atualizar status bar
    if (info.type != SelectionType::NONE) {
        SetStatusText(wxString::Format("Selecionado: %s", info.displayName), 0);
    } else {
        SetStatusText("Pronto", 0);
    }
}

void EditorFrame::OnPropertyChanged(PropertyChangeEvent& event)
{
    const wxString& propertyName = event.GetPropertyName();
    const wxVariant& propertyValue = event.GetPropertyValue();
    
    wxLogMessage("Property changed: %s = %s", propertyName, propertyValue.GetString());
    
    // Propagar mudança para viewport se necessário
    if (m_mapTabsPanel) {
        ViewportPanel* viewport = m_mapTabsPanel->GetCurrentViewport();
        if (viewport) {
            // TODO: Aplicar mudanças de propriedade no viewport
            viewport->Refresh();
        }
    }
    
    // Atualizar status
    SetStatusText(wxString::Format("Propriedade alterada: %s", propertyName), 0);
}

void EditorFrame::OnProjectChanged(ProjectChangeEvent& event)
{
    const wxString& projectPath = event.GetProjectPath();
    bool isLoaded = event.IsLoaded();
    
    if (isLoaded) {
        wxLogMessage("Project loaded: %s", projectPath);
        
        // Atualizar todas as panes
        if (m_leftSidePanel && m_leftSidePanel->GetProjectTree()) {
            // TODO: Recarregar árvore do projeto
        }
        
        if (m_propertiesTabsPanel && m_propertiesTabsPanel->GetPropertyGrid()) {
            // TODO: Limpar property grid
        }
        
        if (m_mapTabsPanel) {
            ViewportPanel* viewport = m_mapTabsPanel->GetCurrentViewport();
            if (viewport) {
                // TODO: Carregar mapa padrão do projeto
                viewport->Refresh();
            }
        }
    } else {
        wxLogMessage("Project unloaded");
        
        // Limpar todas as panes
        SetStatusText("Nenhum projeto carregado", 2);
    }
}

// Broadcasting methods
void EditorFrame::BroadcastSelectionChange(const SelectionInfo& info)
{
    SelectionChangeEvent event(EVT_SELECTION_CHANGED);
    event.SetSelectionInfo(info);
    
    // Enviar evento para todos os panes
    if (m_leftSidePanel && m_leftSidePanel->GetProjectTree()) {
        m_leftSidePanel->GetProjectTree()->GetEventHandler()->ProcessEvent(event);
    }
    
    if (m_propertiesTabsPanel && m_propertiesTabsPanel->GetPropertyGrid()) {
        m_propertiesTabsPanel->GetPropertyGrid()->GetEventHandler()->ProcessEvent(event);
    }
    
    if (m_mapTabsPanel) {
        m_mapTabsPanel->GetEventHandler()->ProcessEvent(event);
    }
    
    // Processar também no frame principal
    GetEventHandler()->ProcessEvent(event);
}

void EditorFrame::BroadcastPropertyChange(const wxString& propertyName, const wxVariant& value)
{
    PropertyChangeEvent event(EVT_PROPERTY_CHANGED);
    event.SetPropertyChange(propertyName, value);
    
    // Enviar para viewport principalmente
    if (m_mapTabsPanel) {
        m_mapTabsPanel->GetEventHandler()->ProcessEvent(event);
    }
    
    // Processar no frame principal
    GetEventHandler()->ProcessEvent(event);
}

void EditorFrame::BroadcastProjectChange(const wxString& projectPath, bool loaded)
{
    ProjectChangeEvent event(EVT_PROJECT_CHANGED);
    event.SetProjectInfo(projectPath, loaded);
    
    // Enviar para todos os panes
    if (m_leftSidePanel && m_leftSidePanel->GetProjectTree()) {
        m_leftSidePanel->GetProjectTree()->GetEventHandler()->ProcessEvent(event);
    }
    
    if (m_propertiesTabsPanel && m_propertiesTabsPanel->GetPropertyGrid()) {
        m_propertiesTabsPanel->GetPropertyGrid()->GetEventHandler()->ProcessEvent(event);
    }
    
    if (m_mapTabsPanel) {
        m_mapTabsPanel->GetEventHandler()->ProcessEvent(event);
    }
    
    // Processar no frame principal
    GetEventHandler()->ProcessEvent(event);
}

// Métodos auxiliares para carregamento de mapas
bool EditorFrame::IsMapInMapsFolder(const wxString& filePath)
{
    // Verificar se o caminho contém pastas relacionadas a mapas
    wxString lowerPath = filePath.Lower();
    return lowerPath.Contains("map") || 
           lowerPath.Contains("level") || 
           lowerPath.Contains("stage") ||
           lowerPath.Contains("world");
}

void EditorFrame::LoadMapFromPath(const wxString& filePath)
{
    SetStatusText("Carregando mapa...", 0);
    
    if (m_mapManager->LoadMap(filePath.ToStdString())) {
        // Mapa carregado com sucesso
        wxFileName fileName(filePath);
        
        // Adicionar mapa ao MapTabsPanel ou atualizar aba existente
        if (m_mapTabsPanel) {
            auto mapPtr = m_mapManager->GetCurrentMap();
            if (mapPtr) {
                // Adicionar mapa à aba
                m_mapTabsPanel->AddMap(mapPtr, filePath);
                
                // Atualizar viewport
                ViewportPanel* viewport = m_mapTabsPanel->GetCurrentViewport();
                if (viewport) {
                    viewport->SetCurrentMap(mapPtr.get());
                    viewport->RefreshMapDisplay();
                }
            }
        }
        
        // Atualizar caminho do mapa atual
        m_currentMapPath = filePath;
        
        // Atualizar título da janela
        UpdateWindowTitle();
        
        SetStatusText(wxString::Format("Mapa carregado: %s", fileName.GetName()), 0);
        wxLogMessage("Mapa carregado com sucesso: %s", filePath);
    } else {
        SetStatusText("Erro ao carregar mapa", 0);
        wxLogError("Erro ao carregar o mapa: %s", filePath);
        wxMessageBox("Erro ao carregar o mapa selecionado. Verifique se o arquivo é válido.", "Erro", wxOK | wxICON_ERROR);
    }
}

bool EditorFrame::SafeLoadMapFromPath(const wxString& filePath)
{
    // Verificar se já temos esse mapa carregado
    if (m_currentMapPath == filePath) {
        SetStatusText("Esse mapa já está carregado", 0);
        return false;
    }
    
    // Verificar se há mudanças não salvas no mapa atual
    if (m_mapManager->HasUnsavedChanges()) {
        SetStatusText("Verificando mudanças não salvas...", 0);
        
        // Usar o método PromptSaveIfModified do MapManager que já implementa o diálogo
        bool canContinue = m_mapManager->PromptSaveIfModified(this);
        
        if (!canContinue) {
            // Usuário cancelou ou precisa de Save As
            wxString currentMapPath = m_mapManager->GetCurrentMapPath();
            if (currentMapPath.IsEmpty()) {
                // Mapa nunca foi salvo - mostrar Save As
                wxFileDialog saveDialog(this, "Salvar Mapa Como", "", "",
                                       "Arquivos de Mapa (*.json)|*.json",
                                       wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
                
                if (saveDialog.ShowModal() == wxID_OK) {
                    wxString saveFilePath = saveDialog.GetPath();
                    if (!m_mapManager->SaveMapAs(saveFilePath)) {
                        SetStatusText("Erro ao salvar mapa", 0);
                        wxMessageBox("Erro ao salvar o mapa.", "Erro", wxOK | wxICON_ERROR);
                        return false;
                    }
                    // Atualizar m_currentMapPath após salvar
                    m_currentMapPath = saveFilePath;
                    // Atualizar título
                    UpdateWindowTitle();
                    // Atualizar árvore do projeto
                    if (m_leftSidePanel && m_leftSidePanel->GetProjectTree()) {
                        m_leftSidePanel->GetProjectTree()->RefreshMapList();
                    }
                } else {
                    SetStatusText("Troca de mapa cancelada", 0);
                    return false;
                }
            } else {
                // Erro ao salvar ou usuário cancelou
                SetStatusText("Troca de mapa cancelada", 0);
                return false;
            }
        }
    }
    
    // Carregar o novo mapa
    LoadMapFromPath(filePath);
    return true;
}

void EditorFrame::UpdateWindowTitle()
{
    if (!m_mapManager || !m_mapManager->HasMap()) {
        SetTitle("Lumy Editor - M1 Brilho");
        return;
    }
    
    wxString mapName;
    
    // Tentar obter nome do arquivo do caminho atual
    if (!m_currentMapPath.IsEmpty()) {
        wxFileName fileName(m_currentMapPath);
        mapName = fileName.GetName();
    } else {
        // Usar nome do mapa se não temos caminho
        mapName = m_mapManager->GetCurrentMapName();
    }
    
    // Adicionar asterisco se há mudanças não salvas
    if (m_mapManager->HasUnsavedChanges()) {
        mapName += "*";
    }
    
    SetTitle(wxString::Format("Lumy Editor - M1 Brilho [%s]", mapName));
}

void EditorFrame::OnMapChangeRequest(MapChangeRequestEvent& event)
{
    const wxString& requestedMapPath = event.GetMapPath();
    
    wxLogMessage("Solicitação de troca de mapa recebida: %s", requestedMapPath);
    
    SafeLoadMapFromPath(requestedMapPath);
}

// ============================================================================
// Handlers de Idioma
// ============================================================================

void EditorFrame::OnSetLanguagePtBr(wxCommandEvent& WXUNUSED(event))
{
    if (Localization::Get().LoadLanguage("pt_BR")) {
        wxMessageBox(
            UTF8("Idioma alterado para Português (Brasil).\n\nReinicie o editor para aplicar todas as mudanças."),
            UTF8("Idioma Alterado"),
            wxOK | wxICON_INFORMATION,
            this
        );
        wxLogMessage("Idioma alterado para: pt_BR");
    } else {
        wxMessageBox(
            UTF8("Erro ao carregar traduções em Português."),
            UTF8("Erro"),
            wxOK | wxICON_ERROR,
            this
        );
    }
}

void EditorFrame::OnSetLanguageEnUs(wxCommandEvent& WXUNUSED(event))
{
    if (Localization::Get().LoadLanguage("en_US")) {
        wxMessageBox(
            "Language changed to English (US).\n\nRestart the editor to apply all changes.",
            "Language Changed",
            wxOK | wxICON_INFORMATION,
            this
        );
        wxLogMessage("Language changed to: en_US");
    } else {
        wxMessageBox(
            "Error loading English translations.",
            "Error",
            wxOK | wxICON_ERROR,
            this
        );
    }
}
