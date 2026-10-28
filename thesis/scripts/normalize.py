# /// script
# requires-python = ">=3.11"
# dependencies = ["pymupdf"]
# ///
"""Split the 4-up pages of thesis.pdf into single book pages.

Chapters 2-4 and References/Acronyms/Appendix were printed 4-up (2x2, scale 0.45).
The result has one book page per PDF page, all in the geometry of the 1-up pages.

Usage: uv run scripts/normalize.py thesis.pdf work/pages.pdf
"""
import sys

import pymupdf

SCALE = 0.45                       # 6.3pt 4-up body text vs. 14pt 1-up
HEADER_Y = (71.4, 460.8)           # header baseline top in the upper / lower row
LEFT_X, RIGHT_X = 58.7, 527.6      # left edge of verso, right edge of recto (4-up)
VERSO_X, RECTO_X = 70.9, 524.4     # the same edges on 1-up pages
TOP_Y = 84.6


def is_4up(page):
    sizes = [round(s["size"], 1) for b in page.get_text("dict")["blocks"] if not b["type"]
             for l in b["lines"] for s in l["spans"] if s["text"].strip()]
    return bool(sizes) and max(set(sizes), key=sizes.count) < 7


def main(src_path, out_path):
    src = pymupdf.open(src_path)
    out = pymupdf.open()
    w, h = src[0].rect.width, src[0].rect.height
    for page in src:
        if not is_4up(page):
            out.new_page(width=w, height=h).show_pdf_page(pymupdf.Rect(0, 0, w, h), src, page.number)
            continue
        for row in (0, 1):
            for col in (0, 1):
                clip = pymupdf.Rect(col * w / 2, row * h / 2, (col + 1) * w / 2, (row + 1) * h / 2)
                if not page.get_text("text", clip=clip).strip() and not [
                        d for d in page.get_drawings() if clip.contains(d["rect"])]:
                    continue
                k = 1 / SCALE
                if col == 0:
                    x0 = VERSO_X + (clip.x0 - LEFT_X) * k
                else:
                    x0 = RECTO_X - (RIGHT_X - clip.x0) * k
                y0 = TOP_Y + (clip.y0 - HEADER_Y[row]) * k
                target = pymupdf.Rect(x0, y0, x0 + clip.width * k, y0 + clip.height * k)
                out.new_page(width=w, height=h).show_pdf_page(target, src, page.number, clip=clip)
    out.save(out_path, garbage=3, deflate=True)
    print(f"{len(src)} pages -> {len(out)} pages")


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
