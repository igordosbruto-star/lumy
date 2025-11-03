/**
 * Implementação da classe Map
 */

#pragma execution_character_set("utf-8")

#include "map.h"
#include "utf8_strings.h"
#include <wx/datetime.h>
#include <wx/ffile.h>
#include <nlohmann/json.hpp>
#include <map>

// Construtor padrão
Map::Map()
    : m_layerManager(std::make_unique<LayerManager>())
    , m_modified(false)
{
    m_metadata = MapMetadata(); // Usar valores padrão
    InitializeDefaultLayers();
    m_modified = false; // Garantir que inicia limpo
    if (m_layerManager) {
        m_layerManager->MarkAllLayersSaved();
    }
}

// Construtor com dimensões específicas
Map::Map(int width, int height, int tileSize)
    : m_layerManager(std::make_unique<LayerManager>())
    , m_modified(false)
{
    m_metadata = MapMetadata();
    m_metadata.width = width;
    m_metadata.height = height;
    m_metadata.tileSize = tileSize;
    InitializeDefaultLayers();
    m_modified = false; // Garantir que inicia limpo
    if (m_layerManager) {
        m_layerManager->MarkAllLayersSaved();
    }
}

// Construtor de cópia
Map::Map(const Map& other)
    : m_metadata(other.m_metadata)
    , m_layerManager(std::make_unique<LayerManager>())
    , m_modified(other.m_modified)
{
    // Copiar layers do LayerManager
    if (other.m_layerManager) {
        wxString layerData = other.m_layerManager->SaveToJson();
        m_layerManager->LoadFromJson(layerData);
    }
}

// Operador de atribuição
Map& Map::operator=(const Map& other)
{
    if (this != &other) {
        m_metadata = other.m_metadata;
        m_modified = other.m_modified;
        
        // Copiar layers do LayerManager
        if (!m_layerManager) {
            m_layerManager = std::make_unique<LayerManager>();
        }
        
        if (other.m_layerManager) {
            wxString layerData = other.m_layerManager->SaveToJson();
            m_layerManager->LoadFromJson(layerData);
        }
    }
    return *this;
}

// Getter para tile (delega ao layer ativo)
int Map::GetTile(int x, int y) const
{
    if (!m_layerManager || !IsValidPosition(x, y)) {
        return -1; // Tile inválido
    }
    return m_layerManager->GetTile(x, y);
}

// Setters
void Map::SetMetadata(const MapMetadata& metadata)
{
    m_metadata = metadata;
    SetModified();
}

void Map::SetName(const wxString& name)
{
    m_metadata.name = name;
    SetModified();
}

void Map::SetTile(int x, int y, int tileId)
{
    if (!m_layerManager || !IsValidPosition(x, y)) {
        return;
    }
    
    int oldTile = m_layerManager->GetTile(x, y);
    if (oldTile != tileId) {
        m_layerManager->SetTile(x, y, tileId);
        SetModified();
    }
}

void Map::SetSize(int width, int height)
{
    if (width != m_metadata.width || height != m_metadata.height) {
        Resize(width, height);
    }
}

// Operações
void Map::Clear()
{
    if (m_layerManager) {
        Layer* activeLayer = m_layerManager->GetActiveLayer();
        if (activeLayer) {
            activeLayer->Clear();
            SetModified();
        }
    }
}

void Map::Fill(int tileId)
{
    if (m_layerManager) {
        Layer* activeLayer = m_layerManager->GetActiveLayer();
        if (activeLayer) {
            activeLayer->Fill(tileId);
            SetModified();
        }
    }
}

void Map::Resize(int newWidth, int newHeight, int defaultTile)
{
    if (newWidth <= 0 || newHeight <= 0) {
        wxLogError("Dimensões de mapa inválidas: %dx%d", newWidth, newHeight);
        return;
    }
    
    // Redimensionar todos os layers
    if (m_layerManager) {
        m_layerManager->ResizeAllLayers(newWidth, newHeight, defaultTile);
    }
    
    // Atualizar metadados
    m_metadata.width = newWidth;
    m_metadata.height = newHeight;
    
    SetModified();
}

bool Map::IsValidPosition(int x, int y) const
{
    return x >= 0 && x < m_metadata.width && y >= 0 && y < m_metadata.height;
}

