# /// script
# requires-python = ">=3.11"
# dependencies = ["pymupdf"]
# ///
"""Convert the normalised thesis PDF (FrameMaker, one book page per PDF page) into AsciiDoc.

Usage: uv run scripts/pdf2adoc.py work/pages.pdf src

Run scripts/normalize.py first; it splits the 4-up pages of thesis.pdf.
"""
import collections
import re
import sys
import unicodedata
from pathlib import Path

import pymupdf

FIRST_CONTENT_PAGE = 9      # chapter 1; title page, committee, TOC and Stellingen are hand-written
HEADER_MAX_Y = 105
FIG_DPI = 200
FOOT_MARKS = "*†‡§"
# tables with arrows, shading, code or nested rows: rendered like figures
TABLE_IMAGES = {3, 5, 8, 11, 13}

LIGATURES = {"ﬁ": "fi", "ﬂ": "fl", "ﬀ": "ff", "ﬃ": "ffi", "ﬄ": "ffl"}


def fix_text(t):
    for k, v in LIGATURES.items():
        t = t.replace(k, v)
    return t


class Span:
    __slots__ = ("text", "font", "size", "x0", "x1", "y0", "y1")

    def __init__(self, s):
        self.text = fix_text(s["text"])
        self.font = s["font"].split("+")[-1]
        self.size = round(s["size"], 1)
        self.x0, self.y0, self.x1, self.y1 = s["bbox"]

    @property
    def italic(self):
        return "Italic" in self.font or "Oblique" in self.font

    @property
    def bold(self):
        return "Bold" in self.font

    @property
    def helv(self):
        return self.font.startswith("Helvetica")

    @property
    def mark(self):
        return self.text.strip() in FOOT_MARKS and self.text.strip() != "" and 10.5 < self.size < 11.5


class Line:
    def __init__(self, spans):
        self.spans = sorted(spans, key=lambda s: s.x0)
        self.x0 = min(s.x0 for s in spans if not s.mark) if any(not s.mark for s in spans) else spans[0].x0
        self.x1 = max(s.x1 for s in spans)
        main = max(spans, key=lambda s: (len(s.text.strip()) > 0, s.size, len(s.text)))
        self.y = main.y1
        self.y0 = min(s.y0 for s in spans)
        self.font, self.size = main.font, main.size

    @property
    def text(self):
        return "".join(s.text for s in self.spans)

    def chars(self, pred):
        return sum(len(s.text.strip()) for s in self.spans if pred(s))

    def kind(self):
        text = self.text.strip()
        f, sz = self.font, self.size
        allbold_helv = all(s.font.startswith("Helvetica-Bold") for s in self.spans if s.text.strip())
        if f.startswith("Helvetica-Bold") and sz > 17:
            return "title"
        if f.startswith("Helvetica-Bold") and 15.5 < sz < 17:
            return "section"
        if allbold_helv and 13.5 < sz < 14.5 and re.match(r"\d+\.\d+\.\d+\s", text):
            return "subsection"
        if self.spans[0].font.startswith("Times-BoldItalic") and re.match(r"(Figure|Table|Algorithm)\s+\d+", text):
            return "caption"
        if 11.5 < sz < 12.5 and f.startswith("Times") or (self.spans[0].size < 10 and self.spans[0].text.strip()[:1] in FOOT_MARKS
                                                           and self.spans[0].text.strip()[1:] == "."):
            return "foot"
        if self.spans[0].font.startswith("Helvetica-Bold") and 11.5 < self.spans[0].size < 12.5:
            return "item"
        helv = self.chars(lambda s: s.helv)
        if helv and helv >= self.chars(lambda s: not s.helv):
            return "code"
        return "body"


