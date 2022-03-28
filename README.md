# Authors

 - Carolette Saguil (cas699)
 - Hasnain Ali (ha430)

# Test Plan
## How We Determined Program was Correct
We determined that in order for our code to be correct we needed to be able to:

 - read from stdin and print to stdout, if there is no filename given
 - wrap a regular file and print to stdout, if the second argument is a regular file
 - wrap the regular files in a directory (ignoring files that begin with '.' and 'wrap.') and write to a new file in the same directory, if the second argument is a directory

In order for the wrapped file to be correct it would need to:

 - have lines that are less than the given width without cutting off or separating words
 - make paragraphs when the inputted file has a whitespace sequence containing at  least two newlines
 - print a word on a single line when it exceeds the width and finish wrapping, then return EXIT_FAILURE

## Types of Files and Scenarios
We tested scenarios that would cause errors or return exit failure such as:

 - giving more or less than 2 arguments
 - giving a first argument that is not a positive integer
 - giving a first argument that is smaller than the size of a word in a given file
 - giving a second argument that is not a file
 - giving a second argument that is not an existing or regular file

We tested files that included:
 - whitespace sequences containing at least two newlines
 - extremely long words
 - lines where words had one or multiple spaces in between
 - lines that started or ended with whitespace

We tested directories that included:
 - files that started with '.' or 'wrap.'
 - directories with files inside of them

# Design Properties 
`char  checkArgs(int  argc,  char  **argv)`
`char*  readPathName(char*  dir,  char*  de)`
`char*  writePathName(char*  dir,  char*  de)`