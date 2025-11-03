# Sistema de Internacionalização (i18n) do Lumy Editor

## 📁 Estrutura de Arquivos

```
locale/
├── README.md       # Este arquivo
├── pt_BR.json      # Português do Brasil
├── en_US.json      # English (US)
└── [idioma].json   # Adicione mais idiomas aqui
```

## 🌍 Como Usar no Código

### 1. Incluir o header
```cpp
#include "i18n.h"
```

### 2. Inicializar no início do programa
```cpp
// No main() ou no construtor da EditorFrame
Localization::Get().Initialize("pt_BR");
```

### 3. Usar as macros para obter textos traduzidos

**Macro simples:**
```cpp
// Usar macro _(chave)
button->SetLabel(_("tools.paint"));  // Retorna: "Pintar" (pt_BR) ou "Paint" (en_US)
```

**Macro com formatação:**
```cpp
// Usar macro _F(chave, argumentos...)
wxString msg = _F("messages.map_loaded", "meu_mapa.json");
// Retorna: "Mapa carregado: meu_mapa.json"
```

## 📝 Estrutura do JSON

Os arquivos JSON usam **notação de ponto** para organizar as traduções:

```json
{
  "categoria": {
    "subcategoria": "Texto traduzido"
  }
}
```

### Exemplo:
```json
{
  "tools": {
    "paint": "Pintar"
  }
}
```

**Acesso no código:**
```cpp
_("tools.paint")  // Retorna: "Pintar"
```

## 🔑 Categorias Existentes

| Categoria      | Descrição                          | Exemplo                |
|----------------|------------------------------------|-----------------------|
| `app`          | Informações do aplicativo          | `app.name`            |
| `menu`         | Itens de menu                      | `menu.file`           |
| `tools`        | Nomes de ferramentas               | `tools.paint`         |
| `tooltips`     | Dicas de ferramentas               | `tooltips.paint`      |
| `panels`       | Títulos de painéis                 | `panels.properties`   |
| `properties`   | Nomes de propriedades              | `properties.width`    |
| `messages`     | Mensagens e diálogos               | `messages.save_changes` |
| `status`       | Mensagens de status                | `status.ready`        |
| `dialogs`      | Textos de diálogos                 | `dialogs.confirm`     |
| `tabs`         | Textos relacionados a abas         | `tabs.close`          |

## ➕ Como Adicionar um Novo Idioma

1. **Criar arquivo JSON:**
   ```
   locale/[codigo_idioma].json
   ```
   Exemplo: `es_ES.json` para Espanhol

2. **Copiar estrutura de um arquivo existente:**
   ```bash
   cp locale/pt_BR.json locale/es_ES.json
   ```

3. **Traduzir os valores:**
   ```json
   {
     "tools": {
       "paint": "Pintar"  // Espanhol
     }
   }
   ```

4. **Carregar no programa:**
   ```cpp
   Localization::Get().LoadLanguage("es_ES");
   ```

## 🔄 Trocar Idioma em Runtime

```cpp
// Obter idiomas disponíveis
std::vector<wxString> langs = Localization::Get().GetAvailableLanguages();

// Trocar para outro idioma
Localization::Get().LoadLanguage("en_US");

// TODO: Atualizar toda a UI após trocar o idioma
```

## ✅ Boas Práticas

1. **Sempre use chaves descritivas:**
   ```cpp
   // ✅ Bom
   _("messages.save_changes")
   
   // ❌ Ruim
   _("msg1")
   ```

2. **Organize por contexto:**
   ```json
   {
     "menu": { ... },
     "tools": { ... }
   }
   ```

3. **Use formatação quando necessário:**
   ```cpp
   _F("messages.file_not_found", filepath)
   ```

4. **Teste com múltiplos idiomas**

## 🐛 Troubleshooting

### Chave não encontrada
Se uma chave não for encontrada, o sistema retorna **a própria chave**.
```cpp
_("chave.inexistente")  // Retorna: "chave.inexistente"
```

### Arquivo JSON não carregado
Verifique:
- O arquivo existe em `locale/[idioma].json`
- O JSON está válido (use um validador online)
- O encoding é UTF-8

### Acentos não aparecem
Certifique-se de:
- Usar `#pragma execution_character_set("utf-8")` no .cpp
- Salvar arquivos JSON em UTF-8
- Usar `wxString::FromUTF8()` para conversão

## 🎯 Próximos Passos

- [ ] Adicionar seletor de idioma na UI
- [ ] Implementar recarregamento dinâmico da UI
- [ ] Adicionar mais idiomas (es_ES, ja_JP, etc.)
- [ ] Criar ferramenta de extração de strings
