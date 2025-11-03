/**
 * Implementação do Viewport Panel
 */

#pragma execution_character_set("utf-8")

#include "viewport_panel.h"
#include "editor_frame.h"
#include "editor_events.h"
#include "utf8_strings.h"
#include "i18n.h"
#include "command.h"
#include "map.h"
#include "shader_program.h"
#include "map_manager.h"
#include "tileset_manager.h"
#include "map_renderer.h"  // Inclui ViewportCamera
#include "texture_atlas.h"
#include "grid_renderer.h"
#include <wx/toolbar.h>
#include <cmath>

// Event table principal
wxBEGIN_EVENT_TABLE(ViewportPanel, wxPanel)
    EVT_TOOL(ID_VP_TOOL_SELECT, ViewportPanel::OnToolSelect)
    EVT_TOOL(ID_VP_TOOL_PAINT, ViewportPanel::OnToolPaint)
    EVT_TOOL(ID_VP_TOOL_BUCKET, ViewportPanel::OnToolBucket)
    EVT_TOOL(ID_VP_TOOL_ERASE, ViewportPanel::OnToolErase)
    EVT_TOOL(ID_VP_TOOL_COLLISION, ViewportPanel::OnToolCollision)
    EVT_TOOL(ID_VP_TOOL_SELECT_RECT, ViewportPanel::OnToolSelectRect)
    EVT_TOOL(ID_VP_TOOL_SELECT_CIRCLE, ViewportPanel::OnToolSelectCircle)
    EVT_TOOL(ID_VP_ZOOM_IN, ViewportPanel::OnZoomIn)
    EVT_TOOL(ID_VP_ZOOM_OUT, ViewportPanel::OnZoomOut)
    EVT_TOOL(ID_VP_RESET_VIEW, ViewportPanel::OnResetView)
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
    EVT_KEY_UP(ViewportPanel::GLCanvas::OnKeyUp)
wxEND_EVENT_TABLE()

ViewportPanel::ViewportPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY)
    , m_mapManager(nullptr)
    , m_tilesetManager(nullptr)
    , m_currentMap(nullptr)
    , m_commandHistory(std::make_unique<CommandHistory>(100))
{
    CreateControls();
}

void ViewportPanel::CreateControls()
{
    // Layout principal
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    
    // Criar toolbox
    CreateToolbox();
    sizer->Add(m_toolbox, 0, wxEXPAND);
    
    // Criar canvas OpenGL
    m_glCanvas = new GLCanvas(this);
    sizer->Add(m_glCanvas, 1, wxEXPAND);
    
    SetSizer(sizer);
}

