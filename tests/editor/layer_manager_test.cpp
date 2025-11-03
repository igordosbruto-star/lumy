/**
 * Testes unitários para LayerManager
 */

#include <gtest/gtest.h>
#include <wx/log.h>
#include "../../editor/layer_manager.h"
#include "../../editor/layer.h"

// Environment global para desabilitar logs do wxWidgets
class TestEnvironment : public ::testing::Environment {
public:
    void SetUp() override {
        // Suprimir todos os logs do wxWidgets durante os testes
        wxLog::EnableLogging(false);
    }
    
    void TearDown() override {
        wxLog::EnableLogging(true);
    }
};

class LayerManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = std::make_unique<LayerManager>();
        // Limpar layers padrão para testes começarem em estado limpo
        manager->ClearLayers();
    }

    void TearDown() override {
        manager.reset();
    }

    std::unique_ptr<LayerManager> manager;
};

// ============================================================================
// Testes de Criação de Layers
// ============================================================================

TEST_F(LayerManagerTest, CreateLayerBasic) {
    LayerProperties props;
    props.name = "Test Layer";
    props.type = LayerType::TILE_LAYER;
    
    Layer* layer = manager->CreateLayer(props);
    
    ASSERT_NE(layer, nullptr);
    EXPECT_EQ(layer->GetName(), "Test Layer");
    EXPECT_EQ(layer->GetType(), LayerType::TILE_LAYER);
    EXPECT_EQ(manager->GetLayerCount(), 1);
}

TEST_F(LayerManagerTest, CreateLayerWithDimensions) {
    Layer* layer = manager->CreateLayer(20, 15);
    
    ASSERT_NE(layer, nullptr);
    EXPECT_EQ(layer->GetWidth(), 20);
    EXPECT_EQ(layer->GetHeight(), 15);
    EXPECT_EQ(manager->GetLayerCount(), 1);
}

TEST_F(LayerManagerTest, CreateMultipleLayers) {
    manager->CreateLayer();
    manager->CreateLayer();
    manager->CreateLayer();
    
    EXPECT_EQ(manager->GetLayerCount(), 3);
}

TEST_F(LayerManagerTest, CreateLayerAutoIncrementId) {
    Layer* layer1 = manager->CreateLayer();
    Layer* layer2 = manager->CreateLayer();
    Layer* layer3 = manager->CreateLayer();
    
    // Layers are unique objects, checked by pointer
    EXPECT_NE(layer1, layer2);
    EXPECT_NE(layer2, layer3);
}

// ============================================================================
// Testes de Remoção de Layers
// ============================================================================

TEST_F(LayerManagerTest, RemoveLayerByIndex) {
    manager->CreateLayer();
    manager->CreateLayer();
    manager->CreateLayer();
    
    EXPECT_EQ(manager->GetLayerCount(), 3);
    
    bool removed = manager->RemoveLayer(1);
    
    EXPECT_TRUE(removed);
    EXPECT_EQ(manager->GetLayerCount(), 2);
}

TEST_F(LayerManagerTest, RemoveLayerByName) {
    Layer* layer = manager->CreateLayer();
    layer->SetName("ToRemove");
    manager->CreateLayer();
    
    EXPECT_EQ(manager->GetLayerCount(), 2);
    
    bool removed = manager->RemoveLayer("ToRemove");
    
    EXPECT_TRUE(removed);
    EXPECT_EQ(manager->GetLayerCount(), 1);
}

TEST_F(LayerManagerTest, RemoveLayerInvalidIndex) {
    manager->CreateLayer();
    
    bool removed = manager->RemoveLayer(10);
    
    EXPECT_FALSE(removed);
    EXPECT_EQ(manager->GetLayerCount(), 1);
}

TEST_F(LayerManagerTest, ClearAllLayers) {
    manager->CreateLayer();
    manager->CreateLayer();
    manager->CreateLayer();
    
    EXPECT_EQ(manager->GetLayerCount(), 3);
    
    manager->ClearLayers();
    
    EXPECT_EQ(manager->GetLayerCount(), 0);
}

// ============================================================================
// Testes de Acesso a Layers
// ============================================================================

TEST_F(LayerManagerTest, GetLayerByIndex) {
    Layer* layer1 = manager->CreateLayer();
    Layer* layer2 = manager->CreateLayer();
    
    layer1->SetName("First");
    layer2->SetName("Second");
    
    EXPECT_STREQ(manager->GetLayer(0)->GetName().c_str(), "First");
    EXPECT_STREQ(manager->GetLayer(1)->GetName().c_str(), "Second");
}

TEST_F(LayerManagerTest, GetLayerByName) {
    Layer* layer = manager->CreateLayer();
    layer->SetName("MyLayer");
    
    Layer* found = manager->GetLayer("MyLayer");
    
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->GetName(), "MyLayer");
}

TEST_F(LayerManagerTest, GetLayerInvalidIndex) {
    manager->CreateLayer();
    
    Layer* layer = manager->GetLayer(10);
    
    EXPECT_EQ(layer, nullptr);
}

