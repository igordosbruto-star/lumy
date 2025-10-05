/**
 * Implementação da EditorFrame
 */

#include "editor_frame.h"
#include "project_tree_panel.h"
#include "property_grid_panel.h"
#include "viewport_panel.h"
#include <wx/dirdlg.h>

// Event table
wxBEGIN_EVENT_TABLE(EditorFrame, wxFrame)
    EVT_MENU(wxID_EXIT, EditorFrame::OnExit)
    EVT_MENU(wxID_ABOUT, EditorFrame::OnAbout)
    EVT_MENU(ID_NewProject, EditorFrame::OnNewProject)
    EVT_MENU(ID_OpenProject, EditorFrame::OnOpenProject)
    EVT_MENU(ID_SaveProject, EditorFrame::OnSaveProject)
    EVT_CLOSE(EditorFrame::OnClose)
wxEND_EVENT_TABLE()

EditorFrame::EditorFrame()
    : wxFrame(nullptr, wxID_ANY, "Lumy Editor - M1 Brilho", wxDefaultPosition, wxSize(1200, 800))
{
    // Configurar ícone (se houver)
    SetIcon(wxICON(sample));

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
    
    // Carregar projeto padrão (diretório atual)
    wxString currentDir = wxGetCwd();
    LoadProject(currentDir);
    
    // Status inicial
    SetStatusText("Lumy Editor iniciado - Pronto para criar!");
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
    fileMenu->Append(ID_NewProject, "&Novo Projeto\tCtrl+N", "Criar novo projeto Lumy");
    fileMenu->Append(ID_OpenProject, "&Abrir Projeto\tCtrl+O", "Abrir projeto existente");
    fileMenu->AppendSeparator();
    fileMenu->Append(ID_SaveProject, "&Salvar Projeto\tCtrl+S", "Salvar projeto atual");
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_EXIT, "Sai&r\tAlt+F4", "Sair do editor");

    // Menu Ajuda
    wxMenu* helpMenu = new wxMenu;
    helpMenu->Append(wxID_ABOUT, "&Sobre", "Informações sobre o Lumy Editor");

    // Criar barra de menu
    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(fileMenu, "&Arquivo");
    menuBar->Append(helpMenu, "&Ajuda");

    SetMenuBar(menuBar);
}

void EditorFrame::CreateStatusBar()
{
    wxFrame::CreateStatusBar(3);
    SetStatusText("Pronto", 0);
    SetStatusText("M1 - Brilho", 1);
    SetStatusText("Nenhum projeto", 2);
}

void EditorFrame::CreateAuiPanes()
{
    // Criar panes
    m_projectTree = std::make_unique<ProjectTreePanel>(this);
    m_propertyGrid = std::make_unique<PropertyGridPanel>(this);
    m_viewport = std::make_unique<ViewportPanel>(this);

    // Adicionar panes ao AUI Manager
    
    // Árvore do projeto (esquerda)
    m_auiManager.AddPane(m_projectTree.get(),
        wxAuiPaneInfo()
        .Name("ProjectTree")
        .Caption("Árvore do Projeto")
        .Left()
        .MinSize(200, -1)
        .BestSize(250, -1)
        .CloseButton(false)
        .MaximizeButton(false)
    );

    // Property Grid (direita)
    m_auiManager.AddPane(m_propertyGrid.get(),
        wxAuiPaneInfo()
        .Name("PropertyGrid")
        .Caption("Propriedades")
        .Right()
        .MinSize(200, -1)
        .BestSize(300, -1)
        .CloseButton(false)
        .MaximizeButton(false)
    );

    // Viewport (centro)
    m_auiManager.AddPane(m_viewport.get(),
        wxAuiPaneInfo()
        .Name("Viewport")
        .Caption("Editor de Mapa")
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
        "Lumy Editor - Marco 1 \"Brilho\"\n"
        "Editor de RPGs 2D com wxWidgets e OpenGL\n\n"
        "Recursos do M1:\n"
        "- Interface wxAUI com docks\n"
        "- Árvore do projeto\n"
        "- Property Grid\n"
        "- Viewport OpenGL\n"
        "- Hot-reload de mapas\n\n"
        "Versão: 0.1.1\n"
        "Engine: C++20 + SFML + wxWidgets",
        "Sobre o Lumy Editor",
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
            "Deseja realmente sair do editor?",
            "Confirmar saída",
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
    SetStatusText("Criando novo projeto...", 0);
    // TODO: Implementar criação de projeto
    wxMessageBox("Função 'Novo Projeto' será implementada em breve!", "M1 - Em Desenvolvimento", wxOK | wxICON_INFORMATION);
    SetStatusText("Pronto", 0);
}

void EditorFrame::OnOpenProject(wxCommandEvent& WXUNUSED(event))
{
    SetStatusText("Abrindo projeto...", 0);
    
    wxDirDialog dirDialog(this, "Selecionar pasta do projeto Lumy", m_currentProjectPath);
    
    if (dirDialog.ShowModal() == wxID_OK) {
        wxString selectedPath = dirDialog.GetPath();
        if (LoadProject(selectedPath)) {
            SetStatusText(wxString::Format("Projeto carregado: %s", selectedPath), 0);
        } else {
            SetStatusText("Erro ao carregar projeto", 0);
            wxMessageBox("Erro ao carregar projeto do diretório selecionado.", "Erro", wxOK | wxICON_ERROR);
        }
    } else {
        SetStatusText("Pronto", 0);
    }
}

void EditorFrame::OnSaveProject(wxCommandEvent& WXUNUSED(event))
{
    SetStatusText("Salvando projeto...", 0);
    // TODO: Implementar salvamento de projeto
    wxMessageBox("Função 'Salvar Projeto' será implementada em breve!", "M1 - Em Desenvolvimento", wxOK | wxICON_INFORMATION);
    SetStatusText("Pronto", 0);
}

// Hot-reload callbacks
void EditorFrame::OnMapFileChanged(const wxString& path, const wxString& filename)
{
    wxLogMessage("Mapa modificado: %s", filename);
    
    // Recarregar no viewport
    if (m_viewport) {
        // TODO: Implementar recarga real do mapa
        SetStatusText(wxString::Format("Hot-reload: %s", filename), 0);
        
        // For now, just refresh the viewport
        m_viewport->Refresh();
    }
}

void EditorFrame::OnDataFileChanged(const wxString& path, const wxString& filename)
{
    wxLogMessage("Database modificado: %s", filename);
    
    // Recarregar no property grid
    if (m_propertyGrid) {
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
    
    // Atualizar árvore do projeto
    if (m_projectTree) {
        // TODO: Implementar carregamento real da árvore do projeto
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
