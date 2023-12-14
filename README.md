# Discord-Fixer
Disables threads responsible for calling GetRawInputData for Discord, Discord PTB and Discord Canary desktop clients.

[Research Part by AMIT](https://twitter.com/amitxv/status/1636094504905179138)

## Usage

1. Download the latest release.
2. Locate the correct Directory for your Discord Client.
3. Copy the executable to the folder where the Update.exe is located.
4. Launch the executable, this will also launch the relevant executable for Discord.

## How to autostart the fixer

1. Remove Discord from autostart or disable it in the settings.
2. Preferably install [OpenAsar](https://github.com/GooseMod/OpenAsar)
3. Create a shortcut of DiscordFixer.exe and copy it into **shell:startup**
4. Execute Shortcut. It will now start DiscordFixer.exe and apply the fix after Discord starts with a delay of 8 seconds.

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
