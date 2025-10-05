/**
 * Implementação do Property Grid Panel
 */

#pragma execution_character_set("utf-8")

#include "property_grid_panel.h"
#include "utf8_strings.h"

wxBEGIN_EVENT_TABLE(PropertyGridPanel, wxPanel)
    EVT_PG_CHANGED(wxID_ANY, PropertyGridPanel::OnPropertyChanged)
    EVT_PG_SELECTED(wxID_ANY, PropertyGridPanel::OnPropertySelected)
wxEND_EVENT_TABLE()

PropertyGridPanel::PropertyGridPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY)
{
    CreateControls();
    PopulateDefaultProperties();
}

void PropertyGridPanel::CreateControls()
{
    // Criar property grid
    m_propertyGrid = new wxPropertyGrid(this, wxID_ANY,
        wxDefaultPosition, wxDefaultSize,
        wxPG_SPLITTER_AUTO_CENTER | wxPG_DEFAULT_STYLE
    );
    
    // Layout
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    
    // Título
    wxStaticText* title = new wxStaticText(this, wxID_ANY, UTF8("Propriedades"));
    wxFont titleFont = title->GetFont();
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    title->SetFont(titleFont);
    
    sizer->Add(title, 0, wxALL | wxEXPAND, 5);
    sizer->Add(m_propertyGrid, 1, wxALL | wxEXPAND, 5);
    
    SetSizer(sizer);
}

void PropertyGridPanel::PopulateDefaultProperties()
{
    m_propertyGrid->Clear();
    
    // Categoria principal
    m_propertyGrid->Append(new wxPropertyCategory("Lumy Editor"));
    
    // Propriedades básicas
    m_propertyGrid->Append(new wxStringProperty(UTF8("Versão"), "version", "0.1.1"));
    m_propertyGrid->Append(new wxStringProperty("Marco", "milestone", "M1 - Brilho"));
    m_propertyGrid->Append(new wxStringProperty("Status", "status", "Em desenvolvimento"));
    
    // Marcar como somente leitura
    m_propertyGrid->SetPropertyAttribute("version", wxPG_BOOL_USE_DOUBLE_CLICK_CYCLING, false);
    m_propertyGrid->SetPropertyAttribute("milestone", wxPG_BOOL_USE_DOUBLE_CLICK_CYCLING, false);
    m_propertyGrid->SetPropertyAttribute("status", wxPG_BOOL_USE_DOUBLE_CLICK_CYCLING, false);
    
    // Desabilitar edição
    m_propertyGrid->EnableProperty("version", false);
    m_propertyGrid->EnableProperty("milestone", false);
    m_propertyGrid->EnableProperty("status", false);
}

void PropertyGridPanel::PopulateMapProperties()
{
    m_propertyGrid->Clear();
    
    // Categoria do mapa
    m_propertyGrid->Append(new wxPropertyCategory("Propriedades do Mapa"));
    
    // Propriedades do mapa
    m_propertyGrid->Append(new wxStringProperty("Nome", "map_name", "hello.tmx"));
    m_propertyGrid->Append(new wxIntProperty("Largura", "map_width", 25));
    m_propertyGrid->Append(new wxIntProperty("Altura", "map_height", 15));
    m_propertyGrid->Append(new wxIntProperty("Tile Width", "tile_width", 32));
    m_propertyGrid->Append(new wxIntProperty("Tile Height", "tile_height", 32));
    
    // Propriedades de renderização
    m_propertyGrid->Append(new wxPropertyCategory("Renderização"));
    m_propertyGrid->Append(new wxColourProperty("Cor de Fundo", "bg_color", *wxLIGHT_GREY));
    m_propertyGrid->Append(new wxBoolProperty("Mostrar Grade", "show_grid", true));
    m_propertyGrid->Append(new wxBoolProperty("Mostrar Colisão", "show_collision", false));
    
    // Propriedades de eventos
    m_propertyGrid->Append(new wxPropertyCategory("Sistema de Eventos"));
    m_propertyGrid->Append(new wxIntProperty("Total de Eventos", "event_count", 3));
    m_propertyGrid->Append(new wxBoolProperty("Hot Reload Ativo", "hot_reload", true));
}

void PropertyGridPanel::PopulateEventProperties()
{
    m_propertyGrid->Clear();
    
    // Categoria do evento
    m_propertyGrid->Append(new wxPropertyCategory("Evento Selecionado"));
    
    // Propriedades básicas do evento
    m_propertyGrid->Append(new wxStringProperty("Nome", "event_name", "NPC da Vila"));
    m_propertyGrid->Append(new wxIntProperty("ID", "event_id", 1));
    m_propertyGrid->Append(new wxIntProperty("X", "event_x", 5));
    m_propertyGrid->Append(new wxIntProperty("Y", "event_y", 7));
    
    // Tipo de trigger
    wxEnumProperty* triggerProp = new wxEnumProperty("Trigger");
    triggerProp->AddChoice("Ação");
    triggerProp->AddChoice("Toque");
    triggerProp->AddChoice("Automático");
    triggerProp->AddChoice("Paralelo");
    triggerProp->SetValue(0);
    m_propertyGrid->Append(triggerProp);
    
    // Comandos
    m_propertyGrid->Append(new wxPropertyCategory("Comandos"));
    m_propertyGrid->Append(new wxStringProperty("Comando 1", "cmd_1", "ShowText: Olá, aventureiro!"));
    m_propertyGrid->Append(new wxStringProperty("Comando 2", "cmd_2", "SetSwitch: 1, true"));
    m_propertyGrid->Append(new wxStringProperty("Comando 3", "cmd_3", "Wait: 1000"));
}

void PropertyGridPanel::OnPropertyChanged(wxPropertyGridEvent& event)
{
    wxPGProperty* property = event.GetProperty();
    if (!property) return;
    
    wxString propName = property->GetName();
    wxVariant value = property->GetValue();
    
    // TODO: Implementar atualização das propriedades no modelo/runtime
    // Para M1, apenas mostrar que a propriedade mudou
    
    // Log da mudança (temporário para desenvolvimento)
    wxString msg = wxString::Format("Propriedade alterada: %s = %s", propName, value.GetString());
    // Para agora, só guardamos internamente - depois implementaremos hot-reload
    
    event.Skip();
}

void PropertyGridPanel::OnPropertySelected(wxPropertyGridEvent& event)
{
    wxPGProperty* property = event.GetProperty();
    if (!property) return;
    
    // TODO: Mostrar descrição da propriedade ou ajuda contextual
    // Para M1, apenas registrar a seleção
    
    event.Skip();
}
