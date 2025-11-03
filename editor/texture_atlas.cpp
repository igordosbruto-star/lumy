/**
 * Implementação de TextureAtlas - Sistema de texturas de tilesets
 */

#include "texture_atlas.h"
#include "tileset_manager.h"
#include <wx/log.h>
#include <cmath>
#include <algorithm>

// ============================================================================
// TextureAtlas
// ============================================================================

TextureAtlas::TextureAtlas()
    : m_textureId(0)
{
}

TextureAtlas::~TextureAtlas()
{
    Unload();
}

TextureAtlas::TextureAtlas(TextureAtlas&& other) noexcept
    : m_textureId(other.m_textureId)
    , m_info(std::move(other.m_info))
    , m_uvCache(std::move(other.m_uvCache))
{
    other.m_textureId = 0;
}

TextureAtlas& TextureAtlas::operator=(TextureAtlas&& other) noexcept
{
    if (this != &other) {
        Unload();
        
        m_textureId = other.m_textureId;
        m_info = std::move(other.m_info);
        m_uvCache = std::move(other.m_uvCache);
        
        other.m_textureId = 0;
    }
    return *this;
}

bool TextureAtlas::LoadTileset(const wxString& filePath, int tileWidth, int tileHeight)
{
    if (!wxFileExists(filePath)) {
        wxLogError("TextureAtlas: Arquivo não encontrado: %s", filePath);
        return false;
    }

    wxImage image;
    if (!image.LoadFile(filePath)) {
        wxLogError("TextureAtlas: Falha ao carregar imagem: %s", filePath);
        return false;
    }

    return LoadFromImage(image, tileWidth, tileHeight, filePath);
}

bool TextureAtlas::LoadFromTilesetInfo(const TilesetInfo& tilesetInfo)
{
    if (tilesetInfo.tiles.empty()) {
        wxLogError("TextureAtlas: TilesetInfo '%s' está vazio", tilesetInfo.name);
        return false;
    }

    int tileWidth = tilesetInfo.tileSize.GetWidth();
    int tileHeight = tilesetInfo.tileSize.GetHeight();
    if (tileWidth <= 0 || tileHeight <= 0) {
        wxLogError("TextureAtlas: Tamanho de tile inválido (%d x %d) para tileset '%s'",
                   tileWidth, tileHeight, tilesetInfo.name);
        return false;
    }

    int tilesPerRow = tilesetInfo.tilesPerRow > 0
        ? tilesetInfo.tilesPerRow
        : static_cast<int>(std::ceil(std::sqrt(static_cast<double>(tilesetInfo.tiles.size()))));

    int totalTiles = static_cast<int>(tilesetInfo.tiles.size());
    int rows = (totalTiles + tilesPerRow - 1) / tilesPerRow;

    if (tilesPerRow <= 0 || rows <= 0) {
        wxLogError("TextureAtlas: Configuração inválida para tileset '%s'", tilesetInfo.name);
        return false;
    }

    int atlasWidth = tilesPerRow * tileWidth;
    int atlasHeight = rows * tileHeight;

    wxImage atlasImage(atlasWidth, atlasHeight, false);
    atlasImage.InitAlpha();

    unsigned char* atlasData = atlasImage.GetData();
    unsigned char* atlasAlpha = atlasImage.GetAlpha();

    std::fill(atlasData, atlasData + atlasWidth * atlasHeight * 3, 0);
    if (atlasAlpha) {
        std::fill(atlasAlpha, atlasAlpha + atlasWidth * atlasHeight, 0);
    }

    for (int index = 0; index < totalTiles; ++index) {
        const TileInfo& tileInfo = tilesetInfo.tiles[index];
        wxImage tileImage = tileInfo.image;
        if (!tileImage.IsOk()) {
            continue;
        }

        if (tileImage.GetWidth() != tileWidth || tileImage.GetHeight() != tileHeight) {
            tileImage = tileImage.Scale(tileWidth, tileHeight, wxIMAGE_QUALITY_NEAREST);
        }

        if (!tileImage.HasAlpha()) {
            tileImage.InitAlpha();
            unsigned char* tileAlphaPtr = tileImage.GetAlpha();
            if (tileAlphaPtr) {
                std::fill(tileAlphaPtr, tileAlphaPtr + tileWidth * tileHeight, 255);
            }
        }

        unsigned char* tileData = tileImage.GetData();
        unsigned char* tileAlpha = tileImage.GetAlpha();

        int col = index % tilesPerRow;
        int row = index / tilesPerRow;
        int destX = col * tileWidth;
        int destY = row * tileHeight;

        for (int y = 0; y < tileHeight; ++y) {
            for (int x = 0; x < tileWidth; ++x) {
                int srcIndex = y * tileWidth + x;
                int destIndex = (destY + y) * atlasWidth + (destX + x);

                atlasData[destIndex * 3 + 0] = tileData[srcIndex * 3 + 0];
                atlasData[destIndex * 3 + 1] = tileData[srcIndex * 3 + 1];
                atlasData[destIndex * 3 + 2] = tileData[srcIndex * 3 + 2];

                if (atlasAlpha && tileAlpha) {
                    atlasAlpha[destIndex] = tileAlpha[srcIndex];
                }
            }
        }
    }

    wxString sourceName = tilesetInfo.name.IsEmpty() ? wxString("<inline>") : tilesetInfo.name;
    int totalOverride = tilesetInfo.totalTiles > 0 ? tilesetInfo.totalTiles : totalTiles;

    return LoadFromImage(atlasImage, tileWidth, tileHeight, sourceName, totalOverride);
}

