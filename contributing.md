# CONTRIBUTING — Lumy

Obrigado por querer contribuir com a Lumy! 🌟  
Este guia explica como propor ideias, reportar bugs e enviar PRs de forma eficiente.

## Como posso ajudar?
- **Bugs**: abra uma *issue* descrevendo passos para reproduzir.
- **Novas features**: proponha primeiro em uma *issue* explicando contexto e motivação.
- **Docs**: melhorias no `VISION.md`, `ROADMAP.md`, tutoriais e exemplos são muito bem-vindas.
- **Plugins**: você pode sugerir APIs e enviar exemplos em Lua/C++.

## Requisitos básicos
- **Linguagem**: C++20
- **Build**: CMake
- **Runtime**: SFML
- **Editor**: wxWidgets
- **Mapas**: Tiled (TMX)
- **Dados**: JSON (nlohmann/json)
- **Scripts**: Lua 5.4 (sol2)

> Dica: consulte o `ROADMAP.md` para alinhar sua contribuição com os marcos do projeto.

## Fluxo para contribuir
1. **Abra uma issue** (bug/feature) e descreva o objetivo.
2. **Crie uma branch** a partir da `main`:  
   `feature/nome-curto` ou `fix/descricao-breve`
3. **Faça commits pequenos e claros** (ver padrão abaixo).
4. **Inclua testes** quando alterar lógica central (eventos, grid, batalha).
5. **Atualize a documentação** afetada (ex.: exemplos, tutoriais).
6. **Atualize o CHANGELOG** (seção **[Unreleased]**).
7. **Abra o PR** linkando a issue e marcando o checklist.

## Padrão de commits
Use mensagens descritivas e no imperativo. Você pode seguir *Conventional Commits* (opcional), por exemplo:
- `feat(events): adiciona ShowChoice com 2–4 opções`
- `fix(tmx): corrige crash com tileset externo`
- `docs(roadmap): marca M1 como In Progress`
- `perf(render): reduz draw calls em layers estáticos`

## Diretrizes de código (resumo)
- **Estilo**: C++ moderno, RAII, `unique_ptr`/`shared_ptr` quando fizer sentido.
- **Headers** mínimos, `.cpp` separados; evite macros desnecessárias.
- **Nomes**: `PascalCase` para tipos, `camelCase` para variáveis/funções.
- **Erros**: use `expected`/retornos claros ou exceções onde for coerente (sem esconder falhas).
- **Dados**: formatos estáveis (JSON/TMX); mudanças exigem nota no CHANGELOG e migração se necessário.
- **Testes**: cubra casos felizes e de erro em sistemas-base (eventos, variáveis, grid, batalha).

## Abrindo issues
- **Bug**: passos para reproduzir, resultado esperado x obtido, logs/prints, versão/OS.
- **Feature**: problema que resolve, proposta, impacto, alternativas consideradas, risco.

## Abrindo PRs (checklist)
- [ ] Issue linkada (`Closes #123`).
- [ ] Pequeno e focado (ou explique por que precisa ser maior).
- [ ] Testes passam (local/CI).
- [ ] **CHANGELOG** atualizado em **[Unreleased]**.
- [ ] Documentação atualizada (se aplicável).
- [ ] Alinhado ao `ROADMAP.md` **ou** justificada a exceção.

## Documentação
- Mudanças relevantes em docs entram no CHANGELOG em **Docs**.
- Tutoriais devem ser curtos e práticos (GIFs bem-vindos).

## Licença
Ao contribuir, você concorda que sua contribuição será licenciada sob **MIT** junto com a Lumy.

## Código de Conduta
Seja respeitoso, acolhedor e colaborativo. (Arquivo `CODE_OF_CONDUCT.md` — em breve.)

## Segurança
Vulnerabilidades? Evite issues públicas. Relate em **security@exemplo.lumy** (placeholder).

—  
*Última atualização: 21-08-2025*
