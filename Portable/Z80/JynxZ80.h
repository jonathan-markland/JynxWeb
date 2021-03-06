//
// JynxZ80 - Jonathan's Z80 Emulator - Initially for Camputers Lynx Emulation Project.
// Copyright (C) 2014  Jonathan Markland
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//		jynx_emulator {at} yahoo {dot} com
//

// - See Z80ExternalImplementation.h for user tasks.
//
// - This was made for the Camputers Lynx emulator, but does NOT have Lynx-specific code!
//
// - The Lynx had almost no use for interrupts anyway, and no use at all for them in the
//   parts of the Lynx that I emulate!  So, the implementation of interrupts is
//   work-in-progress.  I will need to substitute into another oper-source emulator to
//   properly develop interrupts.  Sorry!
//
// - This is restricted to little-endian machines at present.  TheLoByte() and HiByte()
//   functions are responsible for this restriction.  I cannot test big-endian.
//

#pragma once

#include <stdint.h>
#include "JynxZ80Declarations.h"
#include "IZ80ExternalHandler.h"

namespace JynxZ80
{
	class Z80: private Z80SerialisableState
	{
	public:

		// Main Z80 emulator.

		static void InitialiseGlobalTables();
			// Must be called ONCE before any instances created.
			// (Exposing this side-steps any multi-threading issues).

		Z80();

		// Attach the Z80 to the user-defined object for 
		// I/O and address space handling.
		void SetExternalHandler(IZ80ExternalHandler *externalHandler)  { _externalHandler = externalHandler; }

		void Reset();
		void RunForTimeslice();

		//
		// Timeslice support.
		//

		// The number of Z80 cycles desired per timeslice.
		// The Z80 MAY overshoot this by 1 instruction's time, but that
		// will automatically be subtracted from the next timeslice.
		// You can change the timeslice length during run, but that will
		// only take effect on the expiration of the current remainder.
		void SetTimesliceLength( int32_t numCycles );

		// If called from Read/Write handlers, this gets the number of cycles 
		// worth of work done so far in the current timeslice.
		//
		// If called after RunForTimeslice() returns, a new timeslice has
		// already begun, and so this should be 0, except it will actually
		// obtain a very small value which is the overshoot from the previous 
		// timeslicc in cycles, since the instructions cannot be guaranteed to 
		// precisely divide into the timeslice.
		//
		int32_t GetCyclesDoneInTimeslice() const        { return _timesliceLength - _remainingCycles; }

		// Get the number of remaining cycles in the current timeslice.
		// This may be negative if we overshot by part of the final instruction.
		int32_t GetRemainingCycles() const              { return _remainingCycles; }
		
		// The number of Z80 cycles desired per timeslice.
		int32_t GetTimesliceLength() const              { return _timesliceLength; }

		//
		// Serialisation support.
		// This does set / get everything pertinent, but this interface
		// dissuades general fiddling.
		//

		const Z80SerialisableState &GetSerialisableVariables() const;
		void SetSerialisableVariables( const Z80SerialisableState & );

	private: // (No serialisation required).

		enum { _interruptSignalled = false };  // TODO: Until interrupt support perhaps!

		uint8_t _currentOpcode;
		uint8_t _hiddenRegisterSix;   // Unifies the handling of temporaries fetched from (HL) / (IX+nn) / (IY+nn)

		uint8_t   *_addressesOf8BitRegisters[8];                   // Access to register file, supporting re-direction H->IXH, L->IXL, or H->IYH, L->IYL, if prefixed.
		uint8_t   *_addressesOf8BitRegisters_NoRedirection[8];     // Access to register file, H and L are never re-directed to IXH/IXL IYH/IYL.
		uint16_t  *_addressesOf16BitRegisterPairs_BC_DE_HL_SP[4];  // Access to register file as register pairs.  Indexed based on bits from the opcode.
		uint16_t  *_addressesOf16BitRegisterPairs_BC_DE_HL_AF[4];  // Access to register file as register pairs.  Indexed based on bits from the opcode.

		static uint8_t _signParityAndZeroTable[256];  // Calculated on construction.

	private:

		// The user-defined object that will handle external Z80 interfacing.
		IZ80ExternalHandler *_externalHandler;

		// Beautify the main Z80 code using inlines to allow shorter syntax.
		
		inline void GuestWrite( uint16_t address, uint8_t dataByte )
		{
			_externalHandler->Z80_AddressWrite( address, dataByte );
		}
		
		inline void GuestWriteIOSpace( uint16_t portNumber, uint8_t dataByte )
		{
			_externalHandler->Z80_IOSpaceWrite( portNumber, dataByte );
		}
		
		inline uint8_t GuestRead( uint16_t address )
		{
			return _externalHandler->Z80_AddressRead( address );
		}
		
		inline uint8_t GuestReadIOSpace( uint16_t portNumber )
		{
			return _externalHandler->Z80_IOSpaceRead( portNumber );
		}
		
		inline void OnAboutToBranch()
		{
			_externalHandler->OnAboutToBranch();
		}
		
		inline void OnAboutToReturn()
		{
			_externalHandler->OnAboutToReturn();
		}

	private:

		static void CalculateSignParityAndZeroTable();

		//
		// Execution of opcodes
		//

		void ExecuteOpcodeMainSet();

		void MainSet_Quarter0_Column0();
		void MainSet_Quarter0_Column1();
		void MainSet_Quarter0_Column2();
		void MainSet_Quarter0_Column3();
		void MainSet_Quarter0_Column4();
		void MainSet_Quarter0_Column5();
		void MainSet_Quarter0_Column6();
		void MainSet_Quarter0_Column7();

