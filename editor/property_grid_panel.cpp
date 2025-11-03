/**
 * Property Grid Panel - Implementação da grade de propriedades dinâmica
 */

#pragma execution_character_set("utf-8")

#include "property_grid_panel.h"
#include "map.h"
#include "layer.h"
#include "utf8_strings.h"
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

// ============================================================================
// Property IDs (for identification)
// ============================================================================

namespace PropertyID {
    // Map properties
    const wxString MAP_NAME = "map.name";
    const wxString MAP_AUTHOR = "map.author";
    const wxString MAP_DESCRIPTION = "map.description";
    const wxString MAP_VERSION = "map.version";
    const wxString MAP_WIDTH = "map.width";
    const wxString MAP_HEIGHT = "map.height";
    const wxString MAP_TILE_SIZE = "map.tileSize";
    const wxString MAP_CREATED_DATE = "map.createdDate";
    const wxString MAP_MODIFIED_DATE = "map.modifiedDate";
    
    // Layer properties
    const wxString LAYER_NAME = "layer.name";
    const wxString LAYER_TYPE = "layer.type";
    const wxString LAYER_VISIBLE = "layer.visible";
    const wxString LAYER_OPACITY = "layer.opacity";
    const wxString LAYER_LOCKED = "layer.locked";
    const wxString LAYER_Z_ORDER = "layer.zOrder";
    const wxString LAYER_TINT_COLOR = "layer.tintColor";
    const wxString LAYER_DESCRIPTION = "layer.description";
    const wxString LAYER_WIDTH = "layer.width";
    const wxString LAYER_HEIGHT = "layer.height";
    
    // Tile properties
    const wxString TILE_X = "tile.x";
    const wxString TILE_Y = "tile.y";
    const wxString TILE_ID = "tile.id";
    const wxString TILE_COLLISION = "tile.collision";
}

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
    // Create property grid with splitter
    m_propertyGrid = new wxPropertyGrid(this, wxID_ANY,
        wxDefaultPosition, wxDefaultSize,
        wxPG_SPLITTER_AUTO_CENTER | wxPG_BOLD_MODIFIED | wxPG_DESCRIPTION);
    
    // Set default column proportions
    m_propertyGrid->SetColumnProportion(0, 40);
    m_propertyGrid->SetColumnProportion(1, 60);
    
    // Layout
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(m_propertyGrid, 1, wxEXPAND | wxALL, 0);
    SetSizer(sizer);
}

void PropertyGridPanel::PopulateDefaultProperties()
{
    m_propertyGrid->Clear();
    
    // Add a message indicating no selection
    m_propertyGrid->Append(new wxStringProperty(UTF8("Informação"), wxPG_LABEL,
        UTF8("Nenhum objeto selecionado")));
    m_propertyGrid->DisableProperty(UTF8("Informação"));
}