def page_lines(page, header=False):
    spans = []
    for b in page.get_text("dict", flags=0)["blocks"]:
        if b["type"]:
            continue
        for l in b["lines"]:
            for s in l["spans"]:
                if s["text"].strip() or s["text"] == " ":
                    spans.append(Span(s))
    marks = [s for s in spans if s.mark]
    spans = [s for s in spans if not s.mark]
    groups = []
    # base-size text first, so that smaller marks and indices attach to an existing line
    for s in sorted(spans, key=lambda s: (s.size < 11, s.y1)):
        tol = 4.0
        for g in groups:
            ref = max(g, key=lambda x: x.size)
            if abs(ref.y1 - s.y1) < tol or (s.size < ref.size - 2 and ref.y0 - 1 < s.y1 < ref.y1 + 4):
                g.append(s)
                break
        else:
            groups.append([s])
    for s in marks:
        # footnote marks sit above their base line
        cands = [g for g in groups if -1.5 < max(x.y1 for x in g) - s.y1 < 9 and any(x.x1 <= s.x0 + 2 or x.x0 >= s.x1 - 2 for x in g)]
        if cands:
            min(cands, key=lambda g: max(x.y1 for x in g) - s.y1).append(s)
        else:
            groups.append([s])
    lines = [Line(g) for g in groups]
    if not header:
        lines = [l for l in lines if l.y > HEADER_MAX_Y]
    lines.sort(key=lambda l: (round(l.y), l.x0))
    return lines


def render_spans(spans, base_italic=False, footnotes=True, inline_code=True):
    """Render a sequence of spans to (style, text) parts."""
    out = []
    prev = None
    base = max((s for s in spans if not s.mark), key=lambda s: (s.size, len(s.text)), default=None)
    for s in spans:
        t = s.text
        if prev is not None and s.x0 - prev.x1 > 1.5 and not prev.text.endswith(" ") and not t.startswith(" "):
            out.append(("plain", " "))
        prev = s
        if s.mark:
            out.append(("fnref", t.strip()) if footnotes else ("plain", ""))
            continue
        if base is not None and s.size < base.size - 2 and t.strip():
            if s.y1 > base.y1 + 1.5:
                out.append(("raw", "~" + t.strip() + "~"))
                continue
            if s.y1 < base.y1 - 2:
                out.append(("raw", "^" + t.strip() + "^"))
                continue
        if s.helv and inline_code and not s.bold:
            style = "code"
        elif s.italic != base_italic and s.font.startswith("Times"):
            style = "em"
        elif s.bold and s.font.startswith("Times"):
            style = "strong"
        elif s.helv and s.bold and inline_code:
            style = "strongcode"
        else:
            style = "plain"
        out.append((style, t))
    return out


def escape(t):
    # only a brace that would start an attribute reference needs escaping
    return re.sub(r"\{(?=[\w-]+\})", r"\\{", t)


def fmt(parts):
    """Merge styled parts into AsciiDoc."""
    merged = []
    for style, t in parts:
        if merged and merged[-1][0] == style and style not in ("fnref", "raw"):
            merged[-1] = (style, merged[-1][1] + t)
        elif merged and style == "plain" and t == " " and merged[-1][0] not in ("fnref", "raw"):
            merged[-1] = (merged[-1][0], merged[-1][1] + t)
        else:
            merged.append((style, t))
    res = []
    for style, t in merged:
        if style == "plain":
            res.append(escape(t))
        elif style == "raw":
            res.append(t)
        elif style == "fnref":
            if res:
                res[-1] = res[-1].rstrip()
            res.append(f"\x00FN{t}\x00")
        else:
            lead = t[: len(t) - len(t.lstrip())]
            trail = t[len(t.rstrip()):]
            core = t.strip()
            if not core:
                res.append(t)
                continue
            if style == "code":
                res.append(lead + f"``+{core}+``" + trail)
            elif style == "strongcode":
                res.append(lead + f"**``+{core}+``**" + trail)
            else:
                mark = "__" if style == "em" else "**"
                res.append(lead + f"{mark}{escape(core)}{mark}" + trail)
    return "".join(res)


def code_line(line, minx):
    """Render a code line with indentation and bold keywords (for a [subs=+quotes] listing)."""
    indent = max(0, round((line.x0 - minx) / 7.5))
    out = []
    prev = None
    for s in line.spans:
        t = s.text
        if prev is not None and s.x0 - prev.x1 > 1.5 and not prev.text.endswith(" ") and not t.startswith(" "):
            out.append(" ")
        prev = s
        if s.mark:
            out.append("\x00FN" + t.strip() + "\x00")
            continue
        # quotes run before attributes, so marks hidden in attributes stay literal
        t = (t.replace("{", "{lbrace}").replace("*", "{asterisk}").replace("_", "{us}")
             .replace("#", "{hash}").replace("^", "{caret}").replace("~", "{tilde}")
             .replace("`", "{backtick}").replace("+", "{plus}"))
        if s.bold and t.strip():
            core = t.strip()
            t = t[: len(t) - len(t.lstrip())] + f"**{core}**" + t[len(t.rstrip()):]
        elif s.italic and t.strip():
            core = t.strip()
            t = t[: len(t) - len(t.lstrip())] + f"__{core}__" + t[len(t.rstrip()):]
        out.append(t)
    text = "".join(out).strip()
    text = re.sub(r"\*\*( *)\*\*", r"\1", text)
    return " " * indent + text


