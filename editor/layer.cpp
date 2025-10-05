/**
 * Implementação da classe Layer
 */

#pragma execution_character_set("utf-8")

#include "layer.h"
#include "utf8_strings.h"
#include <wx/datetime.h>
#include <nlohmann/json.hpp>
#include <algorithm>

Layer::Layer()
    : m_properties(LayerProperties())
    , m_width(25)
    , m_height(15)
    , m_modified(false)
{
    wxDateTime now = wxDateTime::Now();
    m_createdDate = now.Format("%Y-%m-%d %H:%M:%S");
    m_modifiedDate = m_createdDate;
    
    InitializeTileData();
}

Layer::Layer(int width, int height, const LayerProperties& properties)
    : m_properties(properties)
    , m_width(width)
    , m_height(height)
    , m_modified(false)
{
    wxDateTime now = wxDateTime::Now();
    m_createdDate = now.Format("%Y-%m-%d %H:%M:%S");
    m_modifiedDate = m_createdDate;
    
    InitializeTileData();
}

Layer::Layer(const Layer& other)
    : m_properties(other.m_properties)
    , m_width(other.m_width)
    , m_height(other.m_height)
    , m_tiles(other.m_tiles)
    , m_modified(other.m_modified)
    , m_createdDate(other.m_createdDate)
    , m_modifiedDate(other.m_modifiedDate)
{
}

Layer& Layer::operator=(const Layer& other)
{
    if (this != &other) {
        m_properties = other.m_properties;
        m_width = other.m_width;
        m_height = other.m_height;
        m_tiles = other.m_tiles;
        m_modified = other.m_modified;
        m_createdDate = other.m_createdDate;
        m_modifiedDate = other.m_modifiedDate;
    }
    return *this;
}

int Layer::GetTile(int x, int y) const
{
    if (!IsValidPosition(x, y)) {
        return -1; // Tile inválido
    }
    return m_tiles[y][x];
}

void Layer::SetProperties(const LayerProperties& properties)
{
    m_properties = properties;
    SetModified();
}

void Layer::SetOpacity(float opacity)
{
    m_properties.opacity = std::clamp(opacity, 0.0f, 1.0f);
    SetModified();
}

void Layer::SetTile(int x, int y, int tileId)
{
    if (!IsValidPosition(x, y)) {
        return;
    }
    
    if (m_tiles[y][x] != tileId) {
        m_tiles[y][x] = tileId;
        SetModified();
    }
}

void Layer::SetSize(int width, int height)
{
    if (width != m_width || height != m_height) {
        Resize(width, height);
    }
}

void Layer::Clear()
{
    Fill(-1); // -1 representa tile vazio
}

void Layer::Fill(int tileId)
{
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            m_tiles[y][x] = tileId;
        }
    }
    SetModified();
}

void Layer::Resize(int newWidth, int newHeight, int defaultTile)
{
    if (newWidth <= 0 || newHeight <= 0) {
        wxLogError("Dimensões de layer inválidas: %dx%d", newWidth, newHeight);
        return;
    }
    
    // Criar novo array de tiles
    std::vector<std::vector<int>> newTiles(newHeight, std::vector<int>(newWidth, defaultTile));
    
    // Copiar dados existentes (se houver sobreposição)
    int copyWidth = std::min(newWidth, m_width);
    int copyHeight = std::min(newHeight, m_height);
    
    for (int y = 0; y < copyHeight; ++y) {
        for (int x = 0; x < copyWidth; ++x) {
            newTiles[y][x] = m_tiles[y][x];
        }
    }
    
    // Substituir dados
    m_tiles = std::move(newTiles);
    m_width = newWidth;
    m_height = newHeight;
    
    SetModified();
}

bool Layer::IsValidPosition(int x, int y) const
{
    return x >= 0 && x < m_width && y >= 0 && y < m_height;
}

void Layer::FillRect(int x, int y, int width, int height, int tileId)
{
    for (int dy = 0; dy < height; ++dy) {
        for (int dx = 0; dx < width; ++dx) {
            int px = x + dx;
            int py = y + dy;
            if (IsValidPosition(px, py)) {
                m_tiles[py][px] = tileId;
            }
        }
    }
    SetModified();
}

void Layer::CopyRect(int srcX, int srcY, int width, int height, Layer& destLayer, int destX, int destY)
{
    for (int dy = 0; dy < height; ++dy) {
        for (int dx = 0; dx < width; ++dx) {
            int srcPx = srcX + dx;
            int srcPy = srcY + dy;
            int destPx = destX + dx;
            int destPy = destY + dy;
            
            if (IsValidPosition(srcPx, srcPy) && destLayer.IsValidPosition(destPx, destPy)) {
                destLayer.SetTile(destPx, destPy, GetTile(srcPx, srcPy));
            }
        }
    }
}

std::vector<std::vector<int>> Layer::GetRect(int x, int y, int width, int height) const
{
    std::vector<std::vector<int>> result(height, std::vector<int>(width, -1));
    
    for (int dy = 0; dy < height; ++dy) {
        for (int dx = 0; dx < width; ++dx) {
            int px = x + dx;
            int py = y + dy;
            if (IsValidPosition(px, py)) {
                result[dy][dx] = m_tiles[py][px];
            }
        }
    }
    
    return result;
}

void Layer::SetRect(int x, int y, const std::vector<std::vector<int>>& data)
{
    int height = static_cast<int>(data.size());
    if (height == 0) return;
    
    int width = static_cast<int>(data[0].size());
    if (width == 0) return;
    
    for (int dy = 0; dy < height; ++dy) {
        for (int dx = 0; dx < width; ++dx) {
            int px = x + dx;
            int py = y + dy;
            if (IsValidPosition(px, py)) {
                m_tiles[py][px] = data[dy][dx];
            }
        }
    }
    SetModified();
}