void PropertyGridPanel::PopulateMapProperties()
{
    m_propertyGrid->Clear();
    
    // Add Map category
    m_propertyGrid->Append(new wxPropertyCategory(UTF8("Propriedades do Mapa")));
    
    // Metadata
    auto* nameProp = m_propertyGrid->Append(new wxStringProperty(UTF8("Nome"), PropertyID::MAP_NAME));
    nameProp->SetHelpString(UTF8("Nome do mapa"));
    
    auto* authorProp = m_propertyGrid->Append(new wxStringProperty(UTF8("Autor"), PropertyID::MAP_AUTHOR));
    authorProp->SetHelpString(UTF8("Nome do autor do mapa"));
    
    auto* descProp = m_propertyGrid->Append(new wxLongStringProperty(UTF8("Descrição"), PropertyID::MAP_DESCRIPTION));
    descProp->SetHelpString(UTF8("Descrição do mapa"));
    
    auto* versionProp = m_propertyGrid->Append(new wxStringProperty(UTF8("Versão"), PropertyID::MAP_VERSION));
    versionProp->SetHelpString(UTF8("Versão do mapa"));
    
    // Dimensions category
    m_propertyGrid->Append(new wxPropertyCategory(UTF8("Dimensões")));
    
    auto* widthProp = m_propertyGrid->Append(new wxIntProperty(UTF8("Largura"), PropertyID::MAP_WIDTH));
    widthProp->SetHelpString(UTF8("Largura do mapa em tiles"));
    widthProp->SetAttribute(wxPG_ATTR_MIN, 1);
    widthProp->SetAttribute(wxPG_ATTR_MAX, 1000);
    
    auto* heightProp = m_propertyGrid->Append(new wxIntProperty(UTF8("Altura"), PropertyID::MAP_HEIGHT));
    heightProp->SetHelpString(UTF8("Altura do mapa em tiles"));
    heightProp->SetAttribute(wxPG_ATTR_MIN, 1);
    heightProp->SetAttribute(wxPG_ATTR_MAX, 1000);
    
    auto* tileSizeProp = m_propertyGrid->Append(new wxIntProperty(UTF8("Tamanho do Tile"), PropertyID::MAP_TILE_SIZE));
    tileSizeProp->SetHelpString(UTF8("Tamanho de cada tile em pixels"));
    tileSizeProp->SetAttribute(wxPG_ATTR_MIN, 8);
    tileSizeProp->SetAttribute(wxPG_ATTR_MAX, 128);
    
    // Dates category (read-only)
    m_propertyGrid->Append(new wxPropertyCategory(UTF8("Informações")));
    
    auto* createdProp = m_propertyGrid->Append(new wxStringProperty(UTF8("Data de Criação"), PropertyID::MAP_CREATED_DATE));
    createdProp->SetHelpString(UTF8("Data e hora de criação do mapa"));
    m_propertyGrid->DisableProperty(PropertyID::MAP_CREATED_DATE);
    
    auto* modifiedProp = m_propertyGrid->Append(new wxStringProperty(UTF8("Última Modificação"), PropertyID::MAP_MODIFIED_DATE));
    modifiedProp->SetHelpString(UTF8("Data e hora da última modificação"));
    m_propertyGrid->DisableProperty(PropertyID::MAP_MODIFIED_DATE);
}

