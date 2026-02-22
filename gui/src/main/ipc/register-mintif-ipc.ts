import { ipcMain, type IpcMainInvokeEvent } from "electron";

import { runMintInfrastructureConvert, type MintInfrastructureConvertRequest } from "../../mint-infrastructure.js";
import { pickInputMarkdownPath, pickOutputHtmlPath } from "../dialogs/file-dialogs.js";
import { inspectPath, openPath } from "../services/path-service.js";

export const registerMintifIpc = (): void => {
    ipcMain.removeHandler("mintif:convert");
    ipcMain.handle("mintif:convert", async (event, request: MintInfrastructureConvertRequest) => {
        void event;
        return runMintInfrastructureConvert(request);
    });

    ipcMain.removeHandler("mintif:pick-input");
    ipcMain.handle("mintif:pick-input", pickInputMarkdownPath);

    ipcMain.removeHandler("mintif:pick-output");
    ipcMain.handle("mintif:pick-output", pickOutputHtmlPath);

    ipcMain.removeHandler("mintif:inspect-path");
    ipcMain.handle("mintif:inspect-path", async (event: IpcMainInvokeEvent, candidatePath?: string) => {
        void event;
        return inspectPath(candidatePath);
    });

    ipcMain.removeHandler("mintif:open-path");
    ipcMain.handle("mintif:open-path", async (event: IpcMainInvokeEvent, targetPath?: string) => {
        void event;
        return openPath(targetPath);
    });
};