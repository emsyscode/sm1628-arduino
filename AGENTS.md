# Working-panel development rules

The source of truth is the user-supplied working sketch under reference/WorkingPanel.
SHA-256: 4E9521DC761201B3C6FEB347DB51230398487458F13151BA866038D02293B9B4.
Do not edit that file. Keep its byte-level identity checked in CI.

The exact accepted comparison is firmware/LegacyPanelDemo. Active PanelDemo is now the explicitly corrected numeric-only physical-order loop; ManualMap slot is clean, and legacy-slot reproduces inherited icon bits.
Keep sender timing, command order, pattern bytes, brightness, scan-slot order,
per-frame blanking and register setup until a particular change has passed a
separate hardware comparison. Label any untested replacement plainly.
Do not infer a panel connector map or physical digit order from generic fonts.

The reference's whole-port setup owns Uno D0..D13; never silently combine it
with joystick/button connections. Remove that setup only as an explicit,
separately tested change.
No disabled-by-default example guards in the primary baseline demonstration.
Distinguish "creator reports original works", "refactor trace matches", and
"creator has tested refactor on hardware".
Do not add game, key reading, clock demo or generalized family support until
the display baseline is accepted. Preserve attribution and original sources.

Creator-authorized correction2026-10-26: normal numeral paths must not copy inherited second-byte icon masks. Preserve exact payloads only in the untouched reference, LegacyPanelDemo and explicit legacy-slot reproduction. Raw symbol-column tests remain legitimate. Explain setup, first command, expected result, and whether indices mean raw columns or physical digits in source and external docs. See docs/symbol-bit-correction.md.

