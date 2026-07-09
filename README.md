# jaPRO

jaPRO is a Jedi Academy multiplayer mod started by loda.

This branch is maintained in sync with [TaystJK](https://github.com/taysta/TaystJK), which is itself an OpenJK fork. New jaPRO features may be developed here and may later be incorporated into TaystJK, while this repo can also pull in unrelated engine, client, and platform changes from TaystJK.

The previous jaPRO history from before the TaystJK rebase is preserved on the `legacy` branch.

## What jaPRO Does

jaPRO focuses on competitive multiplayer, defrag, full force combat, admin tools, tribes gamemode, with fully configurable gameplay.

- Player accounts and stat database
- Improved netcode with lag compensation
- Multiple duel types, including saber, force, and gun duel support
- Full-featured race mode with records, rankings, race commands, and mapper support
- Elo ranking support for duels
- Improved weapon, saber, force, and movement balancing
- Skill-based grapple hook and custom jetpack physics
- Advanced bot AI options
- Improved vote and admin systems
- Server-side demo recording and logging support
- Client HUD tools for race, movement, speed, keys, strafe helper, and visual feedback

Most gameplay systems are controlled through server cvars and admin commands so servers can choose the style they want instead of using one fixed ruleset.

## Documentation

- [jaPRO cvars and commands](docs/japro_docs.md)
- [Defrag mapping guide](docs/Defrag%20Mapping%20Guide.md)
- [Developer docs](docs/developer)

The cvar documentation is the best overview of the mod's priorities: race/accounts, duel rules, movement physics, saber/force/weapon tuning, admin tools, bot behavior, logging, demo recording, and client HUD options.

## Repository Layout

- `main` - current jaPRO development branch, kept in sync with TaystJK
- `legacy` - preserved pre-rebase jaPRO branch
- `upstream` - expected to point at `https://github.com/taysta/TaystJK.git`

## Building

This codebase uses the OpenJK/TaystJK CMake build structure.

General OpenJK build references:

- [OpenJK compilation guide](https://github.com/JACoders/OpenJK/wiki/Compilation-guide)
- [OpenJK debugging guide](https://github.com/JACoders/OpenJK/wiki/Debugging)

Exact output names, packaging, and runtime setup may still inherit TaystJK/OpenJK naming in places while jaPRO is rebased onto the newer base.

## License

OpenJK and derived forks are licensed under GPLv2. See [LICENSE.txt](LICENSE.txt).
