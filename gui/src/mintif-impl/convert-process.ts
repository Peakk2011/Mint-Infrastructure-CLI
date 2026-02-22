import { spawn } from "node:child_process";
import type { MintInfrastructureConvertResult } from "../mint-infrastructure.js";

type RunConvertProcessArgs = {
    exePath: string;
    args: string[];
    outputPath: string;
    timeoutMs: number;
    cleanupGeneratedStyles: () => void;
};

export const runConvertProcess = async ({
    exePath,
    args,
    outputPath,
    timeoutMs,
    cleanupGeneratedStyles,
}: RunConvertProcessArgs): Promise<MintInfrastructureConvertResult> => {
    const startedAt = Date.now();

    return new Promise<MintInfrastructureConvertResult>((resolve) => {
        const child = spawn(exePath, args, {
            shell: false,
            windowsHide: true,
            stdio: ["ignore", "pipe", "pipe"],
        });

        let stdout = "";
        let stderr = "";
        let timedOut = false;

        child.stdout.setEncoding("utf8");
        child.stderr.setEncoding("utf8");

        child.stdout.on("data", (chunk: string) => {
            stdout += chunk;
        });
        
        child.stderr.on("data", (chunk: string) => {
            stderr += chunk;
        });

        const timeout = setTimeout(() => {
            timedOut = true;
            child.kill();
        }, timeoutMs);

        child.on("error", (error) => {
            clearTimeout(timeout);
            cleanupGeneratedStyles();

            resolve({
                ok: false,
                error: `Failed to start mintif.exe: ${error.message}`,
                outputPath,
                stdout,
                stderr,
                exePath,
                args,
                durationMs: Date.now() - startedAt,
            });
        });

        child.on("close", (code) => {
            clearTimeout(timeout);
            cleanupGeneratedStyles();
            const durationMs = Date.now() - startedAt;

            if (timedOut) {
                resolve({
                    ok: false,
                    error: `mintif.exe timed out after ${timeoutMs} ms.`,
                    code,
                    outputPath,
                    stdout,
                    stderr,
                    exePath,
                    args,
                    durationMs,
                });
                return;
            }

            if (code !== 0) {
                resolve({
                    ok: false,
                    error: `mintif.exe failed with exit code ${code}.`,
                    code,
                    outputPath,
                    stdout,
                    stderr,
                    exePath,
                    args,
                    durationMs,
                });
                return;
            }

            resolve({
                ok: true,
                code,
                outputPath,
                stdout,
                stderr,
                exePath,
                args,
                durationMs,
            });
        });
    });
};