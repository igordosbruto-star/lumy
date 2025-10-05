/**
 * Implementação do Viewport Panel
 */

#pragma execution_character_set("utf-8")

#include "viewport_panel.h"
#include "editor_frame.h"
#include "utf8_strings.h"
#include <wx/toolbar.h>
#include <GL/gl.h>
#include <cmath>

// Event table principal
wxBEGIN_EVENT_TABLE(ViewportPanel, wxPanel)
    EVT_TOOL(ID_TOOL_SELECT, ViewportPanel::OnToolSelect)
    EVT_TOOL(ID_TOOL_PAINT, ViewportPanel::OnToolPaint)
    EVT_TOOL(ID_TOOL_ERASE, ViewportPanel::OnToolErase)
    EVT_TOOL(ID_TOOL_COLLISION, ViewportPanel::OnToolCollision)
    EVT_TOOL(ID_ZOOM_IN, ViewportPanel::OnZoomIn)
    EVT_TOOL(ID_ZOOM_OUT, ViewportPanel::OnZoomOut)
    EVT_TOOL(ID_RESET_VIEW, ViewportPanel::OnResetView)
wxEND_EVENT_TABLE()

// Event table para GLCanvas
wxBEGIN_EVENT_TABLE(ViewportPanel::GLCanvas, wxGLCanvas)
    EVT_PAINT(ViewportPanel::GLCanvas::OnPaint)
    EVT_SIZE(ViewportPanel::GLCanvas::OnSize)
    EVT_LEFT_DOWN(ViewportPanel::GLCanvas::OnMouseLeftDown)
    EVT_RIGHT_DOWN(ViewportPanel::GLCanvas::OnMouseRightDown)
    EVT_MOTION(ViewportPanel::GLCanvas::OnMouseMove)
    EVT_MOUSEWHEEL(ViewportPanel::GLCanvas::OnMouseWheel)
    EVT_MIDDLE_DOWN(ViewportPanel::GLCanvas::OnMouseMiddleDown)
    EVT_MIDDLE_UP(ViewportPanel::GLCanvas::OnMouseMiddleUp)
    EVT_KEY_DOWN(ViewportPanel::GLCanvas::OnKeyDown)
wxEND_EVENT_TABLE()

ViewportPanel::ViewportPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY)
    , m_mapManager(nullptr)
{
    CreateControls();
}

void ViewportPanel::CreateControls()
{
    // Layout principal
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    
    // Criar toolbar
    CreateToolbar();
    sizer->Add(m_toolbar, 0, wxEXPAND);
    
    // Criar canvas OpenGL
    m_glCanvas = new GLCanvas(this);
    sizer->Add(m_glCanvas, 1, wxEXPAND);
    
    SetSizer(sizer);
}

void ViewportPanel::CreateToolbar()
{
    m_toolbar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL | wxTB_TEXT);
    
    // Criar bitmaps simples (16x16) temporários
    wxBitmap selectBmp = CreateSimpleBitmap(wxColour(100, 150, 255)); // Azul
    wxBitmap paintBmp = CreateSimpleBitmap(wxColour(100, 255, 100));  // Verde
    wxBitmap eraseBmp = CreateSimpleBitmap(wxColour(255, 100, 100));  // Vermelho
    wxBitmap collisionBmp = CreateSimpleBitmap(wxColour(255, 255, 100)); // Amarelo
    wxBitmap zoomInBmp = CreateSimpleBitmap(wxColour(150, 150, 150)); // Cinza
    wxBitmap zoomOutBmp = CreateSimpleBitmap(wxColour(120, 120, 120)); // Cinza escuro
    wxBitmap resetBmp = CreateSimpleBitmap(wxColour(180, 180, 180)); // Cinza claro
    
    // Ferramentas de edição (usar AddCheckTool para ter estado visual de pressionado)
    m_toolbar->AddCheckTool(ID_TOOL_SELECT, "Selecionar", selectBmp, wxNullBitmap, UTF8("Ferramenta de seleção"));
    m_toolbar->AddCheckTool(ID_TOOL_PAINT, "Pintar", paintBmp, wxNullBitmap, "Ferramenta de pintura");
    m_toolbar->AddCheckTool(ID_TOOL_ERASE, "Apagar", eraseBmp, wxNullBitmap, "Ferramenta de apagar");
    m_toolbar->AddCheckTool(ID_TOOL_COLLISION, UTF8("Colisão"), collisionBmp, wxNullBitmap, UTF8("Editar colisão"));
    
    m_toolbar->AddSeparator();
    
    // Ferramentas de visualização
    m_toolbar->AddTool(ID_ZOOM_IN, "Zoom +", zoomInBmp, "Aumentar zoom");
    m_toolbar->AddTool(ID_ZOOM_OUT, "Zoom -", zoomOutBmp, "Diminuir zoom");
    m_toolbar->AddTool(ID_RESET_VIEW, "Reset", resetBmp, UTF8("Resetar visualização"));
    
    // Selecionar ferramenta inicial
    m_toolbar->ToggleTool(ID_TOOL_SELECT, true);
    
    m_toolbar->Realize();
}

