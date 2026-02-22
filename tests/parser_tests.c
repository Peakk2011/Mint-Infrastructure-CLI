#include "../src/parser/parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures = 0;

static void expect_contains(const char *test_name, const char *haystack, const char *needle) {
    if (strstr(haystack, needle) != NULL) {
        return;
    }

    fprintf(stderr, "[FAIL] %s\n  expected to find: %s\n  actual: %s\n", test_name, needle, haystack);
    failures++;
}

static void expect_not_contains(const char *test_name, const char *haystack, const char *needle) {
    if (strstr(haystack, needle) == NULL) {
        return;
    }

    fprintf(stderr, "[FAIL] %s\n  expected to NOT find: %s\n  actual: %s\n", test_name, needle, haystack);
    failures++;
}

static void run_case(
    const char *test_name,
    const char *markdown,
    const char *must_contain,
    const char *must_not_contain
) {
    char *out = parse_markdown(markdown, strlen(markdown));
    if (!out) {
        fprintf(stderr, "[FAIL] %s\n  parse_markdown returned NULL\n", test_name);
        failures++;
        return;
    }

    if (must_contain) {
        expect_contains(test_name, out, must_contain);
    }
    if (must_not_contain) {
        expect_not_contains(test_name, out, must_not_contain);
    }

    free(out);
}

int main(void) {
    run_case(
        "atx heading parses as h1",
        "# Mint\n",
        "<h1>Mint</h1>",
        NULL
    );

    run_case(
        "horizontal rule accepts tab separators",
        "-\t-\t-\n",
        "<hr>",
        NULL
    );

    run_case(
        "setext h2 requires at least two '-' markers",
        "Title\n-\n",
        "<p>Title -</p>",
        "<h2>Title</h2>"
    );

    run_case(
        "setext h2 still works for two '-' markers",
        "Title\n--\n",
        "<h2>Title</h2>",
        NULL
    );

    if (failures > 0) {
        fprintf(stderr, "\nparser tests: %d failure(s)\n", failures);
        return 1;
    }

    puts("parser tests: all passed");
    return 0;
}