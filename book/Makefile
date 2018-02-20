all: book.html

CHAPTERS=01-preface.markdown 02-preliminaries.markdown 03-introduction.markdown \
	04-multithreading.markdown 05-fork-join.markdown 06-async-finish.markdown \
	07-futures.markdown 08-mutual-exclusion.markdown 09-experimenting-with-sptl.markdown \
	10-work-efficiency.markdown 11-granularity-control.markdown 12-parallel-arrays.markdown \
	13-parallel-sorting.markdown  14-graph-theory.markdown 15-tree-computations.markdown \
	16-models-of-parallelism.markdown 17-graphs.markdown 18-work-stealing.markdown \
	19-parallelism-vs-concurrency.markdown

BOOK_CONTENT=title.txt $(CHAPTERS)

ALL_SOURCES=$(BOOK_CONTENT) book.css

book.pdf : $(ALL_SOURCES)
	pandoc $(BOOK_CONTENT) -s -o book.pdf

book.html : $(ALL_SOURCES)
	pandoc $(BOOK_CONTENT) -s --toc --toc-depth=2 --number-sections --mathjax -c book.css -o book.html

clean:
	rm -f *.pdf *.html