		void MainSet_Quarter1();
		void MainSet_Quarter2();

		void MainSet_Quarter3_Column0();
		void MainSet_Quarter3_Column1();
		void MainSet_Quarter3_Column2();
		void MainSet_Quarter3_Column3();
		void MainSet_Quarter3_Column4();
		void MainSet_Quarter3_Column5();
		void MainSet_Quarter3_Column6();
		void MainSet_Quarter3_Column7();

		void ExecuteOpcodeED();
		void ExecuteED_Quarter1();
		void ExecuteED_Quarter2();
		void BlockCopy( int16_t directionDelta );
		void BlockCompare( int16_t directionDelta );
		void BlockReadIoSpace( int16_t directionDelta );
		void BlockWriteIoSpace( int16_t directionDelta );

		void ExecuteOpcodeCB();
		void ExecuteCB_OnSpecificSlot( uint8_t &targetRegister );

		void ExecuteOpcodeDD();
		void ExecuteOpcodeFD();

		void ExecuteOpcodeDDCB();
		void ExecuteOpcodeFDCB();
		void Execute_DDCB_FDCB( uint16_t &regIXIY );

		void UndefinedInstruction();

		//
		// Addressing of the register file.
		//
		// - Notice I infer the existence of a temporary register (encoding 110) that
		//   holds the value for the (HL) cases of otherwise regular-register instructions.
		//

		uint16_t &GetReferenceTo_HL_IX_IY();
		uint16_t &GetReferenceTo_BC_DE_HLIXIY_SP_FromOpcodeBits5and4();
		uint16_t &GetReferenceToRegisterForPushPopGroupFromOpcode();
		uint8_t  &GetReferenceToReg8_FromBits2to0OfOpcode();
		uint8_t  &GetReferenceToReg8_NoRedirection_FromBits2to0OfOpcode();
		uint8_t  &GetReferenceToReg8_FromBits5to3();
		uint8_t  &GetReferenceToReg8_NoRedirection_FromBits5to3();

		uint16_t  GetIndirectTargetAddress();

		//
		// Condition testing, avoiding C++ "bool".
		//

		uint8_t  IsConditionSatisfied( const ConditionTestingData &conditionTestingData );
		uint8_t  IsConditionSatisfiedBasedOnOpcode();

		//
		// Support subroutines for instruction execution.
		//

		inline   void     PrimaryOpcodeFetch()                  { _currentOpcode = CodeStreamFetch(); }
		inline   uint8_t  CodeStreamFetch()                     { return GuestRead( _programCounter++ ); }

		inline   void     Spend( int32_t z80Cycles )            { _remainingCycles -= z80Cycles; }

		inline   void     JumpTo( uint16_t address )            { _programCounter = address; OnAboutToBranch(); }
		inline   void     JumpRelative( int16_t displacement )  { _programCounter += displacement;  }

		inline   void     Countdown8()                          { --HiByte(_BC); }
		inline   void     Countdown16()                         { --_BC; }

		uint16_t ReadSixteenBitsFromInstructionStream();
		void DoSixteenBitLoadConstant( uint16_t &PC, uint16_t &targetRegister );
		uint16_t DoFetchSixteenBits( uint16_t address );
		void DoStoreSixteenBits( uint16_t address, uint16_t dataRegister );

		void DoPushSixteenBits( uint16_t valueToPush );
		void DoPopSixteenBitsIntoRegister( uint16_t &targetRegister );
		void DoSubroutineCall( uint16_t address );
		void DoSubroutineReturn();
		void DoArithLogicCompareWithAccumulatorAndValue( uint8_t rhsValue );
		void DoSixteenBitADD( uint16_t &leftOperand, const uint32_t rightOperand );
		void DoDAA();
		void DoRLDOrRRD( int32_t value1, int32_t value2, int32_t value3 );

		void AccumulatorAdd( uint8_t rhsValue );
		void AccumulatorAdc( uint8_t rhsValue );
		void AccumulatorSub( uint8_t rhsValue );
		void AccumulatorSbc( uint8_t rhsValue );
		void AccumulatorAnd( uint8_t rhsValue );
		void AccumulatorOr(  uint8_t rhsValue );
		void AccumulatorXor( uint8_t rhsValue );
		void AccumulatorCp(  uint8_t rhsValue );

		void DestructiveAdd8(      uint8_t &theRegister, uint8_t rhsValue, uint8_t carry );
		void DestructiveSubtract8( uint8_t &theRegister, uint8_t rhsValue, uint8_t carry );

		//
		// Flags reading
		//

		inline   uint8_t  Flags()                               { return LoByte(_AF); }
		inline   uint8_t  CurrentCarry()                        { return Flags() & Z80Flags::CF; }
		inline   uint8_t  HalfCarryBasedOnCurrentCarry()        { return CurrentCarry() << 4; }

		//
		// Flags writing
		//

		inline   void SetFlags( uint8_t value )                                 { LoByte(_AF) = value; }
		inline   void ClearFlagsThenMerge( uint8_t toClear, uint8_t toMerge )   { SetFlags( (Flags() & (~toClear)) | toMerge ); }

		//
		// Accumulator read and write
		//

		inline   uint8_t Accumulator()                                          { return HiByte(_AF); }
		inline   void SetAccumulator( uint8_t value )                           { HiByte(_AF) = value; }
		inline   void SetAccumulatorFromIOrR( uint8_t value );

		//
		// A lot of instruction need the Zero, Sign and Parity flags setting based on the value.
		//

		inline   static uint8_t ZeroSignAndParity8( uint8_t value );

	};

} // end namespace
