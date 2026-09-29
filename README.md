# hl2dm-sp

Source SDK 2013 game code for the singleplayer campaigns, built as mods that run on **Half-Life 2: Deathmatch** base (x64).

Features:
 - Stable 2026 SP-only (due to engine patches) Source branch, with stock HL games working out of the box.
 - Sound channel bug fixed.
 - All 224 SM3 constants available (stock SDK only has 32 exposed)

## Build instructions

Requirements:
 - Half-Life 2: Deathmatch installed via Steam.
 
Optionally (for the example mods):
 - Half-Life 2 Complete installed. I think Lost Coast too, I can't remember if it's merged by now.

### Windows

Visual Studio 2022 with the v143 x64 toolset, and Python 3. Inside `src`, run
`createallprojects.bat`, then build the `client_win64_<game>`, `server_win64_<game>` and
`game_shader_generic_example_win64_<game>` projects for `Release|x64`. The DLLs land in
`game/<mod>/bin/x64`.

### Linux

podman. Inside `src`, run `./buildallprojects release`.

## Run

```bat
<Half-Life 2: Deathmatch>\hl2mp_win64.exe -game "<checkout>\game\mod_ep2"
```
