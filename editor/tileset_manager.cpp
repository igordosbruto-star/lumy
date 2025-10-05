/**
 * Implementação do TilesetManager
 */

#pragma execution_character_set("utf-8")

#include "tileset_manager.h"
#include "utf8_strings.h"
#include <wx/filename.h>
#include <wx/ffile.h>
#include <nlohmann/json.hpp>

TilesetManager::TilesetManager()
{
    // Carregar tileset padrão
    LoadDefaultTileset();
}

bool TilesetManager::LoadTilesetFromFile(const wxString& filepath, const wxSize& tileSize)
{
    if (!wxFileExists(filepath)) {
        wxLogError("Arquivo de tileset não encontrado: %s", filepath);
        return false;
    }
    
    // Carregar imagem
    wxImage sourceImage;
    if (!sourceImage.LoadFile(filepath)) {
        wxLogError("Erro ao carregar imagem do tileset: %s", filepath);
        return false;
    }
    
    // Criar info do tileset
    TilesetInfo tilesetInfo;
    wxFileName fileName(filepath);
    tilesetInfo.name = fileName.GetName();
    tilesetInfo.filepath = filepath;
    tilesetInfo.tileSize = tileSize;
    
    // Calcular quantos tiles cabem na imagem
    int imageWidth = sourceImage.GetWidth();
    int imageHeight = sourceImage.GetHeight();
    tilesetInfo.tilesPerRow = imageWidth / tileSize.x;
    int totalRows = imageHeight / tileSize.y;
    tilesetInfo.totalTiles = tilesetInfo.tilesPerRow * totalRows;
    
    if (tilesetInfo.totalTiles == 0) {
        wxLogError("Nenhum tile encontrado na imagem com tamanho %dx%d", 
                   tileSize.x, tileSize.y);
        return false;
    }
    
    // Dividir imagem em tiles individuais
    if (!SplitImageIntoTiles(sourceImage, tileSize, tilesetInfo.tiles)) {
        wxLogError("Erro ao dividir imagem em tiles");
        return false;
    }
    
    // Adicionar ou substituir tileset
    return AddTileset(tilesetInfo);
}

bool TilesetManager::LoadDefaultTileset()
{
    TilesetInfo defaultTileset;
    defaultTileset.name = "Padrão";
    defaultTileset.filepath = ""; // Tileset virtual
    defaultTileset.tileSize = wxSize(32, 32);
    defaultTileset.tilesPerRow = 2;
    defaultTileset.totalTiles = 10;
    
    // Criar tiles com cores padrão
    for (int i = 0; i < 10; ++i) {
        wxImage tileImage = CreateDefaultTileImage(i);
        
        wxString tileNames[] = {
            "Grama", "Parede", "Colisão", "Água",
            "Areia", "Pedra", "Lava", "Gelo", 
            "Madeira", "Metal"
        };
        
        TileInfo tileInfo(i, tileImage, tileNames[i]);
        
        // Definir propriedades especiais para alguns tiles
        if (i == 1 || i == 2) { // Parede e Colisão têm colisão
            tileInfo.hasCollision = true;
        }
        
        defaultTileset.tiles.push_back(tileInfo);
    }
    
    bool result = AddTileset(defaultTileset);
    if (result) {
        SetCurrentTileset("Padrão");
    }
    
    return result;
}

bool TilesetManager::AddTileset(const TilesetInfo& tilesetInfo)
{
    // Verificar se já existe um tileset com esse nome
    auto it = std::find_if(m_tilesets.begin(), m_tilesets.end(),
        [&tilesetInfo](const TilesetInfo& existing) {
            return existing.name == tilesetInfo.name;
        });
    
    if (it != m_tilesets.end()) {
        // Substituir tileset existente
        *it = tilesetInfo;
        wxLogMessage("Tileset '%s' substituído", tilesetInfo.name);
    } else {
        // Adicionar novo tileset
        m_tilesets.push_back(tilesetInfo);
        wxLogMessage("Tileset '%s' adicionado com %d tiles", 
                     tilesetInfo.name, tilesetInfo.totalTiles);
    }
    
    // Limpar cache
    m_tileImageCache.clear();
    
    return true;
}

bool TilesetManager::RemoveTileset(const wxString& name)
{
    auto it = std::find_if(m_tilesets.begin(), m_tilesets.end(),
        [&name](const TilesetInfo& tileset) {
            return tileset.name == name;
        });
    
    if (it != m_tilesets.end()) {
        m_tilesets.erase(it);
        
        // Se removeu o tileset atual, selecionar o primeiro disponível
        if (m_currentTilesetName == name && !m_tilesets.empty()) {
            m_currentTilesetName = m_tilesets[0].name;
        }
        
        m_tileImageCache.clear();
        wxLogMessage("Tileset '%s' removido", name);
        return true;
    }
    
    return false;
}