// Estado
bool Map::IsModified() const
{
    if (m_modified) {
        return true;
    }
    
    // Verificar se algum layer foi modificado
    if (m_layerManager && m_layerManager->HasUnsavedChanges()) {
        return true;
    }
    
    return false;
}

void Map::SetModified(bool modified)
{
    if (m_modified != modified) {
        m_modified = modified;
        if (modified) {
            UpdateModifiedTime();
        }
    }
}

void Map::MarkSaved()
{
    m_modified = false;
    
    // Marcar todos os layers como salvos
    if (m_layerManager) {
        m_layerManager->MarkAllLayersSaved();
    }
}

// Serialização
bool Map::LoadFromJson(const wxString& jsonData)
{
    try {
        // Parse JSON usando nlohmann/json
        nlohmann::json root = nlohmann::json::parse(jsonData.ToStdString());
        
        // Carregar metadados
        if (root.contains("metadata")) {
            auto metadata = root["metadata"];
            m_metadata.name = wxString::FromUTF8(metadata.value("name", "Mapa Carregado").c_str());
            m_metadata.author = wxString::FromUTF8(metadata.value("author", "Desconhecido").c_str());
            m_metadata.description = wxString::FromUTF8(metadata.value("description", "").c_str());
            m_metadata.version = wxString::FromUTF8(metadata.value("version", "1.0").c_str());
            m_metadata.width = metadata.value("width", 25);
            m_metadata.height = metadata.value("height", 15);
            m_metadata.tileSize = metadata.value("tileSize", 32);
            m_metadata.createdDate = wxString::FromUTF8(metadata.value("createdDate", "").c_str());
            m_metadata.modifiedDate = wxString::FromUTF8(metadata.value("modifiedDate", "").c_str());
        }
        
        // Validar dimensões
        if (m_metadata.width <= 0 || m_metadata.height <= 0 || m_metadata.tileSize <= 0) {
            wxLogError("Dimensões inválidas no mapa: %dx%d, tileSize=%d", 
                       m_metadata.width, m_metadata.height, m_metadata.tileSize);
            return false;
        }
        
        // Carregar dados dos layers e tiles
        if (root.contains("layers")) {
            // Formato novo com layers
            wxString layersJson = wxString::FromUTF8(root["layers"].dump().c_str());
            if (!m_layerManager->LoadFromJson(layersJson)) {
                wxLogError("Erro ao carregar layers do JSON");
                InitializeDefaultLayers();
            }
        } else if (root.contains("tiles")) {
            // Formato legado com tiles simples - converter para layers
            auto tilesArray = root["tiles"];
            
            if (tilesArray.is_array() && tilesArray.size() == static_cast<size_t>(m_metadata.height)) {
                // Inicializar layers padrão
                InitializeDefaultLayers();
                
                // Carregar tiles no layer base
                Layer* baseLayer = m_layerManager->GetLayer(0);
                if (baseLayer) {
                    for (int y = 0; y < m_metadata.height; ++y) {
                        auto row = tilesArray[y];
                        if (row.is_array() && row.size() == static_cast<size_t>(m_metadata.width)) {
                            for (int x = 0; x < m_metadata.width; ++x) {
                                baseLayer->SetTile(x, y, row[x].get<int>());
                            }
                        } else {
                            wxLogError("Linha %d do mapa tem tamanho incorreto", y);
                            return false;
                        }
                    }
                }
            } else {
                wxLogError("Array de tiles tem formato incorreto");
                return false;
            }
        } else {
            // Se não há dados de tiles ou layers, inicializar com padrão
            InitializeDefaultLayers();
        }
        
        m_modified = false;
        wxLogMessage("Mapa carregado com sucesso: %s (%dx%d)", 
                     m_metadata.name, m_metadata.width, m_metadata.height);
        return true;
        
    } catch (const nlohmann::json::exception& e) {
        wxLogError("Erro ao fazer parse do JSON do mapa: %s", e.what());
        return false;
    } catch (...) {
        wxLogError("Erro inesperado ao carregar mapa do JSON");
        return false;
    }
}

