# Makefile for "Encapsulating Tools into an EDA Framework" (Olav Schettler)
# Builds EPUB and HTML from the AsciiDoc sources using Docker

# Variables
PROJECT_NAME = encapsulating-tools
SRC_DIR = ../src
OUT_DIR = ../output
MASTER_DOC = $(SRC_DIR)/00-master.adoc
BUILD_NUMBER_FILE = build-number.txt
PDF_SOURCE = thesis.pdf

# Docker
DOCKER_IMAGE = asciidoctor/docker-asciidoctor:latest
DOCKER_RUN = docker run --rm -v "$(CURDIR)/..":/documents -w /documents/build $(DOCKER_IMAGE)
TOOLS_IMAGE = eda-tools
TOOLS_RUN = docker run --rm -v "$(CURDIR)/..":/documents -w /documents $(TOOLS_IMAGE)

# Default target
.PHONY: all
all: epub
	@echo "All formats built"

# Create directories
.PHONY: dirs
dirs:
	@mkdir -p $(OUT_DIR)

# Increment build number
.PHONY: increment-build
increment-build:
	@if [ -f $(BUILD_NUMBER_FILE) ]; then \
		BUILD_NUM=$$(cat $(BUILD_NUMBER_FILE)); \
		NEW_BUILD_NUM=$$((BUILD_NUM + 1)); \
		echo $$NEW_BUILD_NUM > $(BUILD_NUMBER_FILE); \
		echo "Build number: $$BUILD_NUM -> $$NEW_BUILD_NUM"; \
	else \
		echo "1" > $(BUILD_NUMBER_FILE); \
		echo "Build number initialised: 1"; \
	fi

# Pull Docker image
.PHONY: docker-pull
docker-pull:
	@docker pull $(DOCKER_IMAGE)

# Build the tools image (poppler, ImageMagick, fonts)
.PHONY: tools-image
tools-image:
	@docker build -q -f Dockerfile.tools -t $(TOOLS_IMAGE) .

# Re-extract text and figures from the PDF (overwrites src/chapters!)
# Python runs locally via uv.
.PHONY: convert
convert:
	@mkdir -p ../work
	@cd .. && uv run scripts/normalize.py $(PDF_SOURCE) work/pages.pdf
	@cd .. && uv run scripts/pdf2adoc.py work/pages.pdf src
	@echo "AsciiDoc sources regenerated in $(SRC_DIR)/chapters"

# Vocabulary check of the AsciiDoc sources against the PDF text
.PHONY: qa
qa:
	@cd .. && uv run scripts/qa_words.py work/pages.pdf src/chapters

# Generate the cover image with text overlay
.PHONY: generate-cover
generate-cover: tools-image
	@echo "Generating cover..."
	@$(TOOLS_RUN) bash build/generate-cover.sh

# Build EPUB
.PHONY: epub
epub: dirs increment-build generate-cover
	@echo "Building EPUB..."
	@BUILD_NUM=$$(cat $(BUILD_NUMBER_FILE)); \
	$(DOCKER_RUN) asciidoctor-epub3 \
		-D $(OUT_DIR) \
		-o $(PROJECT_NAME).epub \
		-a ebook-format=epub3 \
		-a build-number=$$BUILD_NUM \
		$(MASTER_DOC)
	@echo "EPUB created: $(OUT_DIR)/$(PROJECT_NAME).epub"

# Validate EPUB with EPUBCheck (bundled with the asciidoctor-epub3 gem)
.PHONY: validate
validate:
	@$(DOCKER_RUN) asciidoctor-epub3 \
		-D /tmp -o check.epub -a ebook-format=epub3 \
		-a ebook-validate \
		$(MASTER_DOC)

# Build HTML
.PHONY: html
html: dirs
	@echo "Building HTML..."
	@$(DOCKER_RUN) asciidoctor \
		-D $(OUT_DIR) \
		-o $(PROJECT_NAME).html \
		-a toc=left \
		-a toclevels=2 \
		$(MASTER_DOC)
	@echo "HTML created: $(OUT_DIR)/$(PROJECT_NAME).html"

# Statistics
.PHONY: stats
stats:
	@echo "Chapters: $$(ls $(SRC_DIR)/chapters/*.adoc | wc -l)"
	@echo "Words:    $$(cat $(SRC_DIR)/chapters/*.adoc | wc -w)"

# Clean up
.PHONY: clean
clean:
	@rm -rf $(OUT_DIR)
	@echo "Output directory removed"

# Help
.PHONY: help
help:
	@echo "Targets:"
	@echo ""
	@echo "  all             - Build EPUB"
	@echo "  epub            - Build EPUB (increments build number)"
	@echo "  validate        - Run EPUBCheck on the EPUB"
	@echo "  html            - Build HTML"
	@echo "  generate-cover  - Generate cover with title overlay"
	@echo "  convert         - Re-extract AsciiDoc from thesis.pdf (overwrites src/chapters!)"
	@echo "  qa              - Compare the vocabulary of the AsciiDoc with the PDF"
	@echo "  docker-pull     - Pull/update the Asciidoctor Docker image"
	@echo "  tools-image     - Build the eda-tools Docker image"
	@echo "  stats           - Show statistics"
	@echo "  clean           - Remove output directory"
	@echo ""
	@echo "Current build number: $$(cat $(BUILD_NUMBER_FILE) 2>/dev/null || echo '0')"
