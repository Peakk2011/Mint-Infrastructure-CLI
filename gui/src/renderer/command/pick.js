import { state } from "../state.js";
import { ensureBridge } from "../bridge.js";
import { showError } from "../status/status.js";
import { getFileExtension, isSupportedMarkdownExtension } from "../util/file.js";
import { MARKDOWN_EXTENSIONS, PHASE, STATUS_TONE } from "../const.js";
import { inferOutputPath, basename } from "../util/path.js";
import { setStatus } from "../status/status.js";

const setPendingFile = (inputPath) => {
    state.phase = PHASE.CONFIRM;
    state.inputPath = inputPath;
    state.outputPath = inferOutputPath(inputPath);
    setStatus(`Ready to convert: ${basename(inputPath)}`, STATUS_TONE.INFO);
};

export const handleCandidatePath = async (candidatePath) => {
    const api = ensureBridge();
    if (!api) {
        return;
    }

    const extension = getFileExtension(candidatePath);
    if (!isSupportedMarkdownExtension(extension)) {
        const supportedFormats = MARKDOWN_EXTENSIONS.map((ext) => `.${ext}`).join(", ");
        showError(`Only markdown files are accepted (${supportedFormats}).`);
        return;
    }

    const inspected = await api.inspectPath(candidatePath);
    if (!inspected.ok) {
        showError(`Path check failed: ${inspected.error}`);
        return;
    }

    if (!inspected.exists) {
        showError("Dropped file was not found on disk.");
        return;
    }

    if (inspected.isDirectory || !inspected.isFile) {
        showError("Drop one markdown file only.");
        return;
    }

    setPendingFile(inspected.path);
};

export const chooseMarkdownFile = async () => {
    const api = ensureBridge();
    if (!api || state.busy) {
        return;
    }

    const pickedPath = await api.pickMarkdownFile();
    if (!pickedPath) {
        return;
    }

    await handleCandidatePath(pickedPath);
};