/**
 * Diálogo para criação de novos projetos
 */

#pragma execution_character_set("utf-8")

#pragma once

#include <wx/wx.h>
#include <wx/filepicker.h>
#include "project_manager.h"

class NewProjectDialog : public wxDialog
{
public:
    NewProjectDialog(wxWindow* parent);
    virtual ~NewProjectDialog() = default;
    
    // Getters para os dados do projeto
    wxString GetProjectName() const;
    wxString GetProjectPath() const;
    wxString GetProjectDescription() const;
    wxString GetProjectAuthor() const;
    
    // Criar ProjectInfo baseado nos dados do diálogo
    ProjectInfo GetProjectInfo() const;
    
private:
    void CreateControls();
    void CreateControlsSimple();
    void CreateControlsWithoutDirPicker();
    void ConnectEvents();
    
    // Event handlers
    void OnProjectNameChanged(wxCommandEvent& event);
    void OnOK(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    
    // Validação
    bool ValidateInput();
    void UpdateProjectPath();
    void UpdateProjectPathForTextCtrl();
    
    // Controles
    wxTextCtrl* m_projectNameCtrl;
    wxDirPickerCtrl* m_locationPicker;
    wxTextCtrl* m_locationTextCtrl; // Nova versão sem DirPicker
    wxStaticText* m_fullPathLabel;
    wxTextCtrl* m_descriptionCtrl;
    wxTextCtrl* m_authorCtrl;
    wxButton* m_okButton;
    wxButton* m_cancelButton;
    
    // Estado
    wxString m_suggestedLocation;
    
    wxDECLARE_EVENT_TABLE();
};