void ViewportPanel::CreateToolbox()
{
    m_toolbox = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL | wxTB_TEXT);
    
    // Criar bitmaps simples (16x16) temporários
    wxBitmap selectBmp = CreateSimpleBitmap(wxColour(100, 150, 255));     // Azul
    wxBitmap paintBmp = CreateSimpleBitmap(wxColour(100, 255, 100));      // Verde
    wxBitmap bucketBmp = CreateSimpleBitmap(wxColour(0, 200, 200));       // Ciano
    wxBitmap eraseBmp = CreateSimpleBitmap(wxColour(255, 100, 100));      // Vermelho
    wxBitmap collisionBmp = CreateSimpleBitmap(wxColour(255, 255, 100));  // Amarelo
    wxBitmap selectRectBmp = CreateSimpleBitmap(wxColour(150, 100, 255)); // Roxo
    wxBitmap selectCircleBmp = CreateSimpleBitmap(wxColour(255, 150, 200)); // Rosa
    wxBitmap zoomInBmp = CreateSimpleBitmap(wxColour(150, 150, 150));     // Cinza
    wxBitmap zoomOutBmp = CreateSimpleBitmap(wxColour(120, 120, 120));    // Cinza escuro
    wxBitmap resetBmp = CreateSimpleBitmap(wxColour(180, 180, 180));      // Cinza claro
    
    // Ferramentas de edição (usar AddCheckTool para ter estado visual de pressionado)
    m_toolbox->AddCheckTool(ID_VP_TOOL_SELECT, L_("tools.select"), selectBmp, wxNullBitmap, L_("tools.select_desc"));
    m_toolbox->AddCheckTool(ID_VP_TOOL_PAINT, L_("tools.paint"), paintBmp, wxNullBitmap, L_("tools.paint_desc"));
    m_toolbox->AddCheckTool(ID_VP_TOOL_BUCKET, L_("tools.bucket"), bucketBmp, wxNullBitmap, L_("tools.bucket_desc"));
    m_toolbox->AddCheckTool(ID_VP_TOOL_ERASE, L_("tools.erase"), eraseBmp, wxNullBitmap, L_("tools.erase_desc"));
    m_toolbox->AddCheckTool(ID_VP_TOOL_COLLISION, L_("tools.collision"), collisionBmp, wxNullBitmap, L_("tools.collision_desc"));
    
    m_toolbox->AddSeparator();
    
    // Ferramentas de seleção
    m_toolbox->AddCheckTool(ID_VP_TOOL_SELECT_RECT, L_("tools.select_rect"), selectRectBmp, wxNullBitmap, L_("tools.select_rect_desc"));
    m_toolbox->AddCheckTool(ID_VP_TOOL_SELECT_CIRCLE, L_("tools.select_circle"), selectCircleBmp, wxNullBitmap, L_("tools.select_circle_desc"));
    
    m_toolbox->AddSeparator();
    
    // Ferramentas de visualização
    m_toolbox->AddTool(ID_VP_ZOOM_IN, L_("tools.zoom_in"), zoomInBmp, L_("tools.zoom_in_desc"));
    m_toolbox->AddTool(ID_VP_ZOOM_OUT, L_("tools.zoom_out"), zoomOutBmp, L_("tools.zoom_out_desc"));
    m_toolbox->AddTool(ID_VP_RESET_VIEW, L_("tools.reset"), resetBmp, L_("tools.reset_desc"));
    
    // Selecionar ferramenta inicial
    m_toolbox->ToggleTool(ID_VP_TOOL_SELECT, true);
    
    m_toolbox->Realize();
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
    , m_showGrid(true)
    , m_showCollision(false)
    , m_selectedTile(1) // Padrão: wall tile
    , m_isPanning(false)
    , m_isTemporaryPanning(false)
    , m_currentTool(TOOL_SELECT)
{
    // Criar contexto OpenGL
    m_glContext = new wxGLContext(this);
    
    // Configurar SmoothTransform
    m_smoothTransform.SetTransform(1.0f, 0.0f, 0.0f);
    m_smoothTransform.SetUpdateCallback([this](float zoom, float panX, float panY) {
        (void)zoom; (void)panX; (void)panY;
        Refresh();
    });
    
    // Configurar collision overlay
    m_collisionOverlay.SetEnabled(true);  // Habilitado por padrão
    m_collisionOverlay.SetOpacity(0.5f);
    
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
    
    // Inicializar shader de overlay
    m_overlayShader = std::make_unique<ShaderProgram>();
    if (!m_overlayShader->LoadFromFiles("shaders/overlay.vert", "shaders/overlay.frag")) {
        wxLogError("Falha ao carregar shader de overlay");
    }
    
    // Inicializar MapRenderer e TextureAtlas
    m_mapRenderer = std::make_unique<MapRenderer>();
    m_tileAtlas = std::make_unique<TextureAtlas>();
    
    // Configurar MapRenderer
    m_mapRenderer->SetTilesetAtlas(m_tileAtlas.get());
    m_mapRenderer->SetFrustumCullingEnabled(true);
    
    // Inicializar GridRenderer
    m_gridRenderer = std::make_unique<GridRenderer>();
    GridConfig gridConfig;
    gridConfig.tileSize = 32.0f;
    gridConfig.color[0] = 0.3f; // R
    gridConfig.color[1] = 0.3f; // G
    gridConfig.color[2] = 0.3f; // B
    gridConfig.color[3] = 0.5f; // A (50% transparente)
    gridConfig.adaptive = true;
    m_gridRenderer->SetConfig(gridConfig);
    m_gridRenderer->SetGridSize(100, 100); // Grid grande por padrão
    m_gridRenderer->SetEnabled(m_showGrid);
    
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
    
    // Atualizar limites de viewport baseados no tamanho
    UpdateViewportBounds();
    
    event.Skip();
}

