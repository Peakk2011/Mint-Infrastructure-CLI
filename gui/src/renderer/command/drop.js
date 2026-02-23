import { state } from "../state.js";
import { bridge } from "../bridge.js";
import { showError } from "../status/status.js";
import { handleCandidatePath } from "./pick.js";
import { ui } from "../ui.js";
import { chooseMarkdownFile } from "./pick.js";

export const onDropFiles = async (files) => {
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

export const bindDropEvents = () => {
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

export const bindWindowDnDGuards = () => {
    window.addEventListener("dragover", (event) => {
        event.preventDefault();
    });

    window.addEventListener("drop", (event) => {
        event.preventDefault();
    });
};