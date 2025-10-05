/**
 * Editor Events - Implementação dos eventos customizados
 */

#include "editor_events.h"

// Definir os tipos de eventos
wxDEFINE_EVENT(EVT_SELECTION_CHANGED, SelectionChangeEvent);
wxDEFINE_EVENT(EVT_PROPERTY_CHANGED, PropertyChangeEvent);
wxDEFINE_EVENT(EVT_PROJECT_CHANGED, ProjectChangeEvent);
