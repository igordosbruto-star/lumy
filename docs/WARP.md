# WARP.md — Regras do Agente para este repositório (lumy)

Este documento orienta como o agente deve atuar neste projeto C++20 com CMake, vcpkg, SFML 3 e GoogleTest em Windows (MSVC 2022).

## Estilo de respostas do agente
- Responder em PT-BR, de forma concisa e direta (1–3 frases, no máximo alguns passos quando necessário).
- Usar Markdown comum (CommonMark). Evitar emojis e floreios.
- Preferir agir (ler/editar arquivos, rodar comandos) quando o usuário pedir algo específico.
- Nunca abrir pager; em comandos de VCS, desabilitar paginação.

## Segurança e boas práticas
- Nunca expor segredos em texto plano; usar variáveis de ambiente.
- Não executar comandos destrutivos/disruptivos sem confirmação explícita.
- Não criar commits a menos que o usuário peça explicitamente.
- Após alterações relevantes em código, sempre compilar e rodar testes.

## Pré-requisitos
- vcpkg em modo manifest e variável de ambiente `VCPKG_ROOT` definida.
  - PowerShell (exemplo):
    ```pwsh
    # Ajuste o caminho conforme sua instalação do vcpkg
    $env:VCPKG_ROOT = "C:\\path\\to\\vcpkg"
    ```
- CMake ≥ 3.25 e MSVC 2022 (x64).

## Presets do CMake (definidos em CMakePresets.json)
- Configure: `msvc-vcpkg`
- Build: `msvc-Debug` e `msvc-Release`
- Testes (CTest): `msvc-tests`

## Tarefas comuns (PowerShell)
- Configurar:
  ```pwsh
  cmake --preset msvc-vcpkg
  ```
- Compilar (Debug):
  ```pwsh
  cmake --build build/msvc --config Debug -- /m
  ```
- Compilar (Release):
  ```pwsh
  cmake --build build/msvc --config Release -- /m
  ```
- Executar demo (hello-town):
  ```pwsh
  build/msvc/bin/Debug/hello-town.exe
  ```
- Testes (todos, Debug):
  ```pwsh
  ctest --preset msvc-tests
  ```
- Testes (filtrar por nome):
  ```pwsh
  ctest -C Debug -R <padrao>
  ```
- Limpar artefatos (sem deletar pastas):
  ```pwsh
  cmake --build build/msvc --config Debug --target clean
  ```

## Estrutura e alvos principais
- Executáveis:
  - `hello-town` (jogo/exemplo) → sai em `build/msvc/bin/<Config>`
  - `lumy-editor` (editor com wxWidgets) → sai em `build/msvc/bin/<Config>`
- Testes:
  - `lumy-tests` (engine) — registrado como `basic_startup` e outros via CTest
  - `lumy-editor-tests` (editor)
- CMake copia automaticamente assets (`game/`) e `locale/` para o diretório de saída pós-build.

## Fontes de verdade
  - `docs/ROADMAP.md` - prioridades e milestones
  - `docs/VISION.md` - direção do produto
  - `docs/CHANGELOG.md` - histórico de mudanças

## Convenções de código/edição
- Fonte da engine em `src/`; editor em `editor/`; testes em `tests/`.
- Ao adicionar novos arquivos fonte ou testes, registre-os no `CMakeLists.txt` apropriado.
- Siga `docs/CODING_STANDARDS.md`. Se houver divergência, padronize conforme esse documento.
- Para internacionalização do editor, mantenha chaves em `editor/locale/en_US.json` e `pt_BR.json` consistentes.

## Fluxo recomendado para mudanças
1) Editar arquivos necessários.
2) Adicionar no CHANGELOG.md as atualizações feitas.
3) `cmake --preset msvc-vcpkg` (se ainda não configurado).
4) `cmake --build build/msvc --config Debug`.
5) `ctest --preset msvc-tests` (ou `ctest -C Debug -R <padrao>` para foco).

## Documentação
  - Arquivos de documentação na pasta docs


## GitHub e PRs
- Use os templates em `.github/`. Envie PRs com descrição objetiva e resultados de build/test.

## Solução de problemas
- Erro sobre vcpkg ou pacotes não encontrados: valide `VCPKG_ROOT` e reconfigure com o preset.
- Arquivos de assets/locale não aparecendo na saída: garanta que o build foi feito (pós-build faz a cópia) e verifique `build/msvc/bin/<Config>/`.

## Como o agente deve agir neste repo
- Priorizar automação via CMake Presets; evitar configurar manualmente toolchains.
- Para tarefas envolvendo testes/execução, sempre usar os comandos desta página.
- Ser explícito quando uma ação puder ser destrutiva e solicitar confirmação antes.