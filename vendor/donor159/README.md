# Donor source transport

The user-supplied ThanLong Clean Route v1.5.9 donor controller/bridge is split into exact `.inc` parts and stored in ZIP chunks only because the GitHub connector cannot upload the ~192 KB controller file directly. CMake extracts every archive into `build/generated/donor159` and the thin wrappers in `src/` include the parts in original order.

This is a transport detail, not a second source of truth or a logic rewrite.

SHA-256 of archive payloads used for grouped chunks:
- controller_04_05.zip: `ac6c5986ffda792d5c7542ea5c19fe19dc9a80ae1cd773a9536542e9a7d8f5ad`
- controller_06_07.zip: `1ad740ae6a788790354e0e3e2504f53997e079a4b84e8853e31d3b74666292f3`
- controller_08_09.zip: `26f919ac79748f145205d5e7c2a161b649ab215047ce007dd15443fa862c22fe`
- controller_10_11.zip: `56c5c88d9fd74a3835188d1bdd4c29f6acc9491ae878a7aa520bb6b5d36b4947`
- bridge_00_01.zip: `e6d3d5a2e52803f1b56a56148c9ba084c9a3871f9a39302d8d30aba4706a200a`
