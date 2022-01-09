let audioContext = null;
let squareGainRange;
let oscGainRange;
let squareGenNode;
let gainNode;
let squareGainParam;

async function createSquareProcessor() {

  if (!audioContext) {
    try {
      audioContext = new AudioContext();
    } catch(e) {
      console.log("** Error: Unable to create audio context");
      return null;
    }
  }
  
  console.log("JM TRACE: 1");
  
  let wasmMod = await WebAssembly
		.compileStreaming(fetch('square.wasm'))
		.then(mod => { 
			console.log("JM TRACE: A");
			return mod;
		});

  console.log("JM TRACE: 2");
  console.log("JM TRACE: wasmMod name is: " + wasmMod.constructor.name);
  console.log("JM TRACE: wasmMod = " + JSON.stringify(wasmMod));

	let audioWorkletNodeProcessorOptions = {
	  processorOptions: { wasmMod: wasmMod } 
	};

  console.log("JM TRACE: options = " + JSON.stringify(audioWorkletNodeProcessorOptions));
  console.log("JM TRACE: 3");
	  
  let processorNode;

  try {
  console.log("JM TRACE: 4");
	  
    processorNode = new AudioWorkletNode(audioContext, "square-generator", audioWorkletNodeProcessorOptions);
  console.log("JM TRACE: 5");
  } catch(e) {
    try {
      console.log("adding...")
      await audioContext.audioWorklet.addModule("square-generator.js");
      processorNode = new AudioWorkletNode(audioContext, "square-generator", audioWorkletNodeProcessorOptions);
    } catch(e) {
      console.log(`** Error: Unable to create worklet node: ${e}`);
      return null;
    }
  }

  await audioContext.resume();
  return processorNode;
}

async function audioDemoStart() {
  squareGenNode = await createSquareProcessor();
  if (!squareGenNode) {
    console.log("** Error: unable to create square processor");
    return;
  }
  const soundSource = new OscillatorNode(audioContext);
  gainNode = audioContext.createGain();

  // Configure the oscillator node
  
  soundSource.type = "square";
  soundSource.frequency.setValueAtTime(440, audioContext.currentTime); // (A4)
  
  // Configure the gain for the oscillator
  
  gainNode.gain.setValueAtTime(oscGainRange.value, audioContext.currentTime);
  
  // Connect and start
  
  soundSource.connect(gainNode).connect(squareGenNode).connect(audioContext.destination);
  soundSource.start();
  
  // Get access to the worklet's gain parameter
  
  squareGainParam = squareGenNode.parameters.get("gain");
  squareGainParam.setValueAtTime(squareGainRange.value, audioContext.currentTime);
}

window.addEventListener("load", event => {
  document.getElementById("toggle").addEventListener("click", toggleSound);
  
  squareGainRange = document.getElementById("square-gain");
  oscGainRange = document.getElementById("osc-gain");
  
  squareGainRange.oninput = updateSquareGain;
  oscGainRange.oninput = updateOscGain;
  
  squareGainRange.disabled = true;
  oscGainRange.disabled = true;
});

async function toggleSound(event) {
  if (!audioContext) {
    audioDemoStart();
    
    squareGainRange.disabled = false;
    oscGainRange.disabled = false;
  } else {
    squareGainRange.disabled = true;
    oscGainRange.disabled = true;

    await audioContext.close();
    audioContext = null;
  }
}

function updateSquareGain(event) {
  squareGainParam.setValueAtTime(event.target.value, audioContext.currentTime);
}

function updateOscGain(event) {
  gainNode.gain.setValueAtTime(event.target.value, audioContext.currentTime);
}