/**
 * Mint Infrastructure - Markdown Compiler
 * Written in C99.
 *
 * Compile:
 *   gcc main.c src/buf/buf.c src/io/io.c src/parser/inline.c \
 *       src/parser/parser.c src/path/path.c src/html/html.c \
 *       -o mintif.exe -std=c99 -O2 -I.
 *
 * Usage:
 *   ./mintif input.md -o output.html
 *   ./mintif input.md                   -> outputs input.html
 *   ./mintif input.md -t "My Title"
 *   ./mintif input.md -s custom.css
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "src/buf/buf.h"
#include "src/io/io.h"
#include "src/parser/parser.h"
#include "src/path/path.h"
#include "src/html/html.h"

static void print_help(void)
{
    puts(
        "\n"
        "  Mint Infrastructure - Markdown Compiler\n"
        "\n"
        "  Usage:\n"
        "    mintif <input.md> -o <output.html>\n"
        "    mintif <input.md>                     auto-names output\n"
        "    mintif <input.md> -t \"Custom Title\"\n"
        "    mintif <input.md> -s custom.css\n"
        "\n"
        "  Options:\n"
        "    -o, --output <file>   Output HTML file\n"
        "    -t, --title  <text>   Custom <title> tag\n"
        "    -s, --styles <file>   CSS file (default: styles.css next to exe)\n"
        "    -h, --help            Show this help\n");
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        print_help();
        return 0;
    }

    const char *input_file = NULL;
    const char *output_file = NULL;
    const char *custom_title = NULL;
    const char *styles_file = NULL;

    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help"))
        {
            print_help();
            return 0;
        }
        else if ((!strcmp(argv[i], "-o") || !strcmp(argv[i], "--output")) && i + 1 < argc)
        {
            output_file = argv[++i];
        }
        else if ((!strcmp(argv[i], "-t") || !strcmp(argv[i], "--title")) && i + 1 < argc)
        {
            custom_title = argv[++i];
        }
        else if ((!strcmp(argv[i], "-s") || !strcmp(argv[i], "--styles")) && i + 1 < argc)
        {
            styles_file = argv[++i];
        }
        else if (argv[i][0] != '-')
        {
            input_file = argv[i];
        }
    }

    if (!input_file)
    {
        fprintf(stderr, "  x  No input file.\n");
        return 1;
    }

    /* Resolve paths */
    char out_path[1024];
    if (output_file)
        strncpy(out_path, output_file, sizeof(out_path) - 1);
    else
        path_replace_ext(input_file, out_path, sizeof(out_path));

    char css_path[1024];
    if (styles_file)
    {
        strncpy(css_path, styles_file, sizeof(css_path) - 1);
    }
    else
    {
        char dir[1024];
        path_exe_dir(argv[0], dir, sizeof(dir));
        strncpy(css_path, dir, 1012);
        css_path[1012] = '\0';
        strcat(css_path, "/styles.css");
    }

    /* Read markdown */
    size_t md_len;
    char *markdown = io_read_file(input_file, &md_len);

    if (!markdown)
    {
        fprintf(stderr, "  x  Cannot open: %s\n", input_file);
        return 1;
    }

    /* Read CSS */
    char *css = io_read_file(css_path, NULL);
    if (!css)
    {
        fprintf(stderr, "  !  No styles.css at %s — no styles applied.\n", css_path);
        css = (char *)calloc(1, 1);
    }

    /* Title */
    char title[512], fallback[256];
    path_stem(input_file, fallback, sizeof(fallback));
    
    if (custom_title)
        strncpy(title, custom_title, sizeof(title) - 1);
    else
        extract_title(markdown, title, sizeof(title), fallback);

    /* Compile */
    char *body = parse_markdown(markdown, md_len);
    char *html = html_build(title, css, body);

    /* Write */
    if (!io_write_file(out_path, html, strlen(html)))
    {
        fprintf(stderr, "  x  Cannot write: %s\n", out_path);
        free(markdown);
        free(css);
        free(body);
        free(html);
        return 1;
    }

    /* Print result */
    const char *in_name = strrchr(input_file, '/');
    const char *out_name = strrchr(out_path, '/');
#ifdef _WIN32
    const char *in2 = strrchr(input_file, '\\');
    const char *out2 = strrchr(out_path, '\\');
    if (!in_name || (in2 && in2 > in_name))
        in_name = in2;
    if (!out_name || (out2 && out2 > out_name))
        out_name = out2;
#endif
    printf("  ok  %s -> %s\n",
           in_name ? in_name + 1 : input_file,
           out_name ? out_name + 1 : out_path);

    free(markdown);
    free(css);
    free(body);
    free(html);
    return 0;
}