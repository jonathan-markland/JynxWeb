//
// Jynx - Camputers Lynx Emulator
//
// Main bootstrapping and browser interfacing script.
//


	// TODO: How do we say the animation frame applies to the canvas element content only (optimisation) ?
	

// ------------------------------------------------------------------------------------------------------------
//   CONSTANTS
// ------------------------------------------------------------------------------------------------------------

const GUEST_SCREEN_WIDTH  = 256;   // If changed, must also change the C++ #define of the same name.
const GUEST_SCREEN_HEIGHT = 256;   // If changed, must also change the C++ #define of the same name.
const INV_ROWS            = 32;    // The number of entries in globalWasmRowDirtyCountersArray

// ------------------------------------------------------------------------------------------------------------
//   GLOBAL STATE
// ------------------------------------------------------------------------------------------------------------

let globalWasmMemoryArray;
let globalWasmImageSharedUint8ClampedArray;
let globalWasmImageUnsharedUint8ClampedArray;
let globalWasmImage;
let globalWasmKeyTranslationTableSize;
let globalWasmKeyTranslationTableArray;
let globalWasmKeyboardPortsArray;
let globalWasmRowDirtyCountersArray;


// ------------------------------------------------------------------------------------------------------------
//   BOOTSTRAP
// ------------------------------------------------------------------------------------------------------------

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
		(e) => 
		{
			if (e.isTrusted)
			{
				let postedDataForHost = e.data;
				let wasmMemoryArray   = postedDataForHost.memory.buffer;

				let onReadyDetails = { 
					wasmMemoryArray                : wasmMemoryArray,
					wasmImageArray                 : new Uint8ClampedArray(wasmMemoryArray, postedDataForHost.screenBaseAddress, GUEST_SCREEN_WIDTH * GUEST_SCREEN_HEIGHT * 4),
					wasmRowDirtyCountersArray      : new Uint8ClampedArray(wasmMemoryArray, postedDataForHost.screenRowDirtyCountersAddress, INV_ROWS),
					wasmKeyTranslationTableSize    : postedDataForHost.keyTranslationTableSize,
					wasmKeyTranslationTableArray   : new Uint8ClampedArray(wasmMemoryArray, postedDataForHost.keyTranslationTableAddress, postedDataForHost.keyTranslationTableSize),
					wasmKeyboardPortsArray         : new Uint8ClampedArray(wasmMemoryArray, postedDataForHost.keyboardPortsArray, 10) // TODO: constant is number of bytes in the Lynx's keyboard ports array.
				};

				onReady(onReadyDetails);
			}
		};

	await audioContext.resume();
	
	return audioWorkletNode;
}



async function startEmulatorAndDo(audioContext, onReady) 
{
	let emulatorAudioWorkletNode = 
		await createEmulatorAudioWorkletNode(audioContext, onReady);
	
	// TODO: I REALLY need to find out how to get sound output without this mixing in of a silent square wave(!).
	//       Also I need to check whether, without this, the sound sustains on switch-away.
	const squareOscillatorNode = new OscillatorNode(audioContext);
	squareOscillatorNode.type = "square";
	squareOscillatorNode.frequency.setValueAtTime(440, audioContext.currentTime); // (A4)  
	
	// Connect and start
	squareOscillatorNode.connect(emulatorAudioWorkletNode).connect(audioContext.destination);
	squareOscillatorNode.start();
}



// ------------------------------------------------------------------------------------------------------------
//   GLOBAL AUDIO CONTEXT OBJECT MANAGEMENT
// ------------------------------------------------------------------------------------------------------------

let globalAudioContext = null;

function ensureGlobalAudioContextCreated() 
{
	if (!globalAudioContext) 
	{
		try 
		{
			globalAudioContext = new AudioContext();
		}
		catch(e) 
		{
			throw "** Error: Unable to create audio context **"; // TODO sort out
		}
	}
}



/* Kept for future reference

async function ensureGlobalAudioContextDisposed() 
{
	if (globalAudioContext) 
	{
		await globalAudioContext.close();
		globalAudioContext = null;
	}
} */



// ------------------------------------------------------------------------------------------------------------
//   WINDOW INIT
// ------------------------------------------------------------------------------------------------------------

window.addEventListener(
	"load", 
	event => 
		{
			document.getElementById("startEmulator").addEventListener("click", onStartEmulator);
			document.addEventListener('keydown', onKeyDown);
			document.addEventListener('keyup', onKeyUp);
		});
		
		

// ------------------------------------------------------------------------------------------------------------
//   UI EVENT HANDLERS
// ------------------------------------------------------------------------------------------------------------


