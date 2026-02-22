import * as path from "node:path";

export const normalizeCandidatePath = (candidatePath?: string): string | null => {
    const raw = candidatePath?.trim();
    if (!raw) {
        return null;
    }

    return path.resolve(raw);
};