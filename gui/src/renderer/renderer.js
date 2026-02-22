const PHASE = Object.freeze({
    IDLE: "idle",
    CONFIRM: "confirm",
});

const STATUS_TONE = Object.freeze({
    INFO: "info",
    SUCCESS: "success",
    ERROR: "error",
});

const STATUS_MESSAGE_TTL_MS = 3000;

const MARKDOWN_EXTENSIONS = Object.freeze(["md", "markdown", "mdown", "mkd", "mkdn"]);

const state = {
    phase: PHASE.IDLE,
    busy: false,
    inputPath: "",
    outputPath: "",
    statusMessage: "",
    statusTone: STATUS_TONE.INFO,
};

const bridge = window.mintif;

const mustGetElement = (id) => {
    const element = document.getElementById(id);
    if (!element) {
        throw new Error(`Missing required element: #${id}`);
    }
    return element;
};

const ui = {
    app: mustGetElement("app"),
    dropZone: mustGetElement("dropZone"),
    confirmCard: mustGetElement("confirmCard"),
    confirmFileName: mustGetElement("confirmFileName"),
    downloadBtn: mustGetElement("downloadBtn"),
    cancelBtn: mustGetElement("cancelBtn"),
    statusMessage: mustGetElement("statusMessage"),
};

const VIEW_TRANSITION = Object.freeze({
    durationOutMs: 170,
    durationInMs: 230,
    easeOut: "cubic-bezier(0.4, 0, 0.2, 1)",
    easeIn: "cubic-bezier(0.22, 1, 0.36, 1)",
});

let renderedPhase = null;
let transitionTicket = 0;
let statusClearTimer = null;

const basename = (fullPath) => {
    if (!fullPath) {
        return "";
    }

    const segments = fullPath.split(/[\\/]/);
    return segments[segments.length - 1] || fullPath;
};

