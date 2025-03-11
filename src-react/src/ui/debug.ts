import { Application, Sprite } from "pixi.js";
import { Viewport } from "pixi-viewport";

/**
 * Add debug information display to the UI
 * @param app The PixiJS application
 * @param viewport The viewport to monitor
 * @param character The character sprite to monitor (optional)
 */
export const addDebugInfo = (app: Application, viewport: Viewport, character: Sprite | null) => {
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
    let characterInfo = 'Character: Not found';
    if (character) {
      characterInfo = `Character: (${character.x.toFixed(0)}, ${character.y.toFixed(0)})`;
    }
    
    debugDiv.innerHTML = `
      Scale: ${viewport.scale.x.toFixed(2)}x<br>
      Position: (${viewport.x.toFixed(0)}, ${viewport.y.toFixed(0)})<br>
      Center: (${viewport.center.x.toFixed(0)}, ${viewport.center.y.toFixed(0)})<br>
      World Size: ${viewport.worldWidth}x${viewport.worldHeight}<br>
      Screen Size: ${viewport.screenWidth}x${viewport.screenHeight}<br>
      ${characterInfo}
    `;
  });
};
