"""Map loading utilities for Lumy examples.

Provides a function ``load_hello_map`` that parses the TMX map at
``game/assets/maps/hello.tmx`` and returns a simple Python structure
with metadata and layers.

Raises ``MapParseError`` when the TMX file is missing required elements
such as the tileset or when properties are malformed.
"""
from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
import xml.etree.ElementTree as ET


class MapParseError(RuntimeError):
    """Raised when a TMX map cannot be parsed into the expected structure."""


@dataclass
class TileLayer:
    """Represents a layer of tile IDs."""
    name: str
    ids: list[int]


@dataclass
class MapData:
    """Internal representation of the parsed map."""
    width: int
    height: int
    tile_width: int
    tile_height: int
    layers: list[TileLayer]
    properties: dict[str, str]


_DEF_PATH = Path("game/assets/maps/hello.tmx")


def load_hello_map(path: Path | str = _DEF_PATH) -> MapData:
    """Parse ``hello.tmx`` and return a :class:`MapData` instance.

    Parameters
    ----------
    path:
        Optional path to the TMX file. Defaults to ``game/assets/maps/hello.tmx``.

    Returns
    -------
    MapData
        Parsed map representation.

    Raises
    ------
    MapParseError
        If the file is missing, the tileset is absent, or properties are invalid.
    """
    tmx_path = Path(path)
    if not tmx_path.is_file():
        raise MapParseError(f"TMX file not found: {tmx_path}")

    try:
        tree = ET.parse(tmx_path)
    except ET.ParseError as exc:  # pragma: no cover - parsing errors
        raise MapParseError(f"Failed to parse TMX: {exc}") from exc

    root = tree.getroot()
    tileset = root.find("tileset")
    if tileset is None:
        raise MapParseError("TMX missing <tileset> definition")

    width = int(root.get("width", 0))
    height = int(root.get("height", 0))
    tile_width = int(root.get("tilewidth", 0))
    tile_height = int(root.get("tileheight", 0))

    layers: list[TileLayer] = []
    for layer in root.findall("layer"):
        name = layer.get("name") or ""
        data = layer.find("data")
        if data is None or data.text is None:
            raise MapParseError(f"Layer '{name}' missing data")
        try:
            ids = [int(val) for val in data.text.replace("\n", "").split(",") if val]
        except ValueError as exc:
            raise MapParseError(f"Invalid tile id in layer '{name}'") from exc
        layers.append(TileLayer(name=name, ids=ids))

    properties: dict[str, str] = {}
    prop_root = root.find("properties")
    if prop_root is not None:
        for prop in prop_root.findall("property"):
            key = prop.get("name")
            value = prop.get("value")
            if key is None or value is None:
                raise MapParseError("Invalid property without name or value")
            properties[key] = value

    return MapData(
        width=width,
        height=height,
        tile_width=tile_width,
        tile_height=tile_height,
        layers=layers,
        properties=properties,
    )


if __name__ == "__main__":
    map_data = load_hello_map()
    print(
        f"Loaded {map_data.width}x{map_data.height} map with {len(map_data.layers)} layers."
    )
