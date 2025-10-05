/**
 * Implementação do Tileset Panel
 */

#pragma execution_character_set("utf-8")

#include "tileset_panel.h"
#include "utf8_strings.h"

// Definir evento customizado
wxDEFINE_EVENT(TILESET_SELECTION_CHANGED, wxCommandEvent);

// Event table principal
wxBEGIN_EVENT_TABLE(TilesetPanel, wxPanel)
wxEND_EVENT_TABLE()

// Event table para TileGrid
wxBEGIN_EVENT_TABLE(TilesetPanel::TileGrid, wxScrolledWindow)
    EVT_PAINT(TilesetPanel::TileGrid::OnPaint)
    EVT_LEFT_DOWN(TilesetPanel::TileGrid::OnLeftDown)
    EVT_SIZE(TilesetPanel::TileGrid::OnSize)
wxEND_EVENT_TABLE()

TilesetPanel::TilesetPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY)
    , m_selectedTile(1) // Padrão: wall tile
{
    CreateControls();
}

void TilesetPanel::CreateControls()
{
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    
    // Título
    wxStaticText* title = new wxStaticText(this, wxID_ANY, UTF8("Tileset"));
    wxFont titleFont = title->GetFont();
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    title->SetFont(titleFont);
    sizer->Add(title, 0, wxALL, 5);
    
    // Grid de tiles
    m_tileGrid = new TileGrid(this, this);
    sizer->Add(m_tileGrid, 1, wxEXPAND | wxALL, 5);
    
    // Label de informações
    m_infoLabel = new wxStaticText(this, wxID_ANY, UTF8("Tile selecionado: Wall (1)"));
    sizer->Add(m_infoLabel, 0, wxEXPAND | wxALL, 5);
    
    SetSizer(sizer);
}

void TilesetPanel::SetSelectedTile(int tileId)
{
    m_selectedTile = tileId;
    m_tileGrid->SetSelectedTile(tileId);
    
    // Atualizar label de informação
    wxString tileNames[] = {
        UTF8("Grama"), UTF8("Parede"), UTF8("Colisão"), UTF8("Água"),
        UTF8("Areia"), UTF8("Pedra"), UTF8("Lava"), UTF8("Gelo"),
        UTF8("Madeira"), UTF8("Metal")
    };
    
    if (tileId >= 0 && tileId < 10) {
        wxString info = wxString::Format(UTF8("Tile selecionado: %s (%d)"), 
                                         tileNames[tileId], tileId);
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
    
    // Configurar scroll
    int totalRows = (TILE_COUNT + m_tilesPerRow - 1) / m_tilesPerRow;
    int totalHeight = totalRows * (m_tileSize + 4) + 10; // +4 para margem, +10 para padding
    SetScrollRate(5, 5);
    SetVirtualSize(m_tilesPerRow * (m_tileSize + 4), totalHeight);
}

void TilesetPanel::TileGrid::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(this);
    DoPrepareDC(dc);
    
    // Limpar fundo
    dc.SetBackground(wxBrush(GetBackgroundColour()));
    dc.Clear();
    
    // Desenhar todos os tiles
    for (int i = 0; i < TILE_COUNT; ++i) {
        wxPoint gridPos = TileIdToGridPos(i);
        int x = gridPos.x * (m_tileSize + 4) + 5; // +4 margem entre tiles, +5 padding
        int y = gridPos.y * (m_tileSize + 4) + 5;
        
        bool selected = (i == m_selectedTile);
        DrawTile(dc, i, x, y, selected);
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
        int tileId = gridY * m_tilesPerRow + gridX;
        
        if (tileId >= 0 && tileId < TILE_COUNT) {
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
    // Cores dos tiles baseadas no tipo
    wxColour tileColors[] = {
        wxColour(100, 200, 100),  // 0: Grama (verde claro)
        wxColour(139, 69, 19),    // 1: Parede (marrom)
        wxColour(255, 0, 0),      // 2: Colisão (vermelho)
        wxColour(64, 164, 223),   // 3: Água (azul)
        wxColour(238, 203, 173),  // 4: Areia (bege)
        wxColour(128, 128, 128),  // 5: Pedra (cinza)
        wxColour(255, 69, 0),     // 6: Lava (laranja-vermelho)
        wxColour(176, 224, 230),  // 7: Gelo (azul claro)
        wxColour(160, 82, 45),    // 8: Madeira (marrom claro)
        wxColour(192, 192, 192)   // 9: Metal (cinza claro)
    };
    
    // Desenhar fundo do tile
    if (tileId < 10) {
        dc.SetBrush(wxBrush(tileColors[tileId]));
    } else {
        dc.SetBrush(wxBrush(wxColour(255, 255, 255))); // Branco para tiles não definidos
    }
    
    // Borda: mais grossa se selecionado
    if (selected) {
        dc.SetPen(wxPen(wxColour(255, 215, 0), 3)); // Dourado, 3px de espessura
    } else {
        dc.SetPen(wxPen(wxColour(0, 0, 0), 1)); // Preto, 1px
    }
    
    dc.DrawRectangle(x, y, m_tileSize, m_tileSize);
    
    // Desenhar número do tile no centro
    dc.SetTextForeground(wxColour(0, 0, 0));
    wxFont font = dc.GetFont();
    font.SetPointSize(12);
    font.SetWeight(wxFONTWEIGHT_BOLD);
    dc.SetFont(font);
    
    wxString tileText = wxString::Format("%d", tileId);
    wxSize textSize = dc.GetTextExtent(tileText);
    int textX = x + (m_tileSize - textSize.x) / 2;
    int textY = y + (m_tileSize - textSize.y) / 2;
    
    // Fundo branco para o texto para melhor legibilidade
    dc.SetBrush(wxBrush(wxColour(255, 255, 255, 128))); // Semi-transparente
    dc.SetPen(wxPen(wxColour(255, 255, 255, 128)));
    dc.DrawRectangle(textX - 2, textY - 2, textSize.x + 4, textSize.y + 4);
    
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
