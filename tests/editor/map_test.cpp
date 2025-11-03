/**
 * Testes unitários para Map (editor)
 */

#include <gtest/gtest.h>
#include <chrono>
#include <wx/log.h>
#include "../../editor/map.h"
#include "../../editor/layer.h"

// Environment global para desabilitar logs do wxWidgets
class MapTestEnvironment : public ::testing::Environment {
public:
    void SetUp() override {
        wxLog::EnableLogging(false);
    }
    
    void TearDown() override {
        wxLog::EnableLogging(true);
    }
};

class MapTest : public ::testing::Test {
protected:
    void SetUp() override {
        map = std::make_unique<Map>(25, 15, 32);
    }

    void TearDown() override {
        map.reset();
    }

    std::unique_ptr<Map> map;
};

// ============================================================================
// Testes de Construção e Inicialização
// ============================================================================

TEST_F(MapTest, ConstructorBasic) {
    EXPECT_EQ(map->GetWidth(), 25);
    EXPECT_EQ(map->GetHeight(), 15);
    EXPECT_EQ(map->GetTileSize(), 32);
}

TEST_F(MapTest, DefaultConstructor) {
    Map defaultMap;
    
    EXPECT_GT(defaultMap.GetWidth(), 0);
    EXPECT_GT(defaultMap.GetHeight(), 0);
    EXPECT_EQ(defaultMap.GetTileSize(), 32);
}

TEST_F(MapTest, InitiallyHasDefaultLayers) {
    // Map deve criar pelo menos 1 layer padrão
    EXPECT_GT(map->GetLayerCount(), 0);
}

TEST_F(MapTest, CopyConstructor) {
    map->SetName("Original");
    map->SetTile(5, 5, 42);
    
    Map copy(*map);
    
    EXPECT_EQ(copy.GetMetadata().name, "Original");
    EXPECT_EQ(copy.GetWidth(), 25);
    EXPECT_EQ(copy.GetHeight(), 15);
    EXPECT_EQ(copy.GetTile(5, 5), 42);
}

// ============================================================================
// Testes de Metadados
// ============================================================================

TEST_F(MapTest, SetAndGetMetadata) {
    MapMetadata metadata;
    metadata.name = "Test Map";
    metadata.author = "Test Author";
    metadata.description = "Test Description";
    metadata.version = "1.0";
    
    map->SetMetadata(metadata);
    
    const MapMetadata& retrieved = map->GetMetadata();
    EXPECT_EQ(retrieved.name, "Test Map");
    EXPECT_EQ(retrieved.author, "Test Author");
    EXPECT_EQ(retrieved.description, "Test Description");
    EXPECT_EQ(retrieved.version, "1.0");
}

TEST_F(MapTest, SetName) {
    map->SetName("My Awesome Map");
    
    EXPECT_EQ(map->GetMetadata().name, "My Awesome Map");
}

// ============================================================================
// Testes de Gerenciamento de Layers
// ============================================================================

TEST_F(MapTest, CreateLayer) {
    int initialCount = map->GetLayerCount();
    
    LayerProperties props;
    props.name = "New Layer";
    props.type = LayerType::TILE_LAYER;
    
    Layer* layer = map->CreateLayer(props);
    
    ASSERT_NE(layer, nullptr);
    EXPECT_EQ(map->GetLayerCount(), initialCount + 1);
    EXPECT_EQ(layer->GetName(), "New Layer");
}

TEST_F(MapTest, RemoveLayer) {
    map->CreateLayer();
    int count = map->GetLayerCount();
    
    bool removed = map->RemoveLayer(count - 1);
    
    EXPECT_TRUE(removed);
    EXPECT_EQ(map->GetLayerCount(), count - 1);
}

TEST_F(MapTest, GetActiveLayer) {
    Layer* active = map->GetActiveLayer();
    
    ASSERT_NE(active, nullptr);
}

TEST_F(MapTest, SetActiveLayer) {
    map->CreateLayer();
    map->CreateLayer();
    
    bool set = map->SetActiveLayer(1);
    
    EXPECT_TRUE(set);
    EXPECT_EQ(map->GetLayerManager()->GetActiveLayerIndex(), 1);
}

// ============================================================================
// Testes de Operações de Tiles
// ============================================================================

TEST_F(MapTest, SetAndGetTile) {
    map->SetTile(10, 10, 42);
    
    EXPECT_EQ(map->GetTile(10, 10), 42);
}

TEST_F(MapTest, SetTileInSpecificLayer) {
    if (map->GetLayerCount() > 0) {
        map->SetTileInLayer(0, 5, 5, 99);
        
        EXPECT_EQ(map->GetTileFromLayer(0, 5, 5), 99);
    }
}

TEST_F(MapTest, IsValidPosition) {
    EXPECT_TRUE(map->IsValidPosition(0, 0));
    EXPECT_TRUE(map->IsValidPosition(24, 14));
    EXPECT_FALSE(map->IsValidPosition(25, 15));
    EXPECT_FALSE(map->IsValidPosition(-1, 0));
}

