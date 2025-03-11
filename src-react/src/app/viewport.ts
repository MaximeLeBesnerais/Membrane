import { Application } from "pixi.js";
import { Viewport } from "pixi-viewport";

/**
 * Create and configure the viewport with boundary clamping
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
    .drag({
      clampWheel: true,  // Prevent dragging beyond boundaries
      underflow: 'center' // When map is smaller than screen, center it
    })
    .pinch()             // Enable pinch-to-zoom on touch devices
    .wheel({
      smooth: 5,         // Add smooth zooming
      interrupt: true,   // Stop animation on user input
      percent: 0.1,      // Zoom percent per scroll
    })
    .decelerate()        // Add deceleration to dragging
    .clampZoom({         // Constrain zoom levels
      minScale: 2,
      maxScale: 2.0
    })
    .clamp({             // Clamp viewport to world boundaries
      direction: 'all',  // Clamp in all directions
      underflow: 'center' // Center when smaller than screen
    });
    
  return viewport;
};