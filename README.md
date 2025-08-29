# PiePerseus

I won't maintain this anymore as my AL accounts (`4pii4` and `pie guild alt` on Little Enterprise have been banned permanently)

"Development environment" to test your Perseus modifications in under one minute.

## Features and goals:
- [x] No chat/name censorship
- [x] Config file in `json` (instead of archaic `.ini`)
- [x] Chat commands
- [x] Remove low morale warning
- [x] Higher auto repeat limit
- [ ] Skip animations
  - [x] Skip battleship firing animation
- [ ] Hide ~~potentially~~ NSFW images/screens
  - [x] Hide loading background images
- [x] Remove hard mode stat/formation requirements
- [ ] Commander name/id/oil/coin/gem spoof
  - [x] Commander name/id/level spoof
- [x] Convert any ship blueprint to protocore ([unimplemented feedback](https://discord.com/channels/456397991847395339/468804193646542849/1241706652261159022))

I do read AL's feedback channel, so if you have an interesting suggestion you can DM `pi.kt` on discord.

## Documentation

* [Chat commands](docs/chat_commands.md)
* [Configuration](docs/configuration.md)

## Requirements:
* Python with f-string support
* `java` in path
* Android Studio with the following components installed and in `PATH`:
  * NDK: for `ndk-build[.cmd]`
  * Android SDK: for `zipalign[.exe]` and `apksigner[.bat]`

## Usage
```
$ python perseus.py --help
usage: perseus apk builder [-h] [--skip | --no-skip] [--quick-rebuild | --no-quick-rebuild] [--clean | --no-clean]

builds apk for you (this is the default behaviour if called with no arguments)

options:
  -h, --help            show this help message and exit
  --skip, --no-skip     skip decompile and extracting if possible
  --quick-rebuild, --no-quick-rebuild
                        rebuild apk by replacing libs in the apk instead of using apktool (saves 40s)
  --clean, --no-clean   delete built apk, decompiled sources, compiled perseus libs and xapk
```

The first build takes 3 to 5 minutes, subsequent builds take 15-20 seconds.

If the built APK doesn't work for you and you're on Windows, try running the script inside Git Bash/MSYS.


To quote the original creator of Perseus:

![image](https://image.pieland.xyz/file/672d4d4be9f333cb70c49.png)