void PropertyGridPanel::PopulateEventProperties()
{
    m_propertyGrid->Clear();
    
    // Add Layer category
    m_propertyGrid->Append(new wxPropertyCategory(UTF8("Propriedades da Camada")));
    
    // Basic properties
    auto* nameProp = m_propertyGrid->Append(new wxStringProperty(UTF8("Nome"), PropertyID::LAYER_NAME));
    nameProp->SetHelpString(UTF8("Nome da camada"));
    
    // Layer type enum
    wxPGChoices typeChoices;
    typeChoices.Add(UTF8("Tiles"), static_cast<int>(LayerType::TILE_LAYER));
    typeChoices.Add(UTF8("Fundo"), static_cast<int>(LayerType::BACKGROUND_LAYER));
    typeChoices.Add(UTF8("Colisão"), static_cast<int>(LayerType::COLLISION_LAYER));
    typeChoices.Add(UTF8("Objetos"), static_cast<int>(LayerType::OBJECT_LAYER));
    typeChoices.Add(UTF8("Sobreposição"), static_cast<int>(LayerType::OVERLAY_LAYER));
    
    auto* typeProp = m_propertyGrid->Append(new wxEnumProperty(UTF8("Tipo"), PropertyID::LAYER_TYPE, typeChoices));
    typeProp->SetHelpString(UTF8("Tipo da camada"));
    
    // Visibility and appearance
    m_propertyGrid->Append(new wxPropertyCategory(UTF8("Aparência")));
    
    auto* visibleProp = m_propertyGrid->Append(new wxBoolProperty(UTF8("Visível"), PropertyID::LAYER_VISIBLE));
    visibleProp->SetHelpString(UTF8("Se a camada está visível"));
    
    auto* opacityProp = m_propertyGrid->Append(new wxFloatProperty(UTF8("Opacidade"), PropertyID::LAYER_OPACITY));
    opacityProp->SetHelpString(UTF8("Opacidade da camada (0.0 a 1.0)"));
    opacityProp->SetAttribute(wxPG_ATTR_MIN, 0.0);
    opacityProp->SetAttribute(wxPG_ATTR_MAX, 1.0);
    
    auto* tintProp = m_propertyGrid->Append(new wxColourProperty(UTF8("Cor de Tinta"), PropertyID::LAYER_TINT_COLOR));
    tintProp->SetHelpString(UTF8("Cor de tinta aplicada à camada"));
    
    // Behavior
    m_propertyGrid->Append(new wxPropertyCategory(UTF8("Comportamento")));
    
    auto* lockedProp = m_propertyGrid->Append(new wxBoolProperty(UTF8("Bloqueada"), PropertyID::LAYER_LOCKED));
    lockedProp->SetHelpString(UTF8("Se a camada está bloqueada para edição"));
    
    auto* zOrderProp = m_propertyGrid->Append(new wxIntProperty(UTF8("Ordem Z"), PropertyID::LAYER_Z_ORDER));
    zOrderProp->SetHelpString(UTF8("Ordem de renderização (menor = mais atrás)"));
    
    auto* descProp = m_propertyGrid->Append(new wxLongStringProperty(UTF8("Descrição"), PropertyID::LAYER_DESCRIPTION));
    descProp->SetHelpString(UTF8("Descrição da camada"));
    
    // Dimensions (read-only)
    m_propertyGrid->Append(new wxPropertyCategory(UTF8("Dimensões")));
    
    auto* widthProp = m_propertyGrid->Append(new wxIntProperty(UTF8("Largura"), PropertyID::LAYER_WIDTH));
    widthProp->SetHelpString(UTF8("Largura da camada em tiles"));
    m_propertyGrid->DisableProperty(PropertyID::LAYER_WIDTH);
    
    auto* heightProp = m_propertyGrid->Append(new wxIntProperty(UTF8("Altura"), PropertyID::LAYER_HEIGHT));
    heightProp->SetHelpString(UTF8("Altura da camada em tiles"));
    m_propertyGrid->DisableProperty(PropertyID::LAYER_HEIGHT);
}

// ============================================================================
// Public Methods to Load Object Properties
// ============================================================================

void PropertyGridPanel::LoadMapProperties(Map* map)
{
    if (!map) {
        PopulateDefaultProperties();
        return;
    }
    
    PopulateMapProperties();
    
    const MapMetadata& metadata = map->GetMetadata();
    
    // Set values
    m_propertyGrid->SetPropertyValue(PropertyID::MAP_NAME, metadata.name);
    m_propertyGrid->SetPropertyValue(PropertyID::MAP_AUTHOR, metadata.author);
    m_propertyGrid->SetPropertyValue(PropertyID::MAP_DESCRIPTION, metadata.description);
    m_propertyGrid->SetPropertyValue(PropertyID::MAP_VERSION, metadata.version);
    m_propertyGrid->SetPropertyValue(PropertyID::MAP_WIDTH, metadata.width);
    m_propertyGrid->SetPropertyValue(PropertyID::MAP_HEIGHT, metadata.height);
    m_propertyGrid->SetPropertyValue(PropertyID::MAP_TILE_SIZE, metadata.tileSize);
    m_propertyGrid->SetPropertyValue(PropertyID::MAP_CREATED_DATE, metadata.createdDate);
    m_propertyGrid->SetPropertyValue(PropertyID::MAP_MODIFIED_DATE, metadata.modifiedDate);
    
    // Store reference for updates
    m_currentMap = map;
    m_currentLayer = nullptr;
    m_hasTileSelection = false;
}