void ViewportPanel::GLCanvas::Render()
{
    glClear(GL_COLOR_BUFFER_BIT);
    
    glPushMatrix();
    
    // Aplicar transformações de câmera do SmoothTransform
    float zoom = m_smoothTransform.GetZoom();
    float panX = m_smoothTransform.GetPanX();
    float panY = m_smoothTransform.GetPanY();
    
    glTranslatef(panX, panY, 0.0f);
    glScalef(zoom, zoom, 1.0f);
    
    // Desenhar elementos
    if (m_showGrid) {
        DrawGrid();
    }
    
    DrawMap();
    
    // Desenhar collision overlay se habilitado
    if (m_showCollision && m_collisionOverlay.IsEnabled()) {
        if (m_overlayShader) {
            m_overlayShader->Use();
            m_collisionOverlay.Render();
            glUseProgram(0);
        }
    }
    
    DrawSelection();
    
    glPopMatrix();
}

void ViewportPanel::GLCanvas::DrawGrid()
{
    // Usar GridRenderer se disponível
    if (m_gridRenderer && m_showGrid) {
        float zoom = m_smoothTransform.GetZoom();
        
        // Atualizar zoom do GridRenderer para grid adaptativo
        if (m_gridRenderer->GetZoom() != zoom) {
            m_gridRenderer->SetZoom(zoom);
            m_gridRenderer->RebuildGrid();
        }
        
        // Renderizar
        if (m_overlayShader) {
            m_overlayShader->Use();
            m_gridRenderer->Render();
            glUseProgram(0);
        }
        return;
    }
    
    // Fallback: OpenGL imediato (legacy)
    const int gridSize = 32;
    wxSize canvasSize = GetClientSize();
    
    float zoom = m_smoothTransform.GetZoom();
    float panX = m_smoothTransform.GetPanX();
    float panY = m_smoothTransform.GetPanY();
    
    // Calcular limites visíveis considerando zoom e pan
    int startX = static_cast<int>(-panX / zoom / gridSize) - 1;
    int startY = static_cast<int>(-panY / zoom / gridSize) - 1;
    int endX = startX + static_cast<int>(canvasSize.x / zoom / gridSize) + 2;
    int endY = startY + static_cast<int>(canvasSize.y / zoom / gridSize) + 2;
    
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
    // Obter referência para ViewportPanel parent
    ViewportPanel* viewportPanel = dynamic_cast<ViewportPanel*>(GetParent());
    MapManager* mapManager = viewportPanel ? viewportPanel->m_mapManager : nullptr;
    Map* currentMap = viewportPanel ? viewportPanel->m_currentMap : nullptr;
    
    // Usar MapRenderer se disponível e mapa está configurado
    if (m_mapRenderer && currentMap) {
        // Configurar câmera do MapRenderer
        ViewportCamera camera;
        wxSize canvasSize = GetClientSize();
        float zoom = m_smoothTransform.GetZoom();
        float panX = m_smoothTransform.GetPanX();
        float panY = m_smoothTransform.GetPanY();
        
        // Converter pan de pixels para tiles
        camera.x = -panX / (TILE_SIZE * zoom);
        camera.y = -panY / (TILE_SIZE * zoom);
        camera.width = canvasSize.x / (TILE_SIZE * zoom);
        camera.height = canvasSize.y / (TILE_SIZE * zoom);
        camera.zoom = zoom;
        
        m_mapRenderer->SetCamera(camera);
        m_mapRenderer->SetMap(currentMap);
        
        // Renderizar todas as layers
        m_mapRenderer->RenderAllLayers();
        return;
    }
    
    // Fallback: OpenGL imediato com MapManager
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
    float zoom = m_smoothTransform.GetZoom();
    float panX = m_smoothTransform.GetPanX();
    float panY = m_smoothTransform.GetPanY();
    wxPoint worldPos((mousePos.x - panX) / zoom, (mousePos.y - panY) / zoom);
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
            // Disparar evento de seleção para atualizar PropertyGrid
            {
                SelectionInfo info;
                info.type = SelectionType::TILE;
                info.tilePosition = tilePos;
                info.displayName = wxString::Format("Tile (%d, %d)", tilePos.x, tilePos.y);
                
                // Obter tile ID do mapa
                if (mapManager && mapManager->HasMap()) {
                    info.tileType = mapManager->GetTile(tilePos.x, tilePos.y);
                } else {
                    info.tileType = m_mapTiles[tilePos.y][tilePos.x];
                }
                
                // Enviar evento para EditorFrame
                if (viewportPanel) {
                    EditorFrame* editorFrame = dynamic_cast<EditorFrame*>(viewportPanel->GetParent());
                    if (editorFrame) {
                        SelectionChangeEvent selEvent(EVT_SELECTION_CHANGED);
                        selEvent.SetSelectionInfo(info);
                        editorFrame->GetEventHandler()->ProcessEvent(selEvent);
                    }
                }
            }
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
    
    // Pan com mouse do meio ou Espaço+Botão esquerdo
    if (m_isPanning || (m_isTemporaryPanning && event.LeftIsDown())) {
        wxPoint delta = currentPos - m_lastMousePos;
        float panX = m_smoothTransform.GetPanX() + delta.x;
        float panY = m_smoothTransform.GetPanY() + delta.y;
        m_smoothTransform.SetTransform(m_smoothTransform.GetZoom(), panX, panY);
    }
    
    // Pintura contínua quando botão esquerdo está pressionado (mas não em pan temporário)
    if (event.LeftIsDown() && !m_isTemporaryPanning && (m_currentTool == TOOL_PAINT || m_currentTool == TOOL_ERASE || m_currentTool == TOOL_COLLISION)) {
        float zoom = m_smoothTransform.GetZoom();
        float panX = m_smoothTransform.GetPanX();
        float panY = m_smoothTransform.GetPanY();
        wxPoint worldPos((currentPos.x - panX) / zoom, (currentPos.y - panY) / zoom);
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
    bool ctrl = event.ControlDown();
    
    switch (key) {
        case 'G':
            // Toggle grid visibility
            m_showGrid = !m_showGrid;
            if (m_gridRenderer) {
                m_gridRenderer->SetEnabled(m_showGrid);
            }
            wxLogStatus("Grid: %s", m_showGrid ? "ON" : "OFF");
            Refresh();
            break;
            
        case 'C':
            // Toggle collision overlay visibility
            m_showCollision = !m_showCollision;
            m_collisionOverlay.SetEnabled(m_showCollision);
            wxLogStatus("Collision Overlay: %s", m_showCollision ? "ON" : "OFF");
            Refresh();
            break;
            
        case WXK_SPACE:
            // Espaço: ativar pan temporário
            if (!m_isTemporaryPanning) {
                m_isTemporaryPanning = true;
                SetCursor(wxCursor(wxCURSOR_HAND));
            }
            break;
            
        case '0':
            if (ctrl) {
                // Ctrl+0: Reset zoom 100%
                UpdateViewportBounds();
                m_smoothTransform.ResetZoom();
                m_smoothTransform.AnimatePan(0.0f, 0.0f, 300);
            }
            break;
            
        case '1':
            if (ctrl) {
                // Ctrl+1: Fit to map
                ViewportPanel* viewportPanel = dynamic_cast<ViewportPanel*>(GetParent());
                MapManager* mapManager = viewportPanel ? viewportPanel->m_mapManager : nullptr;
                
                int mapWidth = MAP_WIDTH;
                int mapHeight = MAP_HEIGHT;
                
                if (mapManager && mapManager->HasMap()) {
                    mapWidth = mapManager->GetMapWidth();
                    mapHeight = mapManager->GetMapHeight();
                }
                
                wxSize canvasSize = GetClientSize();
                m_smoothTransform.FitToView(
                    mapWidth * TILE_SIZE, 
                    mapHeight * TILE_SIZE,
                    canvasSize.x,
                    canvasSize.y
                );
            }
            break;
            
        default:
            event.Skip();
            break;
    }
}

void ViewportPanel::GLCanvas::OnKeyUp(wxKeyEvent& event)
{
    int key = event.GetKeyCode();
    
    if (key == WXK_SPACE) {
        // Desativar pan temporário
        if (m_isTemporaryPanning) {
            m_isTemporaryPanning = false;
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
                default:
                    SetCursor(wxCursor(wxCURSOR_ARROW));
                    break;
            }
        }
    }
    
    event.Skip();
}

