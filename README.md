# Discord-Fixer
Disables threads responsible for calling GetRawInputData for Discord, Discord PTB and Discord Canary desktop clients.

[Research Part by AMIT](https://twitter.com/amitxv/status/1636094504905179138)

## Usage

1. Download the latest release.
2. Locate the directory in which `Discord.exe`, `DiscordPTB.exe` and `DiscordCanary.exe` is located and place the executable in it.
3. Launch the executable, this will also launch the relevant executable for Discord.

### How does it work?
1. The program attempts to locate a valid `Discord.exe`, `DiscordPTB.exe` and `DiscordCanary.exe` file.
2. Once found, the file will be launched.
3. The program will be iterate through the process list until it finds any threads that use the following module: `discord_utils.node`.
4. Once found, the relevant threads will be suspended.

# Building
1. Install the latest version of [`GCC`](https://winlibs.com/) and [`UPX`](https://upx.github.io/) for optional compression.
2. Run `build.bat`.

## Credits

- [Amit](https://twitter.com/amitxv) * for the scientific analysis of the problem and the idea
- [Aetopia](https://github.com/Aetopia) *thnx for fixing my shizz and rewriting it better <3
- [Me(Sarah)](https://github.com/PrincessAkira) * fixing code from aetopia and making it work :3
