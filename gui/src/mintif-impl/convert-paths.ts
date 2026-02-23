import * as fs from "node:fs";
import * as path from "node:path";
import { fileURLToPath } from "node:url";

import { app } from "electron";

/**
 * Returns the platform-appropriate binary filename for mintif.
 * - Windows: `mintif.exe`
 * - macOS / Linux: `mintif`
 */
const getMintifBin = (): string =>
    process.platform === "win32" ? "mintif.exe" : "mintif";

/**
 * Returns the first path in `candidates` that exists on disk,
 * or `null` if none are found.
 */
const resolveFirstExistingPath = (candidates: string[]): string | null => {
    for (const candidate of candidates) {
        if (fs.existsSync(candidate)) {
            return candidate;
        }
    }

    return null;
};

/**
 * Builds candidate paths for a given filename in the `vendor/mintif` directory,
 * used during development (non-packaged) builds.
 *
 * Checks three locations in order:
 * 1. Relative to the compiled output file (`__dirname`-equivalent)
 * 2. Relative to Electron's app path
 * 3. Relative to the current working directory
 */
const buildMintifDevCandidates = (fileName: string): string[] => {
    const here = path.dirname(fileURLToPath(import.meta.url));

    return [
        path.join(here, "..", "..", "vendor", "mintif", fileName),
        path.join(app.getAppPath(), "vendor", "mintif", fileName),
        path.join(process.cwd(), "vendor", "mintif", fileName),
    ];
};

/**
 * Builds candidate paths for a given filename in a packaged Electron app,
 * resolving from `process.resourcesPath`.
 */
const buildPackagedCandidates = (fileName: string): string[] => [
    path.join(process.resourcesPath, "mintif", fileName),
];

/**
 * Builds the full list of candidate paths for a given filename,
 * prepending any path supplied via the `MINTIF_EXE_PATH` environment variable.
 *
 * Resolution order:
 * 1. `MINTIF_EXE_PATH` env var (if set)
 * 2. Packaged resource path (if `app.isPackaged`)
 * 3. Dev candidate paths (if not packaged)
 */
const buildCandidates = (fileName: string): string[] => {
    const envPath = process.env.MINTIF_EXE_PATH?.trim();
    const envCandidates = envPath ? [path.resolve(envPath)] : [];
    const platformCandidates = app.isPackaged
        ? buildPackagedCandidates(fileName)
        : buildMintifDevCandidates(fileName);

    return [...envCandidates, ...platformCandidates];
};

/**
 * Resolves the absolute path to a mintif asset file (binary or stylesheet)
 * by searching all candidate paths in priority order.
 *
 * @param fileName - The filename to resolve, e.g. `"mintif.exe"` or `"styles.css"`.
 * @returns The resolved absolute path to the file.
 * @throws {Error} If the file cannot be found in any candidate path.
 */
const resolveAsset = (fileName: string): string => {
    const candidates = buildCandidates(fileName);
    const resolvedPath = resolveFirstExistingPath(candidates);

    if (resolvedPath) {
        return resolvedPath;
    }

    throw new Error(
        `${fileName} not found. Checked: ${candidates.map((c) => `"${c}"`).join(", ")}`
    );
};

/**
 * Validates that the given file path points to a Markdown file.
 *
 * @param inputPath - The file path to validate.
 * @throws {Error} If the file extension is not `.md` or `.markdown`.
 */
export const validateInputMarkdown = (inputPath: string): void => {
    const ext = path.extname(inputPath).toLowerCase();

    if (ext !== ".md" && ext !== ".markdown") {
        throw new Error("Input must be a .md or .markdown file.");
    }
};

/**
 * Infers the output HTML file path from a given input Markdown file path,
 * preserving the directory and base name.
 *
 * @param inputPath - The input Markdown file path.
 * @returns The inferred output path with a `.html` extension.
 *
 * @example
 * inferOutputPath("/docs/readme.md") // → "/docs/readme.html"
 */
export const inferOutputPath = (inputPath: string): string => {
    const parsed = path.parse(inputPath);
    return path.join(parsed.dir, `${parsed.name}.html`);
};

/**
 * Resolves the absolute path to the mintif `styles.css` file.
 * @returns The resolved absolute path to `styles.css`.
 * @throws {Error} If `styles.css` cannot be found in any candidate path.
 */
export const resolveMintifStylesPath = (): string =>
    resolveAsset("styles.css");

/**
 * Resolves the absolute path to the mintif binary (`mintif` or `mintif.exe`),
 * depending on the current platform.
 * @returns The resolved absolute path to the mintif binary.
 * @throws {Error} If the binary cannot be found in any candidate path.
 */
export const resolveMintInfrastructureBinaryPath = (): string => {
    const bin = getMintifBin();

    console.log("[resolveMintInfrastructureBinaryPath]", {
        platform: process.platform,
        bin,
        candidates: buildCandidates(bin),
    });

    return resolveAsset(bin);
};