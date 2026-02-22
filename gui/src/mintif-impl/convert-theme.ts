import * as fs from "node:fs";
import * as os from "node:os";
import * as path from "node:path";

import type { MintInfrastructureConvertRequest } from "../mint-infrastructure.js";
import { resolveMintifStylesPath } from "./convert-paths.js";

const HEX_COLOR_RE = /^#(?:[0-9a-fA-F]{3}|[0-9a-fA-F]{6})$/;
const VENDOR_DEFAULT_ACCENT = "#3d5869";

type MintifTheme = Exclude<NonNullable<MintInfrastructureConvertRequest["theme"]>, "default">;

type MintifThemePreset = {
    text: string;
    bg: string;
    link: string;
    codeBg: string;
    codeOutline: string;
    blockquoteBg: string;
    blockquoteText: string;
    border: string;
    tableBorder: string;
    shadowSm: string;
    shadowMd: string;
};

const THEME_PRESETS: Record<MintifTheme, MintifThemePreset> = {
    mint: {
        text: "#182727",
        bg: "#f8fffc",
        link: "#1d8f73",
        codeBg: "#eff8f4",
        codeOutline: "#c9dfd6",
        blockquoteBg: "#e7f4ee",
        blockquoteText: "#48635a",
        border: "#c9ddd6",
        tableBorder: "#aac5bc",
        shadowSm: "rgba(13, 59, 50, 0.09)",
        shadowMd: "rgba(13, 59, 50, 0.14)",
    },
    paper: {
        text: "#2a2117",
        bg: "#fffaf0",
        link: "#b0562d",
        codeBg: "#f7ede3",
        codeOutline: "#e4d1bd",
        blockquoteBg: "#f4e8d7",
        blockquoteText: "#6b5847",
        border: "#dec7af",
        tableBorder: "#b79677",
        shadowSm: "rgba(61, 34, 17, 0.08)",
        shadowMd: "rgba(61, 34, 17, 0.13)",
    },
    noir: {
        text: "#eaf1f3",
        bg: "#10171b",
        link: "#77d6c5",
        codeBg: "#182228",
        codeOutline: "#2b3a43",
        blockquoteBg: "#162129",
        blockquoteText: "#9ab6bd",
        border: "#2e404a",
        tableBorder: "#3f5965",
        shadowSm: "rgba(0, 0, 0, 0.33)",
        shadowMd: "rgba(0, 0, 0, 0.45)",
    },
};

type GeneratedStylesheet = {
    stylesPath: string;
    cleanup: () => void;
};

const isCustomTheme = (theme: MintInfrastructureConvertRequest["theme"]): theme is MintifTheme =>
    theme === "mint" || theme === "paper" || theme === "noir";

const normalizeCustomTheme = (theme: MintInfrastructureConvertRequest["theme"]): MintifTheme => {
    if (isCustomTheme(theme)) {
        return theme;
    }
    return "mint";
};

const normalizeHexColor = (rawColor: string | undefined): string | null => {
    if (!rawColor) {
        return null;
    }

    const trimmed = rawColor.trim();
    if (!HEX_COLOR_RE.test(trimmed)) {
        return null;
    }

    if (trimmed.length === 4) {
        const [, r, g, b] = trimmed;
        return `#${r}${r}${g}${g}${b}${b}`.toLowerCase();
    }

    return trimmed.toLowerCase();
};

const adjustHexColor = (hexColor: string, delta: number): string => {
    const normalized = normalizeHexColor(hexColor);
    if (!normalized) {
        return hexColor;
    }

    const channels = [
        Number.parseInt(normalized.slice(1, 3), 16),
        Number.parseInt(normalized.slice(3, 5), 16),
        Number.parseInt(normalized.slice(5, 7), 16),
    ];

    const nextChannels = channels.map((value) => {
        const adjusted = Math.max(0, Math.min(255, value + delta));
        return adjusted.toString(16).padStart(2, "0");
    });

    return `#${nextChannels.join("")}`;
};

export const maybeCreateStylesheetFromTheme = (
    request: MintInfrastructureConvertRequest
): GeneratedStylesheet | null => {
    const selectedTheme = request.theme ?? "default";
    const layoutWidth = request.layoutWidth === "wide" ? "wide" : "centered";
    const accentOverride = normalizeHexColor(request.accentColor);

    const hasThemeOverride = isCustomTheme(selectedTheme);
    const hasAccentOverride = Boolean(accentOverride);
    const hasLayoutOverride = layoutWidth === "wide";
    const hasThemeOverrides = hasThemeOverride || hasAccentOverride || hasLayoutOverride;
    if (!hasThemeOverrides) {
        return null;
    }

    const baseStylesPath = resolveMintifStylesPath();
    const baseStyles = fs.readFileSync(baseStylesPath, "utf8");

    const preset = THEME_PRESETS[normalizeCustomTheme(selectedTheme)];
    const effectiveAccent = accentOverride ?? (hasThemeOverride ? preset.link : VENDOR_DEFAULT_ACCENT);
    const accentHover = adjustHexColor(effectiveAccent, -24);

    const rootVariableLines: string[] = [];
    if (hasThemeOverride) {
        rootVariableLines.push(
            `--theme-color-text: ${preset.text};`,
            `--theme-color-bg: ${preset.bg};`,
            `--theme-color-code-bg: ${preset.codeBg};`,
            `--theme-color-code-outline: ${preset.codeOutline};`,
            `--theme-color-blockquote-bg: ${preset.blockquoteBg};`,
            `--theme-color-blockquote-text: ${preset.blockquoteText};`,
            `--theme-color-border: ${preset.border};`,
            `--theme-color-table-th-border: ${preset.tableBorder};`,
            `--theme-color-shadow-sm: ${preset.shadowSm};`,
            `--theme-color-shadow-md: ${preset.shadowMd};`
        );
    }

    if (hasAccentOverride || hasThemeOverride) {
        rootVariableLines.push(
            `--theme-color-link: ${effectiveAccent};`,
            `--theme-color-link-hover: ${accentHover};`
        );
    }

    const overrideBlocks: string[] = ["/* Mint Infrastructure generated overrides */"];

    if (rootVariableLines.length > 0) {
        overrideBlocks.push(
            ":root {",
            ...rootVariableLines.map((line) => `    ${line}`),
            "}"
        );
    }

    if (hasLayoutOverride) {
        overrideBlocks.push(
            "",
            "body {",
            "    max-width: 1020px;",
            "}"
        );
    }

    const overrideStyles = `${overrideBlocks.join("\n")}\n`;

    const tempDirectory = fs.mkdtempSync(path.join(os.tmpdir(), "mintif-style-"));
    const generatedStylesPath = path.join(tempDirectory, "styles.css");
    fs.writeFileSync(generatedStylesPath, `${baseStyles}\n${overrideStyles}\n`, "utf8");

    const cleanup = () => {
        try {
            fs.rmSync(tempDirectory, { recursive: true, force: true });
        } catch {
            // Best effort cleanup for temporary styles.
        }
    };

    return {
        stylesPath: generatedStylesPath,
        cleanup,
    };
};