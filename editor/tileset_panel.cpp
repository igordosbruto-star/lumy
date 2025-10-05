/**
 * Implementação do Tileset Panel
 */

#pragma execution_character_set("utf-8")

#include "tileset_panel.h"
#include "utf8_strings.h"
#include <wx/filename.h>

// Definir evento customizado
wxDEFINE_EVENT(TILESET_SELECTION_CHANGED, wxCommandEvent);

// Event table principal
wxBEGIN_EVENT_TABLE(TilesetPanel, wxPanel)
    EVT_BUTTON(TilesetPanel::ID_LOAD_TILESET, TilesetPanel::OnLoadTileset)
    EVT_CHOICE(TilesetPanel::ID_TILESET_CHOICE, TilesetPanel::OnTilesetChanged)
    EVT_BUTTON(TilesetPanel::ID_TILESET_PROPERTIES, TilesetPanel::OnTilesetProperties)
    EVT_TEXT(TilesetPanel::ID_SEARCH_TEXT, TilesetPanel::OnSearchChanged)
    EVT_CHOICE(TilesetPanel::ID_CATEGORY_FILTER, TilesetPanel::OnCategoryFilterChanged)
wxEND_EVENT_TABLE()

// Event table para TileGrid
wxBEGIN_EVENT_TABLE(TilesetPanel::TileGrid, wxScrolledWindow)
    EVT_PAINT(TilesetPanel::TileGrid::OnPaint)
    EVT_LEFT_DOWN(TilesetPanel::TileGrid::OnLeftDown)
    EVT_SIZE(TilesetPanel::TileGrid::OnSize)
wxEND_EVENT_TABLE()

TilesetPanel::TilesetPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY)
    , m_tilesetChoice(nullptr)
    , m_loadButton(nullptr)
    , m_propertiesButton(nullptr)
    , m_searchTextCtrl(nullptr)
    , m_categoryFilterChoice(nullptr)
    , m_tileGrid(nullptr)
    , m_infoLabel(nullptr)
    , m_tilesetManager(std::make_unique<TilesetManager>())
    , m_selectedTile(1) // Padrão: wall tile
{
    CreateControls();
}

