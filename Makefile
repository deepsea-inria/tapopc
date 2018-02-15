all: book.html

CHAPTERS=01-preface.markdown 02-cpp-background.markdown 03-fork-join.markdown \
	04-mutual-exclusion.markdown 05-experimenting-with-sptl.markdown \
	06-work-efficiency.markdown 07-automatic-granularity-control.markdown \
	08-parallel-arrays.markdown 09-parallel-sorting.markdown 10-graphs.markdown

SOURCES=title.txt $(CHAPTERS)

book.pdf : $(SOURCES)
	pandoc $(SOURCES) -s -o book.pdf

book.html : $(SOURCES)
	pandoc $(SOURCES) -s --toc --number-sections --mathjax -c book.css -o book.html

clean:
	rm -f *.pdf *.html
