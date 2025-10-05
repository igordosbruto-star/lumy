/**
 * Implementação do TilePropertiesDialog
 */

#pragma execution_character_set("utf-8")

#include "tile_properties_dialog.h"
#include "utf8_strings.h"
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/statbox.h>

// Event table
wxBEGIN_EVENT_TABLE(TilePropertiesDialog, wxDialog)
    EVT_LIST_ITEM_SELECTED(TilePropertiesDialog::ID_TILE_LIST, TilePropertiesDialog::OnTileSelectionChanged)
    EVT_TEXT(TilePropertiesDialog::ID_NAME_TEXT, TilePropertiesDialog::OnPropertyChanged)
    EVT_CHOICE(TilePropertiesDialog::ID_CATEGORY_CHOICE, TilePropertiesDialog::OnPropertyChanged)
    EVT_TEXT(TilePropertiesDialog::ID_DESCRIPTION_TEXT, TilePropertiesDialog::OnPropertyChanged)
    EVT_CHECKBOX(TilePropertiesDialog::ID_COLLISION_CHECK, TilePropertiesDialog::OnPropertyChanged)
    EVT_CHOICE(TilePropertiesDialog::ID_COLLISION_TYPE_CHOICE, TilePropertiesDialog::OnPropertyChanged)
    EVT_TEXT(TilePropertiesDialog::ID_COLLISION_DATA_TEXT, TilePropertiesDialog::OnPropertyChanged)
    EVT_CHECKBOX(TilePropertiesDialog::ID_ANIMATED_CHECK, TilePropertiesDialog::OnPropertyChanged)
    EVT_TEXT(TilePropertiesDialog::ID_ANIMATION_FRAMES_TEXT, TilePropertiesDialog::OnPropertyChanged)
    EVT_TEXT(TilePropertiesDialog::ID_ANIMATION_SPEED_TEXT, TilePropertiesDialog::OnPropertyChanged)
    EVT_TEXT(TilePropertiesDialog::ID_STEP_SOUND_TEXT, TilePropertiesDialog::OnPropertyChanged)
    EVT_TEXT(TilePropertiesDialog::ID_PLACE_SOUND_TEXT, TilePropertiesDialog::OnPropertyChanged)
    EVT_TEXT(TilePropertiesDialog::ID_BREAK_SOUND_TEXT, TilePropertiesDialog::OnPropertyChanged)
    EVT_BUTTON(TilePropertiesDialog::ID_ADD_PROP_BUTTON, TilePropertiesDialog::OnAddCustomProperty)
    EVT_BUTTON(TilePropertiesDialog::ID_REMOVE_PROP_BUTTON, TilePropertiesDialog::OnRemoveCustomProperty)
    EVT_BUTTON(wxID_OK, TilePropertiesDialog::OnOK)
    EVT_BUTTON(wxID_CANCEL, TilePropertiesDialog::OnCancel)
    EVT_BUTTON(TilePropertiesDialog::ID_APPLY_BUTTON, TilePropertiesDialog::OnApply)
wxEND_EVENT_TABLE()

