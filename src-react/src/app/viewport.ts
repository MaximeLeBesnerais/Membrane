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
  // Log the world dimensions for debugging
  console.log("Creating viewport with dimensions:", { worldWidth, worldHeight });
  
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
  
  // Calculate zoom constraints based on screen and world size
  const screenAspect = window.innerWidth / window.innerHeight;
  const worldAspect = worldWidth / worldHeight;
  
  let minScale = 2.0;
  
  // Adjust zoom to ensure map is always fully visible
  console.log("Viewport size info:", {
    screen: { width: window.innerWidth, height: window.innerHeight },
    world: { width: worldWidth, height: worldHeight }
  });
  
  // Activate plugins with improved configuration
  viewport
    .drag({
      clampWheel: true,             // Prevent dragging beyond boundaries
      pressDrag: true,              // Drag on mouse down
      factor: 1.0,                  // Normal drag speed
      mouseButtons: 'all',          // Allow all mouse buttons for dragging
      underflow: 'center'           // When map is smaller than screen, center it
    })
    .pinch()                        // Enable pinch-to-zoom on touch devices
    .wheel({
      smooth: 5,                    // Add smooth zooming
      interrupt: true,              // Stop animation on user input
      percent: 0.1,                 // Zoom percent per scroll
      trackpadPinch: true           // Support trackpad pinch gesture
    })
    .decelerate({
      friction: 0.95,               // Reduce move speed per frame (higher = less deceleration)
      bounce: 0.8,                  // Bounce at the borders (lower = less bounce)
      minSpeed: 0.01                // Below this speed, deceleration stops
    })
    .clampZoom({                    // Constrain zoom levels
      minScale: minScale,           // Minimum zoom (2x)
      maxScale: 2.0                 // Maximum zoom
    });
  
  // Apply strict clamping to keep viewport within world boundaries
  viewport.clamp({
    direction: 'all',              // Clamp in all directions
    underflow: 'center',           // Center when smaller than screen
    left: 0,                       // Explicit left boundary
    right: worldWidth,             // Explicit right boundary
    top: 0,                        // Explicit top boundary
    bottom: worldHeight            // Explicit bottom boundary
  });
  
  // Force initial clamp to ensure viewport starts within boundaries
  viewport.ensureVisible(0, 0, worldWidth, worldHeight);
  
  // Create a debug function to check if viewport is within boundaries
  // but don't attach it to viewport to avoid TypeScript errors
  const checkViewportBoundaries = () => {
    const vp = viewport;
    const isWithinBoundaries = 
      vp.left >= 0 && 
      vp.right <= worldWidth && 
      vp.top >= 0 && 
      vp.bottom <= worldHeight;
    
    console.log("Viewport within boundaries:", isWithinBoundaries, {
      viewport: { 
        left: vp.left, 
        right: vp.right, 
        top: vp.top, 
        bottom: vp.bottom,
        width: vp.screenWidth,
        height: vp.screenHeight
      },
      world: { width: worldWidth, height: worldHeight }
    });
    
    return isWithinBoundaries;
  };
  
  // Store the function in a global that can be accessed from elsewhere
  (window as any).__checkViewportBoundaries = checkViewportBoundaries;
  
  return viewport;
};