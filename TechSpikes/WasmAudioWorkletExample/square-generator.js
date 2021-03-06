/**
 * Adds square into each channel.
 * This is the simple version where we use WebAssembly only on the worklet (no shared memory).
 *
 * @class SquareGeneratorProcessor
 * @extends AudioWorkletProcessor
 **/

 class SquareGeneratorProcessor extends AudioWorkletProcessor 
 {
    constructor(options) 
	{
		super();
		
		let wasmMod = options.processorOptions.wasmMod;
		
		var importObject = {
		  imports: {
			// imported_func: function(arg) {
			//   console.log(arg);
			// }
		  }
		};
		
		this.ready = false;
		
		let parentThis = this;
		
		WebAssembly
			.instantiate(wasmMod, importObject)
			.then(function(instance) {
				let wasmMemory = instance.exports.memory.buffer;
				let soundBufferAddress = instance.exports.get_static_sound_buffer();
				parentThis.soundBufferArray = new Float32Array(wasmMemory, soundBufferAddress, 128);
				parentThis.fill_sound_buffer = instance.exports.fill_sound_buffer;
				parentThis.ready = true;
			});		
    }
	
	
    
    static get parameterDescriptors() 
	{
      return [
        {
          name: "gain",
          defaultValue: 0.2,
          minValue: 0,
          maxValue: 1
        }
      ];
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

	  const gain = parameters.gain[0];
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
			  outputChannel[i] = this.soundBufferArray[i];
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


  registerProcessor("square-generator", SquareGeneratorProcessor);