// Event handlers da ViewportPanel

void ViewportPanel::OnToolSelect(wxCommandEvent& WXUNUSED(event))
{
    m_glCanvas->m_currentTool = GLCanvas::TOOL_SELECT;
    m_toolbox->ToggleTool(ID_VP_TOOL_SELECT, true);
    m_toolbox->ToggleTool(ID_VP_TOOL_PAINT, false);
    m_toolbox->ToggleTool(ID_VP_TOOL_BUCKET, false);
    m_toolbox->ToggleTool(ID_VP_TOOL_ERASE, false);
    m_toolbox->ToggleTool(ID_VP_TOOL_COLLISION, false);
    m_toolbox->ToggleTool(ID_VP_TOOL_SELECT_RECT, false);
    m_toolbox->ToggleTool(ID_VP_TOOL_SELECT_CIRCLE, false);
    m_glCanvas->SetCursor(wxCursor(wxCURSOR_ARROW));
}

void ViewportPanel::OnToolPaint(wxCommandEvent& WXUNUSED(event))
{
    m_glCanvas->m_currentTool = GLCanvas::TOOL_PAINT;
    m_toolbox->ToggleTool(ID_VP_TOOL_SELECT, false);
    m_toolbox->ToggleTool(ID_VP_TOOL_PAINT, true);
    m_toolbox->ToggleTool(ID_VP_TOOL_BUCKET, false);
    m_toolbox->ToggleTool(ID_VP_TOOL_ERASE, false);
    m_toolbox->ToggleTool(ID_VP_TOOL_COLLISION, false);
    m_toolbox->ToggleTool(ID_VP_TOOL_SELECT_RECT, false);
    m_toolbox->ToggleTool(ID_VP_TOOL_SELECT_CIRCLE, false);
    m_glCanvas->SetCursor(wxCursor(wxCURSOR_CROSS));
}

