<!-- Name: CodingStandards -->
<!-- Version: 5 -->
<!-- Last-Modified: 2009/11/09 09:21:05 -->
<!-- Author: artem -->
# Coding Standards

There are no coding standards yet defined for this project. This is an initial attempt to create some.

This needs to be discussed, since the current document is just a single person's draft.

Also read 'Code commits best practices' and 'Coding Conventions' in [Contributing](https://github.com/mapnik/mapnik/blob/master/docs/contributing.markdown), in the Docs. 

## C++

 * Trailing whitespace is never allowed. This includes lines with nothing but spaces or tabs in them, as well as spaces or tabs being the last characters on a line with non-whitespace characters.
 * Indentation is four spaces.
  * Case statements?
  * Public and private declarations?
  * Line continuation?
 * Should a maximum line limit be enforced?
 * Single spaces should surround all binary operators (e.g. +, -. =, /, <<, >>).
 * Parenthesis should not contain padding spaces:
  * Acceptable: (a == b)
  * Not Acceptable: ( a == b )
 * Function definitions should not be separated from their arguments:
  * Acceptable: void foo(int a) { ... }
  * Not Acceptable: void foo (int a) { ... }
 * Keywords should be separated from their arguments by a single space:
  * Acceptable: if (a == b)
  * Not Acceptable: if(a == b)
 * Braces should always be on a separate line, and indented to the same level as the line that they're associated with.
 * Functions should be separated from each other by a single blank line.
 * Comma-separated lists should have spaces after each comma.

(Emacs C++ mode - copy & paste into .emacs file)

    ;;  mapnik c++ 
    
    (setq c-default-style "bsd")
    ;; no tabs please
    (setq indent-tabs-mode nil)
    ;; ident by four spaces
    (setq c-basic-offset 4)
    ;; don't ident inside namespace decl
    (c-set-offset 'innamespace 0)
    ;;
    (c-set-offset 'template-args-cont 'c-lineup-template-args)
    



## Python

It would probably be best to follow Guido van Rossum's preferred coding style, as documented in [PEP 8](http://www.python.org/dev/peps/pep-0008/).

 * Trailing whitespace is never allowed. This includes lines with nothing but spaces or tabs in them, as well as spaces or tabs being the last characters on a line with non-whitespace characters.
 * Indentation is four spaces.

## Tools

We will need some tools to automatically reformat the code before a commit is done. The initial run of these tools will likely result in a massive commit that only contains formatting changes.

Candidates:

 * [astyle](http://astyle.sourceforge.net/)
 * [GC GreatCode](http://sourceforge.net/projects/gcgreatcode/)
 * [indent](http://www.gnu.org/software/indent/)
