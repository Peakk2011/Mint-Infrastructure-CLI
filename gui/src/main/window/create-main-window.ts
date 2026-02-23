import { BrowserWindow } from "electron";

import { CreateWindow } from "../../darling-runtime/index.js";
import { WINDOW_SIZE, appTitle, preloadPath } from "../config/runtime.js";
import { disableDevTools } from "./devtools.js";

export type DarlingWindowInstance = Awaited<ReturnType<typeof CreateWindow>>;

export type CreatedWindow = {
    browserWindow: BrowserWindow;
    darlingWindow: DarlingWindowInstance | null;
};

const createElectronFallbackWindow = (windowUrl: string): CreatedWindow => {
    const browserWindow = new BrowserWindow({
        width: WINDOW_SIZE.width,
        height: WINDOW_SIZE.height,
        minWidth: WINDOW_SIZE.width,
        minHeight: WINDOW_SIZE.height,
        maxWidth: WINDOW_SIZE.width,
        maxHeight: WINDOW_SIZE.height,
        center: true,
        resizable: false,
        maximizable: false,
        fullscreenable: false,
        autoHideMenuBar: true,
        title: appTitle,
        webPreferences: {
            nodeIntegration: false,
            contextIsolation: true,
            preload: preloadPath,
            devTools: false,
        },
    });

    void browserWindow.loadURL(windowUrl);
    return {
        browserWindow,
        darlingWindow: null,
    };
};

const createDarlingWindow = async (windowUrl: string): Promise<CreatedWindow> => {
    try {
        const win = await CreateWindow({
            width: WINDOW_SIZE.width,
            height: WINDOW_SIZE.height,
            url: windowUrl,
            title: appTitle,
            showIcon: false,
            webPreferences: {
                nodeIntegration: false,
                contextIsolation: true,
                preload: preloadPath,
                devTools: false,
            },
            electron: (browserWindow: BrowserWindow) => {
                browserWindow.setMinimumSize(WINDOW_SIZE.width, WINDOW_SIZE.height);
                browserWindow.setMaximumSize(WINDOW_SIZE.width, WINDOW_SIZE.height);
                browserWindow.setResizable(false);
                browserWindow.setMaximizable(false);
                browserWindow.setFullScreenable(false);
                browserWindow.setAutoHideMenuBar(true);
                browserWindow.setTitle(appTitle);
            },
        });

        return {
            browserWindow: win.browserWindow as BrowserWindow,
            darlingWindow: win,
        };
    } catch (error) {
        console.error("Failed to create Darling window. Falling back to BrowserWindow.", error);
        return createElectronFallbackWindow(windowUrl);
    }
};

export const createMainWindow = async (windowUrl: string): Promise<CreatedWindow> => {
    const createdWindow = await createDarlingWindow(windowUrl);
    disableDevTools(createdWindow.browserWindow);
    return createdWindow;
};