# v0.2.2 patch transport manifest

This folder stores only transport chunks for `controller_v022.patch` because the GitHub connector cannot safely accept the full patch in one opaque payload.

Concatenate every `part.*` file in lexical filename order. Expected SHA-256 of the reconstructed patch:

`65bafda2c9980f67c1202be01ca1391bd68b73685cc451e8b8130c8c80ddb32b`

The patch is applied only after the exact v0.2.1 generated controller has been verified. Expected final v0.2.2 controller SHA-256 after LF normalization:

`732fcfdd6ab497b1f1da442ec94b63a5f63a2d5757a8fcb8b5c9ee9efc5a1066`

The chunks are a transport detail, not independent logic or alternate source files. `tools/rehydrate_v159.ps1` is the authoritative reconstruction/checksum pipeline.
