/**
 * Implementação da classe LayerManager
 */

#pragma execution_character_set("utf-8")

#include "layer_manager.h"
#include "utf8_strings.h"
#include <nlohmann/json.hpp>
#include <algorithm>

LayerManager::LayerManager()
    : m_activeLayerIndex(-1)
    , m_nextLayerId(1)
{
    // Criar layer padrão
    LayerProperties defaultProps;
    defaultProps.name = "Fundo";
    defaultProps.type = LayerType::BACKGROUND_LAYER;
    CreateLayer(25, 15, defaultProps);
}

Layer* LayerManager::CreateLayer(const LayerProperties& properties)
{
    return CreateLayer(25, 15, properties); // Tamanho padrão
}

Layer* LayerManager::CreateLayer(int width, int height, const LayerProperties& properties)
{
    auto layer = std::make_unique<Layer>(width, height, properties);
    Layer* layerPtr = layer.get();
    
    if (AddLayer(std::move(layer))) {
        return layerPtr;
    }
    
    return nullptr;
}

bool LayerManager::AddLayer(std::unique_ptr<Layer> layer)
{
    if (!layer) {
        return false;
    }
    
    // Definir Z-order baseado na posição
    LayerProperties props = layer->GetProperties();
    props.zOrder = static_cast<int>(m_layers.size());
    layer->SetProperties(props);
    
    m_layers.push_back(std::move(layer));
    
    // Se é o primeiro layer, torná-lo ativo
    if (m_activeLayerIndex == -1) {
        m_activeLayerIndex = 0;
    }
    
    wxLogMessage("Layer adicionado: %s (total: %zu)", props.name, m_layers.size());
    return true;
}

bool LayerManager::RemoveLayer(int index)
{
    if (!IsValidLayerIndex(index)) {
        return false;
    }
    
    wxString layerName = m_layers[index]->GetName();
    m_layers.erase(m_layers.begin() + index);
    
    // Ajustar índice ativo
    if (m_activeLayerIndex >= index) {
        m_activeLayerIndex--;
        if (m_activeLayerIndex < 0 && !m_layers.empty()) {
            m_activeLayerIndex = 0;
        } else if (m_layers.empty()) {
            m_activeLayerIndex = -1;
        }
    }
    
    // Reordenar z-orders
    UpdateLayerZOrders();
    
    wxLogMessage("Layer removido: %s", layerName);
    return true;
}

bool LayerManager::RemoveLayer(const wxString& name)
{
    int index = FindLayerIndex(name);
    if (index >= 0) {
        return RemoveLayer(index);
    }
    return false;
}

void LayerManager::ClearLayers()
{
    m_layers.clear();
    m_activeLayerIndex = -1;
    m_nextLayerId = 1;
    wxLogMessage("Todos os layers removidos");
}

Layer* LayerManager::GetLayer(int index)
{
    if (!IsValidLayerIndex(index)) {
        return nullptr;
    }
    return m_layers[index].get();
}

const Layer* LayerManager::GetLayer(int index) const
{
    if (!IsValidLayerIndex(index)) {
        return nullptr;
    }
    return m_layers[index].get();
}

Layer* LayerManager::GetLayer(const wxString& name)
{
    int index = FindLayerIndex(name);
    return GetLayer(index);
}

const Layer* LayerManager::GetLayer(const wxString& name) const
{
    int index = FindLayerIndex(name);
    return GetLayer(index);
}

bool LayerManager::SetActiveLayer(int index)
{
    if (!IsValidLayerIndex(index)) {
        return false;
    }
    
    m_activeLayerIndex = index;
    wxLogMessage("Layer ativo alterado para: %s", m_layers[index]->GetName());
    return true;
}

bool LayerManager::SetActiveLayer(const wxString& name)
{
    int index = FindLayerIndex(name);
    if (index >= 0) {
        return SetActiveLayer(index);
    }
    return false;
}

bool LayerManager::MoveLayerUp(int index)
{
    if (!IsValidLayerIndex(index) || index == 0) {
        return false;
    }
    
    std::swap(m_layers[index], m_layers[index - 1]);
    
    // Ajustar índice ativo
    if (m_activeLayerIndex == index) {
        m_activeLayerIndex = index - 1;
    } else if (m_activeLayerIndex == index - 1) {
        m_activeLayerIndex = index;
    }
    
    UpdateLayerZOrders();
    return true;
}