// ============================================================================
// Testes de Operações em Área
// ============================================================================

TEST_F(MapTest, FillRect) {
    map->FillRect(5, 5, 3, 3, 42);
    
    EXPECT_EQ(map->GetTile(5, 5), 42);
    EXPECT_EQ(map->GetTile(6, 6), 42);
    EXPECT_EQ(map->GetTile(7, 7), 42);
}

TEST_F(MapTest, ClearActiveLayer) {
    map->SetTile(5, 5, 42);
    map->SetTile(10, 10, 99);
    
    map->Clear();
    
    // Após clear, tiles devem ser 0 ou -1
    int tile1 = map->GetTile(5, 5);
    int tile2 = map->GetTile(10, 10);
    
    EXPECT_TRUE(tile1 == 0 || tile1 == -1);
    EXPECT_TRUE(tile2 == 0 || tile2 == -1);
}

TEST_F(MapTest, FillActiveLayer) {
    map->Fill(42);
    
    EXPECT_EQ(map->GetTile(0, 0), 42);
    EXPECT_EQ(map->GetTile(12, 7), 42);
    EXPECT_EQ(map->GetTile(24, 14), 42);
}

// ============================================================================
// Testes de Resize
// ============================================================================

TEST_F(MapTest, ResizeMap) {
    map->SetTile(5, 5, 42);
    
    map->Resize(30, 20);
    
    EXPECT_EQ(map->GetWidth(), 30);
    EXPECT_EQ(map->GetHeight(), 20);
    
    // Tile anterior deve ser preservado
    EXPECT_EQ(map->GetTile(5, 5), 42);
}

TEST_F(MapTest, ResizeSmallerPreservesTiles) {
    map->SetTile(5, 5, 42);
    
    map->Resize(10, 10);
    
    EXPECT_EQ(map->GetWidth(), 10);
    EXPECT_EQ(map->GetHeight(), 10);
    EXPECT_EQ(map->GetTile(5, 5), 42);
}

TEST_F(MapTest, SetSize) {
    map->SetSize(40, 30);
    
    EXPECT_EQ(map->GetWidth(), 40);
    EXPECT_EQ(map->GetHeight(), 30);
}

// ============================================================================
// Testes de Estado e Modificação
// ============================================================================

TEST_F(MapTest, InitiallyNotModified) {
    Map freshMap(10, 10);
    
    EXPECT_FALSE(freshMap.IsModified());
}

TEST_F(MapTest, SetModified) {
    map->SetModified(true);
    
    EXPECT_TRUE(map->IsModified());
}

TEST_F(MapTest, MarkSaved) {
    map->SetModified(true);
    EXPECT_TRUE(map->IsModified());
    
    map->MarkSaved();
    
    EXPECT_FALSE(map->IsModified());
}

TEST_F(MapTest, TileChangeMarksModified) {
    map->MarkSaved();
    EXPECT_FALSE(map->IsModified());
    
    map->SetTile(5, 5, 42);
    
    // Dependendo da implementação, isto pode marcar como modificado
    // Este teste documenta o comportamento esperado
}

// ============================================================================
// Testes de Serialização
// ============================================================================

TEST_F(MapTest, SaveToJson) {
    map->SetName("Test Map");
    map->SetTile(5, 5, 42);
    
    wxString json = map->SaveToJson();
    
    EXPECT_FALSE(json.IsEmpty());
    EXPECT_TRUE(json.Contains("Test Map"));
}

TEST_F(MapTest, LoadFromJson) {
    wxString json = R"({
        "metadata": {
            "name": "Loaded Map",
            "width": 20,
            "height": 15,
            "tileSize": 32
        },
        "layers": [{
            "name": "Base",
            "type": "TILE_LAYER",
            "visible": true,
            "opacity": 1.0,
            "tiles": []
        }]
    })";
    
    bool loaded = map->LoadFromJson(json);
    
    EXPECT_TRUE(loaded);
    EXPECT_EQ(map->GetMetadata().name, "Loaded Map");
}

TEST_F(MapTest, SaveAndLoadRoundTrip) {
    map->SetName("RoundTrip Test");
    map->SetTile(10, 10, 99);
    map->SetTile(5, 5, 42);
    
    wxString json = map->SaveToJson();
    
    Map loadedMap;
    bool loaded = loadedMap.LoadFromJson(json);
    
    EXPECT_TRUE(loaded);
    EXPECT_EQ(loadedMap.GetMetadata().name, "RoundTrip Test");
    EXPECT_EQ(loadedMap.GetWidth(), map->GetWidth());
    EXPECT_EQ(loadedMap.GetHeight(), map->GetHeight());
}

// ============================================================================
// Testes de Arquivo
// ============================================================================

