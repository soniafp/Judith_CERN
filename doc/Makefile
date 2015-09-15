$(shell mkdir -p build/)

reference.pdf: reference.tex
	pdflatex -output-directory=build reference.tex
	pdflatex -output-directory=build reference.tex
	mv build/reference.pdf .

clean:
	rm -rf build/
	rm -f reference.pdf

.PHONY: clean