void PropertyGridPanel::LoadLayerProperties(Layer* layer)
{
    if (!layer) {
        PopulateDefaultProperties();
        return;
    }
    
    PopulateEventProperties();
    
    const LayerProperties& props = layer->GetProperties();
    
    // Set values
    m_propertyGrid->SetPropertyValue(PropertyID::LAYER_NAME, props.name);
    m_propertyGrid->SetPropertyValue(PropertyID::LAYER_TYPE, static_cast<int>(props.type));
    m_propertyGrid->SetPropertyValue(PropertyID::LAYER_VISIBLE, props.visible);
    m_propertyGrid->SetPropertyValue(PropertyID::LAYER_OPACITY, props.opacity);
    m_propertyGrid->SetPropertyValue(PropertyID::LAYER_LOCKED, props.locked);
    m_propertyGrid->SetPropertyValue(PropertyID::LAYER_Z_ORDER, props.zOrder);
    
    wxVariant colorVariant;
    colorVariant << props.tintColor;
    m_propertyGrid->SetPropertyValue(PropertyID::LAYER_TINT_COLOR, colorVariant);
    
    m_propertyGrid->SetPropertyValue(PropertyID::LAYER_DESCRIPTION, props.description);
    m_propertyGrid->SetPropertyValue(PropertyID::LAYER_WIDTH, layer->GetWidth());
    m_propertyGrid->SetPropertyValue(PropertyID::LAYER_HEIGHT, layer->GetHeight());
    
    // Store reference for updates
    m_currentLayer = layer;
    m_currentMap = nullptr;
    m_hasTileSelection = false;
}

void PropertyGridPanel::LoadTileProperties(int x, int y, int tileId, bool hasCollision)
{
    m_propertyGrid->Clear();
    
    // Add Tile category
    m_propertyGrid->Append(new wxPropertyCategory(UTF8("Propriedades do Tile")));
    
    // Position (read-only)
    auto* xProp = m_propertyGrid->Append(new wxIntProperty(UTF8("Posição X"), PropertyID::TILE_X, x));
    xProp->SetHelpString(UTF8("Posição X do tile no mapa"));
    m_propertyGrid->DisableProperty(PropertyID::TILE_X);
    
    auto* yProp = m_propertyGrid->Append(new wxIntProperty(UTF8("Posição Y"), PropertyID::TILE_Y, y));
    yProp->SetHelpString(UTF8("Posição Y do tile no mapa"));
    m_propertyGrid->DisableProperty(PropertyID::TILE_Y);
    
    // Tile ID
    auto* idProp = m_propertyGrid->Append(new wxIntProperty(UTF8("ID do Tile"), PropertyID::TILE_ID, tileId));
    idProp->SetHelpString(UTF8("ID do tile do tileset"));
    idProp->SetAttribute(wxPG_ATTR_MIN, -1);
    
    // Collision
    auto* collisionProp = m_propertyGrid->Append(new wxBoolProperty(UTF8("Colisão"), PropertyID::TILE_COLLISION, hasCollision));
    collisionProp->SetHelpString(UTF8("Se o tile possui colisão"));
    
    // Store tile info
    m_hasTileSelection = true;
    m_selectedTileX = x;
    m_selectedTileY = y;
    m_currentMap = nullptr;
    m_currentLayer = nullptr;
}

void PropertyGridPanel::ClearProperties()
{
    PopulateDefaultProperties();
    m_currentMap = nullptr;
    m_currentLayer = nullptr;
    m_hasTileSelection = false;
}

// ============================================================================
// Event Handlers
// ============================================================================