def join_lines(texts, words, hyph):
    """Join raw line texts with dehyphenation."""
    out = ""
    for t in texts:
        t = t.strip()
        if not t:
            continue
        if not out:
            out = t
            continue
        mm = re.search(r"\w-(__|\*\*)$", out)
        if mm and t.startswith(mm.group(1)):
            out, t = out[:-len(mm.group(1))], t[len(mm.group(1)):]
        m = re.search(r"(\w+)-$", out)
        if m and not out.endswith("--"):
            w1 = m.group(1)
            w2m = re.match(r"[_*]*(\w+)", t)
            w2 = w2m.group(1) if w2m else ""
            joined = (w1 + w2).lower()
            keep = False
            if w1[-1:].isdigit():
                keep = True
            elif w2[:1].isupper():
                # names like "Franzis-Verlag" keep the hyphen, split words like "IDENTI-FIER" do not
                keep = words[joined] == 0
            elif words[joined] == 0 and hyph[(w1 + "-" + w2).lower()] > 0:
                keep = True
            out = out + t if keep else out[:-1] + t
        else:
            out = out + " " + t
    return out


def collect_vocab(doc):
    words, hyph = collections.Counter(), collections.Counter()
    for page in doc:
        for line in fix_text(page.get_text()).splitlines():
            body = re.sub(r"\w+-$", "", line)
            for w in re.findall(r"[A-Za-zÀ-ÿ]+", body):
                words[w.lower()] += 1
            for w in re.findall(r"[A-Za-zÀ-ÿ]+-[A-Za-zÀ-ÿ]+", body):
                hyph[w.lower()] += 1
    return words, hyph


def graphics(page):
    # white filled rectangles are FrameMaker frame backgrounds, not part of a figure
    rects = [pymupdf.Rect(d["rect"]) for d in page.get_drawings()
             if not (d["type"] == "f" and d.get("fill") and min(d["fill"]) > 0.98)]
    rects += [pymupdf.Rect(i["bbox"]) for i in page.get_image_info()]
    return [r for r in rects if r.y1 > HEADER_MAX_Y and r.width * r.height > 0 or (r.width > 20 or r.height > 20)]


def two_columns(lines, mid=300):
    """Reading order for a two-column page: the left column first, then the right one."""
    left, right = [], []
    for l in lines:
        a = [s for s in l.spans if s.x0 < mid]
        b = [s for s in l.spans if s.x0 >= mid]
        if a:
            left.append(Line(a))
        if b:
            r = Line(b)
            r.y += 2000     # events are ordered by y: put the right column after the left one
            r.y0 += 2000
            right.append(r)
    return left + right


def is_flow(line, margin):
    """Paragraph text that may sit next to a figure but never belongs to it."""
    return (line.font.startswith("Times") and 13.5 < line.size < 14.5 and abs(line.x0 - margin) < 30
            and line.x1 - line.x0 > 250)


def figure_region(page, lines, cap, top_limit, margin):
    """Bounding box of the figure drawn above caption line `cap`."""
    g = [r for r in graphics(page) if r.y1 <= cap.y0 + 1 and r.y0 >= top_limit - 1 and r.height < 600]
    if not g:
        return None
    box = pymupdf.Rect(min(r.x0 for r in g), min(r.y0 for r in g), max(r.x1 for r in g), max(r.y1 for r in g))
    changed = True
    while changed:
        changed = False
        for l in lines:
            if l.y >= cap.y0 + 1 or l.y0 < top_limit or l.kind() in ("caption", "title", "section", "subsection"):
                continue
            if is_flow(l, margin) and not (box.y0 + 5 < l.y < box.y1 - 2):
                continue
            for s in l.spans:
                r = pymupdf.Rect(s.x0, s.y0, s.x1, s.y1)
                near = pymupdf.Rect(box.x0 - 40, box.y0 - 18, box.x1 + 40, box.y1 + 18)
                if near.intersects(r) and not box.contains(r):
                    box |= r
                    changed = True
    return box


