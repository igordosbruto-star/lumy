/**
 * Implementação da EditorFrame
 */

#include "editor_frame.h"
#include "project_tree_panel.h"
#include "property_grid_panel.h"
#include "viewport_panel.h"

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
    CreateStatusBar(3);
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
    // TODO: Implementar abertura de projeto
    wxMessageBox("Função 'Abrir Projeto' será implementada em breve!", "M1 - Em Desenvolvimento", wxOK | wxICON_INFORMATION);
    SetStatusText("Pronto", 0);
}

void EditorFrame::OnSaveProject(wxCommandEvent& WXUNUSED(event))
{
    SetStatusText("Salvando projeto...", 0);
    // TODO: Implementar salvamento de projeto
    wxMessageBox("Função 'Salvar Projeto' será implementada em breve!", "M1 - Em Desenvolvimento", wxOK | wxICON_INFORMATION);
    SetStatusText("Pronto", 0);
}
