import type {
    MintInfrastructureConvertRequest,
    MintInfrastructureConvertResult,
} from "./mint-infrastructure.js";

import { contextBridge, ipcRenderer, webUtils } from "electron";

type MintifInspectPathResult =
    | {
        ok: true;
        path: string;
        exists: boolean;
        isFile: boolean;
        isDirectory: boolean;
    }
    | {
        ok: false;
        error: string;
    };

type MintifOpenPathResult =
    | {
        ok: true;
    }
    | {
        ok: false;
        error: string;
    };

const mintifApi = {
    convertMarkdown: (
        request: MintInfrastructureConvertRequest
    ): Promise<MintInfrastructureConvertResult> => ipcRenderer.invoke("mintif:convert", request),
    pickMarkdownFile: (): Promise<string | null> => ipcRenderer.invoke("mintif:pick-input"),
    pickOutputFile: (suggestedName?: string): Promise<string | null> =>
        ipcRenderer.invoke("mintif:pick-output", suggestedName),
    inspectPath: (candidatePath: string): Promise<MintifInspectPathResult> =>
        ipcRenderer.invoke("mintif:inspect-path", candidatePath),
    openPath: (targetPath: string): Promise<MintifOpenPathResult> =>
        ipcRenderer.invoke("mintif:open-path", targetPath),
    getPathForFile: (file: File): string | null => {
        try {
            const resolvedPath = webUtils.getPathForFile(file);
            return resolvedPath && resolvedPath.trim() ? resolvedPath : null;
        } catch {
            return null;
        }
    },
};

contextBridge.exposeInMainWorld("mintif", mintifApi);
