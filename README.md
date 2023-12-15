# Discord-Fixer
Disables threads responsible for calling GetRawInputData for Discord, Discord PTB and Discord Canary desktop clients.

[Research Part by AMIT](https://twitter.com/amitxv/status/1636094504905179138)

## Usage

1. Download the latest [Release](https://github.com/PrincessAkira/Discord-Fixer/releases).
2. Locate where Discord Stable/PTB/Canary is installed and place this executable.
3. Launch the downloaded executable, this will also launch the relevant executable for Discord.

### How does it work?
1. The program attempts to locate a valid `Discord.exe`, `DiscordPTB.exe` and `DiscordCanary.exe` file.
2. Once the correct file was located, the located file will be launched.
3. Using [WinEvent](https://learn.microsoft.com/en-us/windows/win32/winauto/what-are-winevents) [Hooks](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwineventhook), the process waits until a window with the class name of `raw_input` is created.
4. The process that is hosting the `raw_input` window will have its threads iterated and if any thread is using `discord_utils.node`, it will be suspended.

# Building
1. Install the latest version of [`GCC`](https://winlibs.com/) and [`UPX`](https://upx.github.io/) for optional compression.
2. Run `build.bat`.

## Credits

- [Amit](https://twitter.com/amitxv) * for the scientific analysis of the problem and the idea
- [Aetopia](https://github.com/Aetopia) * thx for fixing my shizz and rewriting it better <3
- [Me(Sarah)](https://github.com/PrincessAkira) * fixing code from aetopia and making it work :3