void TilesetPanel::CreateControls()
{
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    
    // Título
    wxStaticText* title = new wxStaticText(this, wxID_ANY, UTF8("Paleta de Tiles"));
    wxFont titleFont = title->GetFont();
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    title->SetFont(titleFont);
    sizer->Add(title, 0, wxALL, 5);
    
    // Seletor de tileset
    wxBoxSizer* tilesetSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* tilesetLabel = new wxStaticText(this, wxID_ANY, UTF8("Tileset:"));
    m_tilesetChoice = new wxChoice(this, ID_TILESET_CHOICE);
    tilesetSizer->Add(tilesetLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    tilesetSizer->Add(m_tilesetChoice, 1, wxEXPAND);
    sizer->Add(tilesetSizer, 0, wxEXPAND | wxALL, 5);
    
    // Botões de ação
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    m_loadButton = new wxButton(this, ID_LOAD_TILESET, UTF8("Carregar..."));
    m_propertiesButton = new wxButton(this, ID_TILESET_PROPERTIES, UTF8("Propriedades"));
    buttonSizer->Add(m_loadButton, 1, wxEXPAND | wxRIGHT, 2);
    buttonSizer->Add(m_propertiesButton, 1, wxEXPAND | wxLEFT, 2);
    sizer->Add(buttonSizer, 0, wxEXPAND | wxALL, 5);
    
    // Controles de busca e filtro
    wxBoxSizer* searchSizer = new wxBoxSizer(wxVERTICAL);
    
    // Campo de busca
    wxBoxSizer* searchTextSizer = new wxBoxSizer(wxHORIZONTAL);
    searchTextSizer->Add(new wxStaticText(this, wxID_ANY, UTF8("Buscar:")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    m_searchTextCtrl = new wxTextCtrl(this, ID_SEARCH_TEXT, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    m_searchTextCtrl->SetHint(UTF8("Nome do tile..."));
    searchTextSizer->Add(m_searchTextCtrl, 1, wxEXPAND);
    searchSizer->Add(searchTextSizer, 0, wxEXPAND | wxBOTTOM, 3);
    
    // Filtro de categoria
    wxBoxSizer* filterSizer = new wxBoxSizer(wxHORIZONTAL);
    filterSizer->Add(new wxStaticText(this, wxID_ANY, UTF8("Categoria:")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    m_categoryFilterChoice = new wxChoice(this, ID_CATEGORY_FILTER);
    m_categoryFilterChoice->Append(UTF8("Todas"));
    m_categoryFilterChoice->Append(UTF8("Terreno"));
    m_categoryFilterChoice->Append(UTF8("Estrutura"));
    m_categoryFilterChoice->Append(UTF8("Decoração"));
    m_categoryFilterChoice->Append(UTF8("Especial"));
    m_categoryFilterChoice->SetSelection(0);
    filterSizer->Add(m_categoryFilterChoice, 1, wxEXPAND);
    searchSizer->Add(filterSizer, 0, wxEXPAND);
    
    sizer->Add(searchSizer, 0, wxEXPAND | wxALL, 5);
    
    // Grid de tiles
    m_tileGrid = new TileGrid(this, this);
    sizer->Add(m_tileGrid, 1, wxEXPAND | wxALL, 5);
    
    // Label de informações
    m_infoLabel = new wxStaticText(this, wxID_ANY, UTF8("Tile selecionado: Wall (1)"));
    sizer->Add(m_infoLabel, 0, wxEXPAND | wxALL, 5);
    
    SetSizer(sizer);
    
    // Inicializar lista de tilesets
    RefreshTilesetList();
}

void TilesetPanel::SetSelectedTile(int tileId)
{
    m_selectedTile = tileId;
    m_tileGrid->SetSelectedTile(tileId);
    
    // Atualizar label de informação usando TilesetManager
    if (m_tilesetManager) {
        wxString tileName = m_tilesetManager->GetTileName(tileId);
        wxString info = wxString::Format(UTF8("Tile selecionado: %s (%d)"), 
                                       tileName, tileId);
        m_infoLabel->SetLabel(info);
    }
}

// Implementação do TileGrid

TilesetPanel::TileGrid::TileGrid(wxWindow* parent, TilesetPanel* tilesetPanel)
    : wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER)
    , m_parent(tilesetPanel)
    , m_selectedTile(1)
    , m_tileSize(48) // Tiles de 48x48 pixels para boa visualização
    , m_tilesPerRow(2) // 2 tiles por linha para caber bem no painel lateral
{
    SetBackgroundColour(wxColour(240, 240, 240));
    SetScrollRate(5, 5);
    UpdateVirtualSize();
}

void TilesetPanel::TileGrid::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(this);
    DoPrepareDC(dc);
    
    // Limpar fundo
    dc.SetBackground(wxBrush(GetBackgroundColour()));
    dc.Clear();
    
    // Usar lista filtrada se disponível, senão todos os tiles
    const std::vector<int>& tilesToDraw = m_filteredTiles.empty() ? 
        std::vector<int>() : m_filteredTiles;
    
    TilesetManager* tilesetManager = m_parent->GetTilesetManager();
    int tileCount = tilesetManager ? tilesetManager->GetTileCount() : 0;
    
    // Se não há filtro, desenhar todos os tiles
    if (m_filteredTiles.empty()) {
        for (int i = 0; i < tileCount; ++i) {
            wxPoint gridPos = TileIdToGridPos(i);
            int x = gridPos.x * (m_tileSize + 4) + 5;
            int y = gridPos.y * (m_tileSize + 4) + 5;
            
            bool selected = (i == m_selectedTile);
            DrawTile(dc, i, x, y, selected);
        }
    } else {
        // Desenhar apenas tiles filtrados
        for (size_t i = 0; i < m_filteredTiles.size(); ++i) {
            int tileId = m_filteredTiles[i];
            wxPoint gridPos = TileIdToGridPos(static_cast<int>(i));
            int x = gridPos.x * (m_tileSize + 4) + 5;
            int y = gridPos.y * (m_tileSize + 4) + 5;
            
            bool selected = (tileId == m_selectedTile);
            DrawTile(dc, tileId, x, y, selected);
        }
    }
}

void TilesetPanel::TileGrid::OnLeftDown(wxMouseEvent& event)
{
    wxPoint pos = event.GetPosition();
    CalcUnscrolledPosition(pos.x, pos.y, &pos.x, &pos.y);
    
    // Converter posição do mouse para tile ID
    int gridX = (pos.x - 5) / (m_tileSize + 4);
    int gridY = (pos.y - 5) / (m_tileSize + 4);
    
    if (gridX >= 0 && gridX < m_tilesPerRow) {
        int gridIndex = gridY * m_tilesPerRow + gridX;
        int tileId = -1;
        
        // Determinar ID real do tile baseado no filtro
        if (m_filteredTiles.empty()) {
            // Sem filtro - usar índice direto
            tileId = gridIndex;
        } else {
            // Com filtro - obter da lista filtrada
            if (gridIndex >= 0 && gridIndex < (int)m_filteredTiles.size()) {
                tileId = m_filteredTiles[gridIndex];
            }
        }
        
        // Verificar se o tile é válido
        TilesetManager* tilesetManager = m_parent->GetTilesetManager();
        int tileCount = tilesetManager ? tilesetManager->GetTileCount() : 0;
        
        if (tileId >= 0 && tileId < tileCount) {
            SetSelectedTile(tileId);
            m_parent->SetSelectedTile(tileId);
            
            // Enviar evento de mudança de seleção
            wxCommandEvent evt(TILESET_SELECTION_CHANGED, GetId());
            evt.SetEventObject(this);
            evt.SetInt(tileId);
            GetParent()->GetEventHandler()->ProcessEvent(evt);
            
            Refresh();
        }
    }
}

void TilesetPanel::TileGrid::OnSize(wxSizeEvent& event)
{
    Refresh();
    event.Skip();
}

void TilesetPanel::TileGrid::SetSelectedTile(int tileId)
{
    if (tileId != m_selectedTile) {
        m_selectedTile = tileId;
        Refresh();
    }
}

void TilesetPanel::TileGrid::DrawTile(wxDC& dc, int tileId, int x, int y, bool selected)
{
    // Obter imagem do tile do TilesetManager
    TilesetManager* tilesetManager = m_parent->GetTilesetManager();
    wxImage tileImage;
    
    if (tilesetManager) {
        tileImage = tilesetManager->GetTileImage(tileId);
    }
    
    // Desenhar borda primeiro
    if (selected) {
        dc.SetPen(wxPen(wxColour(255, 215, 0), 3)); // Dourado, 3px de espessura
        dc.SetBrush(wxBrush(wxColour(255, 215, 0), wxBRUSHSTYLE_TRANSPARENT));
    } else {
        dc.SetPen(wxPen(wxColour(0, 0, 0), 1)); // Preto, 1px
        dc.SetBrush(wxBrush(wxColour(0, 0, 0), wxBRUSHSTYLE_TRANSPARENT));
    }
    dc.DrawRectangle(x - 2, y - 2, m_tileSize + 4, m_tileSize + 4);
    
    // Desenhar imagem do tile
    if (tileImage.IsOk()) {
        // Redimensionar se necessário
        if (tileImage.GetWidth() != m_tileSize || tileImage.GetHeight() != m_tileSize) {
            tileImage = tileImage.Scale(m_tileSize, m_tileSize, wxIMAGE_QUALITY_HIGH);
        }
        
        wxBitmap tileBitmap(tileImage);
        dc.DrawBitmap(tileBitmap, x, y, false);
    } else {
        // Fallback: desenhar retângulo colorido
        dc.SetBrush(wxBrush(wxColour(200, 200, 200)));
        dc.SetPen(wxPen(wxColour(100, 100, 100)));
        dc.DrawRectangle(x, y, m_tileSize, m_tileSize);
    }
    
    // Desenhar número do tile no canto inferior direito (menor)
    dc.SetTextForeground(wxColour(255, 255, 255));
    wxFont font = dc.GetFont();
    font.SetPointSize(8);
    font.SetWeight(wxFONTWEIGHT_BOLD);
    dc.SetFont(font);
    
    wxString tileText = wxString::Format("%d", tileId);
    wxSize textSize = dc.GetTextExtent(tileText);
    int textX = x + m_tileSize - textSize.x - 2;
    int textY = y + m_tileSize - textSize.y - 2;
    
    // Fundo preto semi-transparente para o texto
    dc.SetBrush(wxBrush(wxColour(0, 0, 0)));
    dc.SetPen(wxPen(wxColour(0, 0, 0)));
    dc.DrawRectangle(textX - 1, textY - 1, textSize.x + 2, textSize.y + 2);
    
    dc.DrawText(tileText, textX, textY);
}

wxPoint TilesetPanel::TileGrid::TileIdToGridPos(int tileId) const
{
    return wxPoint(tileId % m_tilesPerRow, tileId / m_tilesPerRow);
}

int TilesetPanel::TileGrid::GridPosToTileId(const wxPoint& gridPos) const
{
    return gridPos.y * m_tilesPerRow + gridPos.x;
}

void TilesetPanel::TileGrid::UpdateVirtualSize()
{
    // Determinar quantos tiles mostrar (filtrados ou todos)
    int displayTileCount = 0;
    
    if (m_filteredTiles.empty()) {
        // Sem filtro - usar todos os tiles
        TilesetManager* tilesetManager = m_parent->GetTilesetManager();
        displayTileCount = tilesetManager ? tilesetManager->GetTileCount() : 0;
    } else {
        // Com filtro - usar apenas tiles filtrados
        displayTileCount = static_cast<int>(m_filteredTiles.size());
    }
    
    if (displayTileCount == 0) {
        SetVirtualSize(m_tilesPerRow * (m_tileSize + 4), m_tileSize + 10);
        return;
    }
    
    // Calcular tamanho virtual baseado na quantidade de tiles a mostrar
    int totalRows = (displayTileCount + m_tilesPerRow - 1) / m_tilesPerRow;
    int totalHeight = totalRows * (m_tileSize + 4) + 10; // +4 para margem, +10 para padding
    SetVirtualSize(m_tilesPerRow * (m_tileSize + 4), totalHeight);
}

// Novos métodos do TilesetPanel

bool TilesetPanel::LoadTilesetFromFile(const wxString& filepath, const wxSize& tileSize)
{
    if (m_tilesetManager->LoadTilesetFromFile(filepath, tileSize)) {
        RefreshTilesetList();
        
        // Selecionar o tileset recém-carregado
        wxFileName fileName(filepath);
        wxString tilesetName = fileName.GetName();
        
        int index = m_tilesetChoice->FindString(tilesetName);
        if (index != wxNOT_FOUND) {
            m_tilesetChoice->SetSelection(index);
            m_tilesetManager->SetCurrentTileset(tilesetName);
        }
        
        // Atualizar grid
        m_tileGrid->UpdateVirtualSize();
        m_tileGrid->Refresh();
        
        return true;
    }
    
    return false;
}

void TilesetPanel::RefreshTilesetList()
{
    if (!m_tilesetChoice || !m_tilesetManager) return;
    
    // Salvar seleção atual
    wxString currentSelection;
    if (m_tilesetChoice->GetSelection() != wxNOT_FOUND) {
        currentSelection = m_tilesetChoice->GetStringSelection();
    }
    
    // Limpar e repovoar lista
    m_tilesetChoice->Clear();
    
    const auto& tilesets = m_tilesetManager->GetTilesets();
    for (const auto& tileset : tilesets) {
        m_tilesetChoice->Append(tileset.name);
    }
    
    // Restaurar seleção ou selecionar o primeiro
    if (!currentSelection.IsEmpty()) {
        int index = m_tilesetChoice->FindString(currentSelection);
        if (index != wxNOT_FOUND) {
            m_tilesetChoice->SetSelection(index);
        }
    }
    
    if (m_tilesetChoice->GetSelection() == wxNOT_FOUND && m_tilesetChoice->GetCount() > 0) {
        m_tilesetChoice->SetSelection(0);
        m_tilesetManager->SetCurrentTileset(m_tilesetChoice->GetStringSelection());
    }
}

void TilesetPanel::OnLoadTileset(wxCommandEvent& WXUNUSED(event))
{
    wxFileDialog dialog(
        this,
        UTF8("Carregar Tileset"),
        "",
        "",
        "Imagens (*.png;*.jpg;*.bmp)|*.png;*.jpg;*.bmp|Todos os arquivos (*.*)|*.*",
        wxFD_OPEN | wxFD_FILE_MUST_EXIST
    );
    
    if (dialog.ShowModal() == wxID_OK) {
        wxString filepath = dialog.GetPath();
        
        // Mostrar diálogo para escolher tamanho do tile
        wxString sizes[] = { "16x16", "32x32", "48x48", "64x64" };
        wxSingleChoiceDialog sizeDialog(
            this,
            UTF8("Escolha o tamanho dos tiles:"),
            UTF8("Tamanho do Tile"),
            4,
            sizes
        );
        
        sizeDialog.SetSelection(1); // 32x32 como padrão
        
        if (sizeDialog.ShowModal() == wxID_OK) {
            wxSize tileSize;
            switch (sizeDialog.GetSelection()) {
                case 0: tileSize = wxSize(16, 16); break;
                case 1: tileSize = wxSize(32, 32); break;
                case 2: tileSize = wxSize(48, 48); break;
                case 3: tileSize = wxSize(64, 64); break;
                default: tileSize = wxSize(32, 32); break;
            }
            
            if (!LoadTilesetFromFile(filepath, tileSize)) {
                wxMessageBox(
                    UTF8("Erro ao carregar o tileset. Verifique se o arquivo é uma imagem válida."),
                    UTF8("Erro"),
                    wxOK | wxICON_ERROR
                );
            }
        }
    }
}

void TilesetPanel::OnTilesetChanged(wxCommandEvent& event)
{
    wxString selectedTileset = event.GetString();
    if (m_tilesetManager->SetCurrentTileset(selectedTileset)) {
        // Resetar seleção de tile para o primeiro
        SetSelectedTile(0);
        
        // Limpar filtros e aplicar novamente
        ApplyFilters();
        
        wxLogMessage("Tileset alterado para: %s", selectedTileset);
    }
}

void TilesetPanel::ApplyFilters()
{
    if (!m_tileGrid || !m_tilesetManager) return;
    
    // Obter termos de busca e filtro
    wxString searchTerm;
    wxString categoryFilter;
    
    if (m_searchTextCtrl) {
        searchTerm = m_searchTextCtrl->GetValue().Lower().Trim();
    }
    
    if (m_categoryFilterChoice) {
        categoryFilter = m_categoryFilterChoice->GetStringSelection();
        if (categoryFilter == UTF8("Todas")) {
            categoryFilter.Clear();
        }
    }
    
    // Limpar filtros anteriores
    m_tileGrid->m_filteredTiles.clear();
    
    // Aplicar filtros
    int tileCount = m_tilesetManager->GetTileCount();
    for (int i = 0; i < tileCount; ++i) {
        bool passesFilter = true;
        
        // Filtro de busca por nome
        if (!searchTerm.IsEmpty()) {
            wxString tileName = m_tilesetManager->GetTileName(i).Lower();
            if (!tileName.Contains(searchTerm)) {
                passesFilter = false;
            }
        }
        
        // Filtro de categoria
        if (passesFilter && !categoryFilter.IsEmpty()) {
            wxString tileCategory = m_tilesetManager->GetTileProperty(i, "category").GetString();
            if (tileCategory != categoryFilter) {
                passesFilter = false;
            }
        }
        
        if (passesFilter) {
            m_tileGrid->m_filteredTiles.push_back(i);
        }
    }
    
    // Atualizar visualização
    m_tileGrid->UpdateVirtualSize();
    m_tileGrid->Refresh();
    
    wxLogMessage("Filtro aplicado: %zu tiles de %d visíveis", m_tileGrid->m_filteredTiles.size(), tileCount);
}

void TilesetPanel::OnSearchChanged(wxCommandEvent& WXUNUSED(event))
{
    ApplyFilters();
}

void TilesetPanel::OnCategoryFilterChanged(wxCommandEvent& WXUNUSED(event))
{
    ApplyFilters();
}

void TilesetPanel::OnTilesetProperties(wxCommandEvent& WXUNUSED(event))
{
    if (!m_tilesetManager) {
        wxMessageBox(
            UTF8("Nenhum tileset carregado."),
            UTF8("Aviso"),
            wxOK | wxICON_WARNING
        );
        return;
    }
    
    TilePropertiesDialog dialog(this, m_tilesetManager.get(), m_selectedTile);
    if (dialog.ShowModal() == wxID_OK) {
        // Atualizar grid se houve mudanças
        if (dialog.HasChanges()) {
            RefreshTilesetList();
            m_tileGrid->Refresh();
            
            // Atualizar label de informações
            SetSelectedTile(m_selectedTile);
            
            wxLogMessage("Propriedades dos tiles atualizadas");
        }
    }
}
