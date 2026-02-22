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
            "@typescript-eslint/no-unused-vars": [
                "error",
                {
                    argsIgnorePattern: "^_",
                    varsIgnorePattern: "^_",
                },
            ],
        },
    },
    {
        files: ["src/darling-runtime/**/*.ts"],
        rules: {
            "@typescript-eslint/no-explicit-any": "off",
        },
    },
    {
        files: ["src/preload.cts"],
        rules: {
            "@typescript-eslint/no-require-imports": "off",
            "@typescript-eslint/consistent-type-imports": "off",
        },
    },
];
