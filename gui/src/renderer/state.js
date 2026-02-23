import { PHASE, STATUS_TONE } from "./const.js";

export const state = {
    phase: PHASE.IDLE,
    busy: false,
    inputPath: "",
    outputPath: "",
    statusMessage: "",
    statusTone: STATUS_TONE.INFO,
};