bool Layer::LoadFromJson(const wxString& jsonData)
{
    try {
        nlohmann::json root = nlohmann::json::parse(jsonData.ToUTF8().data());
        
        // Carregar propriedades
        if (root.contains("properties")) {
            auto props = root["properties"];
            m_properties.name = wxString::FromUTF8(props.value("name", "Nova Camada").c_str());
            m_properties.type = static_cast<LayerType>(props.value("type", 0));
            m_properties.visible = props.value("visible", true);
            m_properties.opacity = props.value("opacity", 1.0f);
            m_properties.locked = props.value("locked", false);
            m_properties.zOrder = props.value("zOrder", 0);
            m_properties.description = wxString::FromUTF8(props.value("description", "").c_str());
            
            // Carregar cor de tinta
            if (props.contains("tintColor")) {
                auto color = props["tintColor"];
                m_properties.tintColor = wxColour(
                    color.value("r", 255),
                    color.value("g", 255),
                    color.value("b", 255),
                    color.value("a", 255)
                );
            }
        }
        
        // Carregar dimensões
        m_width = root.value("width", 25);
        m_height = root.value("height", 15);
        
        // Carregar dados dos tiles
        if (root.contains("tiles")) {
            auto tilesArray = root["tiles"];
            if (tilesArray.is_array() && tilesArray.size() == static_cast<size_t>(m_height)) {
                m_tiles.resize(m_height);
                for (int y = 0; y < m_height; ++y) {
                    auto row = tilesArray[y];
                    if (row.is_array() && row.size() == static_cast<size_t>(m_width)) {
                        m_tiles[y].resize(m_width);
                        for (int x = 0; x < m_width; ++x) {
                            m_tiles[y][x] = row[x].get<int>();
                        }
                    }
                }
            } else {
                InitializeTileData();
            }
        } else {
            InitializeTileData();
        }
        
        // Carregar timestamps
        m_createdDate = wxString::FromUTF8(root.value("created", "").c_str());
        m_modifiedDate = wxString::FromUTF8(root.value("modified", "").c_str());
        
        m_modified = false;
        return true;
        
    } catch (const std::exception& e) {
        wxLogError("Erro ao carregar layer do JSON: %s", e.what());
        return false;
    }
}

wxString Layer::SaveToJson() const
{
    try {
        nlohmann::json root;
        
        // Salvar propriedades
        nlohmann::json properties;
        properties["name"] = m_properties.name.ToUTF8().data();
        properties["type"] = static_cast<int>(m_properties.type);
        properties["visible"] = m_properties.visible;
        properties["opacity"] = m_properties.opacity;
        properties["locked"] = m_properties.locked;
        properties["zOrder"] = m_properties.zOrder;
        properties["description"] = m_properties.description.ToUTF8().data();
        
        // Salvar cor de tinta
        nlohmann::json tintColor;
        tintColor["r"] = m_properties.tintColor.Red();
        tintColor["g"] = m_properties.tintColor.Green();
        tintColor["b"] = m_properties.tintColor.Blue();
        tintColor["a"] = m_properties.tintColor.Alpha();
        properties["tintColor"] = tintColor;
        
        root["properties"] = properties;
        
        // Salvar dimensões
        root["width"] = m_width;
        root["height"] = m_height;
        
        // Salvar dados dos tiles
        nlohmann::json tilesArray = nlohmann::json::array();
        for (int y = 0; y < m_height; ++y) {
            nlohmann::json row = nlohmann::json::array();
            for (int x = 0; x < m_width; ++x) {
                row.push_back(m_tiles[y][x]);
            }
            tilesArray.push_back(row);
        }
        root["tiles"] = tilesArray;
        
        // Salvar timestamps
        root["created"] = m_createdDate.ToUTF8().data();
        root["modified"] = m_modifiedDate.ToUTF8().data();
        
        std::string jsonString = root.dump(2);
        return wxString::FromUTF8(jsonString.c_str());
        
    } catch (const std::exception& e) {
        wxLogError("Erro ao salvar layer para JSON: %s", e.what());
        return wxEmptyString;
    }
}

void Layer::PrintDebugInfo() const
{
    wxLogMessage("=== Layer Debug Info ===");
    wxLogMessage("Nome: %s", m_properties.name);
    wxLogMessage("Tipo: %d", static_cast<int>(m_properties.type));
    wxLogMessage("Dimensões: %dx%d", m_width, m_height);
    wxLogMessage("Visível: %s", m_properties.visible ? "Sim" : "Não");
    wxLogMessage("Opacidade: %.2f", m_properties.opacity);
    wxLogMessage("Bloqueado: %s", m_properties.locked ? "Sim" : "Não");
    wxLogMessage("Z-Order: %d", m_properties.zOrder);
    wxLogMessage("Modificado: %s", m_modified ? "Sim" : "Não");
    wxLogMessage("Criado: %s", m_createdDate);
    wxLogMessage("Modificado: %s", m_modifiedDate);
    wxLogMessage("========================");
}

// Métodos privados

void Layer::InitializeTileData()
{
    m_tiles.resize(m_height);
    for (int y = 0; y < m_height; ++y) {
        m_tiles[y].resize(m_width, -1); // -1 = tile vazio
    }
}

void Layer::UpdateModifiedTime()
{
    wxDateTime now = wxDateTime::Now();
    m_modifiedDate = now.Format("%Y-%m-%d %H:%M:%S");
}