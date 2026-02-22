import * as fs from "node:fs";
import * as path from "node:path";

import type {
    MintInfrastructureConvertRequest,
    MintInfrastructureConvertResult,
} from "../mint-infrastructure.js";
import {
    inferOutputPath,
    resolveMintifExePath,
    validateInputMarkdown,
} from "./convert-paths.js";
import { runConvertProcess } from "./convert-process.js";
import { maybeCreateStylesheetFromTheme } from "./convert-theme.js";

const DEFAULT_TIMEOUT_MS = 45_000;

const normalizeTimeout = (timeoutMs: number | undefined): number => {
    if (typeof timeoutMs !== "number" || !Number.isFinite(timeoutMs)) {
        return DEFAULT_TIMEOUT_MS;
    }

    return Math.max(5_000, Math.min(300_000, Math.floor(timeoutMs)));
};

const resolveInputPath = (inputPath: string | undefined): string | null => {
    const rawInput = inputPath?.trim();
    if (!rawInput) {
        return null;
    }

    return path.resolve(rawInput);
};

const resolveOutputPath = (request: MintInfrastructureConvertRequest, inputPath: string): string => {
    if (request.outputPath?.trim()) {
        return path.resolve(request.outputPath);
    }

    return inferOutputPath(inputPath);
};

const resolveCustomStylesArg = (stylesPath: string | undefined): string | null => {
    const trimmedStylesPath = stylesPath?.trim();
    if (!trimmedStylesPath) {
        return null;
    }

    const resolvedStylesPath = path.resolve(trimmedStylesPath);
    if (!fs.existsSync(resolvedStylesPath)) {
        throw new Error(`stylesPath not found: ${resolvedStylesPath}`);
    }

    return resolvedStylesPath;
};

export const runMintInfrastructureConvertComponent = async (
    request: MintInfrastructureConvertRequest
): Promise<MintInfrastructureConvertResult> => {
    const inputPath = resolveInputPath(request.inputPath);
    if (!inputPath) {
        return { ok: false, error: "inputPath is required." };
    }

    if (!fs.existsSync(inputPath)) {
        return { ok: false, error: `Input file not found: ${inputPath}` };
    }

    try {
        validateInputMarkdown(inputPath);
    } catch (error) {
        return { ok: false, error: (error as Error).message };
    }

    const outputPath = resolveOutputPath(request, inputPath);
    const timeoutMs = normalizeTimeout(request.timeoutMs);

    let exePath = "";
    try {
        exePath = resolveMintifExePath();
    } catch (error) {
        return { ok: false, error: (error as Error).message, outputPath };
    }

    const args: string[] = [inputPath, "-o", outputPath];

    const trimmedTitle = request.title?.trim();
    if (trimmedTitle) {
        args.push("-t", trimmedTitle);
    }

    let generatedStylesCleanup: (() => void) | null = null;

    try {
        const customStylesPath = resolveCustomStylesArg(request.stylesPath);
        if (customStylesPath) {
            args.push("-s", customStylesPath);
        } else {
            const generatedStyles = maybeCreateStylesheetFromTheme(request);
            if (generatedStyles) {
                generatedStylesCleanup = generatedStyles.cleanup;
                args.push("-s", generatedStyles.stylesPath);
            }
        }
    } catch (error) {
        generatedStylesCleanup?.();
        return {
            ok: false,
            error: (error as Error).message,
            outputPath,
        };
    }

    const cleanupGeneratedStyles = () => {
        if (!generatedStylesCleanup) {
            return;
        }

        generatedStylesCleanup();
        generatedStylesCleanup = null;
    };

    return runConvertProcess({
        exePath,
        args,
        outputPath,
        timeoutMs,
        cleanupGeneratedStyles,
    });
};