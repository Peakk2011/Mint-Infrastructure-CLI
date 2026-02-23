import { render } from "./render.js";
import { bindAllEvents } from "./events.js";
import { ensureBridge } from "./bridge.js";
import { initStatus, setStatus } from "./status/status.js";
import { STATUS_TONE } from "./const.js";

const init = () => {
    initStatus(render);
    bindAllEvents();
    render();
    setStatus("Choose or drop one markdown file to start.", STATUS_TONE.INFO);
    ensureBridge("Preload bridge not loaded. Run npm run build and restart.");
};

init();