wxBitmap ViewportPanel::CreateSimpleBitmap(const wxColour& color)
{
    // Criar bitmap simples 16x16 com a cor especificada
    wxImage img(16, 16);
    img.SetRGB(wxRect(0, 0, 16, 16), color.Red(), color.Green(), color.Blue());
    
    // Criar borda preta de 1 pixel
    for (int x = 0; x < 16; ++x) {
        img.SetRGB(x, 0, 0, 0, 0);   // Borda superior
        img.SetRGB(x, 15, 0, 0, 0);  // Borda inferior
    }
    for (int y = 0; y < 16; ++y) {
        img.SetRGB(0, y, 0, 0, 0);   // Borda esquerda
        img.SetRGB(15, y, 0, 0, 0);  // Borda direita
    }
    
    return wxBitmap(img);
}

// Implementação do GLCanvas

ViewportPanel::GLCanvas::GLCanvas(wxWindow* parent)
    : wxGLCanvas(parent, wxID_ANY, nullptr, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE)
    , m_glContext(nullptr)
    , m_glInitialized(false)
    , m_zoom(1.0f)
    , m_panX(0.0f)
    , m_panY(0.0f)
    , m_showGrid(true)
    , m_showCollision(false)
    , m_selectedTile(1) // Padrão: wall tile
    , m_isPanning(false)
    , m_currentTool(TOOL_SELECT)
{
    // Criar contexto OpenGL
    m_glContext = new wxGLContext(this);
    
    // Inicializar mapa com padrão (bordas = wall, interior = grass)
    for (int y = 0; y < MAP_HEIGHT; ++y) {
        for (int x = 0; x < MAP_WIDTH; ++x) {
            if (x == 0 || x == MAP_WIDTH - 1 || y == 0 || y == MAP_HEIGHT - 1) {
                m_mapTiles[y][x] = 1; // Wall
            } else {
                m_mapTiles[y][x] = 0; // Grass
            }
        }
    }
}

ViewportPanel::GLCanvas::~GLCanvas()
{
    delete m_glContext;
}

void ViewportPanel::GLCanvas::InitGL()
{
    if (m_glInitialized) return;
    
    SetCurrent(*m_glContext);
    
    // Configurações OpenGL básicas
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    m_glInitialized = true;
}

void ViewportPanel::GLCanvas::OnPaint(wxPaintEvent& WXUNUSED(event))
{
    if (!IsShown()) return;
    
    wxPaintDC dc(this);
    
    if (!m_glInitialized) {
        InitGL();
    }
    
    SetCurrent(*m_glContext);
    Render();
    SwapBuffers();
}

void ViewportPanel::GLCanvas::OnSize(wxSizeEvent& event)
{
    if (!IsShown()) return;
    
    SetCurrent(*m_glContext);
    
    wxSize size = GetClientSize();
    glViewport(0, 0, size.x, size.y);
    
    // Configurar projeção ortográfica
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, size.x, size.y, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    event.Skip();
}

