# Retired model-tested code

The prior generated driver/library/examples are preserved in model-only-v0.2/
and Git history. They are not the active implementation or recommended upload.

The creator reported those examples do not work on the actual panel.
Their simulation tests verified expectations chosen for that implementation,
not compatibility with the newly supplied working sketch.

Known differences include disabled-by-default example guards, lower brightness,
a different startup/per-frame sequence, generic/raw example patterns and a
six-digit map from a different historical board. These are candidate causes,
not a proven single root cause. We do not know exactly which prior build/settings
were tested on hardware.

The replacement is built from the supplied working sketch, with a byte-for-byte
reference and a compiled comparison against its actual output.
