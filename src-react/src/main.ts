import { Application, Assets, Sprite, Spritesheet, Container } from "pixi.js";
import { bgCSV } from "./map/background";
import { fgCSV } from "./map/foreground";
import { mainCSV } from "./map/main";

const loadCSV = async (content: string): Promise<number[][]> => {
  try {
    return content.split('\n').map(line => 
      line.split(',').map(cell => {
        const parsed = parseInt(cell.trim(), 10);
        return isNaN(parsed) ? -1 : parsed;
      })
    );
  } catch (error) {
    console.error("Error loading CSV data:", error);
    return [];
  }
};

const initializeApp = async () => {
  const app = new Application();
  await app.init({ resizeTo: window, backgroundColor: 0x808080 });
  
  const pixiContainer = document.getElementById("pixi-container");
  if (!pixiContainer) throw new Error("Could not find element with id 'pixi-container'");
  pixiContainer.appendChild(app.canvas);
  return app;
};

const loadAssets = async () => {
  console.log("Loading tileset texture...");
  const TownAssets = await Assets.load("game_/TOWN_packed.png");
  const DungeonAssets = await Assets.load("game_/DUNGEON_packed.png");
  console.log("Tileset loaded successfully");
  return [TownAssets, DungeonAssets];
};

const loadCSVFiles = async () => {
  console.log("Loading CSV files...");
  const [backgroundLayer, mainLayer, foregroundLayer] = await Promise.all([
    loadCSV(bgCSV),
    loadCSV(mainCSV),
    loadCSV(fgCSV)
  ]);
  console.log("CSV files loaded");
  return { backgroundLayer, mainLayer, foregroundLayer };
};

const generateAtlasData = (tileWidth: number, tileHeight: number, tileSpacing: number, columns: number, rows: number) => {
  const totalTiles: number = columns * rows;
  // eslint-disable-next-line @typescript-eslint/no-explicit-any
  const atlasData: any = { frames: {}, meta: { image: 'game_/TOWN.png', format: 'RGBA8888', size: { w: columns * (tileWidth + tileSpacing) - tileSpacing, h: rows * (tileHeight + tileSpacing) - tileSpacing }, scale: 1 } };
  
  for (let i = 0; i < totalTiles; i++) {
    const column = i % columns;
    const row = Math.floor(i / columns);
    atlasData.frames[`${i}`] = { frame: { x: column * (tileWidth + tileSpacing), y: row * (tileHeight + tileSpacing), w: tileWidth, h: tileHeight }, sourceSize: { w: tileWidth, h: tileHeight }, spriteSourceSize: { x: 0, y: 0, w: tileWidth, h: tileHeight } };
  }
  return atlasData;
};

// eslint-disable-next-line @typescript-eslint/no-explicit-any
const createSpritesheet = async (tilesetTexture: any, atlasData: any) => {
  console.log("Creating spritesheet...");
  const spritesheet = new Spritesheet(tilesetTexture, atlasData);
  await spritesheet.parse();
  console.log("Spritesheet parsed successfully");
  return spritesheet;
};

const createTiledMap = (mapData: number[][], spritesheet: Spritesheet, tileWidth: number, tileHeight: number, offsetX = 0, offsetY = 0) => {
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

(async () => {
  try {
    const app = await initializeApp();
    const TownAssets = await loadAssets();
    const { backgroundLayer, mainLayer, foregroundLayer } = await loadCSVFiles();
    
    const tileWidth = 16, tileHeight = 16, tileSpacing = 0, columns = 12, rows = 11;
    const atlasData = generateAtlasData(tileWidth, tileHeight, tileSpacing, columns, rows);
    const spritesheet = await createSpritesheet(TownAssets, atlasData);
    
    const scaleFactor = 1.5;
    const centerX = (app.screen.width - 40 * tileWidth * scaleFactor) / 2;
    const centerY = (app.screen.height - 42 * tileHeight * scaleFactor) / 2;
    console.log(`Window size: ${window.innerWidth}x${window.innerHeight}`);
    
    console.log("Creating map layers...");
    const backgroundMap = createTiledMap(backgroundLayer, spritesheet, tileWidth, tileHeight, centerX / scaleFactor, centerY / scaleFactor);
    const mainMap = createTiledMap(mainLayer, spritesheet, tileWidth, tileHeight, centerX / scaleFactor, centerY / scaleFactor);
    const foregroundMap = createTiledMap(foregroundLayer, spritesheet, tileWidth, tileHeight, centerX / scaleFactor, centerY / scaleFactor);
    
    const mapContainer = new Container();
    mapContainer.addChild(backgroundMap);
    mapContainer.addChild(mainMap);
    mapContainer.addChild(foregroundMap);
    mapContainer.scale.set(scaleFactor);
    app.stage.addChild(mapContainer);
    
    console.log("Map rendering complete");
    // add a resize watcher to keep the map centered
    window.addEventListener("resize", () => {
      const centerX = (app.screen.width - 40 * tileWidth * scaleFactor) / 2;
      const centerY = (app.screen.height - 42 * tileHeight * scaleFactor) / 2;
      mapContainer.position.set(centerX, centerY);
      // write window size to console
      console.log(`Window size: ${window.innerWidth}x${window.innerHeight}`);
    });
  } catch (error) {
    console.error("Error in tilemap initialization:", error);
  }
})();
