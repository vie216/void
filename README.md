# Void

A simple and minimalistic text editor written in C with Suckless-style configuration.

THIS SOFTWARE IS UNFINISHED!!!

## Building

### Unix

```shell
./build.sh
./void src/config.h
```

## Keybindings

All of them can be changed in the [configuration file](src/config.h).

| Key                    | Action                        |
|------------------------|-------------------------------|
| Ctrl+Q                 | quit                          |
| Ctrl+S                 | save                          |
| Left/Right Arrows      | move left/right one character |
| Up/Down Arrows         | move up/down one line         |
| Ctrl+Left/Right Arrows | move left/right one word      |
| Ctrl+Up/Down Arrows    | move up/down one paragraph    |
| Alt+Left/Right Arrows  | move to the line start/end    |
| Alt+Up/Down Arrows     | move to the file start/end    |
