export const electronRuntime = (): void => {
    if (process.versions.electron) {
        return;
    }

    console.error("This app must be started with Electron. Use: npm run start");
    process.exit(1);
};