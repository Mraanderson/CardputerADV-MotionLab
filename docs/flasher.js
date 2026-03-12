import ESPLoader from "https://unpkg.com/esptool-js@0.5.4/bundle.js";

async function flash() {
    const log = msg => {
        document.getElementById("log").textContent += msg + "\n";
    };

    try {
        log("Loading manifest...");
        const manifest = await fetch("manifest.json").then(r => r.json());
        const build = manifest.builds[0];

        log("Requesting serial port...");
        const port = await navigator.serial.requestPort();

        const loader = new ESPLoader(port, { log });

        log("Connecting...");
        await loader.connect();

        log("Connected. Flashing parts...");

        for (const part of build.parts) {
            log(`Flashing ${part.path} @ 0x${part.offset.toString(16)}`);
            const data = await fetch(part.path).then(r => r.arrayBuffer());
            await loader.flashData(new Uint8Array(data), part.offset);
        }

        log("Done. Rebooting...");
        await loader.disconnect();
        log("Complete.");
    } catch (err) {
        log("ERROR: " + err);
    }
}

document.getElementById("flashBtn").addEventListener("click", flash);