var g_MostRecentUpdateCounts = new Uint8Array(INV_ROWS);

function onPollForGraphicsUpdate() {
	
	// Called by window.requestAnimationFrame()

	if (globalWasmImageSharedUint8ClampedArray && globalWasmRowDirtyCountersArray) // TODO: strictly, no longer needed.
	{
		// TODO: repaint only the strips in question!
		
		var repaintNeeded = false;
		
		for (i=0; i<INV_ROWS; i++)
		{
			var guestValue = globalWasmRowDirtyCountersArray[i];  // Read ONCE (volatile)
			if (guestValue != g_MostRecentUpdateCounts[i])
			{
				repaintNeeded = true;
				g_MostRecentUpdateCounts[i] = guestValue;  // so next time we can do our change-detection.
			}
		}

		if (repaintNeeded)
		{
			globalWasmImageUnsharedUint8ClampedArray.set(globalWasmImageSharedUint8ClampedArray); // TODO: Design issue:  Unfortunately we must copy.  Can Web Canvas not directly accept a portion of the WASM memory space as the image source?
			
			const canvas = document.getElementById('canvas');
			const ctx = canvas.getContext('2d');
			ctx.putImageData(globalWasmImage, 0, 0);  // TODO: investigate portion-painting for the "update bands" that Jynx uses.
		}
	}
	
	// Keep the polling going always:
	
	window.requestAnimationFrame(onPollForGraphicsUpdate);
}



function HideStartEmulatorButton()
{
	document.getElementById("startEmulator").style.display = 'none';
}



async function onStartEmulator(event) 
{
	if (!globalAudioContext) 
	{
		ensureGlobalAudioContextCreated();
		
		await globalAudioContext.audioWorklet.addModule("jynx-emulator-worker.js");
		
		startEmulatorAndDo(
			globalAudioContext, 
			onReadyDetails => 
				{
					globalWasmMemoryArray                    = onReadyDetails.wasmMemoryArray;
					globalWasmImageSharedUint8ClampedArray   = onReadyDetails.wasmImageArray;
					globalWasmImageUnsharedUint8ClampedArray = new Uint8ClampedArray(GUEST_SCREEN_WIDTH * GUEST_SCREEN_HEIGHT * 4);  // This entails unfortunate copying, but we allocate ONCE at least!
					globalWasmImage                          = new ImageData(globalWasmImageUnsharedUint8ClampedArray, GUEST_SCREEN_WIDTH, GUEST_SCREEN_HEIGHT); // TODO: check out the "format" parameter
					globalWasmRowDirtyCountersArray          = onReadyDetails.wasmRowDirtyCountersArray;
					globalWasmKeyTranslationTableSize        = onReadyDetails.wasmKeyTranslationTableSize;
					globalWasmKeyTranslationTableArray       = onReadyDetails.wasmKeyTranslationTableArray;
					globalWasmKeyboardPortsArray             = onReadyDetails.wasmKeyboardPortsArray;
					
					// Start the display update polling:
					
					window.requestAnimationFrame(onPollForGraphicsUpdate);
				});
				
		HideStartEmulatorButton();
	}
	/* Let's never stop.  else
	{
		await ensureGlobalAudioContextDisposed();
	} */
}



function BrowserKeyCodeToLynxKeyIndex(browserKeyCode)
{
	for (i=0; i < globalWasmKeyTranslationTableSize; i++)
	{
		if (browserKeyCode === globalWasmKeyTranslationTableArray[i])
		{
			return i;
		}
	}
	return -1;
}



function onKeyDown(event) 
{
	if (globalWasmKeyTranslationTableArray)
	{
		let bitIndex = BrowserKeyCodeToLynxKeyIndex(event.keyCode);
		if (bitIndex != -1)
		{
			let portIndex = bitIndex >>> 3;
			let mask = 0x80 >>> (bitIndex & 7);
			globalWasmKeyboardPortsArray[portIndex] = (globalWasmKeyboardPortsArray[portIndex] | mask) ^ mask;  // Clear bit in WASM shared memory  (key Down -ve logic)
		}
	}
}



function onKeyUp(event) 
{	
	if (globalWasmKeyTranslationTableArray)
	{
		let bitIndex = BrowserKeyCodeToLynxKeyIndex(event.keyCode);
		if (bitIndex != -1)
		{
			let portIndex = bitIndex >>> 3;
			let mask = 0x80 >>> (bitIndex & 7);
			globalWasmKeyboardPortsArray[portIndex] = globalWasmKeyboardPortsArray[portIndex] | mask;  // Set bit in WASM shared memory (key Up -ve logic)
		}
	}
}
