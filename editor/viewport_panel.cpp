/**
 * Implementação do Viewport Panel
 */

#include "viewport_panel.h"
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
    EVT_KEY_DOWN(ViewportPanel::GLCanvas::OnKeyDown)
wxEND_EVENT_TABLE()

ViewportPanel::ViewportPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY)
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
    m_toolbar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL);
    
    // Ferramentas de edição
    m_toolbar->AddTool(ID_TOOL_SELECT, "Selecionar", wxNullBitmap, "Ferramenta de seleção");
    m_toolbar->AddTool(ID_TOOL_PAINT, "Pintar", wxNullBitmap, "Ferramenta de pintura");
    m_toolbar->AddTool(ID_TOOL_ERASE, "Apagar", wxNullBitmap, "Ferramenta de apagar");
    m_toolbar->AddTool(ID_TOOL_COLLISION, "Colisão", wxNullBitmap, "Editar colisão");
    
    m_toolbar->AddSeparator();
    
    // Ferramentas de visualização
    m_toolbar->AddTool(ID_ZOOM_IN, "Zoom +", wxNullBitmap, "Aumentar zoom");
    m_toolbar->AddTool(ID_ZOOM_OUT, "Zoom -", wxNullBitmap, "Diminuir zoom");
    m_toolbar->AddTool(ID_RESET_VIEW, "Reset", wxNullBitmap, "Resetar visualização");
    
    // Selecionar ferramenta inicial
    m_toolbar->ToggleTool(ID_TOOL_SELECT, true);
    
    m_toolbar->Realize();
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
    , m_currentTool(TOOL_SELECT)
{
    // Criar contexto OpenGL
    m_glContext = new wxGLContext(this);
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
    // Para M1, desenhar um mapa simples de exemplo
    // Simular o mapa hello.tmx com dimensões 25x15
    const int mapWidth = 25;
    const int mapHeight = 15;
    const int tileSize = 32;
    
    // Desenhar tiles de fundo (verde claro para área interna)
    glColor4f(0.4f, 0.8f, 0.4f, 1.0f);
    glBegin(GL_QUADS);
    
    for (int y = 1; y < mapHeight - 1; ++y) {
        for (int x = 1; x < mapWidth - 1; ++x) {
            float xPos = x * tileSize;
            float yPos = y * tileSize;
            
            glVertex2f(xPos, yPos);
            glVertex2f(xPos + tileSize, yPos);
            glVertex2f(xPos + tileSize, yPos + tileSize);
            glVertex2f(xPos, yPos + tileSize);
        }
    }
    glEnd();
    
    // Desenhar bordas (marrom escuro)
    glColor4f(0.6f, 0.4f, 0.2f, 1.0f);
    glBegin(GL_QUADS);
    
    // Borda superior e inferior
    for (int x = 0; x < mapWidth; ++x) {
        // Superior
        float xPos = x * tileSize;
        float yPos = 0;
        glVertex2f(xPos, yPos);
        glVertex2f(xPos + tileSize, yPos);
        glVertex2f(xPos + tileSize, yPos + tileSize);
        glVertex2f(xPos, yPos + tileSize);
        
        // Inferior
        yPos = (mapHeight - 1) * tileSize;
        glVertex2f(xPos, yPos);
        glVertex2f(xPos + tileSize, yPos);
        glVertex2f(xPos + tileSize, yPos + tileSize);
        glVertex2f(xPos, yPos + tileSize);
    }
    
    // Bordas laterais
    for (int y = 1; y < mapHeight - 1; ++y) {
        // Esquerda
        float xPos = 0;
        float yPos = y * tileSize;
        glVertex2f(xPos, yPos);
        glVertex2f(xPos + tileSize, yPos);
        glVertex2f(xPos + tileSize, yPos + tileSize);
        glVertex2f(xPos, yPos + tileSize);
        
        // Direita
        xPos = (mapWidth - 1) * tileSize;
        glVertex2f(xPos, yPos);
        glVertex2f(xPos + tileSize, yPos);
        glVertex2f(xPos + tileSize, yPos + tileSize);
        glVertex2f(xPos, yPos + tileSize);
    }
    
    glEnd();
    
    // Desenhar posição do jogador (círculo azul)
    glColor4f(0.2f, 0.4f, 1.0f, 0.8f);
    float playerX = 12.5f * tileSize; // Centro do mapa
    float playerY = 7.5f * tileSize;
    float radius = tileSize * 0.3f;
    
    glBegin(GL_POLYGON);
    for (int i = 0; i < 16; ++i) {
        float angle = 2.0f * M_PI * i / 16.0f;
        glVertex2f(playerX + radius * cos(angle), playerY + radius * sin(angle));
    }
    glEnd();
}

void ViewportPanel::GLCanvas::DrawSelection()
{
    // TODO: Implementar desenho de seleção baseado na ferramenta atual
    // Para M1, apenas mostrar que a funcionalidade existe
}

void ViewportPanel::GLCanvas::OnMouseLeftDown(wxMouseEvent& event)
{
    // Converter coordenadas do mouse para coordenadas do mundo
    wxPoint mousePos = event.GetPosition();
    float worldX = (mousePos.x - m_panX) / m_zoom;
    float worldY = (mousePos.y - m_panY) / m_zoom;
    
    // Ações baseadas na ferramenta atual
    switch (m_currentTool) {
        case TOOL_SELECT:
            // TODO: Implementar seleção
            break;
        case TOOL_PAINT:
            // TODO: Implementar pintura
            break;
        case TOOL_ERASE:
            // TODO: Implementar apagar
            break;
        case TOOL_COLLISION:
            // TODO: Implementar edição de colisão
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
    // TODO: Implementar pan com mouse middle ou ações de pintura contínua
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
}

void ViewportPanel::OnToolPaint(wxCommandEvent& WXUNUSED(event))
{
    m_glCanvas->m_currentTool = GLCanvas::TOOL_PAINT;
    m_toolbar->ToggleTool(ID_TOOL_SELECT, false);
    m_toolbar->ToggleTool(ID_TOOL_PAINT, true);
    m_toolbar->ToggleTool(ID_TOOL_ERASE, false);
    m_toolbar->ToggleTool(ID_TOOL_COLLISION, false);
}

void ViewportPanel::OnToolErase(wxCommandEvent& WXUNUSED(event))
{
    m_glCanvas->m_currentTool = GLCanvas::TOOL_ERASE;
    m_toolbar->ToggleTool(ID_TOOL_SELECT, false);
    m_toolbar->ToggleTool(ID_TOOL_PAINT, false);
    m_toolbar->ToggleTool(ID_TOOL_ERASE, true);
    m_toolbar->ToggleTool(ID_TOOL_COLLISION, false);
}

void ViewportPanel::OnToolCollision(wxCommandEvent& WXUNUSED(event))
{
    m_glCanvas->m_currentTool = GLCanvas::TOOL_COLLISION;
    m_toolbar->ToggleTool(ID_TOOL_SELECT, false);
    m_toolbar->ToggleTool(ID_TOOL_PAINT, false);
    m_toolbar->ToggleTool(ID_TOOL_ERASE, false);
    m_toolbar->ToggleTool(ID_TOOL_COLLISION, true);
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
