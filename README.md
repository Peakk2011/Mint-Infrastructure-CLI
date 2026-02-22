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
- Parser code split into focused modules (`parser_line`, `parser_html`, `inline_html`)

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

#### macOS (shell script)

```bash
./build-macos.sh
```

#### Manual build

Windows:

```bat
gcc main.c ^
    src/buf/buf.c ^
    src/io/io.c ^
    src/parser/inline.c ^
    src/parser/inline_html.c ^
    src/parser/parser.c ^
    src/parser/parser_line.c ^
    src/parser/parser_html.c ^
    src/path/path.c ^
    src/html/html.c ^
    -o mintif.exe -std=c99 -O2 -I.
```

Unix-like:

```bash
gcc main.c \
  src/buf/buf.c \
  src/io/io.c \
  src/parser/inline.c \
  src/parser/inline_html.c \
  src/parser/parser.c \
  src/parser/parser_line.c \
  src/parser/parser_html.c \
  src/path/path.c \
  src/html/html.c \
  -o mintif -std=c99 -O2 -I.
```

### Usage

```bash
mintif <input.md> [options]
```

Options:

| Option                | Description            |
| --------------------- | ---------------------- |
| `-o, --output <file>` | Output HTML file       |
| `-t, --title <text>`  | Custom `<title>` value |
| `-s, --styles <file>` | CSS file to embed      |
| `-h, --help`          | Show help              |

### Examples

```bash
mintif notes.md -o notes.html
mintif notes.md
mintif notes.md -t "Project Notes"
mintif notes.md -s styles.css
```

### Example Input / Output

Input markdown:

```md
# API Notes

Mint supports **bold**, `inline code`, and tables.

| Name | Status |
| --- | --- |
| Parser | Ready |
```

Generated HTML body (excerpt):

```html
<h1>API Notes</h1>
<p>Mint supports <strong>bold</strong>, <code>inline code</code>, and tables.</p>
<table>
<thead>
<tr><th>Name</th><th>Status</th></tr>
</thead>
<tbody>
<tr><td>Parser</td><td>Ready</td></tr>
</tbody>
</table>
```

### Theme Configuration

- CLI default: loads `styles.css` from the executable directory.
- CLI override: pass `-s <path-to-css>` to embed a custom stylesheet.
- GUI themes (`gui/`): supports `theme` values `default`, `mint`, `paper`, `noir`.
- GUI accent override: `accentColor` accepts hex (`#RGB` or `#RRGGBB`).
- GUI layout override: `layoutWidth` supports `centered` and `wide`.

### Error Handling

- CLI is fail-fast: exits non-zero on file read/write, parse, or allocation failures.
- Missing default CSS is non-fatal; conversion continues without styles.
- GUI process wrapper enforces timeout (default `45_000 ms`, clamped to `5_000..300_000 ms`).
- GUI conversion result includes `stdout`, `stderr`, `code`, and `durationMs` for diagnostics.

### C Module API Summary

- `src/buf/buf.h`: dynamic buffer (`buf_init`, `buf_append`, `buf_puts`, `buf_escape`, `buf_free`).
- `src/io/io.h`: file I/O (`io_read_file`, `io_write_file`).
- `src/parser/parser.h`: block parse + title extraction (`parse_markdown`, `extract_title`).
- `src/parser/inline.h`: inline markdown parsing (`parse_inline`).
- `src/parser/parser_line.h`: line splitting and block helpers (`split_lines`, `is_rule`, `setext_level`, etc.).
- `src/parser/parser_html.h`: raw HTML block helpers (`parse_html_tag_line`, `contains_closing_html_tag`).
- `src/parser/inline_html.h`: inline HTML + URL sanitization helpers.
- `src/path/path.h`: path operations (`path_replace_ext`, `path_stem`, `path_exe_dir`).
- `src/html/html.h`: full HTML document assembly (`html_build`).

### Testing

Run parser regression tests:

```bat
tests\run-parser-tests.bat
```

```bash
sh tests/run-parser-tests.sh
```

Run GUI lint/tests:

```bash
cd gui
npm run lint
npm test
```

### Behavior Notes

- If `-o` is omitted, output is `<input-name>.html`
- If `-t` is omitted, title is taken from the first `# Heading`
- If no heading is found, filename stem is used as title
- If `-s` is omitted, `styles.css` is loaded from the executable directory
- If default CSS cannot be read, HTML is still generated without styles

### Project Files

- `main.c`: CLI entrypoint and compile orchestration
- `src/parser/parser.c`: Main block parser flow
- `src/parser/parser_line.c`: Line splitting + line helper utilities
- `src/parser/parser_html.c`: HTML block-line validation helpers
- `src/parser/inline.c`: Markdown inline token parser
- `src/parser/inline_html.c`: Inline raw HTML and URL sanitization helpers
- `build.bat`: Windows build helper
- `build-macos.sh`: macOS build helper
- `styles.css`: Default stylesheet loaded at runtime from the executable directory
- `dist/mintif.exe`: Built binary output (after running `build.bat`)
- `dist/styles.css`: Copied stylesheet output (after running `build.bat`)

### Scope and Limitations

This is a custom lightweight parser, not a full CommonMark implementation.<br>
Some advanced Markdown behaviors (deep nesting, edge-case compatibility, extended plugins) are intentionally out of scope.