import * as path from "node:path";
import { fileURLToPath, pathToFileURL } from "node:url";

export const WINDOW_SIZE = Object.freeze({
    width: 320,
    height: 360,
});

export const MARKDOWN_FILE_EXTENSIONS = Object.freeze(["md", "markdown", "mdown", "mkd", "mkdn"]);

const thisFile = fileURLToPath(import.meta.url);
const thisDir = path.dirname(thisFile);

export const defaultWindowPath = path.join(thisDir, "..", "..", "renderer", "index.html");
export const preloadPath = path.join(thisDir, "..", "..", "preload.cjs");
export const appTitle = "Mint Infrastructure";

export const resolveWindowUrl = (
    argv: string[] = process.argv,
    env: NodeJS.ProcessEnv = process.env
): string => {
    const argUrl = argv.find((arg) => arg.startsWith("--url="))?.slice("--url=".length)?.trim();
    const envUrl = env.MINT_GUI_URL?.trim();
    const raw = argUrl || envUrl || defaultWindowPath;

    if (/^https?:\/\//i.test(raw) || /^file:\/\//i.test(raw)) {
        return raw;
    }

    if (/^[a-z0-9.-]+\.[a-z]{2,}([/:?#].*)?$/i.test(raw)) {
        return `https://${raw}`;
    }

    return pathToFileURL(path.resolve(raw)).toString();
};