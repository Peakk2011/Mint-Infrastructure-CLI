import { MARKDOWN_EXTENSIONS } from "../const.js";

export const getFileExtension = (filePath) =>
    (filePath.match(/\.([^.\\/]+)$/)?.[1] || "").toLowerCase();

export const isSupportedMarkdownExtension = (extension) =>
    MARKDOWN_EXTENSIONS.includes(extension);