import { Container, Sprite, Spritesheet } from "pixi.js";
import { Viewport } from "pixi-viewport";

/**
 * Create a character sprite and set up movement
 * @param spritesheet Spritesheet containing character texture
 * @param tileId ID of the character tile in the spritesheet
 * @param x Initial X position in tiles
 * @param y Initial Y position in tiles
 * @param tileWidth Width of each tile in pixels
 * @param tileHeight Height of each tile in pixels
 * @param mapWidth Map width in pixels
 * @param mapHeight Map height in pixels
 * @param viewport Viewport instance for camera following
 * @returns Object containing character sprite and container
 */
export const createCharacter = (
  spritesheet: Spritesheet, 
  tileId: number, 
  x: number, 
  y: number, 
  tileWidth: number, 
  tileHeight: number,
  mapWidth: number,
  mapHeight: number,
  viewport: Viewport
) => {
  const textureKey = `${tileId}`;
  
  if (!spritesheet.textures[textureKey]) {
    console.error(`Texture with ID ${tileId} not found in the spritesheet`);
    return { character: null, characterContainer: new Container() };
  }
  
  const character = new Sprite(spritesheet.textures[textureKey]);
  character.x = x * tileWidth;
  character.y = y * tileHeight;
  
  // Set anchor to center for better positioning
  character.anchor.set(0.5);
  
  // Create a container for the character
  const characterContainer = new Container();
  characterContainer.addChild(character);
  
  console.log(`Character created at position (${x}, ${y})`);
  
  // Set up character movement with arrow keys and map boundaries
  setupCharacterMovement(character, mapWidth, mapHeight, viewport);
  
  return { character, characterContainer };
};

/**
 * Set up keyboard controls for character movement with boundary checking
 * @param character The character sprite to move
 * @param mapWidth Map width in pixels
 * @param mapHeight Map height in pixels 
 * @param viewport Viewport to follow the character
 */
const setupCharacterMovement = (
  character: Sprite, 
  mapWidth: number, 
  mapHeight: number,
  viewport: Viewport
) => {
  const movementSpeed = 2; // Pixels per frame
  const halfWidth = character.width / 2;
  const halfHeight = character.height / 2;
  
  // Store key states to allow for smooth movement and diagonal motion
  const keys = {
    ArrowUp: false,
    ArrowDown: false,
    ArrowLeft: false,
    ArrowRight: false,
    // Add WASD support for alternative controls
    w: false,
    a: false,
    s: false,
    d: false
  };
  
  // Handle keydown events
  document.addEventListener('keydown', (e) => {
    const key = e.key;
    if (key in keys) {
      keys[key as keyof typeof keys] = true;
      e.preventDefault(); // Prevent scrolling with arrow keys
    }
  });
  
  // Handle keyup events
  document.addEventListener('keyup', (e) => {
    const key = e.key;
    if (key in keys) {
      keys[key as keyof typeof keys] = false;
    }
  });
  
  // Track previous time for consistent movement speed
  let lastTime = Date.now();
  
  // Use the app ticker for movement updates
  const updateMovement = () => {
    // Calculate delta time for smooth movement
    const currentTime = Date.now();
    const deltaTime = (currentTime - lastTime) / (1000 / 60); // Normalize to ~60fps
    lastTime = currentTime;
    
    // Calculate movement speed based on delta
    const frameSpeed = movementSpeed * Math.min(deltaTime, 2); // Cap to prevent jumps
    
    let moved = false;
    let newX = character.x;
    let newY = character.y;
    
    // Calculate new position based on keys pressed
    if (keys.ArrowUp || keys.w) {
      newY -= frameSpeed;
      moved = true;
    }
    
    if (keys.ArrowDown || keys.s) {
      newY += frameSpeed;
      moved = true;
    }
    
    if (keys.ArrowLeft || keys.a) {
      newX -= frameSpeed;
      moved = true;
    }
    
    if (keys.ArrowRight || keys.d) {
      newX += frameSpeed;
      moved = true;
    }
    
    // Apply boundary constraints
    newX = Math.max(halfWidth, Math.min(mapWidth - halfWidth, newX));
    newY = Math.max(halfHeight, Math.min(mapHeight - halfHeight, newY));
    
    // Update character position
    character.x = newX;
    character.y = newY;
    
    // Update viewport to follow character
    if (moved) {
      viewport.follow(character, {
        speed: 8,
        acceleration: 0.2
      });
    }
    
    // Continue the animation loop
    requestAnimationFrame(updateMovement);
  };
  
  // Start the animation loop
  updateMovement();
  
  // Initial viewport centering on character
  viewport.follow(character);
};
