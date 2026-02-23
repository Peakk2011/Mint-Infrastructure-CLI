import { state } from "../state.js";
import { ensureBridge } from "../bridge.js";
import { showError, setStatus } from "../status/status.js";
import { ui } from "../ui.js";
import { PHASE, STATUS_TONE } from "../const.js";
import { basename, getParentDirectory } from "../util/path.js";

export const setBusy = (busy) => {
    state.busy = busy;
    ui.app.classList.toggle("busy", busy);
};

export const toIdle = (render) => {
    state.phase = PHASE.IDLE;
    state.inputPath = "";
    state.outputPath = "";
    render();
};

export const runDownload = async (render) => {
    if (!state.inputPath || state.busy) {
        return;
    }

    const api = ensureBridge();
    if (!api) {
        return;
    }

    setBusy(true);
    setStatus("Converting markdown to HTML...", STATUS_TONE.INFO);

    try {
        const result = await api.convertMarkdown({
            inputPath: state.inputPath,
            outputPath: state.outputPath,
        });

        if (!result.ok) {
            showError(result.error || "Unknown conversion error.");
            return;
        }

        const outputName = basename(result.outputPath);
        const parentDirectory = getParentDirectory(result.outputPath) || result.outputPath;
        const openResult = await api.openPath(parentDirectory);

        if (!openResult.ok) {
            showError(`Converted ${outputName}, but failed to open folder: ${openResult.error}`);
            return;
        }

        setStatus(`Converted successfully: ${outputName}`, STATUS_TONE.SUCCESS);
        toIdle(render);
    } catch (error) {
        showError(`Unexpected error: ${String(error)}`);
    } finally {
        setBusy(false);
        render();
    }
};