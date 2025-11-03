# Sistema de Save/Load - Lumy Engine

## Visão Geral

O sistema de save/load do Lumy permite salvar o estado completo do jogo em múltiplos slots independentes, usando formato JSON legível e facilmente editável.

## Funcionalidades

### 🎮 **Múltiplos Slots**
- **9 slots independentes** (numerados de 1 a 9)
- Cada slot mantém estado completamente separado
- Possibilidade de comparar/gerenciar múltiplas partidas

### 💾 **Dados Salvos**
O sistema persiste os seguintes dados:

```json
{
  "metadata": {
    "version": "1.0",
    "timestamp": "2025-08-30T12:34:56Z",
    "playtime_seconds": 3600
  },
  "player": {
    "mapId": 1,
    "x": 320.5,
    "y": 240.0,
    "direction": 2
  },
  "switches": {
    "1": true,
    "2": false
  },
  "variables": {
    "1": 100,
    "2": 42
  },
  "party": [
    {"actorId": 1, "level": 5, "exp": 250}
  ],
  "inventory": [
    {"itemId": 1, "amount": 3},
    {"itemId": 2, "amount": 1}
  ]
}
```

### ⌨️ **Controles**

#### Quick Save/Load
- **F5**: Quick Save (slot 1)
- **F9**: Quick Load (slot 1)

#### Gerenciamento Avançado
- **AltGr + 1-9**: Salvar no slot específico
- **Ctrl + AltGr + 1-9**: Carregar do slot específico
- **Shift + AltGr + 1-9**: Deletar save do slot específico

## Implementação Técnica

### **Arquitetura**
```cpp
class SaveSystem {
public:
    bool initialize(const std::string& gameId);
    bool saveGame(int slot);
    bool loadGame(int slot);
    bool deleteSave(int slot);
    bool saveExists(int slot);
    
    // Getters/Setters para dados específicos
    void setPlayerPosition(int mapId, float x, float y, int direction);
    void getPlayerPosition(int& mapId, float& x, float& y, int& direction);
    void setSwitch(int id, bool value);
    bool getSwitch(int id);
    // ... outros métodos
};
```

### **Localização dos Arquivos**
```
game/saves/
├── save1.json    # Slot 1
├── save2.json    # Slot 2
├── ...
└── save9.json    # Slot 9
```

### **Validação e Segurança**
- Verificação de integridade dos arquivos JSON
- Fallback para valores padrão em caso de erro
- Backup automático antes de sobrescrever
- Logs detalhados de operações

## Uso no Código

### **Inicialização**
```cpp
auto saveSystem = std::make_unique<SaveSystem>();
if (!saveSystem->initialize("meu-jogo")) {
    // Lidar com erro de inicialização
}
```

### **Salvando Estado**
```cpp
// Definir posição do jogador
saveSystem->setPlayerPosition(1, 320.0f, 240.0f, 2);

// Definir switches e variáveis
saveSystem->setSwitch(1, true);
saveSystem->setVariable(1, 100);

// Salvar no slot 3
if (saveSystem->saveGame(3)) {
    std::cout << "Jogo salvo com sucesso!" << std::endl;
}
```

### **Carregando Estado**
```cpp
// Verificar se existe save
if (saveSystem->saveExists(3)) {
    // Carregar dados
    if (saveSystem->loadGame(3)) {
        // Recuperar posição
        int mapId, direction;
        float x, y;
        saveSystem->getPlayerPosition(mapId, x, y, direction);
        
        // Aplicar ao jogo
        player.setPosition(x, y);
        currentMap = mapId;
    }
}
```

## Debugging e Desenvolvimento

### **Formato Legível**
Os saves em JSON são facilmente editáveis para:
- Debug de estados específicos
- Testes automatizados
- Modding da comunidade

### **Logs do Sistema**
```
[SaveSystem] Dados resetados para padrões
[SaveSystem] Inicializado com diretório: game/saves
[SaveSystem] Jogo salvo no slot 3: game/saves/save3.json
[SaveSystem] Jogo carregado do slot 3: game/saves/save3.json
[SaveSystem] Save removido: game/saves/save3.json
```

## Extensibilidade

O sistema foi projetado para ser facilmente extensível:

### **Novos Tipos de Dados**
```cpp
// Adicionar novo tipo de dado
class SaveSystem {
    // Exemplo: sistema de quests
    void setQuestStatus(int questId, const std::string& status);
    std::string getQuestStatus(int questId);
};
```

### **Compressão (Futuro)**
- Suporte a compressão opcional para saves grandes
- Backwards compatibility mantida

### **Cloud Saves (Futuro)**
- Interface preparada para extensão com cloud storage
- Sincronização entre dispositivos

## Boas Práticas

1. **Salvar frequentemente**: Use auto-save em pontos críticos
2. **Feedback visual**: Mostre confirmação de save/load ao usuário
3. **Handles de erro**: Sempre verificar retorno das operações
4. **Backup**: Manter backups de saves importantes
5. **Versionamento**: Considerar migração de saves entre versões

## Troubleshooting

### **Problemas Comuns**
- **Save não aparece**: Verificar permissões de escrita na pasta
- **Carregamento falha**: Verificar integridade do JSON
- **Desempenho**: Evitar saves muito frequentes (< 1s de intervalo)

### **Logs de Debug**
Ativar logs detalhados definindo:
```cpp
#define LUMY_DEBUG_SAVES 1
```

---

**Nota**: Este sistema foi implementado no M0 "Faísca" e será expandido nos próximos milestones com funcionalidades adicionais como cloud sync e compressão.
