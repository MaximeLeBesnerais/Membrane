import { Container, Sprite, Spritesheet } from "pixi.js";

/**
 * Create a tiled map from a 2D array of tile IDs
 * @param mapData 2D array of tile IDs
 * @param spritesheet Spritesheet containing tile textures
 * @param tileWidth Width of each tile in pixels
 * @param tileHeight Height of each tile in pixels
 * @param offsetX X offset for map positioning
 * @param offsetY Y offset for map positioning
 * @returns Container with tile sprites
 */
export const createTiledMap = (
  mapData: number[][], 
  spritesheet: Spritesheet, 
  tileWidth: number, 
  tileHeight: number, 
  offsetX = 0, 
  offsetY = 0
): Container => {
  const mapContainer = new Container();
  
  for (let y = 0; y < mapData.length; y++) {
    for (let x = 0; x < mapData[y].length; x++) {
      const tileId = mapData[y][x];
      if (tileId === -1 || isNaN(tileId)) continue;
      
      const textureKey = `${tileId}`;
      if (!spritesheet.textures[textureKey]) continue;
      
      const tile = new Sprite(spritesheet.textures[textureKey]);
      tile.x = x * tileWidth + offsetX;
      tile.y = y * tileHeight + offsetY;
      mapContainer.addChild(tile);
    }
  }
  
  return mapContainer;
};
