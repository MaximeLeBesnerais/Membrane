import { bgCSV } from "./csv/background";
import { fgCSV } from "./csv/foreground";
import { mainCSV } from "./csv/main";

/**
 * Parse CSV content into a 2D array of numbers
 * @param content CSV content as a string
 * @returns 2D array of tile IDs
 */
export const loadCSV = async (content: string): Promise<number[][]> => {
  try {
    return content.split('\n').map(line => 
      line.split(',').map(cell => {
        const parsed = parseInt(cell.trim(), 10);
        return isNaN(parsed) ? -1 : parsed;
      })
    );
  } catch (error) {
    console.error("Error loading CSV data:", error);
    return [];
  }
};

/**
 * Load all map CSV data
 * @returns Object containing all map layers
 */
export const loadCSVFiles = async () => {
  console.log("Loading CSV files...");
  const [backgroundLayer, mainLayer, foregroundLayer] = await Promise.all([
    loadCSV(bgCSV),
    loadCSV(mainCSV),
    loadCSV(fgCSV)
  ]);
  console.log("CSV files loaded");
  return { backgroundLayer, mainLayer, foregroundLayer };
};
