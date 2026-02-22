import {
    BrowserWindow,
    dialog,
    type IpcMainInvokeEvent,
    type OpenDialogOptions,
    type SaveDialogOptions,
} from "electron";

import { MARKDOWN_FILE_EXTENSIONS } from "../config/runtime.js";

const resolveDialogOwner = (event: IpcMainInvokeEvent): BrowserWindow | undefined =>
    BrowserWindow.fromWebContents(event.sender) ?? undefined;

export const pickInputMarkdownPath = async (event: IpcMainInvokeEvent): Promise<string | null> => {
    const owner = resolveDialogOwner(event);
    const openOptions: OpenDialogOptions = {
        properties: ["openFile"],
        filters: [
            { name: "Markdown", extensions: [...MARKDOWN_FILE_EXTENSIONS] },
            { name: "All files", extensions: ["*"] },
        ],
    };

    const result = owner
        ? await dialog.showOpenDialog(owner, openOptions)
        : await dialog.showOpenDialog(openOptions);

    if (result.canceled || result.filePaths.length === 0) {
        return null;
    }

    return result.filePaths[0] ?? null;
};

export const pickOutputHtmlPath = async (
    event: IpcMainInvokeEvent,
    suggestedName?: string
): Promise<string | null> => {
    const owner = resolveDialogOwner(event);
    const safeName = suggestedName?.trim() ? suggestedName.trim() : "output.html";

    const saveOptions: SaveDialogOptions = {
        title: "Save HTML output",
        defaultPath: safeName.endsWith(".html") ? safeName : `${safeName}.html`,
        filters: [
            { name: "HTML", extensions: ["html"] },
            { name: "All files", extensions: ["*"] },
        ],
    };

    const result = owner
        ? await dialog.showSaveDialog(owner, saveOptions)
        : await dialog.showSaveDialog(saveOptions);

    if (result.canceled || !result.filePath) {
        return null;
    }

    return result.filePath;
};