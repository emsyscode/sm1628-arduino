# Datasheets and exact revisions

For the practical summary, start with [SM1628 and SM1628B](driver-variants.md).
Both use the display programming taught in this tutorial.

These are Sunmoon-authored documents hosted by public mirrors. Match the exact
part/revision on your panel; a mirror is not the IC manufacturer.

| Document | Relevant pages | Link |
|---|---|---|
| SM1628, IGBG-IZV1.0, 13 pages | p1 features; p2 pins; p5 memory; p6-7 commands | [PDF](https://taoic.oss-cn-hangzhou.aliyuncs.com/7860/product/xiaoliu888_1608197155000.pdf) |
| SM1628B, 15 pages | p3 pins; p4 memory; p5 data mode; p6 address/brightness; p7 serial format | [PDF](https://datasheet4u.com/pdf/791904/SM1628B.pdf) |

Both include seven-digit/ten-segment mode. The B command table on p5 identifies
auto-increment and fixed-address writes; p6 sets RAM addresses 0x00-0x0D.
Our [full-frame sender](04-memory.md) is a software update strategy, not a claim
that partial protocol writes are unsupported.

In these particular documents, SM1628 lists internal RC oscillation with pin 1
NC. SM1628B identifies pin 1 as OSC with an external frequency-setting resistor
to ground, and describes improved electrostatic/power-interference protection.
An external setting resistor is not necessarily an external oscillator module.
Do not generalize revision differences beyond the documents you have checked.
Initialization explicitly selects mode 0x03 rather than relying on a power-on
mode that can differ between documents/revisions.

Document identity SHA-256:

- SM1628: 77b782fa46876d08068edc68219a1261f0f9ff492389f7e013ed64138fc3b8fa
- SM1628B: b4d44f8075cf34618a423056383d312d1c13e392bccfd5e08536428ad60e488a

The datasheets describe the IC, not the connections made by a panel manufacturer.
Use the [panel map](panel-map.md) for this build and [ManualMap](manual-mapping.md)
to identify the wiring of another panel.
