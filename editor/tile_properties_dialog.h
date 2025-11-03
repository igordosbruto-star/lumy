/**
 * TilePropertiesDialog - Diálogo para configurar propriedades dos tiles
 */

#pragma once

#include <wx/wx.h>
#include <wx/dialog.h>
#include <wx/checkbox.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/listctrl.h>
#include <wx/notebook.h>
#include "tileset_manager.h"

class TilePropertiesDialog : public wxDialog
{
public:
    TilePropertiesDialog(wxWindow* parent, TilesetManager* tilesetManager, int selectedTileId = -1);
    virtual ~TilePropertiesDialog() = default;
    
    // Interface pública
    bool HasChanges() const { return m_hasChanges; }
    
private:
    // Criação da interface
    void CreateControls();
    void CreateBasicPropertiesTab(wxNotebook* notebook);
    void CreateCollisionTab(wxNotebook* notebook);
    void CreateAnimationTab(wxNotebook* notebook);
    void CreateAudioTab(wxNotebook* notebook);
    void CreateCustomPropertiesTab(wxNotebook* notebook);
    
    // Event handlers
    void OnTileSelectionChanged(wxListEvent& event);
    void OnPropertyChanged(wxCommandEvent& event);
    void OnAddCustomProperty(wxCommandEvent& event);
    void OnRemoveCustomProperty(wxCommandEvent& event);
    void OnOK(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnApply(wxCommandEvent& event);
    
    // Métodos auxiliares
    void PopulateTileList();
    void LoadTileProperties(int tileId);
    void SaveTileProperties(int tileId);
    void UpdatePreview(int tileId);
    void ClearControls();
    void EnableControls(bool enabled);
    
    // Dados
    TilesetManager* m_tilesetManager;
    int m_currentTileId;
    bool m_hasChanges;
    
    // Controles - Lista de tiles
    wxListCtrl* m_tileListCtrl;
    wxStaticBitmap* m_tilePreview;
    wxStaticText* m_tileInfoLabel;
    
    // Controles - Propriedades básicas
    wxTextCtrl* m_nameTextCtrl;
    wxChoice* m_categoryChoice;
    wxTextCtrl* m_descriptionTextCtrl;
    
    // Controles - Colisão
    wxCheckBox* m_hasCollisionCheckBox;
    wxChoice* m_collisionTypeChoice;
    wxTextCtrl* m_collisionDataTextCtrl;
    
    // Controles - Animação
    wxCheckBox* m_isAnimatedCheckBox;
    wxTextCtrl* m_animationFramesTextCtrl;
    wxTextCtrl* m_animationSpeedTextCtrl;
    
    // Controles - Áudio
    wxTextCtrl* m_stepSoundTextCtrl;
    wxTextCtrl* m_placeSoundTextCtrl;
    wxTextCtrl* m_breakSoundTextCtrl;
    
    // Controles - Propriedades customizadas
    wxListCtrl* m_customPropsListCtrl;
    wxTextCtrl* m_newPropNameTextCtrl;
    wxTextCtrl* m_newPropValueTextCtrl;
    wxButton* m_addPropButton;
    wxButton* m_removePropButton;
    
    // Botões principais
    wxButton* m_okButton;
    wxButton* m_cancelButton;
    wxButton* m_applyButton;
    
    // IDs para eventos
    enum {
        ID_TILE_LIST = 3000,
        ID_NAME_TEXT,
        ID_CATEGORY_CHOICE,
        ID_DESCRIPTION_TEXT,
        ID_COLLISION_CHECK,
        ID_COLLISION_TYPE_CHOICE,
        ID_COLLISION_DATA_TEXT,
        ID_ANIMATED_CHECK,
        ID_ANIMATION_FRAMES_TEXT,
        ID_ANIMATION_SPEED_TEXT,
        ID_STEP_SOUND_TEXT,
        ID_PLACE_SOUND_TEXT,
        ID_BREAK_SOUND_TEXT,
        ID_CUSTOM_PROPS_LIST,
        ID_NEW_PROP_NAME_TEXT,
        ID_NEW_PROP_VALUE_TEXT,
        ID_ADD_PROP_BUTTON,
        ID_REMOVE_PROP_BUTTON,
        ID_APPLY_BUTTON
    };
    
    wxDECLARE_EVENT_TABLE();
};