void ViewportPanel::GLCanvas::Render()
{
    glClear(GL_COLOR_BUFFER_BIT);
    
    glPushMatrix();
    
    // Aplicar transformações de câmera
    glTranslatef(m_panX, m_panY, 0.0f);
    glScalef(m_zoom, m_zoom, 1.0f);
    
    // Desenhar elementos
    if (m_showGrid) {
        DrawGrid();
    }
    
    DrawMap();
    DrawSelection();
    
    glPopMatrix();
}

void ViewportPanel::GLCanvas::DrawGrid()
{
    // Desenhar grade 32x32 pixels (tamanho padrão dos tiles)
    const int gridSize = 32;
    wxSize canvasSize = GetClientSize();
    
    // Calcular limites visíveis considerando zoom e pan
    int startX = static_cast<int>(-m_panX / m_zoom / gridSize) - 1;
    int startY = static_cast<int>(-m_panY / m_zoom / gridSize) - 1;
    int endX = startX + static_cast<int>(canvasSize.x / m_zoom / gridSize) + 2;
    int endY = startY + static_cast<int>(canvasSize.y / m_zoom / gridSize) + 2;
    
    glColor4f(0.3f, 0.3f, 0.3f, 0.5f);
    glBegin(GL_LINES);
    
    // Linhas verticais
    for (int x = startX; x <= endX; ++x) {
        float xPos = x * gridSize;
        glVertex2f(xPos, startY * gridSize);
        glVertex2f(xPos, endY * gridSize);
    }
    
    // Linhas horizontais
    for (int y = startY; y <= endY; ++y) {
        float yPos = y * gridSize;
        glVertex2f(startX * gridSize, yPos);
        glVertex2f(endX * gridSize, yPos);
    }
    
    glEnd();
}

void ViewportPanel::GLCanvas::DrawMap()
{
    // Obter referência para ViewportPanel parent para acessar MapManager
    ViewportPanel* viewportPanel = dynamic_cast<ViewportPanel*>(GetParent());
    MapManager* mapManager = viewportPanel ? viewportPanel->m_mapManager : nullptr;
    
    // Se temos um MapManager com mapa carregado, usar dados reais
    if (mapManager && mapManager->HasMap()) {
        int mapWidth = mapManager->GetMapWidth();
        int mapHeight = mapManager->GetMapHeight();
        
        glBegin(GL_QUADS);
        
        for (int y = 0; y < mapHeight; ++y) {
            for (int x = 0; x < mapWidth; ++x) {
                int tileType = mapManager->GetTile(x, y);
                float xPos = x * TILE_SIZE;
                float yPos = y * TILE_SIZE;
                
                // Cores baseadas no tipo do tile
                switch (tileType) {
                    case 0: // Grass/Empty
                        glColor4f(0.4f, 0.8f, 0.4f, 1.0f);
                        break;
                    case 1: // Wall
                        glColor4f(0.6f, 0.4f, 0.2f, 1.0f);
                        break;
                    case 2: // Water
                        glColor4f(0.2f, 0.4f, 0.8f, 1.0f);
                        break;
                    case 3: // Stone
                        glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
                        break;
                    case 4: // Tree
                        glColor4f(0.2f, 0.6f, 0.2f, 1.0f);
                        break;
                    default: // Outros tiles - usar cor baseada no ID
                        float hue = (tileType % 10) / 10.0f;
                        glColor4f(0.5f + hue * 0.5f, 0.3f + hue * 0.4f, 0.3f + hue * 0.4f, 1.0f);
                        break;
                }
                
                glVertex2f(xPos, yPos);
                glVertex2f(xPos + TILE_SIZE, yPos);
                glVertex2f(xPos + TILE_SIZE, yPos + TILE_SIZE);
                glVertex2f(xPos, yPos + TILE_SIZE);
            }
        }
        
        glEnd();
        
        // Desenhar posição do jogador (círculo azul) - centrado no mapa real
        glColor4f(0.2f, 0.4f, 1.0f, 0.8f);
        float playerX = (mapWidth / 2.0f) * TILE_SIZE;
        float playerY = (mapHeight / 2.0f) * TILE_SIZE;
        float radius = TILE_SIZE * 0.3f;
        
        glBegin(GL_POLYGON);
        for (int i = 0; i < 16; ++i) {
            float angle = 2.0f * M_PI * i / 16.0f;
            glVertex2f(playerX + radius * cos(angle), playerY + radius * sin(angle));
        }
        glEnd();
    }
    else {
        // Fallback: usar dados simulados se não há mapa carregado
        glBegin(GL_QUADS);
        
        for (int y = 0; y < MAP_HEIGHT; ++y) {
            for (int x = 0; x < MAP_WIDTH; ++x) {
                int tileType = m_mapTiles[y][x];
                float xPos = x * TILE_SIZE;
                float yPos = y * TILE_SIZE;
                
                // Cores baseadas no tipo do tile
                switch (tileType) {
                    case 0: // Grass
                        glColor4f(0.4f, 0.8f, 0.4f, 1.0f);
                        break;
                    case 1: // Wall
                        glColor4f(0.6f, 0.4f, 0.2f, 1.0f);
                        break;
                    case 2: // Collision (special)
                        if (m_showCollision) {
                            glColor4f(1.0f, 0.0f, 0.0f, 0.7f); // Red translucent
                        } else {
                            glColor4f(0.4f, 0.8f, 0.4f, 1.0f); // Same as grass
                        }
                        break;
                    default: // Empty
                        continue; // Don't draw anything
                }
                
                glVertex2f(xPos, yPos);
                glVertex2f(xPos + TILE_SIZE, yPos);
                glVertex2f(xPos + TILE_SIZE, yPos + TILE_SIZE);
                glVertex2f(xPos, yPos + TILE_SIZE);
            }
        }
        
        glEnd();
        
        // Desenhar posição do jogador (círculo azul) - mapa simulado
        glColor4f(0.2f, 0.4f, 1.0f, 0.8f);
        float playerX = 12.5f * TILE_SIZE; // Centro do mapa simulado
        float playerY = 7.5f * TILE_SIZE;
        float radius = TILE_SIZE * 0.3f;
        
        glBegin(GL_POLYGON);
        for (int i = 0; i < 16; ++i) {
            float angle = 2.0f * M_PI * i / 16.0f;
            glVertex2f(playerX + radius * cos(angle), playerY + radius * sin(angle));
        }
        glEnd();
    }
}

