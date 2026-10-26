# Untouched hardware reference

WorkingPanel/ contains the user-supplied .ino, preserved byte for byte.
Creator reports hardware success; no independent hardware test by the assistant.

The source configures all Uno D0..D13 as outputs. Its active behavior is a
repeating numeral test. Unused functions include legacy bugs; do not enable them
merely because the file is retained as a working display reference.

Original SHA-256:
4E9521DC761201B3C6FEB347DB51230398487458F13151BA866038D02293B9B4

Use the clean firmware/PanelDemo for the refactor candidate and the untouched
original for a same-bench comparison. Neither file is currently a Snake game.
