# ffmpegParser

It's a parser from ffmpeg command to [graphviz](https://graphviz.or). Some of ffmpeg's command are really complicated not only for debug, but for reading. 

This tool was created to simplify process of debugging by visualization of filter graph

# Restrictions 

1. You have to use -f option for any output files

For example `ffmpeg <inputs> <filters> <output setup #1> -f <format#1> <output#1> <output setup #2> -f <format#2> <output#2> ...`

2. You have to map all outputs(except filter chain case)
 
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
