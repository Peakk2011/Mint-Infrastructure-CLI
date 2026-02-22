import type { BrowserWindow } from "electron";

export const disableDevTools = (browserWindow: BrowserWindow): void => {
    browserWindow.webContents.on("devtools-opened", () => {
        browserWindow.webContents.closeDevTools();
    });

    browserWindow.webContents.on("before-input-event", (event, input) => {
        const key = (input.key || "").toLowerCase();
        const isF12 = key === "f12";
        const isCtrlShiftInspector = input.control && input.shift && (key === "i" || key === "j" || key === "c");
        const isMetaAltInspector = input.meta && input.alt && (key === "i" || key === "j" || key === "c");

        if (isF12 || isCtrlShiftInspector || isMetaAltInspector) {
            event.preventDefault();
        }
    });
};