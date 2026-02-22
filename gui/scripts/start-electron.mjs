import { spawn } from "node:child_process";
import electronPath from "electron";

const buildElectronEnv = (baseEnv) => {
    const env = { ...baseEnv };
    
    // Some environments set this globally, which turns Electron into plain Node mode.
    delete env.ELECTRON_RUN_AS_NODE;
    return env;
};

const buildElectronArgs = (argv) => ["dist/main.js", ...argv.slice(2)];

const launchElectron = ({ argv, env }) =>
    spawn(electronPath, buildElectronArgs(argv), {
        stdio: "inherit",
        env: buildElectronEnv(env),
    });

const attachProcessHandlers = (child) => {
    child.on("exit", (code) => {
        process.exit(code ?? 0);
    });

    child.on("error", (error) => {
        console.error("Failed to launch Electron:", error);
        process.exit(1);
    });
};

const start = () => {
    const child = launchElectron({
        argv: process.argv,
        env: process.env,
    });
    
    attachProcessHandlers(child);
};

start();