import js from "@eslint/js";
import tseslint from "typescript-eslint";

export default [
    {
        ignores: [
            "dist/**",
            "vendor/**",
            "bindings/**",
            "assets/**",
        ],
    },
    js.configs.recommended,
    ...tseslint.configs.recommended,
    {
        files: ["src/**/*.{ts,cts}"],
        rules: {
            "@typescript-eslint/consistent-type-imports": [
                "error",
                { prefer: "type-imports", fixStyle: "inline-type-imports" },
            ],
        },
    },
];