TEST_F(LayerManagerTest, GetActiveLayer) {
    manager->CreateLayer();
    manager->CreateLayer();
    
    manager->SetActiveLayer(1);
    
    Layer* active = manager->GetActiveLayer();
    ASSERT_NE(active, nullptr);
    EXPECT_EQ(manager->GetActiveLayerIndex(), 1);
}

// ============================================================================
// Testes de Ordenação de Layers
// ============================================================================

TEST_F(LayerManagerTest, MoveLayerUp) {
    Layer* layer1 = manager->CreateLayer();
    Layer* layer2 = manager->CreateLayer();
    
    layer1->SetName("Bottom");
    layer2->SetName("Top");
    
    // Mover layer 1 (Top) para cima (menor índice) -> troca com layer 0
    bool moved = manager->MoveLayerUp(1);
    
    EXPECT_TRUE(moved);
    EXPECT_STREQ(manager->GetLayer(0)->GetName().c_str(), "Top");
    EXPECT_STREQ(manager->GetLayer(1)->GetName().c_str(), "Bottom");
}

TEST_F(LayerManagerTest, MoveLayerDown) {
    Layer* layer1 = manager->CreateLayer();
    Layer* layer2 = manager->CreateLayer();
    
    layer1->SetName("Bottom");
    layer2->SetName("Top");
    
    // Mover layer 0 para baixo (índice 1)
    bool moved = manager->MoveLayerDown(0);
    
    EXPECT_TRUE(moved);
    EXPECT_STREQ(manager->GetLayer(0)->GetName().c_str(), "Top");
    EXPECT_STREQ(manager->GetLayer(1)->GetName().c_str(), "Bottom");
}

TEST_F(LayerManagerTest, MoveLayerUpAtTop) {
    manager->CreateLayer();
    manager->CreateLayer();
    
    // Tentar mover o layer inferior (0) quando já está no fundo
    bool moved = manager->MoveLayerUp(0);
    
    EXPECT_FALSE(moved);
}

TEST_F(LayerManagerTest, MoveLayerDownAtBottom) {
    manager->CreateLayer();
    manager->CreateLayer();
    
    // Tentar mover o layer superior (1) quando já está no topo
    bool moved = manager->MoveLayerDown(1);
    
    EXPECT_FALSE(moved);
}

// ============================================================================
// Testes de Operações de Layer
// ============================================================================

TEST_F(LayerManagerTest, DuplicateLayer) {
    Layer* original = manager->CreateLayer();
    original->SetName("Original");
    original->SetOpacity(0.5f);
    original->SetTile(5, 5, 42);
    
    Layer* duplicate = manager->DuplicateLayer(0);
    
    ASSERT_NE(duplicate, nullptr);
    EXPECT_EQ(manager->GetLayerCount(), 2);
    EXPECT_EQ(duplicate->GetOpacity(), 0.5f);
    EXPECT_EQ(duplicate->GetTile(5, 5), 42);
    EXPECT_NE(duplicate->GetName(), "Original"); // Nome deve ser diferente
}

TEST_F(LayerManagerTest, MergeLayerDown) {
    Layer* upper = manager->CreateLayer(10, 10);  // Índice 0
    Layer* lower = manager->CreateLayer(10, 10);  // Índice 1
    
    upper->SetTile(5, 5, 10);
    lower->SetTile(5, 5, 20);
    lower->SetTile(3, 3, 30);
    
    // Merge layer 0 (upper) down into layer 1 (lower) -> removes layer 0
    bool merged = manager->MergeLayerDown(0);
    
    EXPECT_TRUE(merged);
    EXPECT_EQ(manager->GetLayerCount(), 1);
    
    // Tile do upper deve sobrescrever no lower
    Layer* result = manager->GetLayer(0);  // O lower agora é índice 0
    EXPECT_EQ(result->GetTile(5, 5), 10);
    EXPECT_EQ(result->GetTile(3, 3), 30);
}

TEST_F(LayerManagerTest, FlattenLayers) {
    manager->CreateLayer(10, 10);
    manager->CreateLayer(10, 10);
    manager->CreateLayer(10, 10);
    
    manager->GetLayer(0)->SetTile(0, 0, 1);
    manager->GetLayer(1)->SetTile(1, 1, 2);
    manager->GetLayer(2)->SetTile(2, 2, 3);
    
    manager->FlattenLayers();
    
    EXPECT_EQ(manager->GetLayerCount(), 1);
    
    Layer* flattened = manager->GetLayer(0);
    EXPECT_EQ(flattened->GetTile(0, 0), 1);
    EXPECT_EQ(flattened->GetTile(1, 1), 2);
    EXPECT_EQ(flattened->GetTile(2, 2), 3);
}

// ============================================================================
// Testes de Visibilidade e Opacidade
// ============================================================================

TEST_F(LayerManagerTest, SetLayerVisibility) {
    Layer* layer = manager->CreateLayer();
    
    EXPECT_TRUE(layer->IsVisible());
    
    manager->SetLayerVisibility(0, false);
    
    EXPECT_FALSE(layer->IsVisible());
}