void TilesetManager::ClearTilesets()
{
    m_tilesets.clear();
    m_currentTilesetName.Clear();
    m_tileImageCache.clear();
    wxLogMessage("Todos os tilesets foram removidos");
}

const TilesetInfo* TilesetManager::GetCurrentTileset() const
{
    return GetTileset(m_currentTilesetName);
}

const TilesetInfo* TilesetManager::GetTileset(const wxString& name) const
{
    auto it = std::find_if(m_tilesets.begin(), m_tilesets.end(),
        [&name](const TilesetInfo& tileset) {
            return tileset.name == name;
        });
    
    return (it != m_tilesets.end()) ? &(*it) : nullptr;
}

TileInfo* TilesetManager::GetTileInfo(int tileId)
{
    const TilesetInfo* currentTileset = GetCurrentTileset();
    if (!currentTileset || tileId < 0 || tileId >= (int)currentTileset->tiles.size()) {
        return nullptr;
    }
    
    return const_cast<TileInfo*>(&currentTileset->tiles[tileId]);
}

const TileInfo* TilesetManager::GetTileInfo(int tileId) const
{
    const TilesetInfo* currentTileset = GetCurrentTileset();
    if (!currentTileset || tileId < 0 || tileId >= (int)currentTileset->tiles.size()) {
        return nullptr;
    }
    
    return &currentTileset->tiles[tileId];
}

bool TilesetManager::SetCurrentTileset(const wxString& name)
{
    const TilesetInfo* tileset = GetTileset(name);
    if (tileset) {
        m_currentTilesetName = name;
        m_tileImageCache.clear(); // Limpar cache ao trocar tileset
        wxLogMessage("Tileset atual alterado para: %s", name);
        return true;
    }
    
    wxLogWarning("Tileset '%s' não encontrado", name);
    return false;
}

int TilesetManager::GetTileCount() const
{
    const TilesetInfo* currentTileset = GetCurrentTileset();
    return currentTileset ? currentTileset->totalTiles : 0;
}

wxImage TilesetManager::GetTileImage(int tileId) const
{
    // Verificar cache primeiro
    auto cacheIt = m_tileImageCache.find(tileId);
    if (cacheIt != m_tileImageCache.end()) {
        return cacheIt->second;
    }
    
    // Buscar na estrutura de dados
    const TileInfo* tileInfo = GetTileInfo(tileId);
    if (tileInfo && tileInfo->image.IsOk()) {
        m_tileImageCache[tileId] = tileInfo->image;
        return tileInfo->image;
    }
    
    // Retornar imagem padrão se não encontrar
    wxImage defaultImage = CreateDefaultTileImage(tileId);
    m_tileImageCache[tileId] = defaultImage;
    return defaultImage;
}

wxString TilesetManager::GetTileName(int tileId) const
{
    const TileInfo* tileInfo = GetTileInfo(tileId);
    if (tileInfo && !tileInfo->name.IsEmpty()) {
        return tileInfo->name;
    }
    
    return wxString::Format("Tile %d", tileId);
}

void TilesetManager::SetTileProperty(int tileId, const wxString& property, const wxVariant& value)
{
    TileInfo* tileInfo = GetTileInfo(tileId);
    if (!tileInfo) return;
    
    if (property == "collision") {
        tileInfo->hasCollision = value.GetBool();
    } else if (property == "animated") {
        tileInfo->isAnimated = value.GetBool();
    } else if (property == "sound") {
        tileInfo->soundEffect = value.GetString();
    } else if (property == "name") {
        tileInfo->name = value.GetString();
    } else if (property == "category") {
        tileInfo->category = value.GetString();
    }
}

wxVariant TilesetManager::GetTileProperty(int tileId, const wxString& property) const
{
    const TileInfo* tileInfo = GetTileInfo(tileId);
    if (!tileInfo) return wxVariant();
    
    if (property == "collision") {
        return wxVariant(tileInfo->hasCollision);
    } else if (property == "animated") {
        return wxVariant(tileInfo->isAnimated);
    } else if (property == "sound") {
        return wxVariant(tileInfo->soundEffect);
    } else if (property == "name") {
        return wxVariant(tileInfo->name);
    } else if (property == "category") {
        return wxVariant(tileInfo->category);
    }
    
    return wxVariant();
}

