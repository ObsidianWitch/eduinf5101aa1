all: main.pdf

main.pdf: *.tex
	pdflatex main.tex
	pdflatex main.tex

clean:
	rm -f *.log *.aux *.toc *.blg *.bbl *.out *.pyg main.pdf