TEST_F(LayerManagerTest, SetAllLayersVisible) {
    manager->CreateLayer();
    manager->CreateLayer();
    manager->CreateLayer();
    
    manager->GetLayer(0)->SetVisible(false);
    manager->GetLayer(1)->SetVisible(false);
    
    manager->SetAllLayersVisible(true);
    
    EXPECT_TRUE(manager->GetLayer(0)->IsVisible());
    EXPECT_TRUE(manager->GetLayer(1)->IsVisible());
    EXPECT_TRUE(manager->GetLayer(2)->IsVisible());
}

TEST_F(LayerManagerTest, SetLayerOpacity) {
    Layer* layer = manager->CreateLayer();
    
    manager->SetLayerOpacity(0, 0.75f);
    
    EXPECT_FLOAT_EQ(layer->GetOpacity(), 0.75f);
}

// ============================================================================
// Testes de Busca e Filtros
// ============================================================================

TEST_F(LayerManagerTest, FindLayersByType) {
    manager->CreateLayer()->SetType(LayerType::TILE_LAYER);
    manager->CreateLayer()->SetType(LayerType::BACKGROUND_LAYER);
    manager->CreateLayer()->SetType(LayerType::TILE_LAYER);
    manager->CreateLayer()->SetType(LayerType::COLLISION_LAYER);
    
    auto tileLayers = manager->FindLayersByType(LayerType::TILE_LAYER);
    
    EXPECT_EQ(tileLayers.size(), 2);
}

TEST_F(LayerManagerTest, GetVisibleLayers) {
    manager->CreateLayer()->SetVisible(true);
    manager->CreateLayer()->SetVisible(false);
    manager->CreateLayer()->SetVisible(true);
    
    auto visibleLayers = manager->GetVisibleLayers();
    
    EXPECT_EQ(visibleLayers.size(), 2);
}

TEST_F(LayerManagerTest, GetUnlockedLayers) {
    manager->CreateLayer()->SetLocked(false);
    manager->CreateLayer()->SetLocked(true);
    manager->CreateLayer()->SetLocked(false);
    
    auto unlockedLayers = manager->GetUnlockedLayers();
    
    EXPECT_EQ(unlockedLayers.size(), 2);
}

// ============================================================================
// Testes de Nomes Únicos
// ============================================================================

TEST_F(LayerManagerTest, GenerateUniqueLayerName) {
    manager->CreateLayer()->SetName("Layer");
    
    wxString uniqueName = manager->GenerateUniqueLayerName("Layer");
    
    EXPECT_NE(uniqueName, "Layer");
    EXPECT_TRUE(uniqueName.StartsWith("Layer"));
}

TEST_F(LayerManagerTest, GenerateMultipleUniqueNames) {
    manager->CreateLayer()->SetName("Test");
    manager->CreateLayer()->SetName("Test 1");
    
    wxString name1 = manager->GenerateUniqueLayerName("Test");
    // Criar layer com o nome gerado para que o próximo seja diferente
    manager->CreateLayer()->SetName(name1);
    wxString name2 = manager->GenerateUniqueLayerName("Test");
    
    EXPECT_NE(name1, name2);
}

// ============================================================================
// Testes de Operações de Tiles
// ============================================================================

TEST_F(LayerManagerTest, SetTileInActiveLayer) {
    manager->CreateLayer(10, 10);
    manager->SetActiveLayer(0);
    
    manager->SetTile(5, 5, 42);
    
    EXPECT_EQ(manager->GetTile(5, 5), 42);
}

TEST_F(LayerManagerTest, SetTileInSpecificLayer) {
    manager->CreateLayer(10, 10);
    manager->CreateLayer(10, 10);
    
    manager->SetTileInLayer(1, 3, 3, 99);
    
    EXPECT_EQ(manager->GetTileFromLayer(1, 3, 3), 99);
}

TEST_F(LayerManagerTest, FillRect) {
    manager->CreateLayer(20, 20);
    manager->SetActiveLayer(0);
    
    manager->FillRect(5, 5, 3, 3, 42);
    
    EXPECT_EQ(manager->GetTile(5, 5), 42);
    EXPECT_EQ(manager->GetTile(6, 6), 42);
    EXPECT_EQ(manager->GetTile(7, 7), 42);
}

// ============================================================================
// Testes de Estado
// ============================================================================

TEST_F(LayerManagerTest, HasUnsavedChanges) {
    manager->CreateLayer();
    // Marcar como salvo após criação
    manager->MarkAllLayersSaved();
    
    EXPECT_FALSE(manager->HasUnsavedChanges());
    
    manager->GetLayer(0)->SetModified(true);
    
    EXPECT_TRUE(manager->HasUnsavedChanges());
}

TEST_F(LayerManagerTest, MarkAllLayersSaved) {
    manager->CreateLayer();
    manager->CreateLayer();
    
    manager->GetLayer(0)->SetModified(true);
    manager->GetLayer(1)->SetModified(true);
    
    manager->MarkAllLayersSaved();
    
    EXPECT_FALSE(manager->HasUnsavedChanges());
}

