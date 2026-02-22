import assert from "node:assert/strict";
import process from "node:process";
import test from "node:test";

import { runConvertProcess } from "../dist/mintif-impl/convert-process.js";

const runWithCleanupFlag = async (args, timeoutMs = 2_000) => {
    let cleanupCalled = false;

    const result = await runConvertProcess({
        exePath: process.execPath,
        args,
        outputPath: "output.html",
        timeoutMs,
    
        cleanupGeneratedStyles: () => {
            cleanupCalled = true;
        },
    });

    return { result, cleanupCalled };
};

test("runConvertProcess returns ok=true when child exits 0", async () => {
    const { result, cleanupCalled } = await runWithCleanupFlag([
        "-e",
        "process.stdout.write('converted')",
    ]);

    assert.equal(cleanupCalled, true);
    assert.equal(result.ok, true);
    
    if (result.ok) {
        assert.equal(result.code, 0);
        assert.match(result.stdout, /converted/);
        assert.equal(result.outputPath, "output.html");
    }
});

test("runConvertProcess returns ok=false for non-zero child exit", async () => {
    const { result, cleanupCalled } = await runWithCleanupFlag([
        "-e",
        "process.stderr.write('broken'); process.exit(7);",
    ]);

    assert.equal(cleanupCalled, true);
    assert.equal(result.ok, false);
   
    if (!result.ok) {
        assert.equal(result.code, 7);
        assert.match(result.stderr ?? "", /broken/);
        assert.match(result.error, /exit code/i);
    }
});

test("runConvertProcess times out and returns a timeout error", async () => {
    const { result, cleanupCalled } = await runWithCleanupFlag(
        ["-e", "setTimeout(() => {}, 10_000);"],
        75
    );

    assert.equal(cleanupCalled, true);
    assert.equal(result.ok, false);
   
    if (!result.ok) {
        assert.match(result.error, /timed out/i);
        assert.equal(result.outputPath, "output.html");
    }
});