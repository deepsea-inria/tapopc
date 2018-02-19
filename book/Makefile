all: book.html

CHAPTERS=01-preface.markdown 02-preliminaries.markdown 03-fork-join.markdown \
	04-mutual-exclusion.markdown 05-experimenting-with-sptl.markdown \
	06-work-efficiency.markdown 07-automatic-granularity-control.markdown \
	08-parallel-arrays.markdown 09-parallel-sorting.markdown 10-graphs.markdown

BOOK_CONTENT=title.txt $(CHAPTERS)

ALL_SOURCES=$(BOOK_CONTENT) book.css

book.pdf : $(ALL_SOURCES)
	pandoc $(BOOK_CONTENT) -s -o book.pdf

book.html : $(ALL_SOURCES)
	pandoc $(BOOK_CONTENT) -s --toc --number-sections --mathjax -c book.css -o book.html

clean:
	rm -f *.pdf *.html