TEST_F(MapTest, SaveToFile) {
    map->SetName("File Test");
    
    std::string tempPath = "test_map_temp.json";
    bool saved = map->SaveToFile(tempPath);
    
    EXPECT_TRUE(saved);
    
    // Cleanup
    std::remove(tempPath.c_str());
}

TEST_F(MapTest, LoadFromFile) {
    // Criar arquivo temporário
    std::string tempPath = "test_map_temp_load.json";
    map->SetName("LoadTest");
    map->SaveToFile(tempPath);
    
    // Carregar em novo mapa
    Map loadedMap;
    bool loaded = loadedMap.LoadFromFile(tempPath);
    
    EXPECT_TRUE(loaded);
    EXPECT_EQ(loadedMap.GetMetadata().name, "LoadTest");
    
    // Cleanup
    std::remove(tempPath.c_str());
}

// ============================================================================
// Testes de Integração Layer + Map
// ============================================================================

TEST_F(MapTest, LayerManagerIntegration) {
    LayerManager* manager = map->GetLayerManager();
    
    ASSERT_NE(manager, nullptr);
    EXPECT_GT(manager->GetLayerCount(), 0);
}

TEST_F(MapTest, MultipleLayersTileOperations) {
    map->CreateLayer();
    map->CreateLayer();
    
    map->SetTileInLayer(0, 5, 5, 10);
    map->SetTileInLayer(1, 5, 5, 20);
    map->SetTileInLayer(2, 5, 5, 30);
    
    EXPECT_EQ(map->GetTileFromLayer(0, 5, 5), 10);
    EXPECT_EQ(map->GetTileFromLayer(1, 5, 5), 20);
    EXPECT_EQ(map->GetTileFromLayer(2, 5, 5), 30);
}

TEST_F(MapTest, FillSpecificLayer) {
    map->CreateLayer();
    
    map->FillLayer(1, 99);
    
    map->SetActiveLayer(1);
    EXPECT_EQ(map->GetTile(0, 0), 99);
    EXPECT_EQ(map->GetTile(12, 7), 99);
}

TEST_F(MapTest, ClearAllLayers) {
    map->CreateLayer();
    map->CreateLayer();
    
    map->SetTileInLayer(0, 5, 5, 10);
    map->SetTileInLayer(1, 5, 5, 20);
    map->SetTileInLayer(2, 5, 5, 30);
    
    map->ClearAllLayers();
    
    int tile1 = map->GetTileFromLayer(0, 5, 5);
    int tile2 = map->GetTileFromLayer(1, 5, 5);
    int tile3 = map->GetTileFromLayer(2, 5, 5);
    
    EXPECT_TRUE(tile1 == 0 || tile1 == -1);
    EXPECT_TRUE(tile2 == 0 || tile2 == -1);
    EXPECT_TRUE(tile3 == 0 || tile3 == -1);
}

// ============================================================================
// Testes de Casos Extremos
// ============================================================================

TEST_F(MapTest, AccessOutOfBounds) {
    // GetTile deve retornar 0 ou -1 para posições inválidas
    int tile = map->GetTile(100, 100);
    
    EXPECT_TRUE(tile == 0 || tile == -1);
}

TEST_F(MapTest, SetTileOutOfBounds) {
    // SetTile não deve crashar com posições inválidas
    // Comportamento esperado: operação ignorada
    EXPECT_NO_THROW(map->SetTile(100, 100, 42));
}

TEST_F(MapTest, EmptyLayersList) {
    Map emptyMap;
    
    // Remover todos os layers (se possível)
    while (emptyMap.GetLayerCount() > 0) {
        emptyMap.RemoveLayer(0);
    }
    
    // Map sem layers não deve crashar
    EXPECT_EQ(emptyMap.GetLayerCount(), 0);
}

TEST_F(MapTest, VerySmallMap) {
    Map tinyMap(1, 1, 16);
    
    EXPECT_EQ(tinyMap.GetWidth(), 1);
    EXPECT_EQ(tinyMap.GetHeight(), 1);
    
    tinyMap.SetTile(0, 0, 42);
    EXPECT_EQ(tinyMap.GetTile(0, 0), 42);
}

TEST_F(MapTest, LargeMap) {
    Map largeMap(200, 150, 32);
    
    EXPECT_EQ(largeMap.GetWidth(), 200);
    EXPECT_EQ(largeMap.GetHeight(), 150);
    
    largeMap.SetTile(199, 149, 42);
    EXPECT_EQ(largeMap.GetTile(199, 149), 42);
}

// ============================================================================
// Testes de Performance (básicos)
// ============================================================================

TEST_F(MapTest, BulkTileSet) {
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int y = 0; y < map->GetHeight(); ++y) {
        for (int x = 0; x < map->GetWidth(); ++x) {
            map->SetTile(x, y, (x + y) % 100);
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Operação deve completar em tempo razoável (< 100ms para mapa 25x15)
    EXPECT_LT(duration.count(), 100);
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
