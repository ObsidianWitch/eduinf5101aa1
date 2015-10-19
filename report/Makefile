all: main.pdf

main.pdf: *.tex
	pdflatex -shell-escape main.tex

clean:
	rm -f *.log *.aux *.toc *.blg *.bbl *.out *.pyg main.pdf
