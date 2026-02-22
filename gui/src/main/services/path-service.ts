import { promises as fs } from "node:fs";

import { shell } from "electron";

import { normalizeCandidatePath } from "../utils/path-utils.js";

export type InspectPathResult =
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

export type OpenPathResult =
    | {
        ok: true;
    }
    | {
        ok: false;
        error: string;
    };

export const inspectPath = async (candidatePath?: string): Promise<InspectPathResult> => {
    const resolvedPath = normalizeCandidatePath(candidatePath);
    if (!resolvedPath) {
        return {
            ok: false,
            error: "path is required",
        };
    }

    try {
        const stats = await fs.stat(resolvedPath);
        return {
            ok: true,
            path: resolvedPath,
            exists: true,
            isFile: stats.isFile(),
            isDirectory: stats.isDirectory(),
        };
    } catch (error) {
        const err = error as NodeJS.ErrnoException;
        if (err.code === "ENOENT") {
            return {
                ok: true,
                path: resolvedPath,
                exists: false,
                isFile: false,
                isDirectory: false,
            };
        }

        return {
            ok: false,
            error: err.message || "Failed to inspect path.",
        };
    }
};

export const openPath = async (targetPath?: string): Promise<OpenPathResult> => {
    const resolvedPath = normalizeCandidatePath(targetPath);
    if (!resolvedPath) {
        return { ok: false, error: "path is required" };
    }

    try {
        await fs.access(resolvedPath);
    } catch (error) {
        const err = error as NodeJS.ErrnoException;
        if (err.code === "ENOENT") {
            return { ok: false, error: `File not found: ${resolvedPath}` };
        }
        return { ok: false, error: err.message || "Failed to access file path." };
    }

    const openResult = await shell.openPath(resolvedPath);
    if (openResult && openResult.trim()) {
        return { ok: false, error: openResult };
    }

    return { ok: true };
};