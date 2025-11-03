/**
 * Editor Lumy - Marco 1 "Brilho"
 * Aplicação principal wxWidgets com wxAUI para edição de mapas e eventos
 */

#include <wx/wx.h>
#include <wx/aui/aui.h>
#include "editor_frame.h"

class LumyEditorApp : public wxApp
{
public:
    virtual bool OnInit();
    virtual int OnExit();

private:
    EditorFrame* m_frame;
};

bool LumyEditorApp::OnInit()
{
    if (!wxApp::OnInit())
        return false;

    // Configurar nome da aplicação
    SetAppName("Lumy Editor");
    SetAppDisplayName("Lumy Editor - M1 Brilho");

    // Criar janela principal
    m_frame = new EditorFrame();
    m_frame->Show(true);

    return true;
}

int LumyEditorApp::OnExit()
{
    return wxApp::OnExit();
}

// Macro para implementar a aplicação
wxIMPLEMENT_APP(LumyEditorApp);
