let statusClearTimer = null;

export const clearStatusTimer = () => {
    if (statusClearTimer === null) {
        return;
    }

    window.clearTimeout(statusClearTimer);
    statusClearTimer = null;
};

export const scheduleStatusClear = (callback, delayMs) => {
    clearStatusTimer();
    statusClearTimer = window.setTimeout(callback, delayMs);
};