void ViewportPanel::GLCanvas::DrawSelection()
{
    // TODO: Implementar desenho de seleção baseado na ferramenta atual
    // Para M1, apenas mostrar que a funcionalidade existe
}

void ViewportPanel::GLCanvas::OnMouseLeftDown(wxMouseEvent& event)
{
    SetFocus(); // Para receber eventos de teclado
    
    wxPoint mousePos = event.GetPosition();
    wxPoint worldPos((mousePos.x - m_panX) / m_zoom, (mousePos.y - m_panY) / m_zoom);
    wxPoint tilePos = WorldToTile(worldPos);
    
    // Verificar se está dentro dos limites do mapa
    // Obter referência para ViewportPanel parent para acessar MapManager
    ViewportPanel* viewportPanel = dynamic_cast<ViewportPanel*>(GetParent());
    MapManager* mapManager = viewportPanel ? viewportPanel->m_mapManager : nullptr;
    
    bool isValidPosition = false;
    if (mapManager && mapManager->HasMap()) {
        // Usar limites reais do mapa carregado
        isValidPosition = mapManager->IsValidPosition(tilePos.x, tilePos.y);
    } else {
        // Fallback: usar limites simulados
        isValidPosition = (tilePos.x >= 0 && tilePos.x < MAP_WIDTH && tilePos.y >= 0 && tilePos.y < MAP_HEIGHT);
    }
    
    if (!isValidPosition) {
        return;
    }
    
    // Ações baseadas na ferramenta atual
    switch (m_currentTool) {
        case TOOL_SELECT:
            // TODO: Implementar seleção visual
            break;
        case TOOL_PAINT:
            PaintTile(tilePos.x, tilePos.y);
            break;
        case TOOL_ERASE:
            EraseTile(tilePos.x, tilePos.y);
            break;
        case TOOL_COLLISION:
            ToggleCollision(tilePos.x, tilePos.y);
            break;
    }
    
    Refresh();
    event.Skip();
}

