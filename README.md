<img src='./assest/Mint infrastrucure logo.png' alt='Logo'><br>

# Mint Infrastructure

> A gorgeous lightweight Markdown-to-HTML compiler written in C99.

`Mintif (Mint Infrastructure)` reads a `.md` file, parses common Markdown syntax, wraps the result in a full HTML document, and optionally embeds CSS from a style file.

### Features

- Single native binary (`mintif.exe` (Windows))
- Converts Markdown input into standalone HTML
- Auto-generates output filename when `-o` is not provided
- Auto-detects document title from first `# Heading`
- Supports custom page title and custom CSS file
- Uses `styles.css` next to the executable by default

### Supported Markdown

- Headings:
  - ATX: `#` to `######`
  - Setext: `===` and `---`
- Paragraphs and line joining
- Emphasis:
  - `*italic*`, `_italic_`
  - `**bold**`
  - `***bold+italic***`
  - `~~strikethrough~~`
- Inline code: `` `code` ``
- Links: `[text](url)`
- Images: `![alt](url)`
- Fenced code blocks:
  - Triple backticks: `` ``` ``
  - Triple tildes: `~~~`
  - Optional language hint -> `class="language-..."`
- Blockquotes: `>`
- Lists:
  - Unordered: `-`, `*`, `+`
  - Ordered: `1.`, `2.`, ...
- Horizontal rules: `---`, `***`, `___`
- Pipe tables:
  - Header row + separator row
  - Body rows

### Requirements

- GCC (or compatible C compiler)
- Windows, Linux, or macOS (source is portable C; `build.bat` is Windows-specific)

### Build

#### Windows (batch script)

```bat
build.bat
```

#### Manual build

```bash
gcc mintif.c -o mintif.exe -std=c99 -O2
```

On Unix-like systems, use:

```bash
gcc mintif.c -o mintif -std=c99 -O2
```

### Usage

```bash
mintif <input.md> [options]
```

Options:

| Option | Description |
| --- | --- |
| `-o, --output <file>` | Output HTML file |
| `-t, --title <text>` | Custom `<title>` value |
| `-s, --styles <file>` | CSS file to embed |
| `-h, --help` | Show help |

### Examples

```bash
mintif notes.md -o notes.html
mintif notes.md
mintif notes.md -t "Project Notes"
mintif notes.md -s styles.css
```

### Behavior Notes

- If `-o` is omitted, output is `<input-name>.html`
- If `-t` is omitted, title is taken from the first `# Heading`
- If no heading is found, filename stem is used as title
- If `-s` is omitted, `styles.css` is loaded from the executable directory
- If default CSS cannot be read, HTML is still generated without styles

### Project Files

- `mintif.c`: Main compiler source
- `build.bat`: Windows build helper
- `styles.css`: Default embedded stylesheet
- `mintif.exe`: Built binary (if already compiled)

### Scope and Limitations

This is a custom lightweight parser, not a full CommonMark implementation.<br>
Some advanced Markdown behaviors (deep nesting, edge-case compatibility, extended plugins) are intentionally out of scope.
