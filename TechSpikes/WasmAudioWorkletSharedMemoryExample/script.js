let audioContext = null;
let sharedGainRange;
let oscGainRange;
let sharedGenNode;
let gainNode;
let sharedGainParam;

async function createSharedProcessor() {

  if (!audioContext) {
    try {
      audioContext = new AudioContext();
    } catch(e) {
      console.log("** Error: Unable to create audio context");
      return null;
    }
  }
  
  let memory = new WebAssembly.Memory({ initial:10, maximum:100, shared: true });
  
  let wasmMod = await WebAssembly
		.compileStreaming(fetch('shared.wasm'), { js: { mem: memory } })
		.then(mod => { 
			console.log("JM TRACE: compileStreaming success with shared memory.");
			return mod;
		});

	let audioWorkletNodeProcessorOptions = {
	  processorOptions: { wasmMod: wasmMod } 
	};

  let processorNode;

  try {
    processorNode = new AudioWorkletNode(audioContext, "shared-generator", audioWorkletNodeProcessorOptions);
  } catch(e) {
    try {
      await audioContext.audioWorklet.addModule("shared-generator.js");
      processorNode = new AudioWorkletNode(audioContext, "shared-generator", audioWorkletNodeProcessorOptions);
    } catch(e) {
      console.log(`** Error: Unable to create worklet node: ${e}`);
      return null;
    }
  }

  await audioContext.resume();
  return processorNode;
}

async function audioDemoStart() {
  sharedGenNode = await createSharedProcessor();
  if (!sharedGenNode) {
    console.log("** Error: unable to create shared processor");
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
  
  soundSource.connect(gainNode).connect(sharedGenNode).connect(audioContext.destination);
  soundSource.start();
  
  // Get access to the worklet's gain parameter
  
  sharedGainParam = sharedGenNode.parameters.get("gain");
  sharedGainParam.setValueAtTime(sharedGainRange.value, audioContext.currentTime);
}

window.addEventListener("load", event => {
  document.getElementById("toggle").addEventListener("click", toggleSound);
  
  sharedGainRange = document.getElementById("shared-gain");
  oscGainRange = document.getElementById("osc-gain");
  
  sharedGainRange.oninput = updateSharedGain;
  oscGainRange.oninput = updateOscGain;
  
  sharedGainRange.disabled = true;
  oscGainRange.disabled = true;
});

async function toggleSound(event) {
  if (!audioContext) {
    audioDemoStart();
    
    sharedGainRange.disabled = false;
    oscGainRange.disabled = false;
  } else {
    sharedGainRange.disabled = true;
    oscGainRange.disabled = true;

    await audioContext.close();
    audioContext = null;
  }
}

function updateSharedGain(event) {
  sharedGainParam.setValueAtTime(event.target.value, audioContext.currentTime);
}

function updateOscGain(event) {
  gainNode.gain.setValueAtTime(event.target.value, audioContext.currentTime);
}