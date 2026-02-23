import { showError } from "./status/status.js";

export const bridge = window.mintif;

export const ensureBridge = (missingMessage = "Preload bridge missing. Rebuild and restart app.") => {
    if (bridge) {
        return bridge;
    }

    showError(missingMessage);
    return null;
};