TilePropertiesDialog::TilePropertiesDialog(wxWindow* parent, TilesetManager* tilesetManager, int selectedTileId)
    : wxDialog(parent, wxID_ANY, UTF8("Propriedades dos Tiles"), wxDefaultPosition, wxSize(800, 600), 
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
    , m_tilesetManager(tilesetManager)
    , m_currentTileId(selectedTileId)
    , m_hasChanges(false)
    , m_tileListCtrl(nullptr)
    , m_tilePreview(nullptr)
    , m_tileInfoLabel(nullptr)
    , m_nameTextCtrl(nullptr)
    , m_categoryChoice(nullptr)
    , m_descriptionTextCtrl(nullptr)
    , m_hasCollisionCheckBox(nullptr)
    , m_collisionTypeChoice(nullptr)
    , m_collisionDataTextCtrl(nullptr)
    , m_isAnimatedCheckBox(nullptr)
    , m_animationFramesTextCtrl(nullptr)
    , m_animationSpeedTextCtrl(nullptr)
    , m_stepSoundTextCtrl(nullptr)
    , m_placeSoundTextCtrl(nullptr)
    , m_breakSoundTextCtrl(nullptr)
    , m_customPropsListCtrl(nullptr)
    , m_newPropNameTextCtrl(nullptr)
    , m_newPropValueTextCtrl(nullptr)
    , m_addPropButton(nullptr)
    , m_removePropButton(nullptr)
    , m_okButton(nullptr)
    , m_cancelButton(nullptr)
    , m_applyButton(nullptr)
{
    CreateControls();
    PopulateTileList();
    
    if (selectedTileId >= 0) {
        LoadTileProperties(selectedTileId);
    } else {
        EnableControls(false);
    }
}

void TilePropertiesDialog::CreateControls()
{
    wxBoxSizer* mainSizer = new wxBoxSizer(wxHORIZONTAL);
    
    // Painel esquerdo - Lista de tiles
    wxStaticBoxSizer* tileListSizer = new wxStaticBoxSizer(wxVERTICAL, this, UTF8("Tiles"));
    
    // Lista de tiles
    m_tileListCtrl = new wxListCtrl(this, ID_TILE_LIST, wxDefaultPosition, wxSize(200, 300),
                                   wxLC_REPORT | wxLC_SINGLE_SEL);
    m_tileListCtrl->AppendColumn(UTF8("ID"), wxLIST_FORMAT_LEFT, 40);
    m_tileListCtrl->AppendColumn(UTF8("Nome"), wxLIST_FORMAT_LEFT, 140);
    
    // Preview do tile selecionado
    wxImage emptyImage(64, 64);
    emptyImage.SetRGB(wxRect(0, 0, 64, 64), 200, 200, 200);
    m_tilePreview = new wxStaticBitmap(this, wxID_ANY, wxBitmap(emptyImage));
    
    m_tileInfoLabel = new wxStaticText(this, wxID_ANY, UTF8("Nenhum tile selecionado"));
    
    tileListSizer->Add(m_tileListCtrl, 1, wxEXPAND | wxALL, 5);
    tileListSizer->Add(m_tilePreview, 0, wxALIGN_CENTER | wxALL, 5);
    tileListSizer->Add(m_tileInfoLabel, 0, wxALIGN_CENTER | wxALL, 5);
    
    // Painel direito - Propriedades
    wxNotebook* notebook = new wxNotebook(this, wxID_ANY);
    CreateBasicPropertiesTab(notebook);
    CreateCollisionTab(notebook);
    CreateAnimationTab(notebook);
    CreateAudioTab(notebook);
    CreateCustomPropertiesTab(notebook);
    
    // Botões
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    m_applyButton = new wxButton(this, ID_APPLY_BUTTON, UTF8("&Aplicar"));
    m_okButton = new wxButton(this, wxID_OK, UTF8("&OK"));
    m_cancelButton = new wxButton(this, wxID_CANCEL, UTF8("&Cancelar"));
    
    buttonSizer->Add(m_applyButton, 0, wxALL, 5);
    buttonSizer->AddStretchSpacer();
    buttonSizer->Add(m_okButton, 0, wxALL, 5);
    buttonSizer->Add(m_cancelButton, 0, wxALL, 5);
    
    // Layout principal
    mainSizer->Add(tileListSizer, 0, wxEXPAND | wxALL, 10);
    mainSizer->Add(notebook, 1, wxEXPAND | wxALL, 10);
    
    wxBoxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
    outerSizer->Add(mainSizer, 1, wxEXPAND);
    outerSizer->Add(buttonSizer, 0, wxEXPAND | wxALL, 10);
    
    SetSizer(outerSizer);
}

void TilePropertiesDialog::CreateBasicPropertiesTab(wxNotebook* notebook)
{
    wxPanel* panel = new wxPanel(notebook, wxID_ANY);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    
    // Nome
    wxBoxSizer* nameSizer = new wxBoxSizer(wxHORIZONTAL);
    nameSizer->Add(new wxStaticText(panel, wxID_ANY, UTF8("Nome:")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    m_nameTextCtrl = new wxTextCtrl(panel, ID_NAME_TEXT, "");
    nameSizer->Add(m_nameTextCtrl, 1, wxEXPAND);
    sizer->Add(nameSizer, 0, wxEXPAND | wxALL, 5);
    
    // Categoria
    wxBoxSizer* categorySizer = new wxBoxSizer(wxHORIZONTAL);
    categorySizer->Add(new wxStaticText(panel, wxID_ANY, UTF8("Categoria:")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    m_categoryChoice = new wxChoice(panel, ID_CATEGORY_CHOICE);
    m_categoryChoice->Append(UTF8("Terreno"));
    m_categoryChoice->Append(UTF8("Estrutura"));
    m_categoryChoice->Append(UTF8("Decoração"));
    m_categoryChoice->Append(UTF8("Especial"));
    m_categoryChoice->SetSelection(0);
    categorySizer->Add(m_categoryChoice, 1, wxEXPAND);
    sizer->Add(categorySizer, 0, wxEXPAND | wxALL, 5);
    
    // Descrição
    sizer->Add(new wxStaticText(panel, wxID_ANY, UTF8("Descrição:")), 0, wxALL, 5);
    m_descriptionTextCtrl = new wxTextCtrl(panel, ID_DESCRIPTION_TEXT, "", wxDefaultPosition, wxDefaultSize, 
                                          wxTE_MULTILINE | wxTE_WORDWRAP);
    sizer->Add(m_descriptionTextCtrl, 1, wxEXPAND | wxALL, 5);
    
    panel->SetSizer(sizer);
    notebook->AddPage(panel, UTF8("Básico"));
}

void TilePropertiesDialog::CreateCollisionTab(wxNotebook* notebook)
{
    wxPanel* panel = new wxPanel(notebook, wxID_ANY);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    
    // Checkbox principal
    m_hasCollisionCheckBox = new wxCheckBox(panel, ID_COLLISION_CHECK, UTF8("Este tile tem colisão"));
    sizer->Add(m_hasCollisionCheckBox, 0, wxALL, 5);
    
    // Tipo de colisão
    wxBoxSizer* typeSizer = new wxBoxSizer(wxHORIZONTAL);
    typeSizer->Add(new wxStaticText(panel, wxID_ANY, UTF8("Tipo:")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    m_collisionTypeChoice = new wxChoice(panel, ID_COLLISION_TYPE_CHOICE);
    m_collisionTypeChoice->Append(UTF8("Sólido"));
    m_collisionTypeChoice->Append(UTF8("Plataforma"));
    m_collisionTypeChoice->Append(UTF8("Trigger"));
    m_collisionTypeChoice->Append(UTF8("Customizado"));
    m_collisionTypeChoice->SetSelection(0);
    typeSizer->Add(m_collisionTypeChoice, 1, wxEXPAND);
    sizer->Add(typeSizer, 0, wxEXPAND | wxALL, 5);
    
    // Dados de colisão customizada
    sizer->Add(new wxStaticText(panel, wxID_ANY, UTF8("Dados customizados (formato JSON):")), 0, wxALL, 5);
    m_collisionDataTextCtrl = new wxTextCtrl(panel, ID_COLLISION_DATA_TEXT, "", wxDefaultPosition, wxDefaultSize, 
                                           wxTE_MULTILINE | wxTE_WORDWRAP);
    m_collisionDataTextCtrl->SetValue("{}");
    sizer->Add(m_collisionDataTextCtrl, 1, wxEXPAND | wxALL, 5);
    
    panel->SetSizer(sizer);
    notebook->AddPage(panel, UTF8("Colisão"));
}

void TilePropertiesDialog::CreateAnimationTab(wxNotebook* notebook)
{
    wxPanel* panel = new wxPanel(notebook, wxID_ANY);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    
    // Checkbox principal
    m_isAnimatedCheckBox = new wxCheckBox(panel, ID_ANIMATED_CHECK, UTF8("Este tile é animado"));
    sizer->Add(m_isAnimatedCheckBox, 0, wxALL, 5);
    
    // Número de frames
    wxBoxSizer* framesSizer = new wxBoxSizer(wxHORIZONTAL);
    framesSizer->Add(new wxStaticText(panel, wxID_ANY, UTF8("Frames:")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    m_animationFramesTextCtrl = new wxTextCtrl(panel, ID_ANIMATION_FRAMES_TEXT, "1");
    framesSizer->Add(m_animationFramesTextCtrl, 1, wxEXPAND);
    sizer->Add(framesSizer, 0, wxEXPAND | wxALL, 5);
    
    // Velocidade da animação
    wxBoxSizer* speedSizer = new wxBoxSizer(wxHORIZONTAL);
    speedSizer->Add(new wxStaticText(panel, wxID_ANY, UTF8("Velocidade (ms):")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    m_animationSpeedTextCtrl = new wxTextCtrl(panel, ID_ANIMATION_SPEED_TEXT, "500");
    speedSizer->Add(m_animationSpeedTextCtrl, 1, wxEXPAND);
    sizer->Add(speedSizer, 0, wxEXPAND | wxALL, 5);
    
    // Informações adicionais
    wxStaticText* infoText = new wxStaticText(panel, wxID_ANY, 
        UTF8("Para tiles animados, os frames devem estar na mesma linha\n"
             "do tileset, imediatamente após o tile base."));
    infoText->SetForegroundColour(wxColour(100, 100, 100));
    sizer->Add(infoText, 0, wxALL, 10);
    
    sizer->AddStretchSpacer();
    
    panel->SetSizer(sizer);
    notebook->AddPage(panel, UTF8("Animação"));
}

void TilePropertiesDialog::CreateAudioTab(wxNotebook* notebook)
{
    wxPanel* panel = new wxPanel(notebook, wxID_ANY);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    
    // Som de caminhada
    wxBoxSizer* stepSizer = new wxBoxSizer(wxHORIZONTAL);
    stepSizer->Add(new wxStaticText(panel, wxID_ANY, UTF8("Som ao pisar:")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    m_stepSoundTextCtrl = new wxTextCtrl(panel, ID_STEP_SOUND_TEXT, "");
    stepSizer->Add(m_stepSoundTextCtrl, 1, wxEXPAND);
    sizer->Add(stepSizer, 0, wxEXPAND | wxALL, 5);
    
    // Som de colocação
    wxBoxSizer* placeSizer = new wxBoxSizer(wxHORIZONTAL);
    placeSizer->Add(new wxStaticText(panel, wxID_ANY, UTF8("Som ao colocar:")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    m_placeSoundTextCtrl = new wxTextCtrl(panel, ID_PLACE_SOUND_TEXT, "");
    placeSizer->Add(m_placeSoundTextCtrl, 1, wxEXPAND);
    sizer->Add(placeSizer, 0, wxEXPAND | wxALL, 5);
    
    // Som de quebra
    wxBoxSizer* breakSizer = new wxBoxSizer(wxHORIZONTAL);
    breakSizer->Add(new wxStaticText(panel, wxID_ANY, UTF8("Som ao quebrar:")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    m_breakSoundTextCtrl = new wxTextCtrl(panel, ID_BREAK_SOUND_TEXT, "");
    breakSizer->Add(m_breakSoundTextCtrl, 1, wxEXPAND);
    sizer->Add(breakSizer, 0, wxEXPAND | wxALL, 5);
    
    // Informações
    wxStaticText* infoText = new wxStaticText(panel, wxID_ANY, 
        UTF8("Especifique o nome do arquivo de som (ex: 'grass_step.wav')\n"
             "Os arquivos devem estar na pasta de assets/sounds do projeto."));
    infoText->SetForegroundColour(wxColour(100, 100, 100));
    sizer->Add(infoText, 0, wxALL, 10);
    
    sizer->AddStretchSpacer();
    
    panel->SetSizer(sizer);
    notebook->AddPage(panel, UTF8("Áudio"));
}

void TilePropertiesDialog::CreateCustomPropertiesTab(wxNotebook* notebook)
{
    wxPanel* panel = new wxPanel(notebook, wxID_ANY);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    
    // Lista de propriedades customizadas
    m_customPropsListCtrl = new wxListCtrl(panel, ID_CUSTOM_PROPS_LIST, wxDefaultPosition, wxDefaultSize,
                                          wxLC_REPORT | wxLC_SINGLE_SEL);
    m_customPropsListCtrl->AppendColumn(UTF8("Nome"), wxLIST_FORMAT_LEFT, 150);
    m_customPropsListCtrl->AppendColumn(UTF8("Valor"), wxLIST_FORMAT_LEFT, 200);
    sizer->Add(m_customPropsListCtrl, 1, wxEXPAND | wxALL, 5);
    
    // Controles para adicionar nova propriedade
    wxBoxSizer* addPropSizer = new wxBoxSizer(wxHORIZONTAL);
    addPropSizer->Add(new wxStaticText(panel, wxID_ANY, UTF8("Nome:")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    m_newPropNameTextCtrl = new wxTextCtrl(panel, ID_NEW_PROP_NAME_TEXT, "");
    addPropSizer->Add(m_newPropNameTextCtrl, 1, wxEXPAND | wxRIGHT, 5);
    
    addPropSizer->Add(new wxStaticText(panel, wxID_ANY, UTF8("Valor:")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    m_newPropValueTextCtrl = new wxTextCtrl(panel, ID_NEW_PROP_VALUE_TEXT, "");
    addPropSizer->Add(m_newPropValueTextCtrl, 1, wxEXPAND | wxRIGHT, 5);
    
    m_addPropButton = new wxButton(panel, ID_ADD_PROP_BUTTON, UTF8("Adicionar"));
    addPropSizer->Add(m_addPropButton, 0, wxALL, 2);
    
    m_removePropButton = new wxButton(panel, ID_REMOVE_PROP_BUTTON, UTF8("Remover"));
    addPropSizer->Add(m_removePropButton, 0, wxALL, 2);
    
    sizer->Add(addPropSizer, 0, wxEXPAND | wxALL, 5);
    
    panel->SetSizer(sizer);
    notebook->AddPage(panel, UTF8("Customizadas"));
}

void TilePropertiesDialog::PopulateTileList()
{
    if (!m_tileListCtrl || !m_tilesetManager) return;
    
    m_tileListCtrl->DeleteAllItems();
    
    const TilesetInfo* currentTileset = m_tilesetManager->GetCurrentTileset();
    if (!currentTileset) {
        return;
    }
    
    for (int i = 0; i < currentTileset->totalTiles; ++i) {
        long itemIndex = m_tileListCtrl->InsertItem(i, wxString::Format("%d", i));
        wxString tileName = m_tilesetManager->GetTileName(i);
        m_tileListCtrl->SetItem(itemIndex, 1, tileName);
        m_tileListCtrl->SetItemData(itemIndex, i);
    }
}

void TilePropertiesDialog::LoadTileProperties(int tileId)
{
    if (!m_tilesetManager || tileId < 0) return;
    
    m_currentTileId = tileId;
    EnableControls(true);
    UpdatePreview(tileId);
    
    // Carregar propriedades básicas
    wxString tileName = m_tilesetManager->GetTileName(tileId);
    m_nameTextCtrl->SetValue(tileName);
    
    wxString category = m_tilesetManager->GetTileProperty(tileId, "category").GetString();
    if (!category.IsEmpty()) {
        int categoryIndex = m_categoryChoice->FindString(category);
        if (categoryIndex != wxNOT_FOUND) {
            m_categoryChoice->SetSelection(categoryIndex);
        }
    }
    
    // Colisão
    bool hasCollision = m_tilesetManager->GetTileProperty(tileId, "collision").GetBool();
    m_hasCollisionCheckBox->SetValue(hasCollision);
    
    // Animação
    bool isAnimated = m_tilesetManager->GetTileProperty(tileId, "animated").GetBool();
    m_isAnimatedCheckBox->SetValue(isAnimated);
    
    // Sons
    wxString stepSound = m_tilesetManager->GetTileProperty(tileId, "step_sound").GetString();
    m_stepSoundTextCtrl->SetValue(stepSound);
    
    wxString placeSound = m_tilesetManager->GetTileProperty(tileId, "place_sound").GetString();
    m_placeSoundTextCtrl->SetValue(placeSound);
    
    wxString breakSound = m_tilesetManager->GetTileProperty(tileId, "break_sound").GetString();
    m_breakSoundTextCtrl->SetValue(breakSound);
}

void TilePropertiesDialog::SaveTileProperties(int tileId)
{
    if (!m_tilesetManager || tileId < 0) return;
    
    // Salvar propriedades básicas
    m_tilesetManager->SetTileProperty(tileId, "name", wxVariant(m_nameTextCtrl->GetValue()));
    m_tilesetManager->SetTileProperty(tileId, "category", wxVariant(m_categoryChoice->GetStringSelection()));
    
    // Colisão
    m_tilesetManager->SetTileProperty(tileId, "collision", wxVariant(m_hasCollisionCheckBox->GetValue()));
    
    // Animação
    m_tilesetManager->SetTileProperty(tileId, "animated", wxVariant(m_isAnimatedCheckBox->GetValue()));
    
    // Sons
    m_tilesetManager->SetTileProperty(tileId, "step_sound", wxVariant(m_stepSoundTextCtrl->GetValue()));
    m_tilesetManager->SetTileProperty(tileId, "place_sound", wxVariant(m_placeSoundTextCtrl->GetValue()));
    m_tilesetManager->SetTileProperty(tileId, "break_sound", wxVariant(m_breakSoundTextCtrl->GetValue()));
    
    wxLogMessage("Propriedades do tile %d salvas", tileId);
}

void TilePropertiesDialog::UpdatePreview(int tileId)
{
    if (!m_tilePreview || !m_tileInfoLabel || !m_tilesetManager) return;
    
    wxImage tileImage = m_tilesetManager->GetTileImage(tileId);
    if (tileImage.IsOk()) {
        // Redimensionar para preview
        if (tileImage.GetWidth() != 64 || tileImage.GetHeight() != 64) {
            tileImage = tileImage.Scale(64, 64, wxIMAGE_QUALITY_HIGH);
        }
        m_tilePreview->SetBitmap(wxBitmap(tileImage));
    }
    
    wxString info = wxString::Format(UTF8("Tile %d\n%s"), tileId, m_tilesetManager->GetTileName(tileId));
    m_tileInfoLabel->SetLabel(info);
}

void TilePropertiesDialog::ClearControls()
{
    if (m_nameTextCtrl) m_nameTextCtrl->Clear();
    if (m_categoryChoice) m_categoryChoice->SetSelection(0);
    if (m_descriptionTextCtrl) m_descriptionTextCtrl->Clear();
    if (m_hasCollisionCheckBox) m_hasCollisionCheckBox->SetValue(false);
    if (m_isAnimatedCheckBox) m_isAnimatedCheckBox->SetValue(false);
    if (m_stepSoundTextCtrl) m_stepSoundTextCtrl->Clear();
    if (m_placeSoundTextCtrl) m_placeSoundTextCtrl->Clear();
    if (m_breakSoundTextCtrl) m_breakSoundTextCtrl->Clear();
}

void TilePropertiesDialog::EnableControls(bool enabled)
{
    if (m_nameTextCtrl) m_nameTextCtrl->Enable(enabled);
    if (m_categoryChoice) m_categoryChoice->Enable(enabled);
    if (m_descriptionTextCtrl) m_descriptionTextCtrl->Enable(enabled);
    if (m_hasCollisionCheckBox) m_hasCollisionCheckBox->Enable(enabled);
    if (m_collisionTypeChoice) m_collisionTypeChoice->Enable(enabled);
    if (m_collisionDataTextCtrl) m_collisionDataTextCtrl->Enable(enabled);
    if (m_isAnimatedCheckBox) m_isAnimatedCheckBox->Enable(enabled);
    if (m_animationFramesTextCtrl) m_animationFramesTextCtrl->Enable(enabled);
    if (m_animationSpeedTextCtrl) m_animationSpeedTextCtrl->Enable(enabled);
    if (m_stepSoundTextCtrl) m_stepSoundTextCtrl->Enable(enabled);
    if (m_placeSoundTextCtrl) m_placeSoundTextCtrl->Enable(enabled);
    if (m_breakSoundTextCtrl) m_breakSoundTextCtrl->Enable(enabled);
    if (m_applyButton) m_applyButton->Enable(enabled);
}

// Event Handlers

void TilePropertiesDialog::OnTileSelectionChanged(wxListEvent& event)
{
    long itemIndex = event.GetIndex();
    if (itemIndex >= 0) {
        int tileId = m_tileListCtrl->GetItemData(itemIndex);
        LoadTileProperties(tileId);
    }
}

void TilePropertiesDialog::OnPropertyChanged(wxCommandEvent& WXUNUSED(event))
{
    m_hasChanges = true;
}

void TilePropertiesDialog::OnAddCustomProperty(wxCommandEvent& WXUNUSED(event))
{
    wxString name = m_newPropNameTextCtrl->GetValue().Trim();
    wxString value = m_newPropValueTextCtrl->GetValue().Trim();
    
    if (name.IsEmpty()) {
        wxMessageBox(UTF8("Por favor, digite o nome da propriedade."), UTF8("Aviso"), wxOK | wxICON_WARNING);
        return;
    }
    
    // Adicionar à lista
    long itemIndex = m_customPropsListCtrl->InsertItem(m_customPropsListCtrl->GetItemCount(), name);
    m_customPropsListCtrl->SetItem(itemIndex, 1, value);
    
    // Limpar campos
    m_newPropNameTextCtrl->Clear();
    m_newPropValueTextCtrl->Clear();
    
    m_hasChanges = true;
}

void TilePropertiesDialog::OnRemoveCustomProperty(wxCommandEvent& WXUNUSED(event))
{
    long selected = m_customPropsListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (selected >= 0) {
        m_customPropsListCtrl->DeleteItem(selected);
        m_hasChanges = true;
    }
}

void TilePropertiesDialog::OnOK(wxCommandEvent& WXUNUSED(event))
{
    if (m_hasChanges && m_currentTileId >= 0) {
        SaveTileProperties(m_currentTileId);
    }
    EndModal(wxID_OK);
}

void TilePropertiesDialog::OnCancel(wxCommandEvent& WXUNUSED(event))
{
    EndModal(wxID_CANCEL);
}

void TilePropertiesDialog::OnApply(wxCommandEvent& WXUNUSED(event))
{
    if (m_currentTileId >= 0) {
        SaveTileProperties(m_currentTileId);
        m_hasChanges = false;
    }
}