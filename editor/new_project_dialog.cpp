/**
 * Implementação do diálogo para criar novos projetos
 */

#pragma execution_character_set("utf-8")

#include "new_project_dialog.h"
#include "utf8_strings.h"
#include <wx/statbox.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/dir.h>

wxBEGIN_EVENT_TABLE(NewProjectDialog, wxDialog)
    EVT_TEXT(wxID_ANY, NewProjectDialog::OnProjectNameChanged)
    EVT_BUTTON(wxID_OK, NewProjectDialog::OnOK)
    EVT_BUTTON(wxID_CANCEL, NewProjectDialog::OnCancel)
wxEND_EVENT_TABLE()

NewProjectDialog::NewProjectDialog(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, "Novo Projeto Lumy", wxDefaultPosition, wxSize(500, 400))
{
    // Configurar localização sugerida (Documentos do usuário)
    wxStandardPaths& paths = wxStandardPaths::Get();
    m_suggestedLocation = paths.GetDocumentsDir();
    
    CreateControlsWithoutDirPicker();
    ConnectEvents();
    
    // Focar no campo de nome do projeto se existir
    if (m_projectNameCtrl) {
        m_projectNameCtrl->SetFocus();
    }
}

void NewProjectDialog::CreateControlsWithoutDirPicker()
{
    // Sizer principal
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    
    // === Seção do nome do projeto ===
    wxStaticBoxSizer* projectSizer = new wxStaticBoxSizer(wxVERTICAL, this, UTF8("Informações do Projeto"));
    
    // Nome do projeto
    wxFlexGridSizer* gridSizer = new wxFlexGridSizer(2, 2, 5, 10);
    gridSizer->AddGrowableCol(1, 1);
    
    gridSizer->Add(new wxStaticText(this, wxID_ANY, "Nome:"), 0, wxALIGN_CENTER_VERTICAL);
    m_projectNameCtrl = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
    gridSizer->Add(m_projectNameCtrl, 1, wxEXPAND);
    
    // Localização usando wxTextCtrl em vez de wxDirPickerCtrl
    gridSizer->Add(new wxStaticText(this, wxID_ANY, UTF8("Localização:")), 0, wxALIGN_CENTER_VERTICAL);
    m_locationTextCtrl = new wxTextCtrl(this, wxID_ANY, m_suggestedLocation, wxDefaultPosition, wxDefaultSize);
    gridSizer->Add(m_locationTextCtrl, 1, wxEXPAND);
    
    projectSizer->Add(gridSizer, 0, wxEXPAND | wxALL, 10);
    
    // Caminho completo (somente leitura)
    wxBoxSizer* pathSizer = new wxBoxSizer(wxHORIZONTAL);
    pathSizer->Add(new wxStaticText(this, wxID_ANY, "Pasta completa:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    m_fullPathLabel = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_MIDDLE);
    m_fullPathLabel->SetFont(m_fullPathLabel->GetFont().MakeBold());
    pathSizer->Add(m_fullPathLabel, 1, wxALIGN_CENTER_VERTICAL);
    
    projectSizer->Add(pathSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);
    
    mainSizer->Add(projectSizer, 0, wxEXPAND | wxALL, 10);
    
    // === Seção de detalhes opcionais ===
    wxStaticBoxSizer* detailsSizer = new wxStaticBoxSizer(wxVERTICAL, this, "Detalhes Opcionais");
    
    wxFlexGridSizer* detailsGrid = new wxFlexGridSizer(2, 2, 5, 10);
    detailsGrid->AddGrowableCol(1, 1);
    
    // Autor
    detailsGrid->Add(new wxStaticText(this, wxID_ANY, "Autor:"), 0, wxALIGN_CENTER_VERTICAL);
    m_authorCtrl = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
    detailsGrid->Add(m_authorCtrl, 1, wxEXPAND);
    
    detailsSizer->Add(detailsGrid, 0, wxEXPAND | wxALL, 10);
    
    // Descrição (campo maior)
    detailsSizer->Add(new wxStaticText(this, wxID_ANY, UTF8("Descrição:")), 0, wxLEFT | wxRIGHT, 10);
    m_descriptionCtrl = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(-1, 60), wxTE_MULTILINE);
    detailsSizer->Add(m_descriptionCtrl, 1, wxEXPAND | wxALL, 10);
    
    mainSizer->Add(detailsSizer, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);
    
    // === Botões ===
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonSizer->AddStretchSpacer();
    
    m_okButton = new wxButton(this, wxID_OK, "Criar Projeto");
    m_okButton->SetDefault();
    m_okButton->Enable(false); // Desabilitado até ter nome válido
    buttonSizer->Add(m_okButton, 0, wxRIGHT, 5);
    
    m_cancelButton = new wxButton(this, wxID_CANCEL, "Cancelar");
    buttonSizer->Add(m_cancelButton, 0);
    
    mainSizer->Add(buttonSizer, 0, wxEXPAND | wxALL, 10);
    
    // Aplicar sizers
    SetSizer(mainSizer);
    
    // Inicializar ponteiros que não foram criados
    m_locationPicker = nullptr;
    
    // Atualizar caminho inicial
    UpdateProjectPathForTextCtrl();
}

void NewProjectDialog::CreateControlsSimple()
{
    // Sizer principal
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    
    // Campo simples de nome
    mainSizer->Add(new wxStaticText(this, wxID_ANY, "Nome do Projeto:"), 0, wxALL, 10);
    m_projectNameCtrl = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
    mainSizer->Add(m_projectNameCtrl, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);
    
    // Botões
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonSizer->AddStretchSpacer();
    
    m_okButton = new wxButton(this, wxID_OK, "OK");
    m_okButton->SetDefault();
    buttonSizer->Add(m_okButton, 0, wxRIGHT, 5);
    
    m_cancelButton = new wxButton(this, wxID_CANCEL, "Cancelar");
    buttonSizer->Add(m_cancelButton, 0);
    
    mainSizer->Add(buttonSizer, 0, wxEXPAND | wxALL, 10);
    
    // Aplicar sizer
    SetSizer(mainSizer);
    
    // Inicializar ponteiros que não foram criados
    m_locationPicker = nullptr;
    m_locationTextCtrl = nullptr;
    m_fullPathLabel = nullptr;
    m_descriptionCtrl = nullptr;
    m_authorCtrl = nullptr;
}

void NewProjectDialog::CreateControls()
{
    // Sizer principal
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    
    // === Seção do nome do projeto ===
    wxStaticBoxSizer* projectSizer = new wxStaticBoxSizer(wxVERTICAL, this, "Informações do Projeto");
    
    // Nome do projeto
    wxFlexGridSizer* gridSizer = new wxFlexGridSizer(2, 2, 5, 10);
    gridSizer->AddGrowableCol(1, 1);
    
    gridSizer->Add(new wxStaticText(this, wxID_ANY, "Nome:"), 0, wxALIGN_CENTER_VERTICAL);
    m_projectNameCtrl = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
    gridSizer->Add(m_projectNameCtrl, 1, wxEXPAND);
    
    // Localização
    gridSizer->Add(new wxStaticText(this, wxID_ANY, "Localização:"), 0, wxALIGN_CENTER_VERTICAL);
    m_locationPicker = new wxDirPickerCtrl(this, wxID_ANY, m_suggestedLocation, 
        "Escolher pasta do projeto", wxDefaultPosition, wxDefaultSize,
        wxDIRP_DIR_MUST_EXIST | wxDIRP_USE_TEXTCTRL);
    gridSizer->Add(m_locationPicker, 1, wxEXPAND);
    
    projectSizer->Add(gridSizer, 0, wxEXPAND | wxALL, 10);
    
    // Caminho completo (somente leitura)
    wxBoxSizer* pathSizer = new wxBoxSizer(wxHORIZONTAL);
    pathSizer->Add(new wxStaticText(this, wxID_ANY, "Pasta completa:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    m_fullPathLabel = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_MIDDLE);
    m_fullPathLabel->SetFont(m_fullPathLabel->GetFont().MakeBold());
    pathSizer->Add(m_fullPathLabel, 1, wxALIGN_CENTER_VERTICAL);
    
    projectSizer->Add(pathSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);
    
    mainSizer->Add(projectSizer, 0, wxEXPAND | wxALL, 10);
    
    // === Seção de detalhes opcionais ===
    wxStaticBoxSizer* detailsSizer = new wxStaticBoxSizer(wxVERTICAL, this, "Detalhes Opcionais");
    
    wxFlexGridSizer* detailsGrid = new wxFlexGridSizer(2, 2, 5, 10);
    detailsGrid->AddGrowableCol(1, 1);
    
    // Autor
    detailsGrid->Add(new wxStaticText(this, wxID_ANY, "Autor:"), 0, wxALIGN_CENTER_VERTICAL);
    m_authorCtrl = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
    detailsGrid->Add(m_authorCtrl, 1, wxEXPAND);
    
    detailsSizer->Add(detailsGrid, 0, wxEXPAND | wxALL, 10);
    
    // Descrição (campo maior)
    detailsSizer->Add(new wxStaticText(this, wxID_ANY, "Descrição:"), 0, wxLEFT | wxRIGHT, 10);
    m_descriptionCtrl = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(-1, 60), wxTE_MULTILINE);
    detailsSizer->Add(m_descriptionCtrl, 1, wxEXPAND | wxALL, 10);
    
    mainSizer->Add(detailsSizer, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);
    
    // === Botões ===
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonSizer->AddStretchSpacer();
    
    m_okButton = new wxButton(this, wxID_OK, "Criar Projeto");
    m_okButton->SetDefault();
    m_okButton->Enable(false); // Desabilitado até ter nome válido
    buttonSizer->Add(m_okButton, 0, wxRIGHT, 5);
    
    m_cancelButton = new wxButton(this, wxID_CANCEL, "Cancelar");
    buttonSizer->Add(m_cancelButton, 0);
    
    mainSizer->Add(buttonSizer, 0, wxEXPAND | wxALL, 10);
    
    // Aplicar sizers
    SetSizer(mainSizer);
    
    // Atualizar caminho inicial
    UpdateProjectPath();
}

void NewProjectDialog::ConnectEvents()
{
    // Eventos já conectados via event table
}

void NewProjectDialog::OnProjectNameChanged(wxCommandEvent& event)
{
    if (m_fullPathLabel) {
        // Usar método apropriado baseado no tipo de controle
        if (m_locationTextCtrl) {
            UpdateProjectPathForTextCtrl();
        } else {
            UpdateProjectPath();
        }
    }
    if (m_okButton) {
        m_okButton->Enable(ValidateInput());
    }
    event.Skip();
}

void NewProjectDialog::OnOK(wxCommandEvent& event)
{
    if (!ValidateInput()) {
        wxMessageBox("Por favor, corrija os erros antes de continuar.", "Dados inválidos", 
                     wxOK | wxICON_WARNING, this);
        return;
    }
    
    // Verificar se a pasta já existe
    wxString fullPath = GetProjectPath();
    if (wxDir::Exists(fullPath)) {
        wxDir dir(fullPath);
        if (dir.IsOpened() && (dir.HasFiles() || dir.HasSubDirs())) {
            int result = wxMessageBox(
                wxString::Format("A pasta '%s' já existe e não está vazia.\nDeseja continuar mesmo assim?", fullPath),
                "Pasta já existe",
                wxYES_NO | wxICON_QUESTION, this);
                
            if (result != wxYES) {
                return;
            }
        }
    }
    
    EndModal(wxID_OK);
}

void NewProjectDialog::OnCancel(wxCommandEvent& event)
{
    EndModal(wxID_CANCEL);
}

bool NewProjectDialog::ValidateInput()
{
    if (!m_projectNameCtrl) return false;
    
    wxString projectName = m_projectNameCtrl->GetValue().Trim();
    
    // Nome não pode estar vazio
    if (projectName.IsEmpty()) {
        return false;
    }
    
    // Nome não pode ter caracteres inválidos
    if (projectName.find_first_of("\\/:*?\"<>|") != wxString::npos) {
        return false;
    }
    
    // Se não temos location picker (versão simples), considerar válido
    if (!m_locationPicker) {
        return true;
    }
    
    // Localização deve existir
    wxString location = m_locationPicker->GetPath();
    if (!wxDir::Exists(location)) {
        return false;
    }
    
    return true;
}

void NewProjectDialog::UpdateProjectPath()
{
    wxString projectName = m_projectNameCtrl->GetValue().Trim();
    wxString location = m_locationPicker->GetPath();
    
    if (!projectName.IsEmpty() && wxDir::Exists(location)) {
        wxString fullPath = location + wxFileName::GetPathSeparator() + projectName;
        m_fullPathLabel->SetLabel(fullPath);
    } else {
        m_fullPathLabel->SetLabel("(caminho será exibido quando nome e localização forem preenchidos)");
    }
    
    Layout();
}

void NewProjectDialog::UpdateProjectPathForTextCtrl()
{
    if (!m_projectNameCtrl || !m_locationTextCtrl || !m_fullPathLabel) {
        return;
    }
    
    wxString projectName = m_projectNameCtrl->GetValue().Trim();
    wxString location = m_locationTextCtrl->GetValue().Trim();
    
    if (!projectName.IsEmpty() && !location.IsEmpty()) {
        wxString fullPath = location + wxFileName::GetPathSeparator() + projectName;
        m_fullPathLabel->SetLabel(fullPath);
    } else {
        m_fullPathLabel->SetLabel("(caminho será exibido quando nome e localização forem preenchidos)");
    }
    
    Layout();
}

// Getters

wxString NewProjectDialog::GetProjectName() const
{
    return m_projectNameCtrl->GetValue().Trim();
}

wxString NewProjectDialog::GetProjectPath() const
{
    wxString projectName = GetProjectName();
    
    // Verificar se temos m_locationTextCtrl (nova versão)
    if (m_locationTextCtrl) {
        wxString location = m_locationTextCtrl->GetValue().Trim();
        if (!projectName.IsEmpty() && !location.IsEmpty()) {
            return location + wxFileName::GetPathSeparator() + projectName;
        }
    }
    // Se temos location picker (versão original)
    else if (m_locationPicker) {
        wxString location = m_locationPicker->GetPath();
        if (!projectName.IsEmpty() && wxDir::Exists(location)) {
            return location + wxFileName::GetPathSeparator() + projectName;
        }
    }
    // Versão simples - usar localização sugerida
    else {
        if (!projectName.IsEmpty()) {
            return m_suggestedLocation + wxFileName::GetPathSeparator() + projectName;
        }
    }
    
    return wxString();
}

wxString NewProjectDialog::GetProjectDescription() const
{
    if (m_descriptionCtrl) {
        return m_descriptionCtrl->GetValue().Trim();
    }
    return wxString();
}

wxString NewProjectDialog::GetProjectAuthor() const
{
    if (m_authorCtrl) {
        return m_authorCtrl->GetValue().Trim();
    }
    return wxString();
}

ProjectInfo NewProjectDialog::GetProjectInfo() const
{
    ProjectInfo info;
    info.projectName = GetProjectName();
    info.description = GetProjectDescription();
    info.author = GetProjectAuthor();
    
    return info;
}