def table_region(page, lines, cap, top_limit, below=False):
    """Box, row rules and column separators of the ruled table above (below, for algorithms) the caption."""
    g = graphics(page)
    lo, hi = (cap.y - 2, 900) if below else (top_limit - 1, cap.y0 + 1)
    verts = [r for r in g if r.width < 3 and r.height > 10 and lo <= r.y0 and r.y1 <= hi]
    if below and verts:
        # the table directly below the caption: vertical rules starting close to it
        first = min(v.y0 for v in verts)
        verts = [v for v in verts if v.y0 < first + 40]
    elif verts:
        # the table closest to the caption
        last = max(v.y1 for v in verts)
        verts = [v for v in verts if v.y1 > last - 300 and v.y1 > min(x.y1 for x in verts if x.y1 > last - 60) - 300]
    if not verts:
        return None
    box = pymupdf.Rect(min(v.x0 for v in verts), min(v.y0 for v in verts), max(v.x1 for v in verts), max(v.y1 for v in verts))
    rules = sorted((r for r in g if r.height < 3 and r.width > 30 and box.y0 - 3 <= r.y0 <= box.y1 + 3
                    and r.x0 >= box.x0 - 3 and r.x1 <= box.x1 + 3), key=lambda r: r.y0)
    ys = []
    for r in rules:
        # merge double rules
        if not ys or r.y0 - ys[-1] > 4:
            ys.append(r.y0)
        else:
            ys[-1] = r.y0
    xs = sorted({round(v.x0) for v in verts if v.x0 > box.x0 + 5 and v.x1 < box.x1 - 5})
    seps = []
    for x in xs:
        if not seps or x - seps[-1] > 5:
            seps.append(x)
    return box, ys, seps


def build_table(lines, box, ys, seps):
    """Turn the text inside a ruled table into rows of cells, split at the vertical rules."""
    inside = [l for l in lines if box.y0 - 1 <= l.y - 3 <= box.y1 + 1 and box.x0 - 3 <= l.x0 <= box.x1]
    rows = collections.defaultdict(lambda: collections.defaultdict(list))
    for l in inside:
        row = sum(1 for y in ys if y < l.y - 3) - 1
        for s in l.spans:
            col = sum(1 for x in seps if x < (s.x0 + s.x1) / 2)
            rows[row][col].append(s)
    rows = {k: v for k, v in rows.items() if k >= 0}
    if not rows:
        return None, set()
    ncol = len(seps) + 1
    table = []
    for k in sorted(rows):
        cells = []
        for c in range(ncol):
            spans = rows[k].get(c, [])
            lines_ = collections.defaultdict(list)
            for s in spans:
                lines_[round(s.y1 / 4)].append(s)
            cells.append([fmt(render_spans(sorted(v, key=lambda s: s.x0), inline_code=False)).strip()
                          for _, v in sorted(lines_.items())])
        header = all(s.helv for c in rows[k].values() for s in c if s.text.strip())
        table.append((header, cells))
    return table, {id(l) for l in inside}


def slug(s):
    s = unicodedata.normalize("NFKD", s).encode("ascii", "ignore").decode().lower()
    return re.sub(r"[^a-z0-9]+", "-", s).strip("-")


class Chapter:
    def __init__(self, title, mode="chapter"):
        self.title = title
        self.mode = mode          # chapter | bibliography | glossary
        self.blocks = []
        self.footnotes = {}       # (page, mark) -> [text parts]