void ViewportPanel::GLCanvas::OnMouseRightDown(wxMouseEvent& event)
{
    // Menu contextual ou ação alternativa
    event.Skip();
}

void ViewportPanel::GLCanvas::OnMouseMove(wxMouseEvent& event)
{
    wxPoint currentPos = event.GetPosition();
    
    // Pan com mouse do meio
    if (m_isPanning) {
        wxPoint delta = currentPos - m_lastMousePos;
        m_panX += delta.x;
        m_panY += delta.y;
        Refresh();
    }
    
    // Pintura contínua quando botão esquerdo está pressionado
    if (event.LeftIsDown() && (m_currentTool == TOOL_PAINT || m_currentTool == TOOL_ERASE || m_currentTool == TOOL_COLLISION)) {
        wxPoint worldPos((currentPos.x - m_panX) / m_zoom, (currentPos.y - m_panY) / m_zoom);
        wxPoint tilePos = WorldToTile(worldPos);
        
        // Verificar limites do mapa para pintura contínua
        ViewportPanel* viewportPanel = dynamic_cast<ViewportPanel*>(GetParent());
        MapManager* mapManager = viewportPanel ? viewportPanel->m_mapManager : nullptr;
        
        bool isValidPosition = false;
        if (mapManager && mapManager->HasMap()) {
            // Usar limites reais do mapa carregado
            isValidPosition = mapManager->IsValidPosition(tilePos.x, tilePos.y);
        } else {
            // Fallback: usar limites simulados
            isValidPosition = (tilePos.x >= 0 && tilePos.x < MAP_WIDTH && tilePos.y >= 0 && tilePos.y < MAP_HEIGHT);
        }
        
        if (isValidPosition) {
            switch (m_currentTool) {
                case TOOL_PAINT:
                    PaintTile(tilePos.x, tilePos.y);
                    break;
                case TOOL_ERASE:
                    EraseTile(tilePos.x, tilePos.y);
                    break;
                case TOOL_COLLISION:
                    ToggleCollision(tilePos.x, tilePos.y);
                    break;
                default:
                    break;
            }
            Refresh();
        }
    }
    
    m_lastMousePos = currentPos;
    event.Skip();
}

void ViewportPanel::GLCanvas::OnKeyDown(wxKeyEvent& event)
{
    // Atalhos de teclado para navegação
    int key = event.GetKeyCode();
    
    switch (key) {
        case 'G':
            m_showGrid = !m_showGrid;
            Refresh();
            break;
        case 'C':
            m_showCollision = !m_showCollision;
            Refresh();
            break;
        default:
            event.Skip();
            break;
    }
}

// Event handlers da ViewportPanel

void ViewportPanel::OnToolSelect(wxCommandEvent& WXUNUSED(event))
{
    m_glCanvas->m_currentTool = GLCanvas::TOOL_SELECT;
    m_toolbar->ToggleTool(ID_TOOL_SELECT, true);
    m_toolbar->ToggleTool(ID_TOOL_PAINT, false);
    m_toolbar->ToggleTool(ID_TOOL_ERASE, false);
    m_toolbar->ToggleTool(ID_TOOL_COLLISION, false);
    m_glCanvas->SetCursor(wxCursor(wxCURSOR_ARROW));
}

void ViewportPanel::OnToolPaint(wxCommandEvent& WXUNUSED(event))
{
    m_glCanvas->m_currentTool = GLCanvas::TOOL_PAINT;
    m_toolbar->ToggleTool(ID_TOOL_SELECT, false);
    m_toolbar->ToggleTool(ID_TOOL_PAINT, true);
    m_toolbar->ToggleTool(ID_TOOL_ERASE, false);
    m_toolbar->ToggleTool(ID_TOOL_COLLISION, false);
    m_glCanvas->SetCursor(wxCursor(wxCURSOR_CROSS));
}

