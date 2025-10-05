/**
 * Layer - Classe para representar uma camada de tiles do mapa
 */

#pragma once

#include <wx/wx.h>
#include <vector>
#include <string>

enum class LayerType {
    TILE_LAYER,      // Camada de tiles normais
    BACKGROUND_LAYER, // Camada de fundo
    COLLISION_LAYER,  // Camada de colisão
    OBJECT_LAYER,     // Camada de objetos
    OVERLAY_LAYER     // Camada de sobreposição
};

struct LayerProperties {
    wxString name;
    LayerType type;
    bool visible;
    float opacity;
    bool locked;
    wxColour tintColor;
    int zOrder;         // Ordem de renderização (menor = mais atrás)
    wxString description;
    
    LayerProperties()
        : name("Nova Camada")
        , type(LayerType::TILE_LAYER)
        , visible(true)
        , opacity(1.0f)
        , locked(false)
        , tintColor(255, 255, 255, 255)
        , zOrder(0)
        , description("")
    {}
};

class Layer {
public:
    Layer();
    Layer(int width, int height, const LayerProperties& properties = LayerProperties());
    Layer(const Layer& other);
    Layer& operator=(const Layer& other);
    ~Layer() = default;
    
    // Getters
    const LayerProperties& GetProperties() const { return m_properties; }
    LayerType GetType() const { return m_properties.type; }
    const wxString& GetName() const { return m_properties.name; }
    bool IsVisible() const { return m_properties.visible; }
    float GetOpacity() const { return m_properties.opacity; }
    bool IsLocked() const { return m_properties.locked; }
    int GetZOrder() const { return m_properties.zOrder; }
    wxColour GetTintColor() const { return m_properties.tintColor; }
    
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    int GetTile(int x, int y) const;
    
    // Setters
    void SetProperties(const LayerProperties& properties);
    void SetName(const wxString& name) { m_properties.name = name; }
    void SetType(LayerType type) { m_properties.type = type; }
    void SetVisible(bool visible) { m_properties.visible = visible; }
    void SetOpacity(float opacity);
    void SetLocked(bool locked) { m_properties.locked = locked; }
    void SetZOrder(int zOrder) { m_properties.zOrder = zOrder; }
    void SetTintColor(const wxColour& color) { m_properties.tintColor = color; }
    void SetDescription(const wxString& description) { m_properties.description = description; }
    
    void SetTile(int x, int y, int tileId);
    void SetSize(int width, int height);
    
    // Operações
    void Clear();
    void Fill(int tileId);
    void Resize(int newWidth, int newHeight, int defaultTile = -1);
    bool IsValidPosition(int x, int y) const;
    
    // Operações de área
    void FillRect(int x, int y, int width, int height, int tileId);
    void CopyRect(int srcX, int srcY, int width, int height, Layer& destLayer, int destX, int destY);
    std::vector<std::vector<int>> GetRect(int x, int y, int width, int height) const;
    void SetRect(int x, int y, const std::vector<std::vector<int>>& data);
    
    // Serialização
    bool LoadFromJson(const wxString& jsonData);
    wxString SaveToJson() const;
    
    // Estado
    bool IsModified() const { return m_modified; }
    void SetModified(bool modified = true) { m_modified = modified; }
    void MarkSaved() { m_modified = false; }
    
    // Debug
    void PrintDebugInfo() const;

private:
    void InitializeTileData();
    void UpdateModifiedTime();
    
    LayerProperties m_properties;
    int m_width;
    int m_height;
    std::vector<std::vector<int>> m_tiles;
    bool m_modified;
    wxString m_createdDate;
    wxString m_modifiedDate;
};