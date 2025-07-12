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

- [ ] Step 4: Add Filesystem Setup
  - [ ] Directories created
  - [ ] Log success/failure

- [ ] Step 5: Add Emulator Core (no firmware/game)
  - [ ] Emulator core initializes
  - [ ] No crash

- [ ] Step 6: Add Firmware Loading
  - [ ] Firmware loads
  - [ ] No crash

- [ ] Step 7: Add Game Loading
  - [ ] Game loads
  - [ ] No crash

- [ ] Step 8: Full Feature Test
  - [ ] All features enabled
  - [ ] App stable 