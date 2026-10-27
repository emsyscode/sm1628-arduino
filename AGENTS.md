# Tutorial maintenance

Preserve the working Uno implementation and attribution to José Luis Monteiro.
The source under reference/WorkingPanel is immutable; its SHA-256 remains
4E9521DC761201B3C6FEB347DB51230398487458F13151BA866038D02293B9B4.
The exact comparator is tests/fixtures/LegacyPanelDemo, not a user example.

Keep sender timings, packet bytes/order, brightness, blanking and port/register
setup until a separately justified hardware comparison supports a change.
Do not silently change input pin ownership while cleaning source formatting.
START=D4, active-buzzer transistor control=D11,DATA=D7,CLK=D8,STB=D9,A0/A1 axes.
Never infer a connector pin order, hidden circuit node or panel map from a
generic part. Follow the engineer explanation, exact datasheets and measurements.

Teach identification -> wiring -> first output -> mapping -> memory -> projects.
Keep tutorials sequential, with exact commands and expected results. Raw RAM
columns, physical digits and engineer Grid labels are different namespaces.
Comments explain purpose and constraints; status/history belongs in tests/docs,
not temporary 'untested' source comments. Use readable multiline formatting.
Keep runtime logic separate from rendering and protocol; avoid new abstractions
that make the small examples harder to follow. Keep Arduino folders self-contained.

Run relevant compiled AVR/GPIO/UART tests for source changes. Compiles and
simulation are not optical/electrical hardware validation. Keep developer build
results in docs/validation.md, not the main tutorial narrative. Do not add failed
historical drivers, production scripts, raw footage or internal filming notes.
Retain independent original sources and code credits even when pruning history.

Creator direction: this is Jose Luis Monteiro's updated standalone driver
tutorial, developed/tested with TidusM, not a third-party adaptation. Use current
authorship wording, not 'based on his code'. Preserve immutable regression
fixtures without putting development history in the beginner path. Teach both
SM1628 and SM1628B from the outset. Distinguish shared programming, variant
hardware differences and panel-specific mapping. Keep Snake optional.
