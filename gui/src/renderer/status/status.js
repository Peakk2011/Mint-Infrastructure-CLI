import { STATUS_TONE, STATUS_MESSAGE_TTL_MS } from "../const.js";
import { state } from "../state.js";
import { clearStatusTimer, scheduleStatusClear } from "./timer.js";

let _render = () => {};

export const initStatus = (renderFn) => {
    _render = renderFn;
};

export const clearStatus = () => {
    clearStatusTimer();
    state.statusMessage = "";
    state.statusTone = STATUS_TONE.INFO;
    _render();
};

export const setStatus = (message, tone = STATUS_TONE.INFO) => {
    clearStatusTimer();
    state.statusMessage = message || "";
    state.statusTone = tone;
    _render();

    if (!state.statusMessage) {
        return;
    }

    scheduleStatusClear(() => {
        clearStatus();
    }, STATUS_MESSAGE_TTL_MS);
};

export const showError = (message) => {
    if (!message) {
        return;
    }

    setStatus(message, STATUS_TONE.ERROR);
    console.error(message);
    window.alert(message);
};