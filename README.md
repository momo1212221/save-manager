# Save Manager

Safe controller-accessible save tools for RAC1.

- L2 + R2 + Up/Down: browse the game's real save slots.
- L2 + R2 + Select: inspect the selected slot (planet, bolts, gold bolts, active marker).
- L2 + R2 + Square: request an autosave to the game's current active save slot.

The selected inspection slot is never treated as a write target. OpenRAC 1.11 save_slot_current() identifies the actual save file the running game belongs to. A new game that has never been saved must first use the normal Save Game flow before manual saving is enabled.

Built for OpenRAC RAC1 PAL/NTSC-U. AI-assisted implementation. GPL-3.0-or-later.