bool TextureAtlas::LoadFromImage(const wxImage& image,
                                 int tileWidth,
                                 int tileHeight,
                                 const wxString& sourceName,
                                 int totalTilesOverride)
{
    if (IsLoaded()) {
        Unload();
    }

    if (!image.IsOk() || image.GetWidth() == 0 || image.GetHeight() == 0) {
        wxLogError("TextureAtlas: Imagem inválida ao carregar atlas de '%s'", sourceName);
        return false;
    }

    m_textureId = CreateTextureFromImage(image);
    if (m_textureId == 0) {
        wxLogError("TextureAtlas: Falha ao criar textura OpenGL para '%s'", sourceName);
        return false;
    }

    m_info.filePath = sourceName;
    m_info.textureWidth = image.GetWidth();
    m_info.textureHeight = image.GetHeight();
    m_info.tileWidth = tileWidth;
    m_info.tileHeight = tileHeight;
    m_info.tilesPerRow = tileWidth > 0 ? m_info.textureWidth / tileWidth : 0;
    m_info.tilesPerColumn = tileHeight > 0 ? m_info.textureHeight / tileHeight : 0;
    m_info.totalTiles = (totalTilesOverride >= 0)
        ? totalTilesOverride
        : m_info.tilesPerRow * m_info.tilesPerColumn;

    m_uvCache.clear();

    wxLogMessage("TextureAtlas: Tileset carregado: %s (%dx%d tiles, %d total)",
                 sourceName, m_info.tilesPerRow, m_info.tilesPerColumn, m_info.totalTiles);

    return true;
}

void TextureAtlas::Unload()
{
    if (m_textureId != 0) {
        glDeleteTextures(1, &m_textureId);
        m_textureId = 0;
    }
    
    m_uvCache.clear();
    m_info = AtlasTilesetInfo(); // Reset
}

void TextureAtlas::Bind() const
{
    if (m_textureId != 0) {
        glBindTexture(GL_TEXTURE_2D, m_textureId);
    }
}

void TextureAtlas::Unbind() const
{
    glBindTexture(GL_TEXTURE_2D, 0);
}

TileUV TextureAtlas::GetTileUV(int tileId) const
{
    if (!IsValidTileId(tileId)) {
        wxLogWarning("TextureAtlas: TileId inválido: %d (max: %d)", tileId, m_info.totalTiles - 1);
        return TileUV(); // UVs padrão (0,0) - (1,1)
    }
    
    // Verificar cache
    auto it = m_uvCache.find(tileId);
    if (it != m_uvCache.end()) {
        return it->second;
    }
    
    // Calcular e cachear
    TileUV uv = CalculateTileUV(tileId);
    m_uvCache[tileId] = uv;
    
    return uv;
}

TileUV TextureAtlas::GetTileUVByPosition(int tileX, int tileY) const
{
    // Converter posição (x, y) para tileId
    int tileId = tileY * m_info.tilesPerRow + tileX;
    return GetTileUV(tileId);
}

