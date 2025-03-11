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
    
    // Create viewport with boundary clamping
    const viewport = createViewport(app, mapWidthInPixels, mapHeightInPixels);
    
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
    const characterPosX = Math.floor(mapWidthInTiles / 2);
    const characterPosY = Math.floor(mapHeightInTiles / 2);
    
    // Create character with map boundary awareness and viewport following
    const { character, characterContainer } = createCharacter(
      DungeonSheet, 
      characterTileId, 
      characterPosX, 
      characterPosY, 
      TILE_WIDTH, 
      TILE_HEIGHT,
      mapWidthInPixels,
      mapHeightInPixels,
      viewport
    );
    
    // Add character container between main and foreground layers
    mapContainer.addChild(characterContainer);
    
    // Add foreground layer on top
    mapContainer.addChild(foregroundMap);
    
    // Apply base scale factor to the map container
    const scaleFactor = 1;
    mapContainer.scale.set(scaleFactor);
    
    // Update world size after scaling
    viewport.worldWidth = mapWidthInPixels * scaleFactor;
    viewport.worldHeight = mapHeightInPixels * scaleFactor;
    
    // Add the map container to the viewport
    viewport.addChild(mapContainer);
    
    // Center the viewport on the character
    viewport.moveCenter(character!.position._x, character!.position._y);
    
    console.log("Map rendering complete");
    
    // Handle window resize
    window.addEventListener("resize", () => {
      // Update viewport dimensions
      viewport.resize(window.innerWidth, window.innerHeight);
      
      // Re-apply clamp boundaries after resize
      viewport.plugins.remove('clamp');
      viewport.clamp({
        direction: 'all',
        underflow: 'center'
      });
      
      console.log(`Window size: ${window.innerWidth}x${window.innerHeight}`);
    });
    
    // Add on-screen zoom buttons
    addZoomControls(viewport);
    
    // Add debug information
    addDebugInfo(app, viewport, character);
    
  } catch (error) {
    console.error("Error in tilemap initialization:", error);
  }
})();