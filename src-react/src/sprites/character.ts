import { Container, Sprite, Spritesheet } from "pixi.js";

/**
 * Create a character sprite and set up movement
 * @param spritesheet Spritesheet containing character texture
 * @param tileId ID of the character tile in the spritesheet
 * @param x Initial X position in tiles
 * @param y Initial Y position in tiles
 * @param tileWidth Width of each tile in pixels
 * @param tileHeight Height of each tile in pixels
 * @returns Object containing character sprite and container
 */
export const createCharacter = (
  spritesheet: Spritesheet, 
  tileId: number, 
  x: number, 
  y: number, 
  tileWidth: number, 
  tileHeight: number
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
  
  // Set up character movement with arrow keys
  setupCharacterMovement(character);
  
  return { character, characterContainer };
};

/**
 * Set up keyboard controls for character movement
 * @param character The character sprite to move
 */
const setupCharacterMovement = (character: Sprite) => {
  const movementSpeed = 2; // Pixels per frame
  
  document.addEventListener('keydown', (e) => {
    switch (e.key) {
      case 'ArrowUp':
        character.y -= movementSpeed;
        break;
      case 'ArrowDown':
        character.y += movementSpeed;
        break;
      case 'ArrowLeft':
        character.x -= movementSpeed;
        break;
      case 'ArrowRight':
        character.x += movementSpeed;
        break;
    }
  });
};
