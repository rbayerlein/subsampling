#ifndef COINC_V6_H_
#define COINC_V6_H_

#include <cstdint>
#include <string>

namespace COINC {
	// timestamp
	bool IsTimestamp(uint64_t event);
	short GetYear(uint64_t event);
	short GetMonth(uint64_t event);
	short GetDay(uint64_t event);
	short GetHour(uint64_t event);
	short GetMinute(uint64_t event);
	short GetSecond(uint64_t event);
	short GetMillisecond(uint64_t event);
	std::string GetDateAndTime(uint64_t event);

	// coincidence event (coinc.)
	bool IsCoinc(uint64_t event);
	short GetUiA(uint64_t event);
	short GetUiB(uint64_t event);
	short GetUiDiff(uint64_t event);
	short GetCrysA(uint64_t event);
	short GetCrysB(uint64_t event);
	short GetAxID1(uint64_t event);
	short GetAxID2(uint64_t event);
	short GetBankPair(uint64_t event);
	short GetBankA(uint64_t event);
	short GetBankB(uint64_t event);
	short GetTxID1(uint64_t event);
	short GetTxID2(uint64_t event);
	short GetTATB(uint64_t event); // TOF
	short GetEnergyA(uint64_t event);
	short GetEnergyB(uint64_t event);
	short GetMi1(uint64_t event); // absolute module ID - 0-23 for U0, 24-47 for U1, etc.
	short GetMi2(uint64_t event); // absolute module ID - 0-23 for U0, 24-47 for U1, etc.

	// block rate
	bool IsBlockRate(uint64_t event);
	unsigned short GetUnitID(uint64_t event);
	unsigned short GetBlockIDX(uint64_t event);
	unsigned short GetBlockIDY(uint64_t event);
	unsigned short GetBlockRate(uint64_t event);
	void SetBlockRate(uint64_t& event, unsigned short value);

	// p/d rate
	bool IsPdRate(uint64_t event);

	// bed position
	bool IsBedPos(uint64_t event);

	// determine prompt or delayed coincidence
	bool IsDelayFlag(uint64_t event);
}

#endif /* COINC_V6_H_ */
