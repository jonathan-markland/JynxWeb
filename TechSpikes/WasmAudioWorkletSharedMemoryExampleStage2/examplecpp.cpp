// examplecpp.cpp

// clang -O2 --target=wasm32 --no-standard-libraries -matomics -mbulk-memory -Wl,--max-memory=655360 -Wl,--shared-memory -Wl,--import-memory -Wl,--export-all -Wl,--no-entry -o examplecpp.wasm examplecpp.cpp

// [ ] Raw Heap
// [ ] Managed Object Instance
// [ ] Managed Array Instance -- not resizeable
// [ ] ArrayView Reference
// [ ] Result type
// [ ] String On Heap type -- not resizeable
// [ ] 

extern unsigned char __heap_base;

void *g_NextHeapAllocation = &__heap_base;

void *malloc(unsigned long amount)
{
	return (void*)12340;
}

void free(void *address)
{
	
}

void *operator new(unsigned long amount)
{
	return malloc(amount);
}

void operator delete(void *address) noexcept
{
	free(address);
}

class ExampleClass
{
private:

	int m_Int;
	
public:

	ExampleClass()
	{
		m_Int = 10;
	}

};

ExampleClass *make_new_example()
{
	return new ExampleClass();
}

void erase_new_example(ExampleClass *ec)
{
	delete ec;
}

template<typename ERROR_TYPE>
struct FailureRecord
{
	ERROR_TYPE error;
};

template<typename ERROR_TYPE>
inline FailureRecord<ERROR_TYPE> Fail(ERROR_TYPE errorValue) 
{ 
	FailureRecord<ERROR_TYPE> fr;
	fr.error = errorValue;
	return fr;
}

template<typename SUCCESS_TYPE, typename ERROR_TYPE>
struct Result
{
	// ERROR_TYPE must be convertble to/from int, where 0 == no failure.
	
	Result(SUCCESS_TYPE value)
		: m_success(value), m_error((ERROR_TYPE) 0) {}
		
	Result(FailureRecord<ERROR_TYPE> failureRecord) 
		: m_error(failureRecord.error) {}
	
	SUCCESS_TYPE m_success;
	ERROR_TYPE   m_error;
	
	inline bool IsSuccess() const { return ((int) m_error) == 0; }
	inline operator bool() const  { return IsSuccess(); }
};



Result<int, bool> TryParseInteger(const char *source)
{
	if (*source == '1')
	{
		return 1;
	}
	else
	{
		return Fail(false);
	}
}


void Example1()
{
	const char *someNumber = "12345";
	
	auto parseResult = TryParseInteger(someNumber);
	
	if (parseResult)
	{
		
	}
	
}






/* EXCEPTIONS THROWING 

Search "throw" (32 hits in 12 files of 65 searched)
  C:\Users\Jonathan\Documents\Work\Jynx\Portable\FileLoader.cpp (3 hits)
	Line 50: 	        throw VectorLoadSaveException( e.what() );
	Line 53: 		throw VectorLoadSaveException( "File is too large to load." );
	Line 76: 	        throw VectorLoadSaveException( e.what() );
  C:\Users\Jonathan\Documents\Work\Jynx\Portable\FileLoader.h (2 hits)
	Line 37: 	void LoadFileIntoVector( IFileOpener *fileOpener, std::vector<uint8_t> &result );            // throws VectorLoadSaveException  [re-maps file exceptions]
	Line 38: 	void SaveFileFromVector( IFileOpener *fileOpener, const std::vector<uint8_t> &fileImage );   // throws VectorLoadSaveException  [re-maps file exceptions]
  C:\Users\Jonathan\Documents\Work\Jynx\Portable\FileSerialiser.cpp (1 hit)
	Line 123: 		throw std::runtime_error( "File saving failed because internal data values cannot be represented in the file." );
  C:\Users\Jonathan\Documents\Work\Jynx\Portable\FileSerialiser.h (2 hits)
	Line 57:             // throws const std::invalid_argument on parse error
	Line 58:             // throws const std::ifstream::failure
  C:\Users\Jonathan\Documents\Work\Jynx\Portable\Lexer.cpp (1 hit)
	Line 114: 		throw LexerException( "File parsing failed because of a syntax error." );
  C:\Users\Jonathan\Documents\Work\Jynx\Portable\LynxEmulatorGuest.cpp (3 hits)
	Line 237: 		throw std::runtime_error( "One or more of the ROM files are missing or damaged.  Please refer to the readme.htm file for information." );  // very base class std::exception should terminate program.
	Line 1803: 			throw;
	Line 1886: 		auto newTape = std::make_shared<TapFileReader>( fileOpener, this );  // throws
  C:\Users\Jonathan\Documents\Work\Jynx\Portable\LynxUserInterfaceModel.cpp (8 hits)
	Line 386:                 _lynxEmulator->RunExistingTAPFile( fileOpener.get() ); // throws
	Line 405:                     _lynxEmulator->LoadExistingTAPFile( fileOpener.get() ); // throws
	Line 515:                 _lynxEmulator->LoadState( fileOpener.get() );  // throws
	Line 525: 		_lynxEmulator->LoadState( fileOpener );  // throws
	Line 533: 		_lynxEmulator->RunExistingTAPFile( fileOpener );  // throws
	Line 579:             _lynxEmulator->RecordSoundToFile( fileOpener.get() ); // throws
	Line 604:             _lynxEmulator->RecordLynxTextToFile( fileOpener.get() ); // throws
	Line 628:             _lynxEmulator->TypeInTextFromFile( fileOpener.get() );  // throws
  C:\Users\Jonathan\Documents\Work\Jynx\Portable\LynxUserInterfaceModel.h (1 hit)
	Line 67: 		// These throw:
  C:\Users\Jonathan\Documents\Work\Jynx\Portable\ParameterParsing.cpp (2 hits)
	Line 26: 	throw CommandLineParsingException( "Syntax error in command line." );
	Line 34: 	throw CommandLineParsingException( "Command line parameter is missing an operand." );
  C:\Users\Jonathan\Documents\Work\Jynx\Portable\TapFileSplitter.cpp (3 hits)
	Line 37:             // Throws TapFileLexerException if parse fails.
	Line 177: 		throw TapFileLexerException( "Failed to parse TAP file." );
	Line 267: 		LoadFileIntoVector( tapFileOpener, fileImage ); // throws
  C:\Users\Jonathan\Documents\Work\Jynx\Portable\UnexpectedExceptionGuard.h (1 hit)
	Line 38:         throw; // never executed
  C:\Users\Jonathan\Documents\Work\Jynx\Portable\Waitable.h (5 hits)
	Line 34: 			// Throws std::exception if failed to create.
	Line 41: 			// Throws std::exception if failed.
	Line 47: 			// Throws std::exception if failed.
	Line 52: 			// Throws std::exception if failed.
	Line 56: 			// Throws std::exception if failed.


*/