GLuint TextureAtlas::CreateTextureFromImage(const wxImage& image)
{
    // Converter wxImage para formato RGB ou RGBA
    int width = image.GetWidth();
    int height = image.GetHeight();
    bool hasAlpha = image.HasAlpha();
    
    GLuint textureId = 0;
    glGenTextures(1, &textureId);
    if (textureId == 0) {
        wxLogError("TextureAtlas: Falha ao gerar textura OpenGL");
        return 0;
    }
    
    glBindTexture(GL_TEXTURE_2D, textureId);
    
    // Configurar parâmetros da textura
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // Pixel art friendly
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    // Upload dos dados da imagem para OpenGL
    if (hasAlpha) {
        // RGBA
        unsigned char* data = new unsigned char[width * height * 4];
        unsigned char* rgb = image.GetData();
        unsigned char* alpha = image.GetAlpha();
        
        for (int i = 0; i < width * height; ++i) {
            data[i * 4 + 0] = rgb[i * 3 + 0]; // R
            data[i * 4 + 1] = rgb[i * 3 + 1]; // G
            data[i * 4 + 2] = rgb[i * 3 + 2]; // B
            data[i * 4 + 3] = alpha[i];       // A
        }
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        delete[] data;
    }
    else {
        // RGB
        unsigned char* rgb = image.GetData();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, rgb);
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    return textureId;
}

TileUV TextureAtlas::CalculateTileUV(int tileId) const
{
    // Calcular posição do tile no grid
    int tileX = tileId % m_info.tilesPerRow;
    int tileY = tileId / m_info.tilesPerRow;
    
    // Calcular coordenadas em pixels
    int pixelX = tileX * m_info.tileWidth;
    int pixelY = tileY * m_info.tileHeight;
    
    // Normalizar para UVs (0.0 - 1.0)
    float u0 = static_cast<float>(pixelX) / m_info.textureWidth;
    float v0 = static_cast<float>(pixelY) / m_info.textureHeight;
    float u1 = static_cast<float>(pixelX + m_info.tileWidth) / m_info.textureWidth;
    float v1 = static_cast<float>(pixelY + m_info.tileHeight) / m_info.textureHeight;
    
    return TileUV(u0, v0, u1, v1);
}

// ============================================================================
// TextureAtlasManager - Singleton
// ============================================================================

TextureAtlasManager& TextureAtlasManager::Get()
{
    static TextureAtlasManager instance;
    return instance;
}

TextureAtlasManager::~TextureAtlasManager()
{
    UnloadAll();
}

TextureAtlas* TextureAtlasManager::LoadTileset(const wxString& filePath, int tileWidth, int tileHeight)
{
    std::string key = filePath.ToStdString();
    
    // Verificar se já está carregado no cache
    auto it = m_atlases.find(key);
    if (it != m_atlases.end()) {
        wxLogMessage("TextureAtlasManager: Usando tileset do cache: %s", filePath);
        return it->second.get();
    }
    
    // Carregar novo tileset
    auto atlas = std::make_unique<TextureAtlas>();
    if (!atlas->LoadTileset(filePath, tileWidth, tileHeight)) {
        wxLogError("TextureAtlasManager: Falha ao carregar tileset: %s", filePath);
        return nullptr;
    }
    
    // Adicionar ao cache
    TextureAtlas* ptr = atlas.get();
    m_atlases[key] = std::move(atlas);
    
    wxLogMessage("TextureAtlasManager: Tileset adicionado ao cache: %s (total: %zu)", 
                 filePath, m_atlases.size());
    
    return ptr;
}

TextureAtlas* TextureAtlasManager::GetTileset(const wxString& filePath)
{
    std::string key = filePath.ToStdString();
    
    auto it = m_atlases.find(key);
    if (it != m_atlases.end()) {
        return it->second.get();
    }
    
    return nullptr;
}

void TextureAtlasManager::UnloadTileset(const wxString& filePath)
{
    std::string key = filePath.ToStdString();
    
    auto it = m_atlases.find(key);
    if (it != m_atlases.end()) {
        wxLogMessage("TextureAtlasManager: Removendo tileset do cache: %s", filePath);
        m_atlases.erase(it);
    }
}

void TextureAtlasManager::UnloadAll()
{
    if (!m_atlases.empty()) {
        wxLogMessage("TextureAtlasManager: Limpando todos os tilesets do cache (%zu)", m_atlases.size());
        m_atlases.clear();
    }
}
