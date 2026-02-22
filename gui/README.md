## Mint Infrastructure GUI

Electron desktop UI for `mintif.exe` conversion.

## Build GUI (Windows)

### Prerequisites

- Windows (x64)
- Node.js + npm
- GCC in `PATH` (used by root `build.bat`)

### Build and run (dev)

From repo root:

```powershell
.\build.bat
cd gui
npm install
npm run dev
```

### Build installer (`.exe`)

From repo root:

```powershell
.\build.bat
cd gui
npm install
npm run package:win
```

Output files:

- `gui/release/Mint Infrastructure GUI Setup 1.0.0.exe`
- `gui/release/Mint Infrastructure GUI Setup 1.0.0.exe.blockmap`
- `gui/release/win-unpacked/Mint Infrastructure GUI.exe` (portable test run)

What gets bundled:

- `mintif.exe` and `styles.css` from repo `dist/` into `resources/mintif`
- `darling.node` from `gui/bindings/build/Release` into `resources/darling`
- App icon and installer icon from `gui/assets/logo/Mintif Logo.ico`

### If built app is not using Darling Window

When native addon load fails, app falls back to plain `BrowserWindow`.

Check bundled native addon:

```powershell
Get-ChildItem .\gui\release\win-unpacked\resources\darling
```

Expected file:

- `gui/release/win-unpacked/resources/darling/darling.node`

Optional override for local debug:

```powershell
$env:DARLING_NODE_PATH = "E:\path\to\darling.node"
npm run start
```

## Core Commands

- `npm run build` compiles TypeScript and copies renderer assets to `dist/`.
- `npm run start` launches Electron from built output.
- `npm run dev` runs build then start.
- `npm run lint` runs ESLint on TypeScript sources.
- `npm test` runs Node tests after TypeScript build.

## Features

- Click-to-pick markdown file.
- Drag-and-drop one markdown file (`.md`, `.markdown`, `.mdown`, `.mkd`, `.mkdn`).
- Busy/progress status line during conversion (`Converting...`, success/error states).
- Auto-open output folder after successful conversion.

## Convert Request Options

`window.mintif.convertMarkdown(...)` supports:

- `inputPath` (required)
- `outputPath` (optional; inferred as `<input>.html` if omitted)
- `title` (optional)
- `stylesPath` (optional, explicit CSS file)
- `theme`: `default | mint | paper | noir`
- `accentColor`: hex (`#RGB` or `#RRGGBB`)
- `layoutWidth`: `centered | wide`
- `timeoutMs`: clamped to `5_000..300_000` (default `45_000`)

## Renderer API (from preload)

- `window.mintif.pickMarkdownFile(): Promise<string | null>`
- `window.mintif.pickOutputFile(suggestedName?: string): Promise<string | null>`
- `window.mintif.inspectPath(candidatePath: string): Promise<{ ok: true, ... } | { ok: false, error: string }>`
- `window.mintif.openPath(targetPath: string): Promise<{ ok: true } | { ok: false, error: string }>`
- `window.mintif.convertMarkdown(request): Promise<MintInfrastructureConvertResult>`
