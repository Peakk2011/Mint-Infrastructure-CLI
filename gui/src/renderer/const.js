export const PHASE = Object.freeze({
    IDLE: "idle",
    CONFIRM: "confirm",
});

export const STATUS_TONE = Object.freeze({
    INFO: "info",
    SUCCESS: "success",
    ERROR: "error",
});

export const VIEW_TRANSITION = Object.freeze({
    durationOutMs: 170,
    durationInMs: 230,
    easeOut: "cubic-bezier(0.4, 0, 0.2, 1)",
    easeIn: "cubic-bezier(0.22, 1, 0.36, 1)",
});

export const MARKDOWN_EXTENSIONS = Object.freeze(["md", "markdown", "mdown", "mkd", "mkdn"]);

export const STATUS_MESSAGE_TTL_MS = 3000;