void ViewportPanel::OnToolBucket(wxCommandEvent& WXUNUSED(event))
{
    m_glCanvas->m_currentTool = GLCanvas::TOOL_BUCKET;
    m_toolbox->ToggleTool(ID_VP_TOOL_SELECT, false);
    m_toolbox->ToggleTool(ID_VP_TOOL_PAINT, false);
    m_toolbox->ToggleTool(ID_VP_TOOL_BUCKET, true);
    m_toolbox->ToggleTool(ID_VP_TOOL_ERASE, false);
    m_toolbox->ToggleTool(ID_VP_TOOL_COLLISION, false);
    m_toolbox->ToggleTool(ID_VP_TOOL_SELECT_RECT, false);
    m_toolbox->ToggleTool(ID_VP_TOOL_SELECT_CIRCLE, false);
    m_glCanvas->SetCursor(wxCursor(wxCURSOR_HAND));
    wxLogMessage("Átil: Ferramenta Balde selecionada");
}

void ViewportPanel::OnToolErase(wxCommandEvent& WXUNUSED(event))
{
    m_glCanvas->m_currentTool = GLCanvas::TOOL_ERASE;
    m_toolbox->ToggleTool(ID_VP_TOOL_SELECT, false);
    m_toolbox->ToggleTool(ID_VP_TOOL_PAINT, false);
    m_toolbox->ToggleTool(ID_VP_TOOL_BUCKET, false);
    m_toolbox->ToggleTool(ID_VP_TOOL_ERASE, true);
    m_toolbox->ToggleTool(ID_VP_TOOL_COLLISION, false);
    m_toolbox->ToggleTool(ID_VP_TOOL_SELECT_RECT, false);
    m_toolbox->ToggleTool(ID_VP_TOOL_SELECT_CIRCLE, false);
    m_glCanvas->SetCursor(wxCursor(wxCURSOR_NO_ENTRY));
}

