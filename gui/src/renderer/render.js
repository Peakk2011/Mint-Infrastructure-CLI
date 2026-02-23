import { PHASE, STATUS_TONE } from "./const.js";
import { state } from "./state.js";
import { ui } from "./ui.js";
import { syncPhaseView } from "./anim/transition.js";
import { basename } from "./util/path.js";

export const render = () => {
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