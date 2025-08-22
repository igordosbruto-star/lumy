# Mapas (TMX) — Convenções Lumy

Grid:
- M0: ortogonal (quadrado). M3: hex (axial) em arquivo separado.

Camadas:
- "ground_*": piso; "object_*": colisão/objetos; "parallax_*": fundo/efeitos.
- Ordem desenhada do topo para baixo; UI é separada.

Colisão:
- Use uma tilelayer "collision" (0/1). Alternativa: propriedade "collide" no tileset.

Regiões/Terrenos:
- Tilelayer "region" com inteiros (0 = nenhuma). Usado em eventos/IA/batalha.
- Terreno (tático): tileset com propriedade "terrain": "forest"|"mountain"|"water"|...

Propriedades do Mapa:
- "spawn_x","spawn_y" (int) — posição inicial.
- "parallax_x","parallax_y" (float) — velocidade de parallax (default 0).
- "lighting":"day|evening|night" (sugestão inicial para tint).

Autotiles/Brush:
- Livre no Tiled. Engine só lê o resultado final.

Assets:
- Tileset externo recomendado (.tsx). Paths relativos a /assets/tilesets.