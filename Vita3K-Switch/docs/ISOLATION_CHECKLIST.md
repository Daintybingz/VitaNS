# VitaNS Crash Isolation Checklist

- [ ] Step 1: Minimal Main (SDL2 window only)
  - [ ] App runs without crash
  - [ ] early_log.txt created

- [ ] Step 2: Add Logging
  - [ ] Logging initialized
  - [ ] Log file created and contains messages

- [ ] Step 3: Add ImGui UI
  - [ ] ImGui window appears
  - [ ] No crash

- [x] Step 4: Add Filesystem Setup
  - [x] Directories created
  - [x] Log success/failure

- [x] Step 5: Add Emulator Core (no firmware/game)
  - [x] Emulator core initializes
  - [x] No crash

- [x] Step 6: Add Firmware Loading
  - [x] Firmware loads
  - [x] No crash

- [x] Step 7: Add Game Loading
  - [x] Game loads
  - [x] No crash

- [x] Step 8: Full Feature Test
  - [x] All features enabled
  - [x] App stable 