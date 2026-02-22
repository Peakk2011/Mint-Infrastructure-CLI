import { runMintInfrastructureConvertComponent } from "./mintif-impl/convert.js";

export type MintInfrastructureConvertRequest = {
    inputPath: string;
    outputPath?: string;
    title?: string;
    stylesPath?: string;
    theme?: "default" | "mint" | "paper" | "noir";
    accentColor?: string;
    layoutWidth?: "centered" | "wide";
    timeoutMs?: number;
};

export type MintInfrastructureConvertResult =
    | {
        ok: true;
        code: number;
        outputPath: string;
        stdout: string;
        stderr: string;
        exePath: string;
        args: string[];
        durationMs: number;
    }
    | {
        ok: false;
        error: string;
        code?: number | null;
        outputPath?: string;
        stdout?: string;
        stderr?: string;
        exePath?: string;
        args?: string[];
        durationMs?: number;
    };

export const runMintInfrastructureConvert = async (
    request: MintInfrastructureConvertRequest
): Promise<MintInfrastructureConvertResult> => {
    return runMintInfrastructureConvertComponent(request);
};