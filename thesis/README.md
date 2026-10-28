# Encapsulating Tools into an EDA Framework — AsciiDoc / EPUB edition

Olav Schettler, *Encapsulating Tools into an EDA Framework*. Proefschrift, Technische Universiteit Delft, 13 September 1995.
Also published as: Oldenbourg, 1996, ISBN 3-486-23985-6.

The FrameMaker sources are lost. This project turns the PDF `thesis.pdf` into AsciiDoc and builds an EPUB 3 from it.
The structure follows the build setup of *Systemic Relativism* (`../../Patrick_Dewilde_Systemic_Relativism`).

## Layout

```
thesis.pdf                original PDF (143 pages, partly 4-up), made from ps/*.ps
ps/                       original PostScript files per chapter
src/00-master.adoc        master document (metadata, title page, includes)
src/chapters/*.adoc       Stellingen, chapters 1-8, references, acronyms, appendix, back matter
src/images/               figures, image tables, cover
build/Makefile            build targets (Docker)
build/Dockerfile          Asciidoctor build image
build/Dockerfile.tools    eda-tools image: poppler, ImageMagick, DejaVu fonts
build/generate-cover.sh   cover with title overlay (ImageMagick)
scripts/normalize.py      splits the 4-up pages into single book pages (PyMuPDF, run with uv)
scripts/pdf2adoc.py       one-off PDF -> AsciiDoc converter (PyMuPDF, run with uv)
scripts/qa_words.py       vocabulary check of the AsciiDoc against the PDF text
work/                     intermediate files (work/pages.pdf), not under version control
output/                   build results
```

## Build

Requirements: Docker, make, and [uv](https://docs.astral.sh/uv/) (only for `make convert` and `make qa`).

```bash
cd build
make epub       # -> output/encapsulating-tools.epub (increments build-number.txt)
make validate   # EPUBCheck
make html       # -> output/encapsulating-tools.html
make help
```

## How the conversion works

Chapter 2 (book pages 18-73) and References/Acronyms/Appendix (pages 163-176) are printed 4-up in `thesis.pdf`.
`scripts/normalize.py` cuts these pages into quadrants and scales them to the geometry of the 1-up pages,
so that `work/pages.pdf` has one book page per PDF page (185 pages, book pages 3-183).
The Samenvatting and Summary (pages 177-180) are not contained in the PDF.

`scripts/pdf2adoc.py` then reads font and position information:

| PDF                                        | AsciiDoc                                          |
|--------------------------------------------|---------------------------------------------------|
| Helvetica-Bold 18 `1. Introduction`        | chapter (`= 1. Introduction`, numbers kept literally) |
| Helvetica-Bold 16 / 14 `1.1`, `1.2.1`      | `==` / `===` sections                             |
| unnumbered Helvetica-Bold 14 run-in heads  | `[discrete]` `====`                               |
| Times italic / bold in body                | `__…__` / `**…**`                                 |
| Helvetica code with bold keywords          | `[listing,subs="+quotes,+attributes"]` with `**keyword**` |
| `*` / `†` footnotes (per page)             | inline `footnote:[…]`                             |
| `[Katz 86]` citations                      | `<<bib-katz-86,Katz 86>>` links into the references |
| `Figure N.` captions + vector drawings     | `image::figure-NN.png[]`, rendered from the page at 200 dpi |
| `Table N.` with ruled grid                 | AsciiDoc table (columns from the vertical rules)  |
| Tables 3, 5, 8, 11, 13                     | rendered as images (arrows, shading, code, nested rows) |
| `Definition:` / `Property:`                | `[.definition]` paragraph                         |
| `1.` / `R1` / `•` items                    | AsciiDoc lists                                    |

Line-end hyphens are removed unless the hyphenated form occurs elsewhere in the book.

After the automatic conversion the text was checked word by word against the PDF, and fixes were made directly in `src/chapters/*.adoc`
(e.g. Table 4 with row spans, the uncaptioned summary table in Section 7.5, listings split at page breaks, genuine compounds such as *demand-loadable*).
**Running `make convert` again discards these fixes.**