bool LayerManager::MoveLayerDown(int index)
{
    if (!IsValidLayerIndex(index) || index >= GetLayerCount() - 1) {
        return false;
    }
    
    std::swap(m_layers[index], m_layers[index + 1]);
    
    // Ajustar índice ativo
    if (m_activeLayerIndex == index) {
        m_activeLayerIndex = index + 1;
    } else if (m_activeLayerIndex == index + 1) {
        m_activeLayerIndex = index;
    }
    
    UpdateLayerZOrders();
    return true;
}

bool LayerManager::SetLayerOrder(int index, int newOrder)
{
    if (!IsValidLayerIndex(index)) {
        return false;
    }
    
    newOrder = std::clamp(newOrder, 0, GetLayerCount() - 1);
    
    if (index == newOrder) {
        return true; // Já na posição correta
    }
    
    // Mover layer para nova posição
    auto layer = std::move(m_layers[index]);
    m_layers.erase(m_layers.begin() + index);
    m_layers.insert(m_layers.begin() + newOrder, std::move(layer));
    
    // Ajustar índice ativo
    if (m_activeLayerIndex == index) {
        m_activeLayerIndex = newOrder;
    } else if (index < newOrder && m_activeLayerIndex > index && m_activeLayerIndex <= newOrder) {
        m_activeLayerIndex--;
    } else if (index > newOrder && m_activeLayerIndex >= newOrder && m_activeLayerIndex < index) {
        m_activeLayerIndex++;
    }
    
    UpdateLayerZOrders();
    return true;
}

std::vector<Layer*> LayerManager::GetLayersSortedByZOrder() const
{
    std::vector<Layer*> layers;
    layers.reserve(m_layers.size());
    
    for (const auto& layer : m_layers) {
        layers.push_back(layer.get());
    }
    
    // Ordenar por Z-order (menor primeiro = mais atrás)
    std::sort(layers.begin(), layers.end(), [](const Layer* a, const Layer* b) {
        return a->GetZOrder() < b->GetZOrder();
    });
    
    return layers;
}

void LayerManager::SetAllLayersVisible(bool visible)
{
    for (auto& layer : m_layers) {
        layer->SetVisible(visible);
    }
}

void LayerManager::SetLayerVisibility(int index, bool visible)
{
    Layer* layer = GetLayer(index);
    if (layer) {
        layer->SetVisible(visible);
    }
}

void LayerManager::SetLayerOpacity(int index, float opacity)
{
    Layer* layer = GetLayer(index);
    if (layer) {
        layer->SetOpacity(opacity);
    }
}

std::vector<int> LayerManager::FindLayersByType(LayerType type) const
{
    std::vector<int> indices;
    for (size_t i = 0; i < m_layers.size(); ++i) {
        if (m_layers[i]->GetType() == type) {
            indices.push_back(static_cast<int>(i));
        }
    }
    return indices;
}

std::vector<int> LayerManager::FindLayersByName(const wxString& pattern) const
{
    std::vector<int> indices;
    wxString lowerPattern = pattern.Lower();
    
    for (size_t i = 0; i < m_layers.size(); ++i) {
        if (m_layers[i]->GetName().Lower().Contains(lowerPattern)) {
            indices.push_back(static_cast<int>(i));
        }
    }
    return indices;
}

std::vector<int> LayerManager::GetVisibleLayers() const
{
    std::vector<int> indices;
    for (size_t i = 0; i < m_layers.size(); ++i) {
        if (m_layers[i]->IsVisible()) {
            indices.push_back(static_cast<int>(i));
        }
    }
    return indices;
}

std::vector<int> LayerManager::GetUnlockedLayers() const
{
    std::vector<int> indices;
    for (size_t i = 0; i < m_layers.size(); ++i) {
        if (!m_layers[i]->IsLocked()) {
            indices.push_back(static_cast<int>(i));
        }
    }
    return indices;
}

// Operações de tiles

int LayerManager::GetTile(int x, int y) const
{
    const Layer* activeLayer = GetActiveLayer();
    if (activeLayer) {
        return activeLayer->GetTile(x, y);
    }
    return -1;
}

void LayerManager::SetTile(int x, int y, int tileId)
{
    Layer* activeLayer = GetActiveLayer();
    if (activeLayer && !activeLayer->IsLocked()) {
        activeLayer->SetTile(x, y, tileId);
    }
}

int LayerManager::GetTileFromLayer(int layerIndex, int x, int y) const
{
    const Layer* layer = GetLayer(layerIndex);
    if (layer) {
        return layer->GetTile(x, y);
    }
    return -1;
}

