import * as fs from "node:fs";
import * as path from "node:path";
import { fileURLToPath } from "node:url";

import { app } from "electron";

const resolveFirstExistingPath = (candidates: string[]): string | null => {
    for (const candidate of candidates) {
        if (fs.existsSync(candidate)) {
            return candidate;
        }
    }
    
    return null;
};

const formatCheckedCandidates = (candidates: string[]): string =>
    candidates.map((candidate) => `"${candidate}"`).join(", ");

const buildMintifDevCandidates = (fileName: string): string[] => {
    const here = path.dirname(fileURLToPath(import.meta.url));

    return [
        path.join(here, "..", "..", "vendor", "mintif", fileName),
        path.join(app.getAppPath(), "vendor", "mintif", fileName),
        path.join(process.cwd(), "vendor", "mintif", fileName),
    ];
};

export const validateInputMarkdown = (inputPath: string): void => {
    const ext = path.extname(inputPath).toLowerCase();
    
    if (ext !== ".md" && ext !== ".markdown") {
        throw new Error("Input must be a .md or .markdown file.");
    }
};

export const inferOutputPath = (inputPath: string): string => {
    const parsed = path.parse(inputPath);
    return path.join(parsed.dir, `${parsed.name}.html`);
};

export const resolveMintifStylesPath = (): string => {
    const candidates = app.isPackaged
        ? [path.join(process.resourcesPath, "mintif", "styles.css")]
        : buildMintifDevCandidates("styles.css");

    const resolvedPath = resolveFirstExistingPath(candidates);
    
    if (resolvedPath) {
        return resolvedPath;
    }

    throw new Error(`styles.css not found. Checked: ${formatCheckedCandidates(candidates)}`);
};

export const resolveMintifExePath = (): string => {
    const envPath = process.env.MINTIF_EXE_PATH?.trim();
    const envCandidates = envPath ? [path.resolve(envPath)] : [];

    const candidates = app.isPackaged
        ? [...envCandidates, path.join(process.resourcesPath, "mintif", "mintif.exe")]
        : [...envCandidates, ...buildMintifDevCandidates("mintif.exe")];

    const resolvedPath = resolveFirstExistingPath(candidates);

    if (resolvedPath) {
        return resolvedPath;
    }

    throw new Error(`mintif.exe not found. Checked: ${formatCheckedCandidates(candidates)}`);
};