void PropertyGridPanel::OnPropertyChanged(wxPropertyGridEvent& event)
{
    wxPGProperty* property = event.GetProperty();
    if (!property) {
        return;
    }
    
    wxString propertyName = property->GetName();
    
    // Handle Map property changes
    if (m_currentMap) {
        MapMetadata metadata = m_currentMap->GetMetadata();
        bool changed = false;
        
        if (propertyName == PropertyID::MAP_NAME) {
            metadata.name = property->GetValueAsString();
            changed = true;
        }
        else if (propertyName == PropertyID::MAP_AUTHOR) {
            metadata.author = property->GetValueAsString();
            changed = true;
        }
        else if (propertyName == PropertyID::MAP_DESCRIPTION) {
            metadata.description = property->GetValueAsString();
            changed = true;
        }
        else if (propertyName == PropertyID::MAP_VERSION) {
            metadata.version = property->GetValueAsString();
            changed = true;
        }
        else if (propertyName == PropertyID::MAP_WIDTH) {
            int newWidth = property->GetValue().GetInteger();
            m_currentMap->Resize(newWidth, metadata.height);
            return; // Resize handles modified flag
        }
        else if (propertyName == PropertyID::MAP_HEIGHT) {
            int newHeight = property->GetValue().GetInteger();
            m_currentMap->Resize(metadata.width, newHeight);
            return; // Resize handles modified flag
        }
        else if (propertyName == PropertyID::MAP_TILE_SIZE) {
            metadata.tileSize = property->GetValue().GetInteger();
            changed = true;
        }
        
        if (changed) {
            m_currentMap->SetMetadata(metadata);
            m_currentMap->SetModified(true);
        }
    }
    
    // Handle Layer property changes
    if (m_currentLayer) {
        LayerProperties props = m_currentLayer->GetProperties();
        bool changed = false;
        
        if (propertyName == PropertyID::LAYER_NAME) {
            props.name = property->GetValueAsString();
            changed = true;
        }
        else if (propertyName == PropertyID::LAYER_TYPE) {
            props.type = static_cast<LayerType>(property->GetValue().GetInteger());
            changed = true;
        }
        else if (propertyName == PropertyID::LAYER_VISIBLE) {
            props.visible = property->GetValue().GetBool();
            changed = true;
        }
        else if (propertyName == PropertyID::LAYER_OPACITY) {
            props.opacity = property->GetValue().GetDouble();
            changed = true;
        }
        else if (propertyName == PropertyID::LAYER_LOCKED) {
            props.locked = property->GetValue().GetBool();
            changed = true;
        }
        else if (propertyName == PropertyID::LAYER_Z_ORDER) {
            props.zOrder = property->GetValue().GetInteger();
            changed = true;
        }
        else if (propertyName == PropertyID::LAYER_TINT_COLOR) {
            wxVariant variant = property->GetValue();
            wxColour color;
            color << variant;
            props.tintColor = color;
            changed = true;
        }
        else if (propertyName == PropertyID::LAYER_DESCRIPTION) {
            props.description = property->GetValueAsString();
            changed = true;
        }
        
        if (changed) {
            m_currentLayer->SetProperties(props);
            m_currentLayer->SetModified(true);
        }
    }
    
    // Handle Tile property changes
    if (m_hasTileSelection) {
        // TODO: Implement tile property changes when we have a tile collision system
        if (propertyName == PropertyID::TILE_ID) {
            int newTileId = property->GetValue().GetInteger();
            wxLogMessage("TODO: Alterar tile em (%d, %d) para ID %d", 
                        m_selectedTileX, m_selectedTileY, newTileId);
        }
        else if (propertyName == PropertyID::TILE_COLLISION) {
            bool hasCollision = property->GetValue().GetBool();
            wxLogMessage("TODO: Alterar colisão do tile em (%d, %d) para %s",
                        m_selectedTileX, m_selectedTileY, 
                        hasCollision ? "true" : "false");
        }
    }
}

void PropertyGridPanel::OnPropertySelected(wxPropertyGridEvent& event)
{
    wxPGProperty* property = event.GetProperty();
    if (!property) {
        return;
    }
    
    // Show property help string in status bar or log
    wxString helpString = property->GetHelpString();
    if (!helpString.IsEmpty()) {
        wxLogStatus(helpString);
    }
}