wxString Map::SaveToJson() const
{
    try {
        nlohmann::json root;
        
        // Salvar metadados
        nlohmann::json metadata;
        metadata["name"] = m_metadata.name.ToUTF8().data();
        metadata["author"] = m_metadata.author.ToUTF8().data();
        metadata["description"] = m_metadata.description.ToUTF8().data();
        metadata["version"] = m_metadata.version.ToUTF8().data();
        metadata["width"] = m_metadata.width;
        metadata["height"] = m_metadata.height;
        metadata["tileSize"] = m_metadata.tileSize;
        metadata["createdDate"] = m_metadata.createdDate.ToUTF8().data();
        metadata["modifiedDate"] = m_metadata.modifiedDate.ToUTF8().data();
        
        root["metadata"] = metadata;
        
        // Salvar dados dos layers
        if (m_layerManager) {
            wxString layersJson = m_layerManager->SaveToJson();
            if (!layersJson.IsEmpty()) {
                nlohmann::json layersData = nlohmann::json::parse(layersJson.ToUTF8().data());
                root["layers"] = layersData;
            } else {
                wxLogWarning("Falha ao serializar layers, salvando dados vázios");
                root["layers"] = nlohmann::json::array();
            }
        } else {
            // Se não há LayerManager, criar estrutura vazia
            root["layers"] = nlohmann::json::array();
        }
        
        // Converter para string JSON com formatação
        std::string jsonString = root.dump(2); // 2 espaços de indentação
        return wxString::FromUTF8(jsonString.c_str());
        
    } catch (const nlohmann::json::exception& e) {
        wxLogError("Erro ao serializar mapa para JSON: %s", e.what());
        return wxEmptyString;
    } catch (...) {
        wxLogError("Erro inesperado ao serializar mapa para JSON");
        return wxEmptyString;
    }
}

// Debug
void Map::PrintDebugInfo() const
{
    wxLogMessage("=== Informações do Mapa ===");
    wxLogMessage("Nome: %s", m_metadata.name);
    wxLogMessage("Autor: %s", m_metadata.author);
    wxLogMessage("Dimensões: %dx%d", m_metadata.width, m_metadata.height);
    wxLogMessage("Tamanho do Tile: %d", m_metadata.tileSize);
    wxLogMessage("Modificado: %s", m_modified ? "Sim" : "Não");
    wxLogMessage("Criado: %s", m_metadata.createdDate);
    wxLogMessage("Modificado: %s", m_metadata.modifiedDate);
    
    // Contar tipos de tiles do layer ativo
    std::map<int, int> tileCount;
    if (m_layerManager) {
        Layer* activeLayer = m_layerManager->GetActiveLayer();
        if (activeLayer) {
            for (int y = 0; y < m_metadata.height; ++y) {
                for (int x = 0; x < m_metadata.width; ++x) {
                    int tileId = activeLayer->GetTile(x, y);
                    tileCount[tileId]++;
                }
            }
        }
        
        wxLogMessage("Número de layers: %d", m_layerManager->GetLayerCount());
        wxLogMessage("Layer ativo: %s", activeLayer ? activeLayer->GetName() : wxString("Nenhum"));
        wxLogMessage("Contagem de tiles (layer ativo):");
        for (const auto& pair : tileCount) {
            wxLogMessage("  Tile %d: %d vezes", pair.first, pair.second);
        }
    } else {
        wxLogMessage("LayerManager não inicializado");
    }
    wxLogMessage("========================");
}

// Métodos privados
void Map::InitializeDefaultLayers()
{
    if (!m_layerManager) {
        m_layerManager = std::make_unique<LayerManager>();
    }
    
    // Criar layer padrão
    LayerProperties baseProps;
    baseProps.name = "Base";
    baseProps.type = LayerType::TILE_LAYER;
    baseProps.opacity = 1.0f;
    baseProps.visible = true;
    m_layerManager->CreateLayer(baseProps);
    
    // Redimensionar todos os layers com as dimensões atuais
    if (m_metadata.width > 0 && m_metadata.height > 0) {
        m_layerManager->ResizeAllLayers(m_metadata.width, m_metadata.height, 0);
        
        // Preencher com padrão: bordas = parede (1), interior = grama (0)
        Layer* baseLayer = m_layerManager->GetLayer(0);
        if (baseLayer) {
            for (int y = 0; y < m_metadata.height; ++y) {
                for (int x = 0; x < m_metadata.width; ++x) {
                    if (x == 0 || x == m_metadata.width - 1 || 
                        y == 0 || y == m_metadata.height - 1) {
                        baseLayer->SetTile(x, y, 1); // Parede
                    } else {
                        baseLayer->SetTile(x, y, 0); // Grama
                    }
                }
            }
        }
    }
}