void ViewportPanel::OnToolErase(wxCommandEvent& WXUNUSED(event))
{
    m_glCanvas->m_currentTool = GLCanvas::TOOL_ERASE;
    m_toolbar->ToggleTool(ID_TOOL_SELECT, false);
    m_toolbar->ToggleTool(ID_TOOL_PAINT, false);
    m_toolbar->ToggleTool(ID_TOOL_ERASE, true);
    m_toolbar->ToggleTool(ID_TOOL_COLLISION, false);
    m_glCanvas->SetCursor(wxCursor(wxCURSOR_NO_ENTRY));
}

void ViewportPanel::OnToolCollision(wxCommandEvent& WXUNUSED(event))
{
    m_glCanvas->m_currentTool = GLCanvas::TOOL_COLLISION;
    m_toolbar->ToggleTool(ID_TOOL_SELECT, false);
    m_toolbar->ToggleTool(ID_TOOL_PAINT, false);
    m_toolbar->ToggleTool(ID_TOOL_ERASE, false);
    m_toolbar->ToggleTool(ID_TOOL_COLLISION, true);
    m_glCanvas->SetCursor(wxCursor(wxCURSOR_CROSS));
}

void ViewportPanel::OnZoomIn(wxCommandEvent& WXUNUSED(event))
{
    m_glCanvas->m_zoom *= 1.25f;
    m_glCanvas->Refresh();
}

void ViewportPanel::OnZoomOut(wxCommandEvent& WXUNUSED(event))
{
    m_glCanvas->m_zoom /= 1.25f;
    if (m_glCanvas->m_zoom < 0.1f) {
        m_glCanvas->m_zoom = 0.1f;
    }
    m_glCanvas->Refresh();
}

void ViewportPanel::OnResetView(wxCommandEvent& WXUNUSED(event))
{
    m_glCanvas->m_zoom = 1.0f;
    m_glCanvas->m_panX = 0.0f;
    m_glCanvas->m_panY = 0.0f;
    m_glCanvas->Refresh();
}

void ViewportPanel::SetSelectedTile(int tileId)
{
    if (m_glCanvas) {
        m_glCanvas->m_selectedTile = tileId;
    }
}

void ViewportPanel::SetMapManager(MapManager* mapManager)
{
    m_mapManager = mapManager;
    RefreshMapDisplay();
}

void ViewportPanel::RefreshMapDisplay()
{
    if (m_glCanvas) {
        m_glCanvas->Refresh();
    }
}

void ViewportPanel::NotifyMapModified()
{
    // Notificar o EditorFrame (parent) para atualizar o título
    wxWindow* parent = GetParent();
    if (parent) {
        // Assumimos que o parent é EditorFrame
        EditorFrame* editorFrame = dynamic_cast<EditorFrame*>(parent);
        if (editorFrame) {
            editorFrame->UpdateWindowTitle();
        }
    }
}

// Implementações das funções auxiliares do GLCanvas

void ViewportPanel::GLCanvas::OnMouseWheel(wxMouseEvent& event)
{
    float rotation = event.GetWheelRotation();
    if (rotation > 0) {
        m_zoom *= 1.1f;
    } else if (rotation < 0) {
        m_zoom /= 1.1f;
        if (m_zoom < 0.1f) {
            m_zoom = 0.1f;
        }
    }
    Refresh();
    event.Skip();
}

void ViewportPanel::GLCanvas::OnMouseMiddleDown(wxMouseEvent& event)
{
    m_isPanning = true;
    m_lastMousePos = event.GetPosition();
    SetCursor(wxCursor(wxCURSOR_SIZING));
    event.Skip();
}

void ViewportPanel::GLCanvas::OnMouseMiddleUp(wxMouseEvent& event)
{
    m_isPanning = false;
    // Restaurar cursor da ferramenta atual
    switch (m_currentTool) {
        case TOOL_SELECT:
            SetCursor(wxCursor(wxCURSOR_ARROW));
            break;
        case TOOL_PAINT:
            SetCursor(wxCursor(wxCURSOR_CROSS));
            break;
        case TOOL_ERASE:
            SetCursor(wxCursor(wxCURSOR_NO_ENTRY));
            break;
        case TOOL_COLLISION:
            SetCursor(wxCursor(wxCURSOR_CROSS));
            break;
    }
    event.Skip();
}

