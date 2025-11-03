/**
 * i18n - Sistema de Internacionalização do Lumy Editor
 * Suporta múltiplos idiomas através de arquivos JSON
 */

#pragma once

#include <wx/wx.h>
#include <nlohmann/json.hpp>
#include <string>
#include <map>

class Localization
{
public:
    // Singleton
    static Localization& Get();
    
    // Inicializar com idioma padrão
    bool Initialize(const wxString& defaultLang = "pt_BR");
    
    // Carregar idioma específico
    bool LoadLanguage(const wxString& langCode);
    
    // Obter texto traduzido
    wxString GetText(const wxString& key) const;
    wxString GetText(const std::string& key) const;
    
    // Obter idioma atual
    wxString GetCurrentLanguage() const { return m_currentLanguage; }
    
    // Listar idiomas disponíveis
    std::vector<wxString> GetAvailableLanguages() const;
    
    // Verificar se uma chave existe
    bool HasKey(const wxString& key) const;
    
private:
    Localization() = default;
    ~Localization() = default;
    
    // Não permitir cópia
    Localization(const Localization&) = delete;
    Localization& operator=(const Localization&) = delete;
    
    // Carregar arquivo JSON
    bool LoadTranslationFile(const wxString& filePath);
    
    // Obter valor do JSON usando notação de ponto (ex: "menu.file")
    wxString GetValueFromJson(const wxString& key) const;
    
    nlohmann::json m_translations;
    wxString m_currentLanguage;
    wxString m_localeDir;
};

// Macros para facilitar o uso (não conflitar com macros do wxWidgets)
// Use L_() para tradução simples e L_F() para tradução com formato
#define L_(key) Localization::Get().GetText(wxString(key))
#define L_F(key, ...) wxString::Format(Localization::Get().GetText(wxString(key)), __VA_ARGS__)
