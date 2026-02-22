## Mint Infrastructure GUI

Electron desktop UI for `mintif.exe` conversion.

## Install

1. `cd gui`
2. `npm install`

## Run (dev)

1. `npm run sync:mintif`
2. `npm run build`
3. `npm run start`

Notes:

- `npm run build` compiles TypeScript and copies renderer assets/bindings to `dist/`.
- Default startup URL is `dist/renderer/index.html`.
- `npm run start` uses `scripts/start-electron.mjs` and clears `ELECTRON_RUN_AS_NODE`.

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

## Error Handling

- Invalid input extension returns a clear validation error.
- Missing `mintif.exe` or `styles.css` returns searched-path diagnostics.
- Non-zero process exit, spawn failure, and timeout are surfaced with `stdout`/`stderr`.
- Temporary generated theme CSS files are cleaned up in success/error paths.

## Renderer API (from preload)

- `window.mintif.pickMarkdownFile(): Promise<string | null>`
- `window.mintif.pickOutputFile(suggestedName?: string): Promise<string | null>`
- `window.mintif.inspectPath(candidatePath: string): Promise<{ ok: true, ... } | { ok: false, error: string }>`
- `window.mintif.openPath(targetPath: string): Promise<{ ok: true } | { ok: false, error: string }>`
- `window.mintif.convertMarkdown(request): Promise<MintInfrastructureConvertResult>`

## Testing and Lint

- `npm run lint` - ESLint for TypeScript sources.
- `npm test` - builds TS then runs `node:test` cases.

## Package (Windows)

1. `npm run package:win`

`mintif.exe` and `styles.css` are bundled from `vendor/mintif` to
`resources/mintif` via `extraResources`.
