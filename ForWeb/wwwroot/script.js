let globalWasmMemoryArray;
// let globalWasmVolumeLevelArray;
let globalWasmImageSharedUint8ClampedArray;
let globalWasmImageUnsharedUint8ClampedArray;
let globalWasmImage;

async function createEmulatorAudioWorkletNode(audioContext, onReady) 
{
	let compiledWasmModule = 
		await WebAssembly
			.compileStreaming( fetch('jynx-emulator.wasm') )
			.then( mod => { return mod; } );
		
	let audioWorkletNode = 
		new AudioWorkletNode(
			audioContext, 
			"jynx-emulator-worker", 
			{
				processorOptions: 
				{ 
					compiledWasmModule: compiledWasmModule  /* NB: Instantiation of the WASM VM is on the worker, not the main thread. */ 
				} 
			});

	audioWorkletNode.port.onmessage = 
		(e) => {
			if (e.isTrusted)
			{
				let postedDataForHost = e.data;
				let wasmMemoryArray   = postedDataForHost.memory.buffer;

				let wasmReadyDetails = { 
					wasmMemoryArray:       wasmMemoryArray,
					wasmVolumeLevelArray:  new Float32Array(wasmMemoryArray, postedDataForHost.volumeLevelAddress, 4),
					wasmImageArray:        new Uint8ClampedArray(wasmMemoryArray, postedDataForHost.screenBaseAddress, 16 * 16 * 4)
				};

				onReady(wasmReadyDetails);
			}
		};

	await audioContext.resume();
	
	return audioWorkletNode;
}

async function audioDemoStart(audioContext, onReady) {
	
	let emulatorAudioWorkletNode = await createEmulatorAudioWorkletNode(audioContext, onReady);
	
	const squareOscillatorNode = new OscillatorNode(audioContext);
	
	// Configure the oscillator node
	
	squareOscillatorNode.type = "square";
	squareOscillatorNode.frequency.setValueAtTime(440, audioContext.currentTime); // (A4)  // TODO: How to live without this(!).  Check if, without this, the sound sustains on switch-away too.
	
	// Connect and start
	
	squareOscillatorNode.connect(emulatorAudioWorkletNode).connect(audioContext.destination);
	squareOscillatorNode.start();
}

// ------------------------------------------------------------------------------------------------------------

let globalAudioContext = null;

function ensureGlobalAudioContextCreated() {
	if (!globalAudioContext) {
		try {
			globalAudioContext = new AudioContext();
		} catch(e) {
			throw "** Error: Unable to create audio context **"; // TODO sort out
		}
	}
}

async function ensureGlobalAudioContextDisposed() {
	if (globalAudioContext) {
		await globalAudioContext.close();
		globalAudioContext = null;
	}
}

// ------------------------------------------------------------------------------------------------------------

window.addEventListener("load", event => {
	document.getElementById("startEmulator").addEventListener("click", startEmulator);
	// document.getElementById("toggleVolume").addEventListener("click", toggleVolumeLevel);
	document.getElementById("updateGraphics").addEventListener("click", updateGraphics);
});

function HideStartEmulatorButton()
{
	document.getElementById("startEmulator").style.display = 'none';
}

async function startEmulator(event) 
{
	if (!globalAudioContext) 
	{
		ensureGlobalAudioContextCreated();
		await globalAudioContext.audioWorklet.addModule("jynx-emulator-worker.js");
		
		audioDemoStart(
			globalAudioContext, 
			wasmReadyDetails => 
				{
					globalWasmMemoryArray                    = wasmReadyDetails.wasmMemoryArray;
					// globalWasmVolumeLevelArray               = wasmReadyDetails.wasmVolumeLevelArray;
					globalWasmImageSharedUint8ClampedArray   = wasmReadyDetails.wasmImageArray;
					globalWasmImageUnsharedUint8ClampedArray = new Uint8ClampedArray(16 * 16 * 4);  // an unfortunate duplicate, but we allocate ONCE at least!
					globalWasmImage                          = new ImageData(globalWasmImageUnsharedUint8ClampedArray, 16, 16); // TODO: check out the format parameter
				});
				
		HideStartEmulatorButton();
	}
	/* Let's never stop.  else
	{
		await ensureGlobalAudioContextDisposed();
	} */
}

/*
async function toggleVolumeLevel(event) {
	
	if (globalWasmVolumeLevelArray)
	{
		// Modify the volume level by directly altering the volume variable
		// in the WASM memory space.
		
		// ECMA 262 : 29.11 Shared Memory Guidelines
		// "... if memory is treated as strongly typed the racing accesses will not "tear" (bits of their values will not be mixed)."
		
		let currentLevel = globalWasmVolumeLevelArray[0];
		if (currentLevel < 0.2)
		{
			currentLevel = 0.3;
		}
		else
		{
			currentLevel = 0.03;
		}
		globalWasmVolumeLevelArray[0] = currentLevel;
	}
}
*/

function updateGraphics(event) {

	if (globalWasmImageSharedUint8ClampedArray)
	{
		globalWasmImageUnsharedUint8ClampedArray.set(globalWasmImageSharedUint8ClampedArray); // Unfortunate copy.
		
		const canvas = document.getElementById('canvas');
		const ctx = canvas.getContext('2d');
		ctx.putImageData(globalWasmImage, 0, 0);  // TODO: investigate portion-painting
	}
}
