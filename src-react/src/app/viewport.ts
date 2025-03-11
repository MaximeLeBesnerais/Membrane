import { Application } from "pixi.js";
import { Viewport } from "pixi-viewport";

/**
 * Create and configure the viewport
 * @param app The PixiJS application
 * @param worldWidth The width of the world in pixels
 * @param worldHeight The height of the world in pixels
 * @returns The configured viewport
 */
export const createViewport = (app: Application, worldWidth: number, worldHeight: number): Viewport => {
  // Create the viewport
  const viewport = new Viewport({
    screenWidth: window.innerWidth,
    screenHeight: window.innerHeight,
    worldWidth: worldWidth,
    worldHeight: worldHeight,
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
      minScale: 2,
      maxScale: 2.0
    });
    
  return viewport;
};
