/**
 * Configuração global de locale para UTF-8
 */

#pragma once

#include <wx/wx.h>
#include <wx/intl.h>

class GlobalLocaleSetup {
public:
    static void Initialize() {
        // Configurar locale global para UTF-8
        #ifdef _WIN32
            // No Windows, configurar para usar UTF-8
            setlocale(LC_ALL, ".UTF8");
            
            // Configurar wxWidgets para UTF-8
            wxLocale::AddCatalogLookupPathPrefix(".");
        #endif
        
        // Outras configurações globais podem ir aqui
    }
};
