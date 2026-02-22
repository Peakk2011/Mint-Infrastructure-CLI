import type { BrowserWindow } from "electron";
import { app } from "electron";

import { resolveWindowUrl } from "./main/config/runtime.js";
import { registerMintifIpc } from "./main/ipc/register-mintif-ipc.js";
import { registerLifecycleEvents } from "./main/lifecycle/register-lifecycle-events.js";
import { electronRuntime } from "./main/runtime/electron-runtime.js";
import { createMainWindow } from "./main/window/create-main-window.js";

let mainWindow: BrowserWindow | null = null;

const assignMainWindow = async (windowUrl: string): Promise<void> => {
    const created = await createMainWindow(windowUrl);

    created.browserWindow.on("closed", () => {
        if (mainWindow === created.browserWindow) {
            mainWindow = null;
        }
    });

    mainWindow = created.browserWindow;
};

const startMainProcess = async (): Promise<void> => {
    electronRuntime();

    const windowUrl = resolveWindowUrl();

    await app.whenReady();
    registerMintifIpc();

    const createWindow = async () => {
        await assignMainWindow(windowUrl);
    };

    await createWindow();
    registerLifecycleEvents({ createWindow });
};

void startMainProcess().catch((error) => {
    console.error("Error while starting Electron:", error);
    app.exit(1);
});