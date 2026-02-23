import { PHASE, VIEW_TRANSITION } from "../const.js";
import { state } from "../state.js";
import { ui } from "../ui.js";

let renderedPhase = null;
let transitionTicket = 0;

const getViewElement = (phase) =>
    phase === PHASE.IDLE ? ui.dropZone : ui.confirmCard;

const resetViewInlineStyles = (element) => {
    element.style.opacity = "";
};

const applyPhaseImmediately = (phase) => {
    const idle = phase === PHASE.IDLE;
    ui.dropZone.classList.toggle("hidden", !idle);
    ui.confirmCard.classList.toggle("hidden", idle);
    resetViewInlineStyles(ui.dropZone);
    resetViewInlineStyles(ui.confirmCard);
};

const animatePhaseTransition = async (fromPhase, toPhase) => {
    if (fromPhase === toPhase) {
        return;
    }

    const fromView = getViewElement(fromPhase);
    const toView = getViewElement(toPhase);
    const currentTicket = ++transitionTicket;

    toView.classList.remove("hidden");

    const exitAnimation = fromView.animate(
        [{ opacity: 1 }, { opacity: 0 }],
        {
            duration: VIEW_TRANSITION.durationOutMs,
            easing: VIEW_TRANSITION.easeOut,
            fill: "forwards",
        }
    );

    const enterAnimation = toView.animate(
        [{ opacity: 0 }, { opacity: 1 }],
        {
            duration: VIEW_TRANSITION.durationInMs,
            easing: VIEW_TRANSITION.easeIn,
            fill: "forwards",
        }
    );

    await Promise.allSettled([exitAnimation.finished, enterAnimation.finished]);

    if (currentTicket !== transitionTicket) {
        return;
    }

    fromView.classList.add("hidden");
    resetViewInlineStyles(fromView);
    resetViewInlineStyles(toView);
};

export const syncPhaseView = () => {
    if (renderedPhase === null) {
        applyPhaseImmediately(state.phase);
        renderedPhase = state.phase;
        return;
    }

    if (renderedPhase === state.phase) {
        return;
    }

    const previousPhase = renderedPhase;
    renderedPhase = state.phase;
    void animatePhaseTransition(previousPhase, state.phase);
};