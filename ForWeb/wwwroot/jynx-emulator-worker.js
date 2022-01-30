/**
 * The Jynx Emulator.
 *
 * The emulation is entirely run in a WASM module within an AudioWorkletProcessor.
 *
 * This is because the emulator needs to generate wave audio.
 *
 * Shared memory is used to enable the browser's main thread to:-
 *    - Monitor the emulation health (JynxFramework::Panic())
 *    - Update the screen in the host browser (Display invalidation flags).
 *    - Copy the screen bitmap bits from WASM memory to the Web Canvas bitmap.
 *    - Send key presses into the emulator (keydown/up flags).
 *
 * The main thread does not block, so there (theoretically) could be tearing artefacts
 * for the display, however this is not deemed important.
 *
 * So, the main thread modifies some things by directly altering the variables in the WASM memory space.
 *     See:  ECMA 262 : 29.11 Shared Memory Guidelines
 *           "... if memory is treated as strongly typed the racing accesses will not "tear" (bits of their values will not be mixed)."
 *
 * @class JynxEmulatorWorkletProcessor
 * @extends AudioWorkletProcessor
 **/

class JynxEmulatorWorkletProcessor extends AudioWorkletProcessor 
{
	constructor(options) 
	{
		super();
		
		this.port.onmessage = (e) => {
			 // TODO:
			console.log('Message received by JynxEmulatorWorkletProcessor: ' + e.data);
			console.log(e);
			this.port.postMessage('pong');
		};
		
		let compiledWasmModule = options.processorOptions.compiledWasmModule;
		
		this.ready = false;
		
		let parentThis = this;

		let memory = new WebAssembly.Memory({   // TODO: Sort out maximum:100 issue.
			initial:256,
			maximum:256,  // TODO: I certainly intend to have a non-growing size initially because of the shared memory object.  This size is also hard-coded into the C++ memory allocator.
			shared: true  // Must be shared, to allow browser main thread to communicate with the emulator.
		});  

		var importObject = {
			env: { memory: memory }
		};

		WebAssembly
			.instantiate(compiledWasmModule, importObject)
			.then(function(instance) {
				
				// The sound waveform is generated by the WASM module, in its memory space.
				// We need a "view" of that for the "process()" event handler function:
				
				let soundBufferAddress = instance.exports.get_static_sound_buffer();
				parentThis.soundBufferDataView = new DataView(memory.buffer, soundBufferAddress, 128*4);
				parentThis.soundBufferAddress = soundBufferAddress;
				parentThis.fill_sound_buffer = instance.exports.fill_sound_buffer;

				// Obtain addresses from the WASM instance, and post these over to the browser's main thread.
				// We also post the WASM shared memory object.

				let postedDataForHost = {
					// volumeLevelAddress: instance.exports.get_static_level_variable(),
					screenBaseAddress:  instance.exports.get_screen_base_address(),
					memory:             memory
				};

				parentThis.port.postMessage(postedDataForHost);
		
				parentThis.ready = true;
			});		
	}


	/**
	 * Called by the browser's audio subsystem with
	 * packets of audio data to be processed.
	 *
	 * @param[in] inputList    Array of inputs
	 * @param[in] outputList   Array of outputs
	 * @param[in] parameters   Parameters object
	 *
	 * `inputList` and `outputList` are each arrays of inputs
	 * or outputs, each of which is in turn an array of `Float32Array`s,
	 * each of which contains the audio data for one channel (left/right/etc)
	 * for the current sample packet.
	 *
	 * `parameters` is an object containing the `AudioParam` values
	 * for the current block of audio data.
	 **/
	  
	process(inputList, outputList, parameters) {

		if (!this.ready)
		{
			this.processBeforeReady(outputList);
			return true;
		}

		const sourceLimit = Math.min(inputList.length, outputList.length);

		for (let inputNum = 0; inputNum < sourceLimit; inputNum++) 
		{
			let input = inputList[inputNum];
			let output = outputList[inputNum];
			let channelCount = Math.min(input.length, output.length);

			// The input list and output list are each arrays of
			// Float32Array objects, each of which contains the
			// samples for one channel.

			for (let channel = 0; channel < channelCount; channel++) 
			{
				this.fill_sound_buffer();
				let outputChannel = output[channel];
				let sampleCount = Math.min(128, outputChannel.length);
				for (let i = 0; i < sampleCount; i++) 
				{
					outputChannel[i] = this.soundBufferDataView.getFloat32(i * 4, true); // TODO: Float32Array alias, not this!
				}
			}
		};

		// Return; let the system know we're still active
		// and ready to process audio.

		return true;
	}

	processBeforeReady(outputList) {
		// TODO: fill with silence.
	}
}



registerProcessor("jynx-emulator-worker", JynxEmulatorWorkletProcessor);

