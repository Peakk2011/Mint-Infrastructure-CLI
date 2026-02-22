import type { MintInfrastructureConvertRequest, MintInfrastructureConvertResult } from "./mint-infrastructure.ts";

type MintInfrastructureInspectPathResult =
    | {
        ok: true;
        path: string;
        exists: boolean;
        isFile: boolean;
        isDirectory: boolean;
    }
    | {
        ok: false;
        error: string;
    };

type MintInfrastructureOpenPathResult =
    | {
        ok: true;
    }
    | {
        ok: false;
        error: string;
    };

declare global {
    interface Window {
        mintif: {
            convertMarkdown: (request: MintInfrastructureConvertRequest) => Promise<MintInfrastructureConvertResult>;
            pickMarkdownFile: () => Promise<string | null>;
            pickOutputFile: (suggestedName?: string) => Promise<string | null>;
            inspectPath: (candidatePath: string) => Promise<MintInfrastructureInspectPathResult>;
            openPath: (targetPath: string) => Promise<MintInfrastructureOpenPathResult>;
            getPathForFile: (file: File) => string | null;
        };
    }
}

export {};