void ViewportPanel::OnToolCollision(wxCommandEvent& WXUNUSED(event))
{
    m_glCanvas->m_currentTool = GLCanvas::TOOL_COLLISION;
    m_toolbox->ToggleTool(ID_VP_TOOL_SELECT, false);
    m_toolbox->ToggleTool(ID_VP_TOOL_PAINT, false);
    m_toolbox->ToggleTool(ID_VP_TOOL_BUCKET, false);
    m_toolbox->ToggleTool(ID_VP_TOOL_ERASE, false);
    m_toolbox->ToggleTool(ID_VP_TOOL_COLLISION, true);
    m_toolbox->ToggleTool(ID_VP_TOOL_SELECT_RECT, false);
    m_toolbox->ToggleTool(ID_VP_TOOL_SELECT_CIRCLE, false);
    m_glCanvas->SetCursor(wxCursor(wxCURSOR_CROSS));
}

void ViewportPanel::OnToolSelectRect(wxCommandEvent& WXUNUSED(event))
{
    m_glCanvas->m_currentTool = GLCanvas::TOOL_SELECT_RECT;
    m_toolbox->ToggleTool(ID_VP_TOOL_SELECT, false);
    m_toolbox->ToggleTool(ID_VP_TOOL_PAINT, false);
    m_toolbox->ToggleTool(ID_VP_TOOL_BUCKET, false);
    m_toolbox->ToggleTool(ID_VP_TOOL_ERASE, false);
    m_toolbox->ToggleTool(ID_VP_TOOL_COLLISION, false);
    m_toolbox->ToggleTool(ID_VP_TOOL_SELECT_RECT, true);
    m_toolbox->ToggleTool(ID_VP_TOOL_SELECT_CIRCLE, false);
    m_glCanvas->SetCursor(wxCursor(wxCURSOR_CROSS));
    wxLogMessage("Átil: Ferramenta Seleção Retangular selecionada");
}

void ViewportPanel::OnToolSelectCircle(wxCommandEvent& WXUNUSED(event))
{
    m_glCanvas->m_currentTool = GLCanvas::TOOL_SELECT_CIRCLE;
    m_toolbox->ToggleTool(ID_VP_TOOL_SELECT, false);
    m_toolbox->ToggleTool(ID_VP_TOOL_PAINT, false);
    m_toolbox->ToggleTool(ID_VP_TOOL_BUCKET, false);
    m_toolbox->ToggleTool(ID_VP_TOOL_ERASE, false);
    m_toolbox->ToggleTool(ID_VP_TOOL_COLLISION, false);
    m_toolbox->ToggleTool(ID_VP_TOOL_SELECT_RECT, false);
    m_toolbox->ToggleTool(ID_VP_TOOL_SELECT_CIRCLE, true);
    m_glCanvas->SetCursor(wxCursor(wxCURSOR_CROSS));
    wxLogMessage("Átil: Ferramenta Seleção Circular selecionada");
}

void ViewportPanel::OnZoomIn(wxCommandEvent& WXUNUSED(event))
{
    // Atualizar bounds antes do zoom
    m_glCanvas->UpdateViewportBounds();
    
    // Zoom suave
    m_glCanvas->m_smoothTransform.ZoomIn();
    wxLogStatus("Zoom: %.0f%%", m_glCanvas->m_smoothTransform.GetZoom() * 100.0f);
}

void ViewportPanel::OnZoomOut(wxCommandEvent& WXUNUSED(event))
{
    // Atualizar bounds antes do zoom
    m_glCanvas->UpdateViewportBounds();
    
    // Zoom suave
    m_glCanvas->m_smoothTransform.ZoomOut();
    wxLogStatus("Zoom: %.0f%%", m_glCanvas->m_smoothTransform.GetZoom() * 100.0f);
}

void ViewportPanel::OnResetView(wxCommandEvent& WXUNUSED(event))
{
    // Reset suave
    m_glCanvas->m_smoothTransform.ResetZoom();
    m_glCanvas->m_smoothTransform.AnimatePan(0.0f, 0.0f, 300);
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
    if (m_glCanvas) {
        m_glCanvas->UpdateViewportBounds();
        m_glCanvas->m_collisionOverlay.MarkDirty();
    }
    RefreshMapDisplay();
}

