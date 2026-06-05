![AzerothCore](https://img.shields.io/badge/AzerothCore-Module-blue.svg)
![License: AGPL v3](https://img.shields.io/badge/License-AGPLv3-blue.svg)
![WoW Version](https://img.shields.io/badge/WotLK-3.3.5a-FFCC00.svg)

# mod-talentmaster

AzerothCore Module — Free Talent Reset for Regular Players

Lets regular player accounts reset their talents (and Hunter pet talents) at
any time, at no gold cost, through a chat command and/or a custom NPC gossip
menu.

---

## Features

### Chat Commands

Available to all players (no GM account needed):

| Command | Effect |
|---------|--------|
| `.talent reset` | Refunds all talent points for free |
| `.talent resetpet` | Refunds all Hunter pet talent points for free |

Both commands are blocked while in combat.

### NPC Gossip

Attach the gossip script to any creature in `creature_template` and players
can interact with it to reset talents through a two-step confirmation menu.
The NPC entry ID is set in the config — you can use a new custom creature or
repurpose an existing one.

---

## Installation

1. Clone into your AzerothCore modules directory:

   ```
   git clone https://github.com/n70n10/mod-talentmaster modules/mod-talentmaster
   ```

2. Rebuild:

   ```bash
   cd build && cmake .. && make -j$(nproc)
   ```

3. Copy the config:

   ```bash
   cp modules/mod-talentmaster/conf/mod_talentmaster.conf.dist \
      configs/mod_talentmaster.conf
   ```

4. Restart the worldserver.

---

## NPC Setup

### Option A — create a new custom NPC

As GM in-game, or via SQL:

```sql
-- Create a minimal creature_template entry
INSERT INTO creature_template
    (entry, name, subname, minlevel, maxlevel, faction, npcflag, ScriptName)
VALUES
    (190010, 'Talent Master', 'Talents & Specialization',
     1, 80, 35, 1, 'TalentMasterGossipScript');
```

Then spawn it in the world:

```
.npc add 190010
```

Set `TalentMaster.NPC.Entry = 190010` in `mod_talentmaster.conf`.

### Option B — attach to an existing NPC

```sql
UPDATE creature_template
SET ScriptName = 'TalentMasterGossipScript'
WHERE entry = <your_entry>;
```

Set `TalentMaster.NPC.Entry = <your_entry>` in `mod_talentmaster.conf`.

---

## Configuration

```ini
[TalentMaster]

# Enable the .talent chat commands.
TalentMaster.Command.Enable = 1

# Enable the NPC gossip menu.
TalentMaster.NPC.Enable = 1

# creature_template entry of the Talent Master NPC (0 = disabled).
TalentMaster.NPC.Entry = 0
```

---

## Notes

- Talent resets refund all points; redistribution happens through the
  normal talent UI.
- Resets are blocked while the player is in combat.
- The module logs a warning at startup if the NPC is enabled but the entry
  ID is `0` or not found in `creature_template`.
