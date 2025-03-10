import { Application, Assets, Sprite, Spritesheet, Container } from "pixi.js";

const loadCSV = async (path: string): Promise<number[][]> => {
  const response = await fetch(path);
  const text = await response.text();
  
  return text
    .trim()
    .split('\n')
    .map(line => 
      line.split(',')
        .map(cell => parseInt(cell.trim(), 10))
    );
};

(async () => {
  const app = new Application();
  await app.init({ background: "#1099bb", resizeTo: window });
  document.getElementById("pixi-container")!.appendChild(app.canvas);

  // Load the tileset
  const tilesetTexture = await Assets.load("/game_/TOWN.png");
  
  // Load the CSV map layers
  const backgroundLayer = await loadCSV("/maps/background.csv");
  const mainLayer = await loadCSV("/maps/main.csv");
  const foregroundLayer = await loadCSV("/maps/foreground.csv");
  
  // Define tileset configuration
  const tileWidth = 16;
  const tileHeight = 16;
  const tileSpacing = 1;
  const columns = 12;
  const rows = 11;
  const totalTiles = 132;
  
  // Create atlas data
  const atlasData: {
    frames: { [key: string]: { frame: { x: number, y: number, w: number, h: number }, 
                               sourceSize: { w: number, h: number }, 
                               spriteSourceSize: { x: number, y: number, w: number, h: number } } },
    meta: {
      image: string,
      format: string,
      size: { w: number, h: number },
      scale: number
    }
  } = {
    frames: {},
    meta: {
      image: '/assets/game_/TOWN.png',
      format: 'RGBA8888',
      size: { 
        w: columns * (tileWidth + tileSpacing) - tileSpacing, 
        h: rows * (tileHeight + tileSpacing) - tileSpacing 
      },
      scale: 1
    }
  };
  
  // Generate frames for each tile
  for (let i = 0; i < totalTiles; i++) {
    const column = i % columns;
    const row = Math.floor(i / columns);
    
    const x = column * (tileWidth + tileSpacing);
    const y = row * (tileHeight + tileSpacing);
    
    atlasData.frames[`tile${i}`] = {
      frame: { x, y, w: tileWidth, h: tileHeight },
      sourceSize: { w: tileWidth, h: tileHeight },
      spriteSourceSize: { x: 0, y: 0, w: tileWidth, h: tileHeight }
    };
  }
  
  // Create and parse the spritesheet
  const spritesheet = new Spritesheet(tilesetTexture, atlasData);
  await spritesheet.parse();
  
  // Function to create a map from tile IDs
  const createTiledMap = (mapData: number[][], offsetX = 0, offsetY = 0) => {
    const mapContainer = new Container();
    
    for (let y = 0; y < mapData.length; y++) {
      for (let x = 0; x < mapData[y].length; x++) {
        const tileId = mapData[y][x];
        
        if (tileId === -1) continue;
        
        const tile = new Sprite(spritesheet.textures[`tile${tileId}`]);
        tile.x = x * tileWidth + offsetX;
        tile.y = y * tileHeight + offsetY;
        
        mapContainer.addChild(tile);
      }
    }
    
    return mapContainer;
  };
  
  // Calculate the center position
  const centerX = (app.screen.width - (mainLayer[0].length * tileWidth)) / 2;
  const centerY = (app.screen.height - (mainLayer.length * tileHeight)) / 2;
  
  // Create and add map layers
  const backgroundMap = createTiledMap(backgroundLayer, centerX, centerY);
  const mainMap = createTiledMap(mainLayer, centerX, centerY);
  const foregroundMap = createTiledMap(foregroundLayer, centerX, centerY);
  
  app.stage.addChild(backgroundMap);
  app.stage.addChild(mainMap);
  app.stage.addChild(foregroundMap);
})();