void LayerManager::SetTileInLayer(int layerIndex, int x, int y, int tileId)
{
    Layer* layer = GetLayer(layerIndex);
    if (layer && !layer->IsLocked()) {
        layer->SetTile(x, y, tileId);
    }
}

void LayerManager::FillRect(int x, int y, int width, int height, int tileId)
{
    Layer* activeLayer = GetActiveLayer();
    if (activeLayer && !activeLayer->IsLocked()) {
        activeLayer->FillRect(x, y, width, height, tileId);
    }
}

void LayerManager::CopyRect(int srcX, int srcY, int width, int height, int srcLayer, int destLayer, int destX, int destY)
{
    Layer* srcLayerPtr = GetLayer(srcLayer);
    Layer* destLayerPtr = GetLayer(destLayer);
    
    if (srcLayerPtr && destLayerPtr && !destLayerPtr->IsLocked()) {
        srcLayerPtr->CopyRect(srcX, srcY, width, height, *destLayerPtr, destX, destY);
    }
}

// Operações especiais

Layer* LayerManager::DuplicateLayer(int index)
{
    const Layer* sourceLayer = GetLayer(index);
    if (!sourceLayer) {
        return nullptr;
    }
    
    // Criar cópia do layer
    auto duplicatedLayer = std::make_unique<Layer>(*sourceLayer);
    
    // Gerar nome único
    LayerProperties props = duplicatedLayer->GetProperties();
    props.name = GenerateUniqueLayerName(props.name + " Cópia");
    duplicatedLayer->SetProperties(props);
    
    Layer* layerPtr = duplicatedLayer.get();
    
    // Inserir após o layer original
    if (AddLayer(std::move(duplicatedLayer))) {
        // Mover para posição correta (após o original)
        SetLayerOrder(GetLayerCount() - 1, index + 1);
        return layerPtr;
    }
    
    return nullptr;
}

bool LayerManager::MergeLayerDown(int index)
{
    if (!IsValidLayerIndex(index) || index >= GetLayerCount() - 1) {
        return false;
    }
    
    Layer* upperLayer = GetLayer(index);
    Layer* lowerLayer = GetLayer(index + 1);
    
    if (!upperLayer || !lowerLayer) {
        return false;
    }
    
    // Combinar tiles (tiles do upper sobrescrevem os do lower se não forem vazios)
    int width = std::min(upperLayer->GetWidth(), lowerLayer->GetWidth());
    int height = std::min(upperLayer->GetHeight(), lowerLayer->GetHeight());
    
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int upperTile = upperLayer->GetTile(x, y);
            if (upperTile != -1) { // Se não é tile vazio
                lowerLayer->SetTile(x, y, upperTile);
            }
        }
    }
    
    // Remover layer superior
    return RemoveLayer(index);
}

void LayerManager::FlattenLayers()
{
    std::vector<int> visibleLayers = GetVisibleLayers();
    if (visibleLayers.size() <= 1) {
        return; // Não há nada para achatar
    }
    
    // Criar layer combinado
    LayerProperties flattenedProps;
    flattenedProps.name = GenerateUniqueLayerName("Achatado");
    flattenedProps.type = LayerType::TILE_LAYER;
    
    Layer* flattenedLayer = CreateLayer(GetWidth(), GetHeight(), flattenedProps);
    if (!flattenedLayer) {
        return;
    }
    
    // Combinar layers em ordem de Z (do fundo para frente)
    auto sortedLayers = GetLayersSortedByZOrder();
    for (const Layer* layer : sortedLayers) {
        if (!layer->IsVisible()) {
            continue;
        }
        
        for (int y = 0; y < layer->GetHeight(); ++y) {
            for (int x = 0; x < layer->GetWidth(); ++x) {
                int tile = layer->GetTile(x, y);
                if (tile != -1) { // Se não é tile vazio
                    flattenedLayer->SetTile(x, y, tile);
                }
            }
        }
    }
    
    // Remover todos os outros layers
    for (int i = GetLayerCount() - 1; i >= 0; --i) {
        if (GetLayer(i) != flattenedLayer) {
            RemoveLayer(i);
        }
    }
    
    wxLogMessage("Layers achatados em um único layer");
}

// Dimensões

void LayerManager::ResizeAllLayers(int newWidth, int newHeight, int defaultTile)
{
    for (auto& layer : m_layers) {
        layer->Resize(newWidth, newHeight, defaultTile);
    }
}

int LayerManager::GetWidth() const
{
    if (m_layers.empty()) {
        return 0;
    }
    return m_layers[0]->GetWidth();
}

