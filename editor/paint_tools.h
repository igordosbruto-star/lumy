/**
 * PaintTools - Sistema de ferramentas de pintura para o editor
 * Define enums, estruturas e classes base para pincel, preenchimento e seleção
 */

#pragma once

#include <wx/wx.h>
#include <vector>
#include <functional>

// Forward declarations
class Map;

// Tipos de ferramentas disponíveis
enum class PaintTool
{
    BRUSH,      // Pincel - pinta tiles individuais
    BUCKET,     // Balde - preenchimento de área
    SELECTION,  // Seleção - seleciona área retangular
    ERASER      // Borracha - apaga tiles
};

// Estrutura para posição de tile
struct TilePosition
{
    int x;
    int y;
    
    TilePosition() : x(0), y(0) {}
    TilePosition(int x_, int y_) : x(x_), y(y_) {}
    
    bool operator==(const TilePosition& other) const {
        return x == other.x && y == other.y;
    }
    
    bool operator!=(const TilePosition& other) const {
        return !(*this == other);
    }
};

// Estrutura para área de seleção
struct SelectionArea
{
    TilePosition start;
    TilePosition end;
    bool active;
    
    SelectionArea() : active(false) {}
    
    bool IsValid() const { return active; }
    
    int GetWidth() const {
        return abs(end.x - start.x) + 1;
    }
    
    int GetHeight() const {
        return abs(end.y - start.y) + 1;
    }
    
    TilePosition GetTopLeft() const {
        return TilePosition(
            std::min(start.x, end.x),
            std::min(start.y, end.y)
        );
    }
    
    TilePosition GetBottomRight() const {
        return TilePosition(
            std::max(start.x, end.x),
            std::max(start.y, end.y)
        );
    }
    
    bool Contains(int x, int y) const {
        if (!active) return false;
        TilePosition topLeft = GetTopLeft();
        TilePosition bottomRight = GetBottomRight();
        return x >= topLeft.x && x <= bottomRight.x &&
               y >= topLeft.y && y <= bottomRight.y;
    }
};

// Interface base para ferramentas de pintura
class IPaintTool
{
public:
    virtual ~IPaintTool() = default;
    
    // Eventos de mouse
    virtual void OnMouseDown(const TilePosition& pos, int tileId, Map* map) = 0;
    virtual void OnMouseMove(const TilePosition& pos, int tileId, Map* map) = 0;
    virtual void OnMouseUp(const TilePosition& pos, int tileId, Map* map) = 0;
    
    // Nome da ferramenta
    virtual wxString GetName() const = 0;
    virtual PaintTool GetType() const = 0;
    
    // Estado
    virtual bool IsActive() const { return m_isActive; }
    virtual void SetActive(bool active) { m_isActive = active; }
    
protected:
    bool m_isActive = false;
};

// Ferramenta Pincel
class BrushTool : public IPaintTool
{
public:
    void OnMouseDown(const TilePosition& pos, int tileId, Map* map) override;
    void OnMouseMove(const TilePosition& pos, int tileId, Map* map) override;
    void OnMouseUp(const TilePosition& pos, int tileId, Map* map) override;
    
    wxString GetName() const override { return "Pincel"; }
    PaintTool GetType() const override { return PaintTool::BRUSH; }
    
private:
    bool m_isPainting = false;
    TilePosition m_lastPos;
};

// Ferramenta Balde (Preenchimento)
class BucketTool : public IPaintTool
{
public:
    void OnMouseDown(const TilePosition& pos, int tileId, Map* map) override;
    void OnMouseMove(const TilePosition& pos, int tileId, Map* map) override;
    void OnMouseUp(const TilePosition& pos, int tileId, Map* map) override;
    
    wxString GetName() const override { return "Preenchimento"; }
    PaintTool GetType() const override { return PaintTool::BUCKET; }
    
private:
    void FloodFill(const TilePosition& start, int oldTile, int newTile, Map* map);
};

// Ferramenta Seleção
class SelectionTool : public IPaintTool
{
public:
    void OnMouseDown(const TilePosition& pos, int tileId, Map* map) override;
    void OnMouseMove(const TilePosition& pos, int tileId, Map* map) override;
    void OnMouseUp(const TilePosition& pos, int tileId, Map* map) override;
    
    wxString GetName() const override { return "Seleção"; }
    PaintTool GetType() const override { return PaintTool::SELECTION; }
    
    const SelectionArea& GetSelection() const { return m_selection; }
    void ClearSelection() { m_selection.active = false; }
    
private:
    SelectionArea m_selection;
    bool m_isSelecting = false;
};

// Ferramenta Borracha
class EraserTool : public IPaintTool
{
public:
    void OnMouseDown(const TilePosition& pos, int tileId, Map* map) override;
    void OnMouseMove(const TilePosition& pos, int tileId, Map* map) override;
    void OnMouseUp(const TilePosition& pos, int tileId, Map* map) override;
    
    wxString GetName() const override { return "Borracha"; }
    PaintTool GetType() const override { return PaintTool::ERASER; }
    
private:
    bool m_isErasing = false;
};