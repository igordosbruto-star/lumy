# Fluxo de Cenas — Lumy (Boot → Title → Map)

Este documento descreve o fluxo de cenas do exemplo `hello-town` e como usar o `SceneStack`.

## Cenas

1. **BootScene** — carrega recursos essenciais e troca imediatamente para `TitleScene`.
2. **TitleScene** — exibe o texto "Start" usando a fonte `game/font.ttf`. Pressione **Enter** ou clique para ir para `MapScene`.
3. **MapScene** — mostra um herói quadrado que se move com **W/A/S/D** ou cliques do mouse.

## Assets necessários

- `game/font.ttf` — qualquer fonte TrueType para renderizar o texto da `TitleScene`.
- opcional: `game/first.tmx` — mapa de exemplo utilizado apenas para logs de carregamento.

## Uso básico de `SceneStack`

```cpp
SceneStack stack;
stack.pushScene(std::make_unique<BootScene>(stack));

while (window.isOpen()) {
    while (auto event = window.pollEvent()) {
        if (auto* scene = stack.current()) {
            scene->handleEvent(*event);
        }
    }

    if (auto* scene = stack.current()) {
        scene->update(deltaTime);
        scene->draw(window);
    }
}
```

O `SceneStack` mantém apenas a cena ativa no topo e permite `pushScene`, `popScene` e `switchScene` para transições simples.

