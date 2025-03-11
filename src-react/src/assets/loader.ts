import { Assets } from "pixi.js";

/**
 * Load game assets
 * @returns Array of loaded assets
 */
export const loadAssets = async (): Promise<any[]> => {
  console.log("Loading tileset textures...");
  const TownAssets = await Assets.load("game_/TOWN_packed.png");
  const DungeonAssets = await Assets.load("game_/DUNGEON_packed.png");
  console.log("Tilesets loaded successfully");
  return [TownAssets, DungeonAssets];
};
