# Datasheet references

Downloaded 2026-10-26. Both are Sunmoon-authored Chinese documents obtained
through public mirrors; they are not a matched publication-date pair.

- [SM1628, IGBG-IZV1.0, 13 pages](https://taoic.oss-cn-hangzhou.aliyuncs.com/7860/product/xiaoliu888_1608197155000.pdf).
  Page 1 lists internal RC oscillation; page 2 identifies pin 1 as NC.
  SHA-256: 77B782FA46876D08068EDC68219A1261F0F9FF492389F7E013ED64138FC3B8FA.
- [SM1628B, 15 pages](https://datasheet4u.com/pdf/791904/SM1628B.pdf).
  Page 1 describes improved electrostatic/power-interference protection.
  Page 3 identifies pin 1 as OSC: an external resistor to ground determines
  oscillation frequency. This is an external frequency-setting resistor,
  not necessarily an external oscillator module.
  SHA-256: B4D44F8075CF34618A423056383D312D1C13E392BCCFD5E08536428AD60E488A.

Both describe the native seven-digit/ten-segment mode. Read the exact revision
before generalizing oscillator/protection differences across all similarly
named parts. Our panel's swapped wiring is documented separately in panel-map.md.
Original PDF copies are saved in the production project's technical/datasheets.
