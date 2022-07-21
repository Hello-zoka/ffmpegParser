# ffmpegParser

It's a parser from ffmpeg command to [graphviz](https://graphviz.or)

# How to use

## Ffmpeg to graphviz `text`

Build ffmpegParser(needed files you can see at CmakeLists) and then run 

`./ffmpegParser file_with_command.txt`

You will get graphviz formated output in stdout

## Ffmpeg to graphviz `.svg`

You should install graphviz, use 

`brew install graphviz`(MacOS)

`sudo apt install graphviz`(Ubuntu)

Build ffmpegParser(needed files you can see at CmakeLists) and then run 

`./ffmpegParser file_with_command.txt | dot -Tsvg > output.svg`

You will get picutre `.svg` with graph
