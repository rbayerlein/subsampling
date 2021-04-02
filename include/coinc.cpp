#include "coinc.h"

#include <sstream>

#include "bankPairLUT.h"

namespace COINC {

	// timestamp

	bool IsTimestamp(uint64_t event) {
		if ((event & 0x8000006000000070) == 0x8000000000000000)
			return true;
		else
			return false;
	}

	short GetYear(uint64_t event) {
		return (event >> 7) & 0xFFF;
	}

	short GetMonth(uint64_t event) {
		return event & 0xF;
	}

	short GetDay(uint64_t event) {
		return (event >> 19) & 0x1F;
	}

	short GetHour(uint64_t event) {
		return (event >> 32) & 0x1F;
	}

	short GetMinute(uint64_t event) {
		return (event >> 39) & 0x3F;
	}

	short GetSecond(uint64_t event) {
		return (event >> 45) & 0x3F;
	}

	short GetMillisecond(uint64_t event) {
		return (event >> 51) & 0x3FF;
	}

	std::string GetDateAndTime(uint64_t event) {
		std::stringstream ss;
		ss << GetYear(event)
			<< "/"
			<< GetMonth(event)
			<< "/"
			<< GetDay(event)
			<< " "
			<< GetHour(event)
			<< ":"
			<< GetMinute(event)
			<< ":"
			<< GetSecond(event)
			<< "."
			<< GetMillisecond(event);
		return ss.str();
	}

	// coincidence event (coinc.)

	bool IsCoinc(uint64_t event) {
		if ((event & 0x8000000000000010) == 0x8000000000000010)
			return true;
		else
			return false;
	}

	short GetUiA(uint64_t event) { // concatenate UiA[2], UiA[1] and UiA[0]
		return ((event >> 28) & 0x4)
			| ((event >> 17) & 0x2)
			| ((event >> 3) & 0x1);
	}

	short GetUiB(uint64_t event) { // concatenate UiB[2], UiB[1] and UiB[0]
		return ((event >> 60) & 0x4)
			| ((event >> 49) & 0x2)
			| ((event >> 35) & 0x1);
	}

	short GetUiDiff(uint64_t event) {
		return abs(GetUiA(event) - GetUiB(event));
	}

	short GetCrysA(uint64_t event) {
		return (event >> 5) & 0x1FFF;
	}

	short GetCrysB(uint64_t event) {
		return (event >> 37) & 0x1FFF;
	}

	short GetAxID1(uint64_t event) {
		// 84 axial crystals per bank; and 70 transaxial crystals per bank
		return GetUiA(event) * 84 + GetCrysA(event) / 70;
	}

	short GetAxID2(uint64_t event) {
		// 84 axial crystals per bank; and 70 transaxial crystals per bank
		return GetUiB(event) * 84 + GetCrysB(event) / 70;
	}

	short GetBankPair(uint64_t event) { // concatenate Bank_Pair_B and Bank_Pair_A
		return ((event >> 29) & 0x38) | (event & 0x7);
	}

	short GetBankA(uint64_t event) {
		return LUT::bankPairLUT[GetBankPair(event) - 1][0]; // bank_pair_index range from 1-54 not 0-53
	}

	short GetBankB(uint64_t event) {
		return LUT::bankPairLUT[GetBankPair(event) - 1][1]; // bank_pair_index range from 1-54 not 0-53
	}

	short GetTxID1(uint64_t event) {
		return GetBankA(event) * 70 + GetCrysA(event) % 70; // 70 transaxial crystals per bank
	}

	short GetTxID2(uint64_t event) {
		return GetBankB(event) * 70 + GetCrysB(event) % 70; // 70 transaxial crystals per bank
	}

	short GetTATB(uint64_t event) { // concatenate TA_TB_H and TA_TB_L for TOF
		return (int8_t) (((event >> 47) & 0xF0) | ((event >> 19) & 0xF));
	}

	short GetEnergyA(uint64_t event) {
		return (event >> 23) & 0x7F;
	}

	short GetEnergyB(uint64_t event) {
		return (event >> 55) & 0x7F;
	}

	short GetMi1(uint64_t event) {
		return GetUiA(event) * 24 + GetTxID1(event) / 35; // 35 tx crys per module
	}

	short GetMi2(uint64_t event) {
		return GetUiA(event) * 24 + GetTxID2(event) / 35; // 35 tx crys per module
	}

	// block rate

	bool IsBlockRate(uint64_t event) {
		if ((event & 0x8000006000000070) == 0x8000000000000020)
			return true;
		else
			return false;
	}

	unsigned short GetUnitID(uint64_t event) {
		return (event >> 32) & 0x7;
	}

	unsigned short GetBlockIDX(uint64_t event) {
		return (event >> 23) & 0xFF;
	}

	unsigned short GetBlockIDY(uint64_t event) {
		return event & 0xF;
	}

        unsigned short GetBlockRate(uint64_t event) {
		return (event >> 7) & 0xFFFF;
	}

	void SetBlockRate(uint64_t& event, unsigned short value) {
		event &= 0xFFFFFFFFFF80007F; // mask out block rate bits (keep remaining info)
		uint64_t value_new = ((uint64_t) value << 7) & 0x7FFF80; // align value_new to exact block rate position in event
		event |= value_new; // combine value_new to event
	}

	// prompt/delay count rate

	bool IsPdRate(uint64_t event) {
		if ((event & 0x8000006000000070) == 0x8000004000000040)
			return true;
		else
			return false;
	}

	bool IsBedPos(uint64_t event) {
		if ((event & 0x8000006000000070) == 0x8000004000000020)
			return true;
		else
			return false;
	}

	// determine prompt or delayed coincidence

	bool IsDelayFlag(uint64_t event) {
		if (((event >> 36) & 0x1) == 1)
			return true;
		else
			return false;
	}

}
