# ffmpegParser

It's a parser from ffmpeg command to [graphviz](https://graphviz.or). Some of ffmpeg's command are really complicated
not only for debug, but for reading.

This tool was created to simplify process of debugging by visualization of filter graph

# Restrictions

1. Output files format
    1. `-f` option(format: `-f <format> <output>`)
    2. Filename with `.`. In this case you should add all options after which comes word with `.` in config file.
2. You have to map all outputs(except filter chain case)
3. All file's names in filter chain/graph should be in format: `[name]`

# How to use

## Config file

1. `mode:` you can write `dot_parse` mod to parse output files with `.`
2. `bad_options:` write all mapping/global options(use space separator) which comes word with `.`(but not output files)
3. `log_options:` write all filter options(use space separator) which are writing log in file

## Ffmpeg to graphviz `text`

Build ffmpegParser(needed files you can see at CmakeLists) and then run

`./ffmpegParser file_with_command.txt config.txt`

You will get graphviz formatted output in stdout

## Ffmpeg to graphviz `.svg`

You should install graphviz, use

`brew install graphviz`(MacOS)

`sudo apt install graphviz`(Ubuntu)

Build ffmpegParser(needed files you can see at CmakeLists) and then run

`./ffmpegParser file_with_command.txt config.txt | dot -Tsvg > output.svg`

You will get picture `.svg` with graph

