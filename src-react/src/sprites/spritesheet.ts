import { Spritesheet } from "pixi.js";

/**
 * Generate atlas data for a tileset
 * @param tileWidth Width of each tile in pixels
 * @param tileHeight Height of each tile in pixels
 * @param tileSpacing Spacing between tiles in pixels
 * @param columns Number of columns in the tileset
 * @param rows Number of rows in the tileset
 * @returns Atlas data for creating a spritesheet
 */
export const generateAtlasData = (
  tileWidth: number, 
  tileHeight: number, 
  tileSpacing: number, 
  columns: number, 
  rows: number
) => {
  const totalTiles: number = columns * rows;
  // eslint-disable-next-line @typescript-eslint/no-explicit-any
  const atlasData: any = { 
    frames: {}, 
    meta: { 
      image: 'game_/TOWN.png', 
      format: 'RGBA8888', 
      size: { 
        w: columns * (tileWidth + tileSpacing) - tileSpacing, 
        h: rows * (tileHeight + tileSpacing) - tileSpacing 
      }, 
      scale: 1 
    } 
  };
  
  for (let i = 0; i < totalTiles; i++) {
    const column = i % columns;
    const row = Math.floor(i / columns);
    
    atlasData.frames[`${i}`] = { 
      frame: { 
        x: column * (tileWidth + tileSpacing), 
        y: row * (tileHeight + tileSpacing), 
        w: tileWidth, 
        h: tileHeight 
      }, 
      sourceSize: { 
        w: tileWidth, 
        h: tileHeight 
      }, 
      spriteSourceSize: { 
        x: 0, 
        y: 0, 
        w: tileWidth, 
        h: tileHeight 
      } 
    };
  }
  
  return atlasData;
};

/**
 * Create a spritesheet from a texture and atlas data
 * @param tilesetTexture Texture containing the tileset
 * @param atlasData Atlas data generated for the tileset
 * @returns Parsed spritesheet
 */
// eslint-disable-next-line @typescript-eslint/no-explicit-any
export const createSpritesheet = async (tilesetTexture: any, atlasData: any): Promise<Spritesheet> => {
  console.log("Creating spritesheet...");
  const spritesheet = new Spritesheet(tilesetTexture, atlasData);
  await spritesheet.parse();
  console.log("Spritesheet parsed successfully");
  return spritesheet;
};