def convert(pdf_path, out_dir):
    doc = pymupdf.open(pdf_path)
    img_dir = Path(out_dir) / "images"
    img_dir.mkdir(parents=True, exist_ok=True)
    words, hyph = collect_vocab(doc)
    chapters = []
    ch = None
    para = None
    pending = []          # figures waiting for the current paragraph to end

    def flush():
        nonlocal para
        if para:
            ch.blocks.append(para)
            para = None
        while pending:
            ch.blocks.append(pending.pop(0))

    for pno in range(FIRST_CONTENT_PAGE, len(doc) + 1):
        page = doc[pno - 1]
        lines = page_lines(page)
        if not lines and not graphics(page):
            continue
        if any(l.kind() == "title" and l.text.strip() == "Acronyms" for l in lines) or ch is not None and ch.mode == "glossary":
            lines = two_columns(lines)
        body = [l for l in lines if l.kind() == "body" and l.font.startswith("Times")]
        # text margin: leftmost full-width body line (list-heavy pages have most lines indented)
        full = [l.x0 for l in body if l.x1 - l.x0 > 250]
        verso = any(s.x0 < 100 and s.text.strip().isdigit() for s in sum((x.spans for x in page_lines(page, header=True)
                                                                         if x.y <= HEADER_MAX_Y), []))
        margin = min(full + [70.9 if verso else 56.7])
        right = max((l.x1 for l in body), default=margin + 468)

        # figures, tables, algorithms on this page
        skip = set()
        objects = []      # (y, block)
        top_limit = HEADER_MAX_Y
        for i, l in enumerate(lines):
            if l.kind() != "caption":
                continue
            cap_lines = [l]
            j = i + 1
            while j < len(lines) and lines[j].y - cap_lines[-1].y < 20 and lines[j].kind() in ("body", "foot") \
                    and any(s.italic for s in lines[j].spans) and not lines[j].spans[0].bold \
                    and abs(lines[j].x0 - l.x0) < 4:
                cap_lines.append(lines[j])
                j += 1
            label = re.match(r"(Figure|Table|Algorithm)\s+(\d+)", l.text.strip())
            kind, num = label.group(1), int(label.group(2))
            parts = []
            for cl in cap_lines:
                spans, prev = [], None
                for s in cl.spans:
                    if prev is not None and s.x0 - prev.x1 > 25 and not s.italic:
                        break
                    spans.append(s)
                    prev = s
                parts.append(fmt(render_spans(spans, base_italic=True, inline_code=False)))
            caption = fix_text(join_lines(parts, words, hyph)).strip()
            caption = re.sub(r"^\*\*(.*?)\*\*", r"\1", caption)
            caption = re.sub(r"\s+", " ", caption)
            skip.update(id(x) for x in cap_lines)
            ident = f"{kind.lower()}-{num}"
            if kind == "Figure" or kind == "Table" and num in TABLE_IMAGES:
                box = figure_region(page, lines, l, top_limit, margin)
                if box is None:
                    print(f"warning: no graphics for {kind} {num} on p{pno}", file=sys.stderr)
                    objects.append((l.y0 - 1, dict(kind="title-only", id=ident, caption=caption)))
                else:
                    clip = pymupdf.Rect(box.x0 - 4, box.y0 - 4, box.x1 + 4, min(box.y1 + 4, l.y0 - 1))
                    fname = f"{kind.lower()}-{num:02d}.png"
                    page.get_pixmap(dpi=FIG_DPI, clip=clip).save(img_dir / fname)
                    for x in lines:
                        if id(x) in skip:
                            continue
                        if x.kind() not in ("title", "section", "subsection") and all(box.x0 - 2 <= s.x0 and s.x1 <= box.x1 + 2 and box.y0 - 2 <= s.y0 and s.y1 <= box.y1 + 2
                               for s in x.spans):
                            skip.add(id(x))
                    objects.append((box.y0, dict(kind="figure", id=ident, file=fname, caption=caption,
                                                 width=round(clip.width))))
            else:
                reg = table_region(page, lines, l, top_limit, below=(kind == "Algorithm"))
                tab, used = (build_table([x for x in lines if id(x) not in skip], *reg) if reg else (None, set()))
                if tab is None:
                    print(f"warning: no table for {kind} {num} on p{pno}", file=sys.stderr)
                    objects.append((l.y0 - 1, dict(kind="title-only", id=ident, caption=caption)))
                else:
                    skip.update(used)
                    prev = next((b for b in reversed(ch.blocks + pending + [o for _, o in objects])
                                 if b["kind"] == "table"), None)
                    if prev is not None and prev["id"] == ident:
                        # continued on the next page: drop the repeated header
                        prev["rows"] += [r for r in tab if not r[0]]
                        top_limit = cap_lines[-1].y + 8
                        continue
                    y = l.y0 if kind == "Algorithm" else reg[0].y0
                    objects.append((y, dict(kind="table", id=ident, caption=caption, rows=tab)))
            top_limit = cap_lines[-1].y + 8     # below the rule under the caption

        flow = [l for l in lines if id(l) not in skip]
        events = sorted([(l.y0, 0, l) for l in flow] + [(y, 1, o) for y, o in objects], key=lambda e: (e[0], e[1]))

        code = None
        for _, is_obj, item in events:
            if is_obj:
                if item["kind"] == "title-only" or para is None or item["kind"] == "table" and item["id"].startswith("algorithm"):
                    if code:
                        ch.blocks.append(code)
                        code = None
                    flush()
                    ch.blocks.append(item)
                else:
                    pending.append(item)
                continue
            l = item
            k = l.kind()
            text = l.text.strip()
            if k != "code" and code is not None:
                ch.blocks.append(code)
                code = None
            if k == "title" and any(x.y < l.y - 12 and x.kind() != "title" for x in flow):
                print(f"info: inline heading p{pno}: {text}", file=sys.stderr)
                # a heading in chapter size inside the text ("Three principles:")
                flush()
                ch.blocks.append(dict(kind="section", level=4, text=text))
                continue
            if k == "title":
                flush()
                if ch is not None and not ch.blocks and pno == ch.page:
                    ch.title += " " + text
                    continue
                mode = {"References": "bibliography", "Acronyms": "glossary"}.get(text, "chapter")
                ch = Chapter(text, mode)
                ch.page = pno
                chapters.append(ch)
                continue
            if ch is None:
                continue
            if ch.mode == "bibliography":
                if k == "foot" or k == "body":
                    t = fmt(render_spans(l.spans, inline_code=False)).strip()
                    if re.fullmatch(r"\[[^\]]+\]", t):
                        ch.blocks.append(dict(kind="bib", key=t[1:-1], lines=[]))
                    elif ch.blocks:
                        ch.blocks[-1]["lines"].append(t)
                continue
            if ch.mode == "glossary":
                t = fmt(render_spans(l.spans, inline_code=False)).strip()
                if l.spans[0].bold:
                    ch.blocks.append(dict(kind="term", term=t.strip("*"), lines=[]))
                elif ch.blocks:
                    ch.blocks[-1]["lines"].append(t)
                continue
            if k == "section":
                flush()
                ch.blocks.append(dict(kind="section", level=2, text=text))
            elif k == "subsection":
                flush()
                ch.blocks.append(dict(kind="section", level=3, text=text))
            elif k == "foot":
                spans = l.spans
                first = spans[0].text.strip()
                if spans[0].size < 10 and first[:1] in FOOT_MARKS:
                    key = (pno, first[0])
                    ch.footnotes[key] = []
                    ch.last_fn = key
                    spans = spans[1:]
                if not getattr(ch, "last_fn", None):
                    print(f"warning: orphan footnote text p{pno}: {text[:60]}", file=sys.stderr)
                    continue
                ch.footnotes[ch.last_fn].append(fmt(render_spans(spans, inline_code=True)))
            elif k == "code" and not (l.spans[0].bold and abs(l.x0 - margin) < 3 and len(text) > 8
                                       and all(s.bold for s in l.spans if s.text.strip())):
                flush()
                if code is None:
                    code = dict(kind="code", lines=[], minx=l.x0, last_y=l.y)
                if code["lines"] and l.y - code["last_y"] > 24:
                    code["lines"].append((None, None))
                code["lines"].append((l, pno))
                code["last_y"] = l.y
                code["minx"] = min(code["minx"], l.x0)
            elif k == "code":
                # unnumbered run-in heading: all Helvetica-Bold at the margin
                flush()
                ch.blocks.append(dict(kind="section", level=4, text=text))
            else:
                parts = render_spans(l.spans)
                rendered = fmt(parts)
                indent = l.x0 - margin
                if k == "item":
                    flush()
                    marker = l.spans[0].text.strip()
                    para = dict(kind="item", marker=marker, lines=[re.sub(r"^\*\*" + re.escape(escape(marker)) + r"\*\*\s*|^``\+.*?\+``\s*", "",
                                                                        fmt(render_spans(l.spans[1:])))], x=l.spans[1].x0 if len(l.spans) > 1 else l.x0,
                                y=l.y)
                    continue
                if text.startswith("•"):
                    flush()
                    para = dict(kind="item", marker="•", lines=[rendered.lstrip("• ").lstrip()], x=l.x0 + 12, y=l.y, indent=indent)
                    continue
                if l.spans[0].font.startswith("Times-Bold") and re.match(r"(Definition|Property):", text):
                    flush()
                    para = dict(kind="definition", lines=[rendered], y=l.y)
                    continue
                gap = l.y - para["y"] if para else 99
                same_page = para is not None and para.get("page", pno) == pno
                if para and para["kind"] == "item" and (l.x0 > margin + 10 or not same_page and l.x0 > margin + 5) \
                        and (gap < 20 or not same_page):
                    para["lines"].append(rendered)
                    para["y"], para["page"] = l.y, pno
                    continue
                if para and para["kind"] == "definition" and indent > 40 and (gap < 20 or not same_page):
                    para["lines"].append(rendered)
                    para["y"], para["page"] = l.y, pno
                    continue
                quote = indent > 18 and l.x1 < right - 10
                if quote and not (para and para["kind"] == "item"):
                    if para is None or para["kind"] != "quote" or (gap > 24 and same_page):
                        flush()
                        para = dict(kind="quote", lines=[])
                    para["lines"].append(rendered)
                    para["y"], para["page"] = l.y, pno
                    continue
                new_par = (para is None or para["kind"] != "para" or 8 < indent < 18
                           or (same_page and gap > 21))
                if new_par:
                    flush()
                    para = dict(kind="para", lines=[])
                para["lines"].append(rendered)
                para["y"], para["page"] = l.y, pno
        if code is not None:
            ch.blocks.append(code)
            code = None
    flush()
    return chapters, words, hyph