// Métodos de gerenciamento de layers
int Map::GetLayerCount() const
{
    return m_layerManager ? m_layerManager->GetLayerCount() : 0;
}

Layer* Map::GetActiveLayer()
{
    return m_layerManager ? m_layerManager->GetActiveLayer() : nullptr;
}

const Layer* Map::GetActiveLayer() const
{
    return m_layerManager ? m_layerManager->GetActiveLayer() : nullptr;
}

bool Map::SetActiveLayer(int index)
{
    return m_layerManager ? m_layerManager->SetActiveLayer(index) : false;
}

Layer* Map::CreateLayer(const LayerProperties& properties)
{
    if (!m_layerManager) {
        return nullptr;
    }
    
    LayerProperties newProps = properties;
    if (newProps.name.IsEmpty()) {
        newProps.name = wxString::Format("Layer %d", m_layerManager->GetLayerCount() + 1);
    }
    Layer* newLayer = m_layerManager->CreateLayer(newProps);
    
    if (newLayer && m_metadata.width > 0 && m_metadata.height > 0) {
        newLayer->Resize(m_metadata.width, m_metadata.height, 0);
        SetModified();
    }
    
    return newLayer;
}

bool Map::RemoveLayer(int index)
{
    if (!m_layerManager) {
        return false;
    }
    
    bool result = m_layerManager->RemoveLayer(index);
    if (result) {
        SetModified();
    }
    
    return result;
}

// Operações em layers específicos
int Map::GetTileFromLayer(int layerIndex, int x, int y) const
{
    if (!m_layerManager || !IsValidPosition(x, y)) {
        return -1;
    }
    
    Layer* layer = m_layerManager->GetLayer(layerIndex);
    return layer ? layer->GetTile(x, y) : -1;
}

void Map::SetTileInLayer(int layerIndex, int x, int y, int tileId)
{
    if (!m_layerManager || !IsValidPosition(x, y)) {
        return;
    }
    
    Layer* layer = m_layerManager->GetLayer(layerIndex);
    if (layer) {
        int oldTile = layer->GetTile(x, y);
        if (oldTile != tileId) {
            layer->SetTile(x, y, tileId);
            SetModified();
        }
    }
}

// Operações adicionais de layers
void Map::ClearAllLayers()
{
    if (m_layerManager) {
        for (int i = 0; i < m_layerManager->GetLayerCount(); ++i) {
            Layer* layer = m_layerManager->GetLayer(i);
            if (layer) {
                layer->Clear();
            }
        }
        SetModified();
    }
}

void Map::FillLayer(int layerIndex, int tileId)
{
    if (!m_layerManager) {
        return;
    }
    
    Layer* layer = m_layerManager->GetLayer(layerIndex);
    if (layer) {
        layer->Fill(tileId);
        SetModified();
    }
}

// Operações de área
void Map::FillRect(int x, int y, int width, int height, int tileId)
{
    Layer* activeLayer = GetActiveLayer();
    if (!activeLayer) {
        return;
    }
    
    for (int dy = 0; dy < height; ++dy) {
        for (int dx = 0; dx < width; ++dx) {
            int px = x + dx;
            int py = y + dy;
            if (IsValidPosition(px, py)) {
                activeLayer->SetTile(px, py, tileId);
            }
        }
    }
    
    SetModified();
}

void Map::CopyRect(int srcX, int srcY, int width, int height, int srcLayer, int destLayer, int destX, int destY)
{
    if (!m_layerManager) {
        return;
    }
    
    Layer* source = m_layerManager->GetLayer(srcLayer);
    Layer* dest = m_layerManager->GetLayer(destLayer);
    
    if (!source || !dest) {
        return;
    }
    
    for (int dy = 0; dy < height; ++dy) {
        for (int dx = 0; dx < width; ++dx) {
            int srcPx = srcX + dx;
            int srcPy = srcY + dy;
            int destPx = destX + dx;
            int destPy = destY + dy;
            
            if (IsValidPosition(srcPx, srcPy) && IsValidPosition(destPx, destPy)) {
                int tileId = source->GetTile(srcPx, srcPy);
                dest->SetTile(destPx, destPy, tileId);
            }
        }
    }
    
    SetModified();
}

