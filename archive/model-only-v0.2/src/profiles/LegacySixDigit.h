#pragma once
#include "../MappedPanel.h"

namespace sm1628 {
namespace profiles {
// CODE_DERIVED from emsyscode's 2020 SM1628B six-digit clock example.
// Logical HHMMSS order, not a newly verified front-view board measurement.
// Each row lists a,b,c,d,e,f,g RAM byte/bit addresses.
const DigitMap kLegacySixDigit[6] = {
  {{{1, 1}, {3, 1}, {5, 1}, {7, 1}, {9, 1}, {11, 1}, {13, 1}}}, // Legacy grid bit 9
  {{{1, 0}, {3, 0}, {5, 0}, {7, 0}, {9, 0}, {11, 0}, {13, 0}}}, // Legacy grid bit 8
  {{{0, 7}, {2, 7}, {4, 7}, {6, 7}, {8, 7}, {10, 7}, {12, 7}}}, // Legacy grid bit 7
  {{{0, 1}, {2, 1}, {4, 1}, {6, 1}, {8, 1}, {10, 1}, {12, 1}}}, // Legacy grid bit 1
  {{{0, 2}, {2, 2}, {4, 2}, {6, 2}, {8, 2}, {10, 2}, {12, 2}}}, // Legacy grid bit 2
  {{{0, 3}, {2, 3}, {4, 3}, {6, 3}, {8, 3}, {10, 3}, {12, 3}}}, // Legacy grid bit 3
};
}  // namespace profiles
}  // namespace sm1628