int LayerManager::GetHeight() const
{
    if (m_layers.empty()) {
        return 0;
    }
    return m_layers[0]->GetHeight();
}

// Estado

bool LayerManager::HasUnsavedChanges() const
{
    for (const auto& layer : m_layers) {
        if (layer->IsModified()) {
            return true;
        }
    }
    return false;
}

void LayerManager::MarkAllLayersSaved()
{
    for (auto& layer : m_layers) {
        layer->MarkSaved();
    }
}

// Utilitários

wxString LayerManager::GenerateUniqueLayerName(const wxString& baseName) const
{
    wxString uniqueName = baseName;
    int counter = 1;
    
    while (FindLayerIndex(uniqueName) >= 0) {
        uniqueName = wxString::Format("%s %d", baseName, counter);
        counter++;
    }
    
    return uniqueName;
}

bool LayerManager::IsValidLayerIndex(int index) const
{
    return index >= 0 && index < GetLayerCount();
}

void LayerManager::PrintDebugInfo() const
{
    wxLogMessage("=== LayerManager Debug Info ===");
    wxLogMessage("Total de layers: %d", GetLayerCount());
    wxLogMessage("Layer ativo: %d", m_activeLayerIndex);
    
    for (int i = 0; i < GetLayerCount(); ++i) {
        const Layer* layer = GetLayer(i);
        wxLogMessage("Layer %d: %s (Z:%d, Vis:%s, Lock:%s)", 
                     i, layer->GetName(), layer->GetZOrder(),
                     layer->IsVisible() ? "Sim" : "Não",
                     layer->IsLocked() ? "Sim" : "Não");
    }
    
    wxLogMessage("===============================");
}

// Métodos privados

void LayerManager::UpdateLayerZOrders()
{
    for (size_t i = 0; i < m_layers.size(); ++i) {
        LayerProperties props = m_layers[i]->GetProperties();
        props.zOrder = static_cast<int>(i);
        m_layers[i]->SetProperties(props);
    }
}

int LayerManager::FindLayerIndex(const wxString& name) const
{
    for (size_t i = 0; i < m_layers.size(); ++i) {
        if (m_layers[i]->GetName() == name) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

// Serialização

bool LayerManager::LoadFromJson(const wxString& jsonData)
{
    try {
        nlohmann::json root = nlohmann::json::parse(jsonData.ToUTF8().data());
        
        // Limpar layers existentes
        ClearLayers();
        
        // Carregar configurações globais
        m_activeLayerIndex = root.value("activeLayer", -1);
        m_nextLayerId = root.value("nextLayerId", 1);
        
        // Carregar layers
        if (root.contains("layers") && root["layers"].is_array()) {
            for (const auto& layerJson : root["layers"]) {
                auto layer = std::make_unique<Layer>();
                
                // Serializar layer individual
                std::string layerJsonStr = layerJson.dump();
                if (layer->LoadFromJson(wxString::FromUTF8(layerJsonStr.c_str()))) {
                    AddLayer(std::move(layer));
                }
            }
        }
        
        // Validar índice ativo
        if (!IsValidLayerIndex(m_activeLayerIndex)) {
            m_activeLayerIndex = GetLayerCount() > 0 ? 0 : -1;
        }
        
        // Reordenar por Z-order
        UpdateLayerZOrders();
        
        wxLogMessage("LayerManager carregado: %d layers", GetLayerCount());
        return true;
        
    } catch (const std::exception& e) {
        wxLogError("Erro ao carregar LayerManager do JSON: %s", e.what());
        return false;
    }
}

wxString LayerManager::SaveToJson() const
{
    try {
        nlohmann::json root;
        
        // Salvar configurações globais
        root["activeLayer"] = m_activeLayerIndex;
        root["nextLayerId"] = m_nextLayerId;
        
        // Salvar layers
        nlohmann::json layersArray = nlohmann::json::array();
        
        for (const auto& layer : m_layers) {
            wxString layerJsonStr = layer->SaveToJson();
            if (!layerJsonStr.IsEmpty()) {
                nlohmann::json layerJson = nlohmann::json::parse(layerJsonStr.ToUTF8().data());
                layersArray.push_back(layerJson);
            }
        }
        
        root["layers"] = layersArray;
        
        std::string jsonString = root.dump(2);
        return wxString::FromUTF8(jsonString.c_str());
        
    } catch (const std::exception& e) {
        wxLogError("Erro ao salvar LayerManager para JSON: %s", e.what());
        return wxEmptyString;
    }
}