bool TilesetManager::SaveToProject(const wxString& projectPath) const
{
    try {
        nlohmann::json root;
        
        // Salvar tileset atual
        root["current_tileset"] = m_currentTilesetName.ToUTF8().data();
        
        // Salvar informações dos tilesets
        nlohmann::json tilesetsArray = nlohmann::json::array();
        
        for (const auto& tileset : m_tilesets) {
            nlohmann::json tilesetJson;
            tilesetJson["name"] = tileset.name.ToUTF8().data();
            tilesetJson["filepath"] = tileset.filepath.ToUTF8().data();
            tilesetJson["tile_width"] = tileset.tileSize.x;
            tilesetJson["tile_height"] = tileset.tileSize.y;
            tilesetJson["tiles_per_row"] = tileset.tilesPerRow;
            tilesetJson["total_tiles"] = tileset.totalTiles;
            
            // Salvar propriedades dos tiles
            nlohmann::json tilesArray = nlohmann::json::array();
            for (const auto& tile : tileset.tiles) {
                nlohmann::json tileJson;
                tileJson["id"] = tile.id;
                tileJson["name"] = tile.name.ToUTF8().data();
                tileJson["category"] = tile.category.ToUTF8().data();
                tileJson["has_collision"] = tile.hasCollision;
                tileJson["is_animated"] = tile.isAnimated;
                tileJson["sound_effect"] = tile.soundEffect.ToUTF8().data();
                
                tilesArray.push_back(tileJson);
            }
            tilesetJson["tiles"] = tilesArray;
            
            tilesetsArray.push_back(tilesetJson);
        }
        
        root["tilesets"] = tilesetsArray;
        
        // Salvar arquivo
        wxString tilesetConfigPath = projectPath + "/tilesets.json";
        wxFFile file(tilesetConfigPath, "w");
        if (!file.IsOpened()) {
            wxLogError("Não foi possível criar arquivo de configuração: %s", tilesetConfigPath);
            return false;
        }
        
        std::string jsonString = root.dump(2);
        if (!file.Write(wxString::FromUTF8(jsonString.c_str()))) {
            wxLogError("Erro ao escrever arquivo de configuração: %s", tilesetConfigPath);
            return false;
        }
        
        file.Close();
        wxLogMessage("Configurações de tileset salvas: %s", tilesetConfigPath);
        return true;
        
    } catch (const std::exception& e) {
        wxLogError("Erro ao salvar configurações de tileset: %s", e.what());
        return false;
    }
}

bool TilesetManager::LoadFromProject(const wxString& projectPath)
{
    wxString tilesetConfigPath = projectPath + "/tilesets.json";
    
    if (!wxFileExists(tilesetConfigPath)) {
        wxLogMessage("Arquivo de configuração de tilesets não encontrado: %s", tilesetConfigPath);
        return false; // Não é erro, apenas não existe ainda
    }
    
    try {
        wxFFile file(tilesetConfigPath, "r");
        if (!file.IsOpened()) {
            wxLogError("Não foi possível abrir arquivo de configuração: %s", tilesetConfigPath);
            return false;
        }
        
        wxString jsonData;
        if (!file.ReadAll(&jsonData)) {
            wxLogError("Erro ao ler arquivo de configuração: %s", tilesetConfigPath);
            return false;
        }
        
        file.Close();
        
        nlohmann::json root = nlohmann::json::parse(jsonData.ToUTF8().data());
        
        // Carregar tilesets
        if (root.contains("tilesets") && root["tilesets"].is_array()) {
            for (const auto& tilesetJson : root["tilesets"]) {
                wxString name = wxString::FromUTF8(tilesetJson.value("name", "").c_str());
                wxString filepath = wxString::FromUTF8(tilesetJson.value("filepath", "").c_str());
                
                // Pular tileset padrão (virtual) - será recriado automaticamente
                if (filepath.IsEmpty()) {
                    continue;
                }
                
                // Tentar carregar tileset do arquivo
                wxSize tileSize(
                    tilesetJson.value("tile_width", 32),
                    tilesetJson.value("tile_height", 32)
                );
                
                if (LoadTilesetFromFile(filepath, tileSize)) {
                    // Carregar propriedades dos tiles
                    if (tilesetJson.contains("tiles") && tilesetJson["tiles"].is_array()) {
                        const auto& tilesArray = tilesetJson["tiles"];
                        
                        for (const auto& tileJson : tilesArray) {
                            int tileId = tileJson.value("id", -1);
                            if (tileId >= 0) {
                                wxString tileName = wxString::FromUTF8(tileJson.value("name", "").c_str());
                                wxString category = wxString::FromUTF8(tileJson.value("category", "").c_str());
                                bool hasCollision = tileJson.value("has_collision", false);
                                bool isAnimated = tileJson.value("is_animated", false);
                                wxString soundEffect = wxString::FromUTF8(tileJson.value("sound_effect", "").c_str());
                                
                                // Aplicar propriedades
                                SetTileProperty(tileId, "name", wxVariant(tileName));
                                SetTileProperty(tileId, "category", wxVariant(category));
                                SetTileProperty(tileId, "collision", wxVariant(hasCollision));
                                SetTileProperty(tileId, "animated", wxVariant(isAnimated));
                                SetTileProperty(tileId, "sound", wxVariant(soundEffect));
                            }
                        }
                    }
                }
            }
        }
        
        // Definir tileset atual
        if (root.contains("current_tileset")) {
            wxString currentTileset = wxString::FromUTF8(root["current_tileset"].get<std::string>().c_str());
            SetCurrentTileset(currentTileset);
        }
        
        wxLogMessage("Configurações de tileset carregadas: %s", tilesetConfigPath);
        return true;
        
    } catch (const std::exception& e) {
        wxLogError("Erro ao carregar configurações de tileset: %s", e.what());
        return false;
    }
}