wxPoint ViewportPanel::GLCanvas::WorldToTile(const wxPoint& worldPos)
{
    return wxPoint(worldPos.x / TILE_SIZE, worldPos.y / TILE_SIZE);
}

wxPoint ViewportPanel::GLCanvas::TileToWorld(const wxPoint& tilePos)
{
    return wxPoint(tilePos.x * TILE_SIZE, tilePos.y * TILE_SIZE);
}

void ViewportPanel::GLCanvas::PaintTile(int tileX, int tileY)
{
    // Obter referência para ViewportPanel parent para acessar MapManager
    ViewportPanel* viewportPanel = dynamic_cast<ViewportPanel*>(GetParent());
    MapManager* mapManager = viewportPanel ? viewportPanel->m_mapManager : nullptr;
    
    if (mapManager && mapManager->HasMap()) {
        // Usar dados reais do mapa carregado
        if (mapManager->IsValidPosition(tileX, tileY)) {
            mapManager->SetTile(tileX, tileY, m_selectedTile);
            // Notificar EditorFrame para atualizar título
            viewportPanel->NotifyMapModified();
        }
    } else {
        // Fallback: usar dados simulados se não há mapa carregado
        if (tileX >= 0 && tileX < MAP_WIDTH && tileY >= 0 && tileY < MAP_HEIGHT) {
            m_mapTiles[tileY][tileX] = m_selectedTile;
        }
    }
}

void ViewportPanel::GLCanvas::EraseTile(int tileX, int tileY)
{
    // Obter referência para ViewportPanel parent para acessar MapManager
    ViewportPanel* viewportPanel = dynamic_cast<ViewportPanel*>(GetParent());
    MapManager* mapManager = viewportPanel ? viewportPanel->m_mapManager : nullptr;
    
    if (mapManager && mapManager->HasMap()) {
        // Usar dados reais do mapa carregado
        if (mapManager->IsValidPosition(tileX, tileY)) {
            mapManager->SetTile(tileX, tileY, 0); // Grass/Empty
            // Notificar EditorFrame para atualizar título
            viewportPanel->NotifyMapModified();
        }
    } else {
        // Fallback: usar dados simulados se não há mapa carregado
        if (tileX >= 0 && tileX < MAP_WIDTH && tileY >= 0 && tileY < MAP_HEIGHT) {
            m_mapTiles[tileY][tileX] = 0; // Grass
        }
    }
}

void ViewportPanel::GLCanvas::ToggleCollision(int tileX, int tileY)
{
    // Obter referência para ViewportPanel parent para acessar MapManager
    ViewportPanel* viewportPanel = dynamic_cast<ViewportPanel*>(GetParent());
    MapManager* mapManager = viewportPanel ? viewportPanel->m_mapManager : nullptr;
    
    if (mapManager && mapManager->HasMap()) {
        // Usar dados reais do mapa carregado
        if (mapManager->IsValidPosition(tileX, tileY)) {
            int currentTile = mapManager->GetTile(tileX, tileY);
            if (currentTile == 2) {
                // Se já é collision, volta para grass
                mapManager->SetTile(tileX, tileY, 0);
            } else {
                // Se não é collision, torna collision
                mapManager->SetTile(tileX, tileY, 2);
            }
            // Notificar EditorFrame para atualizar título
            viewportPanel->NotifyMapModified();
        }
    } else {
        // Fallback: usar dados simulados se não há mapa carregado
        if (tileX >= 0 && tileX < MAP_WIDTH && tileY >= 0 && tileY < MAP_HEIGHT) {
            int currentTile = m_mapTiles[tileY][tileX];
            if (currentTile == 2) {
                // Se já é collision, volta para grass
                m_mapTiles[tileY][tileX] = 0;
            } else {
                // Se não é collision, torna collision
                m_mapTiles[tileY][tileX] = 2;
            }
        }
    }
}
