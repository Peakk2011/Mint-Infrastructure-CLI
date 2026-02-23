const mustGetElement = (id) => {
    const element = document.getElementById(id);
    if (!element) {
        throw new Error(`Missing required element: #${id}`);
    }
    return element;
};

export const ui = {
    app: mustGetElement("app"),
    dropZone: mustGetElement("dropZone"),
    confirmCard: mustGetElement("confirmCard"),
    confirmFileName: mustGetElement("confirmFileName"),
    downloadBtn: mustGetElement("downloadBtn"),
    cancelBtn: mustGetElement("cancelBtn"),
    statusMessage: mustGetElement("statusMessage"),
};