CITE = re.compile(r"\[([^\[\]<>\\]{2,60})\\?\]")     # also "[Key\]" inside footnote macros


def make_cite(keys):
    def cite(text, in_macro=False):
        def rep(m):
            items = re.split(r",\s*|;\s*", m.group(1))
            if not all(i.strip() in keys for i in items):
                return m.group(0)
            inner = ", ".join(f"<<bib-{slug(i.strip())},{i.strip()}>>" for i in items)
            return "[" + inner + ("\\]" if in_macro else "]")
        return CITE.sub(rep, text)
    return cite


def guard_block_start(t):
    if re.match(r"(\d+\.|[*.\-=]|[A-Za-z]\.)\s", t) or t.startswith((".", "'''", "[", "//", "|")):
        return "{empty}" + t
    return t


def write(chapters, words, hyph, out_dir):
    out_dir = Path(out_dir)
    chap_dir = out_dir / "chapters"
    chap_dir.mkdir(parents=True, exist_ok=True)
    keys = {b["key"] for c in chapters if c.mode == "bibliography" for b in c.blocks}
    cite = make_cite(keys)
    includes = []
    back = 0
    for idx, ch in enumerate(chapters, 1):
        fn = ch.footnotes

        def resolve(text, page_hint=None):
            def rep(m):
                mark = m.group(1)
                cands = [k for k in fn if k[1] == mark]
                if not cands:
                    print(f"warning: missing footnote {mark} in {ch.title}", file=sys.stderr)
                    return ""
                key = cands[0]
                body = fix_text(join_lines(fn.pop(key), words, hyph))
                body = cite(body.replace("]", "\\]"), in_macro=True)
                return f"footnote:[{body}]"
            return re.sub("\x00FN(.)\x00", rep, text)

        out = []
        num = re.match(r"(\d+)\.\s", ch.title)
        if ch.mode == "bibliography":
            out += ["[bibliography]", f"= {ch.title}", ""]
            for b in ch.blocks:
                t = fix_text(join_lines(b["lines"], words, hyph))
                out.append(f"* [[[bib-{slug(b['key'])},{b['key']}]]] {t}")
            out.append("")
            name = "90-references.adoc"
        elif ch.mode == "glossary":
            out += ["[glossary]", f"= {ch.title}", ""]
            for b in ch.blocks:
                out += [f"{b['term']}::", "  " + fix_text(join_lines(b["lines"], words, hyph)), ""]
            name = "91-acronyms.adoc"
        else:
            if num:
                name = f"{int(num.group(1)):02d}-{slug(ch.title[num.end():])}.adoc"
                out += [f"[#ch-{int(num.group(1))}]", f"= {ch.title}", ""]
            else:
                back += 1
                name = f"{91 + back:02d}-{slug(ch.title.split('.')[0] if ch.title.startswith('Appendix') else ch.title)}.adoc"
                out += [f"= {ch.title}", ""]
            attach = False      # the previous block was a list item (or attached to one)
            for b in ch.blocks:
                k = b["kind"]
                if attach and k in ("code", "quote") and out and out[-1] == "":
                    out[-1] = "+"   # list continuation: listing or indented text belongs to the item
                    if k == "quote" and not b["lines"][0].startswith("__"):
                        k = "para"
                else:
                    attach = False
                attach = attach or k == "item"
                if k == "section":
                    if b["level"] == 4:
                        out += ["[discrete]", f"==== {b['text']}", ""]
                    else:
                        out += ["=" * b["level"] + f" {b['text']}", ""]
                elif k == "figure":
                    out += [f"[#{b['id']}]", f".{resolve(cite(b['caption']))}",
                            f"image::{b['file']}[\"{re.sub(r'[_*`+]|\x00FN.\x00', '', b['caption']).replace(chr(34), chr(39))}\"]", ""]
                elif k == "title-only":
                    out += [f"// TODO: {b['id']} not detected automatically", f"[#{b['id']}]", f".{resolve(cite(b['caption']))}", "****", "****", ""]
                elif k == "table":
                    rows = b["rows"]
                    ncol = max(len(c) for _, c in rows)
                    opts = ["%header"] if rows and rows[0][0] else []
                    out += [f"[#{b['id']},{','.join(opts + [f'cols={ncol}*'])}]" if opts else f"[#{b['id']},cols={ncol}*]",
                            f".{resolve(cite(b['caption']))}", "|==="]
                    for _, cells in rows:
                        cells = cells + [[]] * (ncol - len(cells))
                        out.append(" ".join("| " + resolve(cite(join_lines(c, words, hyph))).replace("|", "\\|") for c in cells))
                    out += ["|===", ""]
                elif k == "code":
                    ls = b["lines"]
                    body = []
                    for l, p in ls:
                        if l is None:
                            body.append("")
                        else:
                            body.append(resolve(code_line(l, b["minx"])))
                    subs = "+quotes,+attributes" + (",+macros" if any("footnote:[" in x for x in body) else "")
                    out += [f'[listing,subs="{subs}"]', "----", *body, "----", ""]
                elif k == "item":
                    t = fix_text(join_lines(b["lines"], words, hyph))
                    m = b["marker"]
                    if m == "•":
                        out += [f"* {resolve(cite(t))}", ""]
                    elif re.fullmatch(r"\d+\.", m):
                        n = int(m[:-1])
                        prev = next((x for x in reversed(out) if x), "")
                        if n != 1 and not prev.startswith(". ") and not out[-2:-1] == [""] or n != 1 and not any(
                                x.startswith(". ") for x in out[-6:]):
                            out += [f"[start={n}]"]
                        out += [f". {resolve(cite(t))}", ""]
                    else:
                        out += [f"{m}:: {resolve(cite(t))}", ""]
                elif k == "definition":
                    t = fix_text(join_lines(b["lines"], words, hyph))
                    out += ["[.definition]", resolve(cite(t)), ""]
                elif k == "para":
                    t = fix_text(join_lines(b["lines"], words, hyph))
                    out += [guard_block_start(resolve(cite(t))), ""]
                elif k == "quote":
                    t = fix_text(join_lines(b["lines"], words, hyph))
                    out += ["[quote]", "____", resolve(cite(t)), "____", ""]
            if fn:
                print(f"warning: unused footnotes {sorted(fn)} in {ch.title}", file=sys.stderr)
        text = "\n".join(out)
        text = re.sub(r"(?<=[^\s_])__( *)__(?=[^\s_])", r"\1", text)
        text = re.sub(r"\+``( *)``\+", r"\1", text)
        (chap_dir / name).write_text(text.rstrip() + "\n", encoding="utf-8")
        includes.append(name)
    (out_dir / "chapters.txt").write_text("\n".join(includes) + "\n")


if __name__ == "__main__":
    pdf, out = sys.argv[1], sys.argv[2]
    chapters, words, hyph = convert(pdf, out)
    write(chapters, words, hyph, out)
    print(f"{len(chapters)} chapters")
