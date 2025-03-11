import { Application, Assets, Sprite, Spritesheet, Container } from "pixi.js";
import { Viewport } from "pixi-viewport";
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
    const [TownAssets, DungeonAssets] = await loadAssets();
    const { backgroundLayer, mainLayer, foregroundLayer } = await loadCSVFiles();
    
    const tileWidth = 16, tileHeight = 16, tileSpacing = 0, columns = 12, rows = 11;
    const atlasData = generateAtlasData(tileWidth, tileHeight, tileSpacing, columns, rows);
    const TownSheet = await createSpritesheet(TownAssets, atlasData);
    const DungeonSheet = await createSpritesheet(DungeonAssets, atlasData);
    
    // Calculate map dimensions
    const mapWidthInTiles = 40;
    const mapHeightInTiles = 42;
    const mapWidthInPixels = mapWidthInTiles * tileWidth;
    const mapHeightInPixels = mapHeightInTiles * tileHeight;
    
    // Create the viewport
    const viewport = new Viewport({
      screenWidth: window.innerWidth,
      screenHeight: window.innerHeight,
      worldWidth: mapWidthInPixels,
      worldHeight: mapHeightInPixels,
      events: app.renderer.events
    });
    
    // Add the viewport to the stage
    app.stage.addChild(viewport);
    
    // Activate plugins
    viewport
      .drag()              // Enable dragging/panning
      .pinch()             // Enable pinch-to-zoom on touch devices
      .wheel()             // Enable mouse wheel zooming
      .decelerate()        // Add deceleration to dragging
      .clampZoom({         // Constrain zoom levels
        minScale: 0.5,     // Minimum zoom out (0.5x)
        maxScale: 4.0      // Maximum zoom in (4x)
      });
    
    console.log("Creating map layers...");
    // Create map layers without centering offsets - the viewport will handle positioning
    const backgroundMap = createTiledMap(backgroundLayer, TownSheet, tileWidth, tileHeight, 0, 0);
    const mainMap = createTiledMap(mainLayer, TownSheet, tileWidth, tileHeight, 0, 0);
    const foregroundMap = createTiledMap(foregroundLayer, TownSheet, tileWidth, tileHeight, 0, 0);
    
    // Create a container for all map layers
    const mapContainer = new Container();
    mapContainer.addChild(backgroundMap);
    mapContainer.addChild(mainMap);
    mapContainer.addChild(foregroundMap);
    
    // Apply base scale factor to the map container
    const scaleFactor = 1.5;
    mapContainer.scale.set(scaleFactor);
    
    // Update world size after scaling
    viewport.worldWidth = mapWidthInPixels * scaleFactor;
    viewport.worldHeight = mapHeightInPixels * scaleFactor;
    
    // Add the map container to the viewport
    viewport.addChild(mapContainer);
    
    // Center the viewport on the map
    viewport.moveCenter(viewport.worldWidth/2, viewport.worldHeight/2);
    
    console.log("Map rendering complete");
    
    // Handle window resize
    window.addEventListener("resize", () => {
      // Update viewport dimensions
      viewport.resize(window.innerWidth, window.innerHeight);
      // Re-center on the map
      viewport.moveCenter(viewport.worldWidth/2, viewport.worldHeight/2);
      console.log(`Window size: ${window.innerWidth}x${window.innerHeight}`);
    });
    
    // Optional: Add zoom controls
    const addZoomControls = () => {
      const controlsDiv = document.createElement('div');
      controlsDiv.style.position = 'absolute';
      controlsDiv.style.bottom = '20px';
      controlsDiv.style.right = '20px';
      controlsDiv.style.zIndex = '1000';
      
      const zoomInBtn = document.createElement('button');
      zoomInBtn.textContent = '+';
      zoomInBtn.style.width = '40px';
      zoomInBtn.style.height = '40px';
      zoomInBtn.style.fontSize = '24px';
      zoomInBtn.style.marginRight = '10px';
      zoomInBtn.addEventListener('click', () => {
        viewport.zoom(1.2, true); // Zoom in by 20%
      });
      
      const zoomOutBtn = document.createElement('button');
      zoomOutBtn.textContent = '-';
      zoomOutBtn.style.width = '40px';
      zoomOutBtn.style.height = '40px';
      zoomOutBtn.style.fontSize = '24px';
      zoomOutBtn.addEventListener('click', () => {
        viewport.zoom(0.8, true); // Zoom out by 20%
      });
      
      controlsDiv.appendChild(zoomInBtn);
      controlsDiv.appendChild(zoomOutBtn);
      document.body.appendChild(controlsDiv);
    };
    
    // Uncomment to add on-screen zoom buttons
    // addZoomControls();
    
    // Optional: Add debugging information
    const addDebugInfo = () => {
      const debugDiv = document.createElement('div');
      debugDiv.style.position = 'absolute';
      debugDiv.style.top = '10px';
      debugDiv.style.left = '10px';
      debugDiv.style.zIndex = '1000';
      debugDiv.style.backgroundColor = 'rgba(0, 0, 0, 0.5)';
      debugDiv.style.color = 'white';
      debugDiv.style.padding = '10px';
      debugDiv.style.fontFamily = 'monospace';
      document.body.appendChild(debugDiv);
      
      // Update debug info every frame
      app.ticker.add(() => {
        debugDiv.innerHTML = `
          Scale: ${viewport.scale.x.toFixed(2)}x<br>
          Position: (${viewport.x.toFixed(0)}, ${viewport.y.toFixed(0)})<br>
          Center: (${viewport.center.x.toFixed(0)}, ${viewport.center.y.toFixed(0)})<br>
          World Size: ${viewport.worldWidth}x${viewport.worldHeight}<br>
          Screen Size: ${viewport.screenWidth}x${viewport.screenHeight}
        `;
      });
    };
    
    // Uncomment to add debug information
    // addDebugInfo();
    
  } catch (error) {
    console.error("Error in tilemap initialization:", error);
  }
})();