const inferOutputPath = (inputPath) => {
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

const getParentDirectory = (filePath) => {
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

const getFileExtension = (filePath) => (filePath.match(/\.([^.\\/]+)$/)?.[1] || "").toLowerCase();

const isSupportedMarkdownExtension = (extension) => MARKDOWN_EXTENSIONS.includes(extension);

const clearStatusTimer = () => {
    if (statusClearTimer === null) {
        return;
    }

    window.clearTimeout(statusClearTimer);
    statusClearTimer = null;
};

const clearStatus = () => {
    clearStatusTimer();
    state.statusMessage = "";
    state.statusTone = STATUS_TONE.INFO;
    render();
};

const setStatus = (message, tone = STATUS_TONE.INFO) => {
    clearStatusTimer();
    state.statusMessage = message || "";
    state.statusTone = tone;
    render();

    if (!state.statusMessage) {
        return;
    }

    statusClearTimer = window.setTimeout(() => {
        clearStatus();
    }, STATUS_MESSAGE_TTL_MS);
};

const showError = (message) => {
    if (!message) {
        return;
    }

    setStatus(message, STATUS_TONE.ERROR);
    console.error(message);
    window.alert(message);
};

const setBusy = (busy) => {
    state.busy = busy;
    ui.app.classList.toggle("busy", busy);
};

const getViewElement = (phase) => (phase === PHASE.IDLE ? ui.dropZone : ui.confirmCard);

const resetViewInlineStyles = (element) => {
    element.style.opacity = "";
};

const applyPhaseImmediately = (phase) => {
    const idle = phase === PHASE.IDLE;
    ui.dropZone.classList.toggle("hidden", !idle);
    ui.confirmCard.classList.toggle("hidden", idle);
    resetViewInlineStyles(ui.dropZone);
    resetViewInlineStyles(ui.confirmCard);
};

const animatePhaseTransition = async (fromPhase, toPhase) => {
    if (fromPhase === toPhase) {
        return;
    }

    const fromView = getViewElement(fromPhase);
    const toView = getViewElement(toPhase);
    const currentTicket = ++transitionTicket;

    toView.classList.remove("hidden");

    const exitAnimation = fromView.animate(
        [
            { opacity: 1 },
            { opacity: 0 },
        ],
        {
            duration: VIEW_TRANSITION.durationOutMs,
            easing: VIEW_TRANSITION.easeOut,
            fill: "forwards",
        }
    );

    const enterAnimation = toView.animate(
        [
            { opacity: 0 },
            { opacity: 1 },
        ],
        {
            duration: VIEW_TRANSITION.durationInMs,
            easing: VIEW_TRANSITION.easeIn,
            fill: "forwards",
        }
    );

    await Promise.allSettled([exitAnimation.finished, enterAnimation.finished]);

    if (currentTicket !== transitionTicket) {
        return;
    }

    fromView.classList.add("hidden");
    resetViewInlineStyles(fromView);
    resetViewInlineStyles(toView);
};

const syncPhaseView = () => {
    if (renderedPhase === null) {
        applyPhaseImmediately(state.phase);
        renderedPhase = state.phase;
        return;
    }

    if (renderedPhase === state.phase) {
        return;
    }

    const previousPhase = renderedPhase;
    renderedPhase = state.phase;
    void animatePhaseTransition(previousPhase, state.phase);
};

const render = () => {
    syncPhaseView();

    const idle = state.phase === PHASE.IDLE;
    ui.confirmFileName.textContent = basename(state.inputPath);
    ui.statusMessage.textContent = state.statusMessage;
    ui.statusMessage.classList.toggle("error", state.statusTone === STATUS_TONE.ERROR);
    ui.statusMessage.classList.toggle("success", state.statusTone === STATUS_TONE.SUCCESS);

    ui.dropZone.disabled = state.busy;
    ui.downloadBtn.disabled = idle || state.busy;
    ui.cancelBtn.disabled = idle || state.busy;
};

const ensureBridge = (missingMessage = "Preload bridge missing. Rebuild and restart app.") => {
    if (bridge) {
        return bridge;
    }

    showError(missingMessage);
    return null;
};

const toIdle = () => {
    state.phase = PHASE.IDLE;
    state.inputPath = "";
    state.outputPath = "";

    render();
};

const setPendingFile = (inputPath) => {
    state.phase = PHASE.CONFIRM;
    state.inputPath = inputPath;
    state.outputPath = inferOutputPath(inputPath);
    setStatus(`Ready to convert: ${basename(inputPath)}`, STATUS_TONE.INFO);
};

const handleCandidatePath = async (candidatePath) => {
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

const chooseMarkdownFile = async () => {
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

const onDropFiles = async (files) => {
    if (state.busy) {
        showError("Please wait for current process to finish.");
        return;
    }

    if (!files || files.length === 0) {
        return;
    }

    if (files.length > 1) {
        showError("Only one file is supported.");
        return;
    }

    const [file] = files;
    const candidatePath = (bridge && bridge.getPathForFile(file)) || file.path || "";
    if (!candidatePath) {
        showError("Could not read dropped file path.");
        return;
    }

    await handleCandidatePath(candidatePath);
};

const runDownload = async () => {
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
        toIdle();
    } catch (error) {
        showError(`Unexpected error: ${String(error)}`);
    } finally {
        setBusy(false);
        render();
    }
};

const bindDropEvents = () => {
    ui.dropZone.addEventListener("click", chooseMarkdownFile);

    ui.dropZone.addEventListener("keydown", async (event) => {
        if (event.key !== "Enter" && event.key !== " ") {
            return;
        }

        event.preventDefault();
        await chooseMarkdownFile();
    });

    ui.dropZone.addEventListener("dragover", (event) => {
        event.preventDefault();
        ui.dropZone.classList.add("drag");
    });

    ui.dropZone.addEventListener("dragleave", () => {
        ui.dropZone.classList.remove("drag");
    });

    ui.dropZone.addEventListener("drop", async (event) => {
        event.preventDefault();
        ui.dropZone.classList.remove("drag");
        await onDropFiles(event.dataTransfer && event.dataTransfer.files);
    });
};

const bindActionEvents = () => {
    ui.downloadBtn.addEventListener("click", runDownload);
    ui.cancelBtn.addEventListener("click", toIdle);
};

const bindWindowDnDGuards = () => {
    window.addEventListener("dragover", (event) => {
        event.preventDefault();
    });

    window.addEventListener("drop", (event) => {
        event.preventDefault();
    });
};

const init = () => {
    bindDropEvents();
    bindActionEvents();
    bindWindowDnDGuards();

    render();
    setStatus("Choose or drop one markdown file to start.", STATUS_TONE.INFO);
    ensureBridge("Preload bridge not loaded. Run npm run build and restart.");
};

init();