void Map::UpdateModifiedTime()
{
    wxDateTime now = wxDateTime::Now();
    m_metadata.modifiedDate = now.Format("%Y-%m-%d %H:%M:%S");
}

// Métodos de arquivo para compatibilidade com MapManager
bool Map::LoadFromFile(const std::string& filepath)
{
    try {
        wxFFile file(wxString(filepath), "rb"); // Abrir como binário para evitar problemas de encoding
        if (!file.IsOpened()) {
            wxLogError("Não foi possível abrir o arquivo: %s", wxString(filepath));
            return false;
        }
        
        wxString jsonData;
        if (!file.ReadAll(&jsonData, wxConvUTF8)) { // Forçar leitura UTF-8
            wxLogError("Erro ao ler o arquivo: %s", wxString(filepath));
            return false;
        }
        
        file.Close();
        
        // Tentar primeiro o formato completo (com metadata)
        if (LoadFromJson(jsonData)) {
            return true;
        }
        
        // Se falhou, tentar formato simples
        return LoadFromSimpleJson(jsonData);
        
    } catch (const std::exception& e) {
        wxLogError("Exceção ao carregar arquivo: %s", e.what());
        return false;
    } catch (...) {
        wxLogError("Erro inesperado ao carregar arquivo");
        return false;
    }
}

bool Map::SaveToFile(const std::string& filepath) const
{
    try {
        wxString jsonData = SaveToJson();
        if (jsonData.IsEmpty()) {
            wxLogError("Falha ao serializar mapa para JSON");
            return false;
        }
        
        wxFFile file(wxString(filepath), "wb"); // Abrir como binário
        if (!file.IsOpened()) {
            wxLogError("Não foi possível criar o arquivo: %s", wxString(filepath));
            return false;
        }
        
        // Escrever com encoding UTF-8
        const wxCharBuffer buffer = jsonData.ToUTF8();
        if (!file.Write(buffer.data(), buffer.length())) {
            wxLogError("Erro ao escrever no arquivo: %s", wxString(filepath));
            file.Close();
            return false;
        }
        
        file.Close();
        return true;
        
    } catch (const std::exception& e) {
        wxLogError("Exceção ao salvar arquivo: %s", e.what());
        return false;
    } catch (...) {
        wxLogError("Erro inesperado ao salvar arquivo");
        return false;
    }
}

// Método auxiliar para carregar formato JSON simples
bool Map::LoadFromSimpleJson(const wxString& jsonData)
{
    try {
        nlohmann::json root = nlohmann::json::parse(jsonData.ToUTF8().data());
        
        // Verificar se é formato simples
        if (!root.contains("width") || !root.contains("height") || !root.contains("tiles")) {
            return false;
        }
        
        int width = root["width"].get<int>();
        int height = root["height"].get<int>();
        auto tilesArray = root["tiles"];
        
        if (width <= 0 || height <= 0) {
            wxLogError("Dimensões inválidas: %dx%d", width, height);
            return false;
        }
        
        if (!tilesArray.is_array() || tilesArray.size() != static_cast<size_t>(width * height)) {
            wxLogError("Array de tiles tem tamanho incorreto. Esperado: %d, Encontrado: %zu", 
                      width * height, tilesArray.size());
            return false;
        }
        
        // Atualizar metadados
        m_metadata.width = width;
        m_metadata.height = height;
        m_metadata.name = "Mapa Carregado";
        UpdateModifiedTime();
        
        // Redimensionar layers e carregar tiles no layer ativo
        if (m_layerManager) {
            m_layerManager->ResizeAllLayers(width, height, 0);
            Layer* activeLayer = m_layerManager->GetActiveLayer();
            if (activeLayer) {
                for (int y = 0; y < height; ++y) {
                    for (int x = 0; x < width; ++x) {
                        int index = y * width + x;
                        activeLayer->SetTile(x, y, tilesArray[index].get<int>());
                    }
                }
            }
        }
        
        m_modified = false;
        wxLogMessage("Mapa carregado com sucesso (formato simples): %dx%d", width, height);
        return true;
        
    } catch (const nlohmann::json::exception& e) {
        // Não é erro grave aqui, apenas formato não suportado
        return false;
    } catch (...) {
        return false;
    }
}
