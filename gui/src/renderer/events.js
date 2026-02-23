import { ui } from "./ui.js";
import { render } from "./render.js";
import { runDownload, toIdle } from "./command/convert.js";
import { bindDropEvents, bindWindowDnDGuards } from "./command/drop.js";

const bindActionEvents = () => {
    ui.downloadBtn.addEventListener("click", () => runDownload(render));
    ui.cancelBtn.addEventListener("click", () => toIdle(render));
};

export const bindAllEvents = () => {
    bindDropEvents();
    bindActionEvents();
    bindWindowDnDGuards();
};