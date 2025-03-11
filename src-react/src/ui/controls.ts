import { Viewport } from "pixi-viewport";

/**
 * Add zoom control buttons to the UI
 * @param viewport The viewport to control
 */
export const addZoomControls = (viewport: Viewport) => {
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
