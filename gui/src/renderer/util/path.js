export const basename = (fullPath) => {
    if (!fullPath) {
        return "";
    }

    const segments = fullPath.split(/[\\/]/);
    return segments[segments.length - 1] || fullPath;
};

export const inferOutputPath = (inputPath) => {
    if (!inputPath) {
        return "";
    }

    if (/\.md$/i.test(inputPath)) {
        return inputPath.replace(/\.md$/i, ".html");
    }

    if (/\.markdown$/i.test(inputPath)) {
        return inputPath.replace(/\.markdown$/i, ".html");
    }

    return `${inputPath}.html`;
};

export const getParentDirectory = (filePath) => {
    if (!filePath) {
        return "";
    }

    const normalized = filePath.replace(/[\\/]+$/, "");
    const segments = normalized.split(/[\\/]/);

    if (segments.length <= 1) {
        return normalized;
    }

    segments.pop();

    const separator = normalized.includes("\\") ? "\\" : "/";
    let parentDirectory = segments.join(separator);

    if (/^[a-zA-Z]:$/.test(parentDirectory)) {
        parentDirectory += separator;
    }

    if (!parentDirectory) {
        parentDirectory = separator;
    }

    return parentDirectory;
};