#!/bin/bash
# Generate the EPUB cover: the Y-chart (Figure 1) with title and author.
# Runs inside the eda-tools Docker image (ImageMagick + DejaVu fonts), see Makefile.
set -e
cd "$(dirname "$0")"

INPUT_IMAGE="../src/images/figure-01.png"
OUTPUT_IMAGE="../src/images/cover.jpg"

if command -v magick >/dev/null 2>&1; then IM=magick; else IM=convert; fi

if [ ! -f "$INPUT_IMAGE" ]; then
    echo "Error: input image not found: $INPUT_IMAGE"
    exit 1
fi

$IM -size 1050x1600 xc:"#f4f1ea" \
    -fill "#1f3a5f" -draw "rectangle 0,0 1050,520" \
    \( "$INPUT_IMAGE" -resize 820x -fuzz 8% -transparent white \) -gravity center -geometry +0+170 -composite \
    -fill "#f4f1ea" -font DejaVu-Sans-Bold -pointsize 78 \
    -gravity north -annotate +0+120 "Encapsulating Tools\ninto an\nEDA Framework" \
    -fill "#1f3a5f" -font DejaVu-Sans -pointsize 54 \
    -gravity south -annotate +0+170 "Olav Schettler" \
    -fill "#5a5a5a" -font DejaVu-Serif-Italic -pointsize 32 \
    -annotate +0+110 "Dissertation, TU Delft, 1995" \
    -quality 90 "$OUTPUT_IMAGE"

echo "Cover created: $OUTPUT_IMAGE"
