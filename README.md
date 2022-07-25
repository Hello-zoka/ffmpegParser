# ffmpegParser

It's a parser from ffmpeg command to [graphviz](https://graphviz.or). Some of ffmpeg's command are really complicated not only for debug, but for reading. 

This tool was created to simplify process of debugging by visualization of filter graph

# Restrictions 

1. Output files format 
   1. `-f` option(format: `-f <format> <output>`)
   2. Filename with `.`. In this case you should set first line of config in `true` state and add all options after which comes word with `.` in config file.
2. You have to map all outputs(except filter chain case)
3. All file's names in filter chain/graph should be in format: `[name]`
 
# How to use
## Ffmpeg to graphviz `text`

Build ffmpegParser(needed files you can see at CmakeLists) and then run 

`./ffmpegParser file_with_command.txt`

You will get graphviz formatted output in stdout

## Ffmpeg to graphviz `.svg`

You should install graphviz, use 

`brew install graphviz`(MacOS)

`sudo apt install graphviz`(Ubuntu)

Build ffmpegParser(needed files you can see at CmakeLists) and then run 

`./ffmpegParser file_with_command.txt | dot -Tsvg > output.svg`

You will get picutre `.svg` with graph
