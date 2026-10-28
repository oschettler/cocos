# /// script
# dependencies = ["pymupdf"]
# ///
"""List words in the AsciiDoc sources that do not occur in the PDF text, and vice versa.

Usage: uv run scripts/qa_words.py work/pages.pdf src/chapters
"""
import collections
import re
import sys
from pathlib import Path

import pymupdf

LIG = {"ﬁ": "fi", "ﬂ": "fl", "ﬀ": "ff", "ﬃ": "ffi", "ﬄ": "ffl"}
ATTRS = {"{us}": "_", "{asterisk}": "*", "{hash}": "#", "{caret}": "^", "{tilde}": "~",
         "{backtick}": "`", "{plus}": "+", "{lbrace}": "{", "\\{": "{"}


def norm(t):
    for k, v in LIG.items():
        t = t.replace(k, v)
    return t


doc = pymupdf.open(sys.argv[1])
pdf = norm("\n".join(p.get_text() for p in doc[8:]))
pdf_join = re.sub(r"(\w)-\n(\w)", r"\1\2", pdf)
pdf_keep = re.sub(r"(\w)-\n(\w)", r"\1-\2", pdf)


def words(t):
    return collections.Counter(re.findall(r"[A-Za-zÀ-ÿ][A-Za-zÀ-ÿ'’-]*[A-Za-zÀ-ÿ]", t))


pw = words(pdf_join) + words(pdf_keep)
adoc = "\n".join(p.read_text() for p in sorted(Path(sys.argv[2]).glob("*.adoc")) if not p.name.startswith("00-"))
for k, v in ATTRS.items():
    adoc = adoc.replace(k, v)
adoc = re.sub(r"<<bib-[^,]+,([^>]+)>>", r"\1", adoc)
adoc = re.sub(r"\[\[\[bib-[^,]+,([^\]]+)\]\]\]", r"\1", adoc)
adoc = re.sub(r"footnote:|image::\S+\[\"|\[#[^\]]*\]|\[listing[^\]]*\]|\[discrete\]|\[\.definition\]|__|\*\*|``\+|\+``|%header|cols=\d\*", " ", adoc)
aw = words(adoc)
print("In AsciiDoc, not in PDF:")
for w, n in sorted(aw.items()):
    if w not in pw:
        print(f"  {w} ({n})")
print("In PDF, not in AsciiDoc (excluding figure labels may be expected):")
for w, n in sorted(words(pdf_join).items()):
    if w not in aw and w not in words(adoc.replace("-", "")) and n > 0:
        print(f"  {w} ({n})")
