/**
 * Adds shared into each channel.
 * This is the simple version where we use WebAssembly only on the worklet (no shared memory).
 *
 * @class SharedGeneratorProcessor
 * @extends AudioWorkletProcessor
 **/

class SharedGeneratorProcessor extends AudioWorkletProcessor 
{
	constructor(options) 
	{
		super();
		
		this.port.onmessage = (e) => {
			console.log('Message received by AudioWorkletProcessor: ' + e.data);
			console.log(e);
			this.port.postMessage('pong');
		};
		
		let wasmMod = options.processorOptions.wasmMod;
		
		this.ready = false;
		
		let parentThis = this;

		let memory = new WebAssembly.Memory({   // TODO: Sort out maximum:100 issue.
			initial:10,
			maximum:10,
			shared: true 
		});  

		var importObject = {
			env: { memory: memory }
		};

		WebAssembly
			.instantiate(wasmMod, importObject)
			.then(function(instance) {
				
				let soundBufferAddress = instance.exports.get_static_sound_buffer();
				
				parentThis.soundBufferDataView = new DataView(memory.buffer, soundBufferAddress, 128*4);
				parentThis.soundBufferAddress = soundBufferAddress;
				parentThis.fill_sound_buffer = instance.exports.fill_sound_buffer;

				let postedDataForHost = {
					volumeLevelAddress: instance.exports.get_static_level_variable(),
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



registerProcessor("jynx-emulator-worker", SharedGeneratorProcessor);

