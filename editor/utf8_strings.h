/**
 * Utilitários para strings UTF-8 consistentes
 */

#pragma once

#include <wx/wx.h>

// Macro para uniformizar strings UTF-8 em todo o projeto
#ifdef _WIN32
    // No Windows, sempre usar FromUTF8 para garantir encoding correto
    #define UTF8(str) wxString::FromUTF8(str)
#else
    // Em outros sistemas, usar string direta (já funciona)
    #define UTF8(str) wxString(str)
#endif

// Alternativa mais explícita (se preferir):
#define LUMY_TEXT(str) wxString::FromUTF8(str)
