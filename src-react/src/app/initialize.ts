import { Application } from "pixi.js";

/**
 * Initialize the PixiJS application
 * @returns The initialized PixiJS application
 */
export const initializeApp = async (): Promise<Application> => {
  const app = new Application();
  await app.init({ resizeTo: window, backgroundColor: 0x808080 });
  
  const pixiContainer = document.getElementById("pixi-container");
  if (!pixiContainer) throw new Error("Could not find element with id 'pixi-container'");
  pixiContainer.appendChild(app.canvas);
  return app;
};
