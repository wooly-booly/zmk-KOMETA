# AGENTS.md

## Project

This repository is the ZMK user configuration and custom shield definition for the 42-key wireless split KOMETA keyboard. The physical keyboard is documented at <https://github.com/inpudiy/KOMETA>.

The intended controller is a nice!nano-compatible nRF52840 board. The known hardware reports:

- model: nice!nano;
- UF2 board ID: `nRF52840-nicenano`;

Treat this UF2 information as hardware diagnostics, not as a reason to change the build target. The authoritative target is `build.yaml`, currently `nice_nano@2.0.0//zmk`.

## Sources of truth

Use repository files in this order:

1. The current working-tree files, especially `config/kometa.keymap` and `config/kometa.conf`.
2. `build.yaml`, the shield files under `boards/shields/kometa/`, and `.github/workflows/main.yml`.
3. The upstream KOMETA hardware repository for physical construction details.
4. Historical layout descriptions and old commits only as background.

## Repository map

- `config/kometa.keymap`: behaviors, layer order, and all 42 bindings per layer.
- `config/kometa.conf`: ZMK feature flags, including pointing and split battery reporting.
- `config/kometa.json`: physical-layout metadata used by Nick Coutsos Keymap Editor.
- `config/west.yml`: ZMK manifest; it currently tracks ZMK `main`.
- `build.yaml`: the three CI build targets: `kometa_left`, `kometa_right`, and `settings_reset`.
- `boards/shields/kometa/kometa.dtsi`: shared 4-row by 12-column transform and 42-key physical layout.
- `boards/shields/kometa/kometa_left.overlay`: left-half matrix pins.
- `boards/shields/kometa/kometa_right.overlay`: right-half matrix pins and six-column offset.
- `boards/shields/kometa/Kconfig.defconfig`: split configuration; the left half is central.
- `.github/workflows/main.yml`: delegates firmware builds to ZMK's reusable user-config workflow.

## Keymap invariants

- Each layer must have exactly 42 bindings in physical order: positions `0..11`, `12..23`, `24..35`, then thumbs `36..41`.
- The six outer-column positions are `0`, `11`, `12`, `23`, `24`, and `35`. Preserve them as `&none` on every layer unless the user explicitly requests otherwise.
- Layer numbers are positional. After adding, deleting, or reordering a layer, update every numeric reference in `&lt`, `&mo`, custom hold-taps, comments, and any related documentation in the same change.
- Keep `config/kometa.keymap`, `config/kometa.json`, and the physical layout in `kometa.dtsi` consistent. Do not change matrix coordinates or GPIO assignments as part of a layout-only request.
- Preserve compatibility with Nick Coutsos Keymap Editor. Keep conventional ZMK keymap nodes, `display-name` values, and custom behaviors readable; avoid unnecessary generated rewrites.

## Behavior and safety rules

- Preserve the current urob-style positional home-row-mod behavior unless the task is specifically about tuning it. Verify both left and right trigger-position lists after changing the matrix or physical order.
- Do not reintroduce `&to 0` merely to leave a momentary layer; releasing the originating `&lt`/`&mo` already returns to the prior layer.
- Never flash hardware, clear Bluetooth profiles, or use `settings_reset`.

## Editing guidelines

- Make the smallest change that satisfies the request and preserve unrelated user edits.
- Use valid devicetree syntax: balanced angle brackets, correct binding-cell counts, and semicolons after properties and nodes where required.
- Prefer named ZMK key definitions from the existing `dt-bindings` headers over raw HID values.
- Do not change `config/west.yml` from ZMK `main`, pin a ZMK revision, alter the board revision, or rewrite shield GPIOs unless the task explicitly requires it.
- Because the manifest tracks ZMK `main`, distinguish an upstream compatibility failure from a local keymap error before changing behavior.

## Validation

Do not build firmware locally for validation. Do not run `west build`, launch Docker-based ZMK builds, or download build dependencies. Firmware compilation is handled by GitHub Actions after the user pushes the changes.

For every change, run at least:

```sh
git diff --check
git diff -- AGENTS.md build.yaml config boards/shields/kometa .github/workflows/main.yml
```

For keymap changes, also verify:

- every layer still has 42 bindings;
- layer indices and all numeric layer references agree;
- both halves retain the intended matrix mapping;
