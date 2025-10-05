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
    : m_modified(false)
{
    m_metadata = MapMetadata(); // Usar valores padrão
    InitializeDefaultMap();
}

// Construtor com dimensões específicas
Map::Map(int width, int height, int tileSize)
    : m_modified(false)
{
    m_metadata = MapMetadata();
    m_metadata.width = width;
    m_metadata.height = height;
    m_metadata.tileSize = tileSize;
    InitializeDefaultMap();
}

// Construtor de cópia
Map::Map(const Map& other)
    : m_metadata(other.m_metadata)
    , m_tiles(other.m_tiles)
    , m_modified(other.m_modified)
{
}

// Operador de atribuição
Map& Map::operator=(const Map& other)
{
    if (this != &other) {
        m_metadata = other.m_metadata;
        m_tiles = other.m_tiles;
        m_modified = other.m_modified;
    }
    return *this;
}

// Getter para tile
int Map::GetTile(int x, int y) const
{
    if (!IsValidPosition(x, y)) {
        return -1; // Tile inválido
    }
    return m_tiles[y][x];
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
    if (!IsValidPosition(x, y)) {
        return;
    }
    
    if (m_tiles[y][x] != tileId) {
        m_tiles[y][x] = tileId;
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
    Fill(0); // Preencher com grama
}

void Map::Fill(int tileId)
{
    for (int y = 0; y < m_metadata.height; ++y) {
        for (int x = 0; x < m_metadata.width; ++x) {
            m_tiles[y][x] = tileId;
        }
    }
    SetModified();
}

void Map::Resize(int newWidth, int newHeight, int defaultTile)
{
    if (newWidth <= 0 || newHeight <= 0) {
        wxLogError("Dimensões de mapa inválidas: %dx%d", newWidth, newHeight);
        return;
    }
    
    // Criar novo array de tiles
    std::vector<std::vector<int>> newTiles(newHeight, std::vector<int>(newWidth, defaultTile));
    
    // Copiar dados existentes (se houver sobreposição)
    int copyWidth = std::min(newWidth, m_metadata.width);
    int copyHeight = std::min(newHeight, m_metadata.height);
    
    for (int y = 0; y < copyHeight; ++y) {
        for (int x = 0; x < copyWidth; ++x) {
            newTiles[y][x] = m_tiles[y][x];
        }
    }
    
    // Substituir dados
    m_tiles = std::move(newTiles);
    m_metadata.width = newWidth;
    m_metadata.height = newHeight;
    
    SetModified();
}

bool Map::IsValidPosition(int x, int y) const
{
    return x >= 0 && x < m_metadata.width && y >= 0 && y < m_metadata.height;
}

// Estado
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
        
        // Carregar dados dos tiles
        if (root.contains("tiles")) {
            auto tilesArray = root["tiles"];
            
            if (tilesArray.is_array() && tilesArray.size() == static_cast<size_t>(m_metadata.height)) {
                // Redimensionar array de tiles
                m_tiles.resize(m_metadata.height);
                
                for (int y = 0; y < m_metadata.height; ++y) {
                    auto row = tilesArray[y];
                    if (row.is_array() && row.size() == static_cast<size_t>(m_metadata.width)) {
                        m_tiles[y].resize(m_metadata.width);
                        for (int x = 0; x < m_metadata.width; ++x) {
                            m_tiles[y][x] = row[x].get<int>();
                        }
                    } else {
                        wxLogError("Linha %d do mapa tem tamanho incorreto", y);
                        return false;
                    }
                }
            } else {
                wxLogError("Array de tiles tem formato incorreto");
                return false;
            }
        } else {
            // Se não há dados de tiles, inicializar com padrão
            InitializeDefaultMap();
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
        
        // Salvar dados dos tiles
        nlohmann::json tilesArray = nlohmann::json::array();
        for (int y = 0; y < m_metadata.height; ++y) {
            nlohmann::json row = nlohmann::json::array();
            for (int x = 0; x < m_metadata.width; ++x) {
                row.push_back(m_tiles[y][x]);
            }
            tilesArray.push_back(row);
        }
        root["tiles"] = tilesArray;
        
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
    
    // Contar tipos de tiles
    std::map<int, int> tileCount;
    for (int y = 0; y < m_metadata.height; ++y) {
        for (int x = 0; x < m_metadata.width; ++x) {
            tileCount[m_tiles[y][x]]++;
        }
    }
    
    wxLogMessage("Contagem de tiles:");
    for (const auto& pair : tileCount) {
        wxLogMessage("  Tile %d: %d vezes", pair.first, pair.second);
    }
    wxLogMessage("========================");
}

// Métodos privados
void Map::InitializeDefaultMap()
{
    // Redimensionar array de tiles
    m_tiles.resize(m_metadata.height);
    for (int y = 0; y < m_metadata.height; ++y) {
        m_tiles[y].resize(m_metadata.width);
    }
    
    // Preencher com padrão: bordas = parede, interior = grama
    for (int y = 0; y < m_metadata.height; ++y) {
        for (int x = 0; x < m_metadata.width; ++x) {
            if (x == 0 || x == m_metadata.width - 1 || 
                y == 0 || y == m_metadata.height - 1) {
                m_tiles[y][x] = 1; // Parede
            } else {
                m_tiles[y][x] = 0; // Grama
            }
        }
    }
    
    m_modified = false;
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
        
        // Redimensionar e carregar tiles
        m_tiles.resize(height);
        for (int y = 0; y < height; ++y) {
            m_tiles[y].resize(width);
            for (int x = 0; x < width; ++x) {
                int index = y * width + x;
                m_tiles[y][x] = tilesArray[index].get<int>();
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
