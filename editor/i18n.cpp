/**
 * i18n - Implementação do sistema de internacionalização
 */

#pragma execution_character_set("utf-8")

#include "i18n.h"
#include <wx/filename.h>
#include <wx/dir.h>
#include <wx/stdpaths.h>
#include <fstream>
#include <sstream>

// Singleton
Localization& Localization::Get()
{
    static Localization instance;
    return instance;
}

bool Localization::Initialize(const wxString& defaultLang)
{
    // Determinar diretório de locales
    // Tentar: ./locale, ../locale, ou diretório do executável/locale
    wxFileName exePath(wxStandardPaths::Get().GetExecutablePath());
    wxString exeDir = exePath.GetPath();
    
    std::vector<wxString> possibleDirs = {
        exeDir + "/locale",
        exeDir + "/../locale",
        wxGetCwd() + "/locale",
        wxGetCwd() + "/editor/locale"
    };
    
    for (const auto& dir : possibleDirs) {
        if (wxDirExists(dir)) {
            m_localeDir = dir;
            wxLogMessage("Diretório de locale encontrado: %s", m_localeDir);
            break;
        }
    }
    
    if (m_localeDir.IsEmpty()) {
        // Se não encontrou, criar no diretório atual
        m_localeDir = wxGetCwd() + "/locale";
        if (!wxDirExists(m_localeDir)) {
            wxMkdir(m_localeDir);
            wxLogWarning("Diretório de locale criado: %s", m_localeDir);
        }
    }
    
    // Carregar idioma padrão
    return LoadLanguage(defaultLang);
}

bool Localization::LoadLanguage(const wxString& langCode)
{
    wxString filePath = m_localeDir + "/" + langCode + ".json";
    
    if (!wxFileExists(filePath)) {
        wxLogError("Arquivo de tradução não encontrado: %s", filePath);
        return false;
    }
    
    if (LoadTranslationFile(filePath)) {
        m_currentLanguage = langCode;
        wxLogMessage("Idioma carregado com sucesso: %s", langCode);
        return true;
    }
    
    return false;
}

bool Localization::LoadTranslationFile(const wxString& filePath)
{
    try {
        std::ifstream file(filePath.ToStdString());
        if (!file.is_open()) {
            wxLogError("Não foi possível abrir arquivo: %s", filePath);
            return false;
        }
        
        m_translations = nlohmann::json::parse(file);
        file.close();
        
        wxLogMessage("Arquivo de tradução carregado: %s", filePath);
        return true;
        
    } catch (const nlohmann::json::exception& e) {
        wxLogError("Erro ao parsear JSON: %s", e.what());
        return false;
    } catch (const std::exception& e) {
        wxLogError("Erro ao carregar tradução: %s", e.what());
        return false;
    }
}

wxString Localization::GetText(const wxString& key) const
{
    if (m_translations.empty()) {
        wxLogWarning("Traduções não carregadas, retornando chave: %s", key);
        return key;
    }
    
    wxString value = GetValueFromJson(key);
    
    if (value.IsEmpty()) {
        wxLogWarning("Chave de tradução não encontrada: %s", key);
        return key; // Retornar a chave se não encontrar
    }
    
    return value;
}

wxString Localization::GetText(const std::string& key) const
{
    return GetText(wxString(key));
}

wxString Localization::GetValueFromJson(const wxString& key) const
{
    try {
        // Dividir a chave por pontos (ex: "menu.file" -> ["menu", "file"])
        std::vector<std::string> keys;
        std::string keyStr = key.ToStdString();
        std::stringstream ss(keyStr);
        std::string item;
        
        while (std::getline(ss, item, '.')) {
            keys.push_back(item);
        }
        
        // Navegar no JSON
        nlohmann::json current = m_translations;
        for (const auto& k : keys) {
            if (current.contains(k)) {
                current = current[k];
            } else {
                return wxEmptyString;
            }
        }
        
        // Retornar o valor final como string
        if (current.is_string()) {
            return wxString::FromUTF8(current.get<std::string>());
        }
        
    } catch (const nlohmann::json::exception& e) {
        wxLogWarning("Erro ao obter valor do JSON: %s", e.what());
    }
    
    return wxEmptyString;
}

std::vector<wxString> Localization::GetAvailableLanguages() const
{
    std::vector<wxString> languages;
    
    if (!wxDirExists(m_localeDir)) {
        return languages;
    }
    
    wxDir dir(m_localeDir);
    if (!dir.IsOpened()) {
        return languages;
    }
    
    wxString filename;
    bool cont = dir.GetFirst(&filename, "*.json", wxDIR_FILES);
    
    while (cont) {
        // Remover extensão .json
        wxFileName fn(filename);
        languages.push_back(fn.GetName());
        cont = dir.GetNext(&filename);
    }
    
    return languages;
}

bool Localization::HasKey(const wxString& key) const
{
    return !GetValueFromJson(key).IsEmpty();
}