void ViewportPanel::SetTilesetManager(TilesetManager* tilesetManager)
{
    m_tilesetManager = tilesetManager;
    if (m_glCanvas) {
        // Rebuildar CollisionOverlay com novo TilesetManager
        if (m_currentMap && tilesetManager) {
            m_glCanvas->m_collisionOverlay.BuildFromMap(m_currentMap, tilesetManager);
            wxLogMessage("CollisionOverlay: Rebuilt with new TilesetManager (%d collision tiles)",
                        m_glCanvas->m_collisionOverlay.GetStats().collisionTiles);
        } else {
            m_glCanvas->m_collisionOverlay.MarkDirty();
        }
    }
    RefreshMapDisplay();
}

void ViewportPanel::SetCurrentMap(Map* map)
{
    m_currentMap = map;
    if (m_glCanvas) {
        m_glCanvas->UpdateViewportBounds();
        
        // Atualizar CollisionOverlay com dados reais
        if (map && m_tilesetManager) {
            m_glCanvas->m_collisionOverlay.BuildFromMap(map, m_tilesetManager);
            wxLogMessage("CollisionOverlay: Built from map with %d collision tiles",
                        m_glCanvas->m_collisionOverlay.GetStats().collisionTiles);
        } else {
            m_glCanvas->m_collisionOverlay.MarkDirty();
        }
        
        // Atualizar MapRenderer com novo mapa
        if (m_glCanvas->m_mapRenderer && map) {
            m_glCanvas->m_mapRenderer->SetMap(map);
            m_glCanvas->m_mapRenderer->RebuildAllLayers(true);
        }
        
        // Atualizar GridRenderer com tamanho do mapa
        if (m_glCanvas->m_gridRenderer && map) {
            // Obter dimensões do mapa
            int mapWidth = 100;  // Padrão
            int mapHeight = 100;
            
            if (m_mapManager && m_mapManager->HasMap()) {
                mapWidth = m_mapManager->GetMapWidth();
                mapHeight = m_mapManager->GetMapHeight();
            }
            
            m_glCanvas->m_gridRenderer->SetGridSize(mapWidth, mapHeight);
            m_glCanvas->m_gridRenderer->RebuildGrid();
        }
    }
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
    bool ctrl = event.ControlDown();
    bool shift = event.ShiftDown();
    float rotation = event.GetWheelRotation();
    
    if (shift) {
        // Shift+Scroll: Pan horizontal
        float panDelta = (rotation > 0) ? 50.0f : -50.0f;
        float panX = m_smoothTransform.GetPanX() + panDelta;
        m_smoothTransform.AnimatePan(panX, m_smoothTransform.GetPanY(), 100);
    } else {
        // Atualizar bounds antes do zoom
        UpdateViewportBounds();
        
        // Calcular ponto do mundo sob o cursor
        wxPoint mousePos = event.GetPosition();
        float currentZoom = m_smoothTransform.GetZoom();
        float currentPanX = m_smoothTransform.GetPanX();
        float currentPanY = m_smoothTransform.GetPanY();
        
        float worldX = (mousePos.x - currentPanX) / currentZoom;
        float worldY = (mousePos.y - currentPanY) / currentZoom;
        
        // Calcular novo zoom (mais rápido com Ctrl)
        float zoomFactor = ctrl ? 1.2f : 1.1f;
        float targetZoom = currentZoom;
        if (rotation > 0) {
            targetZoom *= zoomFactor;
        } else if (rotation < 0) {
            targetZoom /= zoomFactor;
        }
        
        // Aplicar zoom suave centrado no cursor
        wxSize canvasSize = GetClientSize();
        m_smoothTransform.ZoomToPoint(targetZoom, worldX, worldY, canvasSize.x, canvasSize.y);
        
        // Mostrar zoom atual na status bar
        ViewportPanel* viewportPanel = dynamic_cast<ViewportPanel*>(GetParent());
        if (viewportPanel) {
            wxLogStatus("Zoom: %.0f%%", targetZoom * 100.0f);
        }
    }
    
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
    // Obter referência para ViewportPanel parent
    ViewportPanel* viewportPanel = dynamic_cast<ViewportPanel*>(GetParent());
    MapManager* mapManager = viewportPanel ? viewportPanel->m_mapManager : nullptr;
    TilesetManager* tilesetManager = viewportPanel ? viewportPanel->m_tilesetManager : nullptr;
    
    if (mapManager && mapManager->HasMap() && tilesetManager) {
        // Usar dados reais do mapa carregado
        if (mapManager->IsValidPosition(tileX, tileY)) {
            // Obter tile ID atual
            int tileId = mapManager->GetTile(tileX, tileY);
            
            // Obter propriedade de colisão atual
            wxVariant collisionVariant = tilesetManager->GetTileProperty(tileId, "hasCollision");
            bool hasCollision = collisionVariant.GetBool();
            
            // Toggle colisão
            tilesetManager->SetTileProperty(tileId, "hasCollision", wxVariant(!hasCollision));
            
            // Rebuildar overlay com dados atualizados
            if (mapManager && mapManager->HasMap()) {
                // Obter mapa real do ViewportPanel parent
                Map* currentMap = viewportPanel ? viewportPanel->m_currentMap : nullptr;
                if (currentMap) {
                    m_collisionOverlay.BuildFromMap(currentMap, tilesetManager);
                }
            } else {
                m_collisionOverlay.MarkDirty();
            }
            
            // Notificar EditorFrame para atualizar título
            viewportPanel->NotifyMapModified();
            
            wxLogStatus("Tile %d collision: %s", tileId, !hasCollision ? "ON" : "OFF");
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

// ============================================================================
// Métodos Undo/Redo do ViewportPanel
// ============================================================================

bool ViewportPanel::CanUndo() const
{
    return m_commandHistory && m_commandHistory->CanUndo();
}

bool ViewportPanel::CanRedo() const
{
    return m_commandHistory && m_commandHistory->CanRedo();
}

bool ViewportPanel::Undo()
{
    if (!m_commandHistory) {
        return false;
    }
    
    bool success = m_commandHistory->Undo();
    if (success) {
        RefreshMapDisplay();
        NotifyMapModified();
    }
    return success;
}

bool ViewportPanel::Redo()
{
    if (!m_commandHistory) {
        return false;
    }
    
    bool success = m_commandHistory->Redo();
    if (success) {
        RefreshMapDisplay();
        NotifyMapModified();
    }
    return success;
}

void ViewportPanel::GLCanvas::UpdateViewportBounds()
{
    // Obter referência para ViewportPanel parent para acessar MapManager
    ViewportPanel* viewportPanel = dynamic_cast<ViewportPanel*>(GetParent());
    MapManager* mapManager = viewportPanel ? viewportPanel->m_mapManager : nullptr;
    
    wxSize canvasSize = GetClientSize();
    if (canvasSize.x <= 0 || canvasSize.y <= 0) {
        return; // Canvas ainda não está pronto
    }
    
    // Determinar dimensões do mapa
    int mapWidth = MAP_WIDTH;
    int mapHeight = MAP_HEIGHT;
    
    if (mapManager && mapManager->HasMap()) {
        mapWidth = mapManager->GetMapWidth();
        mapHeight = mapManager->GetMapHeight();
    }
    
    // Calcular dimensões do mapa em pixels
    float mapPixelWidth = mapWidth * TILE_SIZE;
    float mapPixelHeight = mapHeight * TILE_SIZE;
    
    // Configurar limites de zoom
    TransformBounds bounds;
    bounds.enableBounds = true;
    bounds.minZoom = 0.25f;
    bounds.maxZoom = 4.0f;
    
    // Calcular limites de pan dinâmicos
    // Permitir pan até uma margem de 20% do canvas de cada lado
    float margin = 0.2f;
    float currentZoom = m_smoothTransform.GetZoom();
    
    // Limites mínimos: mapa não pode sair totalmente da tela pela direita/baixo
    bounds.minPanX = -mapPixelWidth * currentZoom + canvasSize.x * (1.0f - margin);
    bounds.minPanY = -mapPixelHeight * currentZoom + canvasSize.y * (1.0f - margin);
    
    // Limites máximos: mapa não pode sair totalmente da tela pela esquerda/topo
    bounds.maxPanX = canvasSize.x * margin;
    bounds.maxPanY = canvasSize.y * margin;
    
    m_smoothTransform.SetBounds(bounds);
}
