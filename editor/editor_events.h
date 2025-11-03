/**
 * Editor Events - Sistema de eventos customizados para comunicação entre panes
 */

#pragma once

#include <wx/wx.h>
#include <wx/event.h>

// Forward declarations
struct TileInfo;
struct MapInfo;
struct ProjectInfo;

// Tipos de elementos selecionáveis no editor
enum class SelectionType
{
    NONE,
    MAP_FILE,
    DATA_FILE,
    ASSET_FILE,
    TILE,
    EVENT,
    NPC
};

// Estrutura para informações de seleção
struct SelectionInfo
{
    SelectionType type;
    wxString filePath;
    wxString displayName;
    wxPoint tilePosition; // Para tiles
    int tileType;         // Tipo do tile selecionado
    
    SelectionInfo() 
        : type(SelectionType::NONE), tilePosition(-1, -1), tileType(-1) {}
};

// Evento de mudança de seleção
class SelectionChangeEvent : public wxCommandEvent
{
public:
    SelectionChangeEvent(wxEventType commandType = wxEVT_NULL, int winid = 0)
        : wxCommandEvent(commandType, winid) {}
    
    SelectionChangeEvent(const SelectionChangeEvent& event)
        : wxCommandEvent(event), m_selectionInfo(event.m_selectionInfo) {}
    
    wxEvent* Clone() const override { return new SelectionChangeEvent(*this); }
    
    void SetSelectionInfo(const SelectionInfo& info) { m_selectionInfo = info; }
    const SelectionInfo& GetSelectionInfo() const { return m_selectionInfo; }
    
private:
    SelectionInfo m_selectionInfo;
};

// Evento de mudança de propriedade
class PropertyChangeEvent : public wxCommandEvent
{
public:
    PropertyChangeEvent(wxEventType commandType = wxEVT_NULL, int winid = 0)
        : wxCommandEvent(commandType, winid) {}
    
    PropertyChangeEvent(const PropertyChangeEvent& event)
        : wxCommandEvent(event), m_propertyName(event.m_propertyName), 
          m_propertyValue(event.m_propertyValue) {}
    
    wxEvent* Clone() const override { return new PropertyChangeEvent(*this); }
    
    void SetPropertyChange(const wxString& name, const wxVariant& value) 
    { 
        m_propertyName = name; 
        m_propertyValue = value; 
    }
    
    const wxString& GetPropertyName() const { return m_propertyName; }
    const wxVariant& GetPropertyValue() const { return m_propertyValue; }
    
private:
    wxString m_propertyName;
    wxVariant m_propertyValue;
};

// Evento de mudança de projeto
class ProjectChangeEvent : public wxCommandEvent
{
public:
    ProjectChangeEvent(wxEventType commandType = wxEVT_NULL, int winid = 0)
        : wxCommandEvent(commandType, winid) {}
    
    ProjectChangeEvent(const ProjectChangeEvent& event)
        : wxCommandEvent(event), m_projectPath(event.m_projectPath),
          m_isLoaded(event.m_isLoaded) {}
    
    wxEvent* Clone() const override { return new ProjectChangeEvent(*this); }
    
    void SetProjectInfo(const wxString& path, bool loaded) 
    { 
        m_projectPath = path; 
        m_isLoaded = loaded; 
    }
    
    const wxString& GetProjectPath() const { return m_projectPath; }
    bool IsLoaded() const { return m_isLoaded; }
    
private:
    wxString m_projectPath;
    bool m_isLoaded;
};

// Evento de solicitação de troca de mapa
class MapChangeRequestEvent : public wxCommandEvent
{
public:
    MapChangeRequestEvent(wxEventType commandType = wxEVT_NULL, int winid = 0)
        : wxCommandEvent(commandType, winid) {}
    
    MapChangeRequestEvent(const MapChangeRequestEvent& event)
        : wxCommandEvent(event), m_mapPath(event.m_mapPath) {}
    
    wxEvent* Clone() const override { return new MapChangeRequestEvent(*this); }
    
    const wxString& GetMapPath() const { return m_mapPath; }
    void SetMapPath(const wxString& path) { m_mapPath = path; }
    
private:
    wxString m_mapPath;
};

// Declarar tipos de eventos customizados
wxDECLARE_EVENT(EVT_SELECTION_CHANGED, SelectionChangeEvent);
wxDECLARE_EVENT(EVT_PROPERTY_CHANGED, PropertyChangeEvent);
wxDECLARE_EVENT(EVT_PROJECT_CHANGED, ProjectChangeEvent);
wxDECLARE_EVENT(EVT_MAP_CHANGE_REQUEST, MapChangeRequestEvent);

// Macros para event table
#define EVT_SELECTION_CHANGE(func) \
    wx__DECLARE_EVT0(EVT_SELECTION_CHANGED, &func)

#define EVT_PROPERTY_CHANGE(func) \
    wx__DECLARE_EVT0(EVT_PROPERTY_CHANGED, &func)

#define EVT_PROJECT_CHANGE(func) \
    wx__DECLARE_EVT0(EVT_PROJECT_CHANGED, &func)

#define EVT_MAP_CHANGE_REQUEST(func) \
    wx__DECLARE_EVT0(EVT_MAP_CHANGE_REQUEST, &func)
