# Padrões de Codificação - Projeto Lumy

## Tratamento de Strings UTF-8

### ✅ **Abordagem Recomendada**

Para manter consistência e facilitar manutenção, use **SEMPRE** a macro `UTF8()` para strings com caracteres especiais:

```cpp
#include "utf8_strings.h"

// ✅ Correto - Sempre usar a macro
.Caption(UTF8("Árvore do Projeto"))
new wxStaticText(this, wxID_ANY, UTF8("Localização:"))
wxMessageBox(UTF8("Versão 1.0"), UTF8("Sobre"))

// ❌ Incorreto - Misturar abordagens
.Caption(wxString::FromUTF8("Árvore do Projeto"))  // Muito verboso
.Caption("Árvore do Projeto")                       // Pode dar problema no Windows
```

### **Regras Gerais**

1. **TODAS** as strings com acentos devem usar `UTF8()`
2. Strings simples sem acentos podem usar string literal direta
3. **Sempre incluir** `"utf8_strings.h"` nos arquivos .cpp que usam interface
4. **Nunca misturar** `wxString::FromUTF8()` com `UTF8()` no mesmo arquivo

### **Arquivos que Precisam Ser Refatorados**

- [x] `editor_frame.cpp` (parcialmente feito)
- [ ] `new_project_dialog.cpp` 
- [ ] `property_grid_panel.cpp`
- [ ] `project_tree_panel.cpp`
- [ ] `viewport_panel.cpp`

### **Exemplos de Refatoração**

#### Antes (Inconsistente):
```cpp
// Mistura problemática
.Caption(wxString::FromUTF8("Propriedades"))
.Caption("Editor de Mapa")
new wxStaticText(this, wxID_ANY, wxString::FromUTF8("Localização:"))
new wxStaticText(this, wxID_ANY, "Nome:")
```

#### Depois (Consistente):
```cpp
#include "utf8_strings.h"

// Padrão uniforme
.Caption(UTF8("Propriedades"))
.Caption("Editor de Mapa")  // Sem acentos, pode usar string direta
new wxStaticText(this, wxID_ANY, UTF8("Localização:"))
new wxStaticText(this, wxID_ANY, "Nome:")  // Sem acentos
```

### **Vantagens da Macro UTF8()**

1. **Consistência**: Um padrão único em todo o projeto
2. **Legibilidade**: Muito mais limpo que `wxString::FromUTF8()`
3. **Manutenibilidade**: Fácil mudança global se necessário
4. **Portabilidade**: Funciona diferente no Windows vs Linux automaticamente
5. **Performance**: Sem overhead desnecessário em sistemas que não precisam
