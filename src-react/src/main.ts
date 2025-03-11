import { Container } from "pixi.js";
import { initializeApp } from "./app/initialize";
import { createViewport } from "./app/viewport";
import { loadAssets } from "./assets/loader";
import { loadCSVFiles } from "./map/parser";
import { createTiledMap } from "./map/generator";
import { createCharacter } from "./sprites/character";
import { generateAtlasData, createSpritesheet } from "./sprites/spritesheet";
import { addZoomControls } from "./ui/controls";
import { addDebugInfo } from "./ui/debug";

// Constants for tile dimensions
const TILE_WIDTH = 16;
const TILE_HEIGHT = 16;
const TILE_SPACING = 0;
const TILESET_COLUMNS = 12;
const TILESET_ROWS = 11;

(async () => {
  try {
    // Initialize app
    const app = await initializeApp();
    
    // Load assets
    const [TownAssets, DungeonAssets] = await loadAssets();
    
    // Load map data
    const { backgroundLayer, mainLayer, foregroundLayer } = await loadCSVFiles();
    
    // Generate atlas data for both tilesets
    const townAtlasData = generateAtlasData(TILE_WIDTH, TILE_HEIGHT, TILE_SPACING, TILESET_COLUMNS, TILESET_ROWS);
    const dungeonAtlasData = generateAtlasData(TILE_WIDTH, TILE_HEIGHT, TILE_SPACING, TILESET_COLUMNS, TILESET_ROWS);
    
    // Update the meta.image for dungeonAtlasData to point to the correct image
    dungeonAtlasData.meta.image = 'game_/DUNGEON.png';
    
    // Create spritesheets
    const TownSheet = await createSpritesheet(TownAssets, townAtlasData);
    const DungeonSheet = await createSpritesheet(DungeonAssets, dungeonAtlasData);
    
    // Calculate map dimensions
    const mapWidthInTiles = 40;
    const mapHeightInTiles = 42;
    const mapWidthInPixels = mapWidthInTiles * TILE_WIDTH;
    const mapHeightInPixels = mapHeightInTiles * TILE_HEIGHT;
    
    console.log("Map dimensions:", {
      widthInTiles: mapWidthInTiles,
      heightInTiles: mapHeightInTiles,
      widthInPixels: mapWidthInPixels,
      heightInPixels: mapHeightInPixels
    });
    
    // Apply base scale factor to determine final world size
    const scaleFactor = 1;
    const worldWidth = mapWidthInPixels * scaleFactor;
    const worldHeight = mapHeightInPixels * scaleFactor;
    
    // Create viewport with boundary clamping using final world dimensions
    const viewport = createViewport(app, worldWidth, worldHeight);
    
    console.log("Creating map layers...");
    // Create map layers
    const backgroundMap = createTiledMap(backgroundLayer, TownSheet, TILE_WIDTH, TILE_HEIGHT);
    const mainMap = createTiledMap(mainLayer, TownSheet, TILE_WIDTH, TILE_HEIGHT);
    const foregroundMap = createTiledMap(foregroundLayer, TownSheet, TILE_WIDTH, TILE_HEIGHT);
    
    // Create a container for all map layers
    const mapContainer = new Container();
    mapContainer.addChild(backgroundMap);
    mapContainer.addChild(mainMap);
    
    // Create character from dungeon tileset (sprite 85)
    const characterTileId = 85;
    // Pick a position in the middle of the map
    const characterPosX = 0;
    const characterPosY = Math.floor(mapHeightInTiles - 10);
    
    // Create character with map boundary awareness and viewport following
    const { character, characterContainer } = createCharacter(
      DungeonSheet, 
      characterTileId, 
      characterPosX, 
      characterPosY, 
      TILE_WIDTH, 
      TILE_HEIGHT,
      worldWidth,     // Use scaled world width
      worldHeight,    // Use scaled world height
      viewport
    );
    
    // Add character container between main and foreground layers
    mapContainer.addChild(characterContainer);
    
    // Add foreground layer on top
    mapContainer.addChild(foregroundMap);
    
    // Apply scale factor to the map container
    mapContainer.scale.set(scaleFactor);
    
    // Add the map container to the viewport
    viewport.addChild(mapContainer);
    
    // Force viewport to center on the character
    if (character) {
      viewport.moveCenter(character.x, character.y);
    }
    
    console.log("Map rendering complete");
    
    // Handle window resize
    window.addEventListener("resize", () => {
      // Update viewport dimensions
      viewport.resize(window.innerWidth, window.innerHeight);
      
      // Re-apply clamp boundaries after resize
      viewport.plugins.remove('clamp');
      viewport.clamp({
        direction: 'all',
        underflow: 'center',
        left: 0,
        right: worldWidth,
        top: 0,
        bottom: worldHeight
      });
      
      // Ensure character remains visible after resize
      if (character) {
        viewport.moveCenter(character.x, character.y);
      }
      
      console.log(`Window size: ${window.innerWidth}x${window.innerHeight}`);
    });
    
    // Add on-screen zoom buttons
    addZoomControls(viewport);
    
    // Add debug information
    addDebugInfo(app, viewport, character);
    
    // Add a key to toggle debug boundary check
    document.addEventListener('keydown', (e) => {
      if (e.key === 'b') {
        // Call the boundary check function from the global scope
        if ((window as any).__checkViewportBoundaries) {
          (window as any).__checkViewportBoundaries();
          console.log("Boundary check triggered");
        }
      }
    });
    
  } catch (error) {
    console.error("Error in tilemap initialization:", error);
  }
})();