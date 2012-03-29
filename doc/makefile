DIAGRAMS=$(shell ls diagrams/*.svg | sed -re "s/[.]svg/.pdf/g")

BIB=refs.bib

TEX=report.tex \
    abstract.tex \
    thanks.tex \
    a4-mancs.sty \
    third-rep.cls \
    appendix1.tex \
    appendix2.tex \
    appendix3.tex \
    appendix4.tex \
    appendix5.tex \
    appendix6.tex \
    chapter1.tex \
    chapter2.tex \
    chapter3.tex \
    chapter4.tex \
    chapter5.tex \
    chapter6.tex

%.pdf : %.svg
	inkscape -D -A "${@}" "${<}"

report.pdf : ${TEX} ${DIAGRAMS} ${BIB}
	pdflatex -interaction=nonstopmode report.tex
	bibtex report.aux
	pdflatex -interaction=nonstopmode report.tex
	pdflatex -interaction=nonstopmode report.tex

clean :
	@rm -rf report.pdf *.aux *.log *.fls *.bbl *.blg *.toc *.lof *.lot
	@rm -rf *.bbl
	@rm -rf *.blg
	@rm -rf diagrams/*.pdf

all : report.pdf
