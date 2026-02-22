import { app, BrowserWindow } from "electron";

type RegisterLifecycleEventsArgs = {
    createWindow: () => Promise<void>;
};

export const registerLifecycleEvents = ({ createWindow }: RegisterLifecycleEventsArgs): void => {
    app.on("activate", () => {
        if (BrowserWindow.getAllWindows().length === 0) {
            void createWindow().catch((error) => {
                console.error("Failed to create window on app activate:", error);
            });
        }
    });

    app.on("window-all-closed", () => {
        if (process.platform !== "darwin") {
            app.quit();
        }
    });
};