void TilesetManager::PrintDebugInfo() const
{
    wxLogMessage("=== TilesetManager Debug Info ===");
    wxLogMessage("Tilesets carregados: %zu", m_tilesets.size());
    wxLogMessage("Tileset atual: %s", m_currentTilesetName);
    
    for (const auto& tileset : m_tilesets) {
        wxLogMessage("Tileset: %s", tileset.name);
        wxLogMessage("  Arquivo: %s", tileset.filepath.IsEmpty() ? wxString("(virtual)") : tileset.filepath);
        wxLogMessage("  Tamanho do tile: %dx%d", tileset.tileSize.x, tileset.tileSize.y);
        wxLogMessage("  Tiles por linha: %d", tileset.tilesPerRow);
        wxLogMessage("  Total de tiles: %d", tileset.totalTiles);
    }
    
    wxLogMessage("Cache de imagens: %zu entradas", m_tileImageCache.size());
    wxLogMessage("===============================");
}

// Métodos privados

bool TilesetManager::SplitImageIntoTiles(const wxImage& sourceImage, const wxSize& tileSize, 
                                       std::vector<TileInfo>& tiles)
{
    try {
        tiles.clear();
        
        int imageWidth = sourceImage.GetWidth();
        int imageHeight = sourceImage.GetHeight();
        int tilesPerRow = imageWidth / tileSize.x;
        int totalRows = imageHeight / tileSize.y;
        
        int tileId = 0;
        for (int row = 0; row < totalRows; ++row) {
            for (int col = 0; col < tilesPerRow; ++col) {
                // Extrair subimagem para este tile
                wxRect tileRect(col * tileSize.x, row * tileSize.y, tileSize.x, tileSize.y);
                wxImage tileImage = sourceImage.GetSubImage(tileRect);
                
                if (tileImage.IsOk()) {
                    TileInfo tileInfo(tileId, tileImage, wxString::Format("Tile %d", tileId));
                    tiles.push_back(tileInfo);
                    tileId++;
                } else {
                    wxLogWarning("Erro ao extrair tile na posição %d,%d", col, row);
                }
            }
        }
        
        wxLogMessage("Dividida imagem em %d tiles (%dx%d cada)", 
                     (int)tiles.size(), tileSize.x, tileSize.y);
        return true;
        
    } catch (const std::exception& e) {
        wxLogError("Exceção ao dividir imagem em tiles: %s", e.what());
        return false;
    }
}

wxImage TilesetManager::CreateDefaultTileImage(int tileId) const
{
    wxImage image(32, 32);
    wxColour color = GetDefaultTileColor(tileId);
    
    unsigned char r = color.Red();
    unsigned char g = color.Green();
    unsigned char b = color.Blue();
    
    // Preencher imagem com a cor
    unsigned char* data = image.GetData();
    for (int i = 0; i < 32 * 32; ++i) {
        data[i * 3] = r;
        data[i * 3 + 1] = g;
        data[i * 3 + 2] = b;
    }
    
    return image;
}

wxColour TilesetManager::GetDefaultTileColor(int tileId) const
{
    // Cores padrão para os tiles (mesmo sistema atual)
    wxColour defaultColors[] = {
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
    
    if (tileId >= 0 && tileId < 10) {
        return defaultColors[tileId];
    }
    
    return wxColour(255, 255, 255); // Branco para tiles não definidos
}