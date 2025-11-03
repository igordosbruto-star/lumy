/**
 * Testes unitários para PaintTools
 */

#include <gtest/gtest.h>
#include <wx/log.h>
#include "../../editor/paint_tools.h"
#include "../../editor/map.h"

// Environment global para desabilitar logs do wxWidgets
class PaintToolsTestEnvironment : public ::testing::Environment {
public:
    void SetUp() override {
        wxLog::EnableLogging(false);
    }
    
    void TearDown() override {
        wxLog::EnableLogging(true);
    }
};

class PaintToolsTest : public ::testing::Test {
protected:
    void SetUp() override {
        map = std::make_unique<Map>(20, 20, 32);
    }

    void TearDown() override {
        map.reset();
    }

    std::unique_ptr<Map> map;
};

// ============================================================================
// TilePosition Tests
// ============================================================================

TEST_F(PaintToolsTest, TilePositionEquality) {
    TilePosition pos1(5, 10);
    TilePosition pos2(5, 10);
    TilePosition pos3(5, 11);
    
    EXPECT_TRUE(pos1 == pos2);
    EXPECT_FALSE(pos1 == pos3);
    EXPECT_TRUE(pos1 != pos3);
}

// ============================================================================
// SelectionArea Tests
// ============================================================================

TEST_F(PaintToolsTest, SelectionAreaInitiallyInactive) {
    SelectionArea selection;
    
    EXPECT_FALSE(selection.IsValid());
    EXPECT_FALSE(selection.active);
}

TEST_F(PaintToolsTest, SelectionAreaGetWidthHeight) {
    SelectionArea selection;
    selection.active = true;
    selection.start = TilePosition(5, 5);
    selection.end = TilePosition(10, 15);
    
    EXPECT_EQ(selection.GetWidth(), 6);
    EXPECT_EQ(selection.GetHeight(), 11);
}

TEST_F(PaintToolsTest, SelectionAreaGetTopLeft) {
    SelectionArea selection;
    selection.active = true;
    selection.start = TilePosition(10, 15);
    selection.end = TilePosition(5, 5);
    
    TilePosition topLeft = selection.GetTopLeft();
    
    EXPECT_EQ(topLeft.x, 5);
    EXPECT_EQ(topLeft.y, 5);
}

TEST_F(PaintToolsTest, SelectionAreaGetBottomRight) {
    SelectionArea selection;
    selection.active = true;
    selection.start = TilePosition(5, 5);
    selection.end = TilePosition(10, 15);
    
    TilePosition bottomRight = selection.GetBottomRight();
    
    EXPECT_EQ(bottomRight.x, 10);
    EXPECT_EQ(bottomRight.y, 15);
}

TEST_F(PaintToolsTest, SelectionAreaContains) {
    SelectionArea selection;
    selection.active = true;
    selection.start = TilePosition(5, 5);
    selection.end = TilePosition(10, 10);
    
    EXPECT_TRUE(selection.Contains(5, 5));
    EXPECT_TRUE(selection.Contains(7, 7));
    EXPECT_TRUE(selection.Contains(10, 10));
    EXPECT_FALSE(selection.Contains(4, 5));
    EXPECT_FALSE(selection.Contains(11, 10));
}

// ============================================================================
// BrushTool Tests
// ============================================================================

TEST_F(PaintToolsTest, BrushToolBasic) {
    BrushTool brush;
    
    EXPECT_EQ(brush.GetName(), "Pincel");
    EXPECT_EQ(brush.GetType(), PaintTool::BRUSH);
}

TEST_F(PaintToolsTest, BrushToolPaintSingleTile) {
    BrushTool brush;
    TilePosition pos(10, 10);
    
    brush.OnMouseDown(pos, 42, map.get());
    
    EXPECT_EQ(map->GetTile(10, 10), 42);
}

// ============================================================================
// EraserTool Tests
// ============================================================================

TEST_F(PaintToolsTest, EraserToolBasic) {
    EraserTool eraser;
    
    EXPECT_EQ(eraser.GetName(), "Borracha");
    EXPECT_EQ(eraser.GetType(), PaintTool::ERASER);
}

TEST_F(PaintToolsTest, EraserToolErasesTile) {
    map->SetTile(10, 10, 42);
    EXPECT_EQ(map->GetTile(10, 10), 42);
    
    EraserTool eraser;
    TilePosition pos(10, 10);
    
    eraser.OnMouseDown(pos, -1, map.get());
    
    int tile = map->GetTile(10, 10);
    EXPECT_TRUE(tile == 0 || tile == -1);
}

// ============================================================================
// SelectionTool Tests
// ============================================================================

TEST_F(PaintToolsTest, SelectionToolBasic) {
    SelectionTool selection;
    
    EXPECT_EQ(selection.GetName(), "Seleção");
    EXPECT_EQ(selection.GetType(), PaintTool::SELECTION);
}

TEST_F(PaintToolsTest, SelectionToolCreateSelection) {
    SelectionTool selTool;
    
    TilePosition start(5, 5);
    TilePosition end(10, 10);
    
    selTool.OnMouseDown(start, 0, map.get());
    selTool.OnMouseMove(end, 0, map.get());
    selTool.OnMouseUp(end, 0, map.get());
    
    const SelectionArea& sel = selTool.GetSelection();
    
    EXPECT_TRUE(sel.IsValid());
}

TEST_F(PaintToolsTest, SelectionToolClearSelection) {
    SelectionTool selTool;
    
    selTool.OnMouseDown(TilePosition(5, 5), 0, map.get());
    selTool.OnMouseUp(TilePosition(10, 10), 0, map.get());
    
    EXPECT_TRUE(selTool.GetSelection().IsValid());
    
    selTool.ClearSelection();
    
    EXPECT_FALSE(selTool.GetSelection().IsValid());
}

// ============================================================================
// BucketTool Tests
// ============================================================================

TEST_F(PaintToolsTest, BucketToolBasic) {
    BucketTool bucket;
    
    EXPECT_EQ(bucket.GetName(), "Preenchimento");
    EXPECT_EQ(bucket.GetType(), PaintTool::BUCKET);
}

TEST_F(PaintToolsTest, BucketToolFillsSingleTile) {
    BucketTool bucket;
    TilePosition pos(10, 10);
    
    bucket.OnMouseDown(pos, 99, map.get());
    
    EXPECT_EQ(map->GetTile(10, 10), 99);
}

TEST_F(PaintToolsTest, BucketToolFillsConnectedArea) {
    // Criar uma área vazia (todos tiles em 0)
    map->Fill(0);
    
    // Colocar bordas - linha horizontal completa
    for (int i = 0; i < map->GetWidth(); ++i) {
        map->SetTile(i, 5, 1);  // Linha horizontal completa
    }
    
    // Preencher área acima da linha
    BucketTool bucket;
    TilePosition pos(5, 3);
    
    bucket.OnMouseDown(pos, 42, map.get());
    
    // Tiles acima da linha devem ser 42
    EXPECT_EQ(map->GetTile(5, 3), 42);
    EXPECT_EQ(map->GetTile(5, 0), 42);
    
    // Tiles abaixo da linha devem permanecer 0
    EXPECT_EQ(map->GetTile(5, 7), 0);
}

// ============================================================================
// Tool Lifecycle Tests
// ============================================================================

TEST_F(PaintToolsTest, ToolActivationCycle) {
    BrushTool brush;
    
    EXPECT_FALSE(brush.IsActive());
    
    brush.SetActive(true);
    EXPECT_TRUE(brush.IsActive());
    
    brush.SetActive(false);
    EXPECT_FALSE(brush.IsActive());
}

