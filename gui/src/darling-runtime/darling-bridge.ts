import * as fs from "node:fs";
import * as path from "node:path";
import { createRequire } from "node:module";
import { fileURLToPath } from "node:url";

const require = createRequire(import.meta.url);

type NativeFunction = (...args: any[]) => any;
type NativeAddon = Record<string, NativeFunction>;

const thisFile = fileURLToPath(import.meta.url);
const thisDir = path.dirname(thisFile);

const getAddonCandidates = (): string[] => {
  const candidates: string[] = [];

  const envPath = process.env.DARLING_NODE_PATH?.trim();
  if (envPath) {
    candidates.push(path.resolve(envPath));
  }

  candidates.push(
    path.resolve(thisDir, "../bindings/build/Release/darling.node"),
    path.resolve(process.cwd(), "dist/bindings/build/Release/darling.node"),
    path.resolve(process.cwd(), "bindings/build/Release/darling.node"),
    path.resolve(process.cwd(), "../Darling/bindings/build/Release/darling.node"),
  );

  return Array.from(new Set(candidates));
};

const resolveAddonExports = (loadedModule: unknown): NativeAddon | null => {
  if (!loadedModule || typeof loadedModule !== "object") {
    return null;
  }

  const direct = loadedModule as Record<string, unknown>;
  if (typeof direct.createWindow === "function") {
    return direct as NativeAddon;
  }

  const defaultExport = direct.default;
  if (defaultExport && typeof defaultExport === "object") {
    const wrapped = defaultExport as Record<string, unknown>;
    if (typeof wrapped.createWindow === "function") {
      return wrapped as NativeAddon;
    }
  }

  return null;
};

const loadNativeAddon = (): NativeAddon => {
  const candidates = getAddonCandidates();
  const loadErrors: string[] = [];

  for (const candidate of candidates) {
    if (!fs.existsSync(candidate)) {
      continue;
    }

    try {
      const loadedModule = require(candidate);
      const resolvedAddon = resolveAddonExports(loadedModule);
      if (resolvedAddon) {
        return resolvedAddon;
      }

      loadErrors.push(`"${candidate}" loaded but missing "createWindow" export.`);
    } catch (error) {
      loadErrors.push(`"${candidate}" failed: ${(error as Error).message}`);
    }
  }

  const details =
    loadErrors.length > 0
      ? loadErrors.join(" | ")
      : `No candidate file exists. Checked: ${candidates.map((candidate) => `"${candidate}"`).join(", ")}`;

  throw new Error(`Darling native addon not loaded. ${details}`);
};

const createErrorAddon = (cause: Error): NativeAddon =>
  new Proxy({} as NativeAddon, {
    get: (_target, propertyKey) => {
      if (typeof propertyKey !== "string") {
        return undefined;
      }

      return (..._args: any[]) => {
        throw new Error(
          `Darling native addon not loaded (${cause.message}). Attempted call: "${propertyKey}".`,
        );
      };
    },
  });

let native: NativeAddon;
try {
  native = loadNativeAddon();
} catch (error) {
  native = createErrorAddon(error as Error);
}

const callNative = (methodName: string, ...args: any[]) => {
  const method = native[methodName];
  if (typeof method !== "function") {
    throw new Error(`Darling native addon missing function "${methodName}".`);
  }
  return method(...args);
};

export const createWindow = (...args: any[]) => callNative("createWindow", ...args);
export const destroyWindow = (win: any) => callNative("destroyWindow", win);
export const onCloseRequested = (cb: () => void) => callNative("onCloseRequested", cb);
export const showDarlingWindow = (win: any) => callNative("showDarlingWindow", win);
export const hideDarlingWindow = (win: any) => callNative("hideDarlingWindow", win);
export const focusDarlingWindow = (win: any) => callNative("focusDarlingWindow", win);
export const setChildWindow = (win: any, childHwnd: any) =>
  callNative("setChildWindow", win, childHwnd);
export const setWindowTitle = (win: any, title: string) =>
  callNative("setWindowTitle", win, title);
export const setWindowIconVisible = (win: any, visible: boolean) =>
  callNative("setWindowIconVisible", win, visible);
export const setWindowOpacity = (win: any, opacity: number) =>
  callNative("setWindowOpacity", win, opacity);
export const setAlwaysOnTop = (win: any, enable: boolean) =>
  callNative("setAlwaysOnTop", win, enable);
export const pollEvents = () => callNative("pollEvents");
export const getHWND = () => callNative("getHWND");
export const paintFrame = (buffer: Buffer, w: number, h: number) =>
  callNative("paintFrame", buffer, w, h);
export const setParent = (child: any, parent: any) =>
  callNative("setParent", child, parent);
export const setWindowStyles = (hwnd: any, add: number, remove: number) =>
  callNative("setWindowStyles", hwnd, add, remove);
export const setWindowExStyles = (hwnd: any, add: number, remove: number) =>
  callNative("setWindowExStyles", hwnd, add, remove);
export const setWindowPos = (
  hwnd: any,
  x: number,
  y: number,
  w: number,
  h: number,
  flags: number,
) => callNative("setWindowPos", hwnd, x, y, w, h, flags);
export const showWindow = (hwnd: any, cmd: number) =>
  callNative("showWindow", hwnd, cmd);
export const isVisible = (win: any) => callNative("isVisible", win);
export const isFocused = (win: any) => callNative("isFocused", win);
export const isDarkMode = () => callNative("isDarkMode");
export const setDarkMode = (win: any, enable: boolean) =>
  callNative("setDarkMode", win, enable);
export const setAutoDarkMode = (win: any) => callNative("setAutoDarkMode", win);
export const setTitlebarColors = (win: any, bg: number, text: number) =>
  callNative("setTitlebarColors", win, bg, text);
export const setTitlebarColor = (win: any, color: number) =>
  callNative("setTitlebarColor", win, color);
export const setCornerPreference = (win: any, pref: number) =>
  callNative("setCornerPreference", win, pref);
export const flashWindow = (win: any, continuous: boolean) =>
  callNative("flashWindow", win, continuous);
export const getDpi = (win: any) => callNative("getDpi", win);
export const getScaleFactor = (win: any) => callNative("getScaleFactor", win);
