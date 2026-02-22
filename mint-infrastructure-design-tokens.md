# Mint Infrastructure — Design Tokens

Color tokens use the `--theme-` prefix declare on `:root` and dark mode override via `@media (prefers-color-scheme: dark)`

## Color Tokens

| Token | Light | Dark |
|---|---|---|
| `--theme-color-text` | `#000000` | `#f4f4f4` |
| `--theme-color-bg` | `#fcfffc` | `#0f0f0f` |
| `--theme-color-link` | `#3d5869` | `hsla(240, 85%, 69%, 1)` |
| `--theme-color-link-hover` | `#666666` | `#999999` |
| `--theme-color-code-bg` | `#f7f7f7` | `#1f1f1f` |
| `--theme-color-code-outline` | `hsl(0, 0%, 88%)` | — |
| `--theme-color-blockquote-bg` | `#f0f0f0` | `hsl(120, 0%, 15%)` |
| `--theme-color-blockquote-text` | `#666666` | `#666666` |
| `--theme-color-border` | `#a7a6a3` | `#343434` |
| `--theme-color-table-th-bg` | `#eae9e5` | `#1f1f1f` |
| `--theme-color-shadow-sm` | `rgba(0, 0, 0, 0.05)` | `rgba(0, 0, 0, 0.20)` |
| `--theme-color-shadow-md` | `rgba(0, 0, 0, 0.06)` | `rgba(0, 0, 0, 0.30)` |

## Notes

- Spacing and font-size all of these are hardcoded value — and is not configurable tokenize
- Shadow tokens on dark mode are chenge the opacity
- `--theme-color-code-outline` is no dark override because `pre` element on dark mode is not used outline