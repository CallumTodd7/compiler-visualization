# Compiler Visualization

A compiler which generates intel syntax x86-64 NASM assembly, indented to be run on Ubuntu, while visualising the inner data transformations from source code to assembly.
 
## Requirements

- macOS 10.14+
- OpenGL
- Git
- Docker for macOS, or Ubuntu and NASM


## Included Third Party Code/Assets

- Love2d v11.3; modified, stripped down to remove lua and unused modules 
- SDL v2.0.10; unmodified
- FreeType v2.8.1; unmodified
- Source Code Pro v2.30.2 font by Adobe; unmodified

## User Guide

### Instillation

1. Pull down source repo
2. Run build.sh to build the application

### Running the Application

In the root project directory run: ./bin/cv -i <source_code> -o <output_filepath>

The --no-ui flag can be added to run the compiler without the visualisation.

Running the application will output an x86-64, NASM and Ubuntu compatible, assembly file which can be assembled into an ELF binary. If the visualisation is enabled, a widow will be spawned visualisation the compilation. Press the space bar to start the visualisation.

In order to run the outputted assembly code on a Ubuntu machine:

- Copy the asm file to the Ubuntu machine
- Assemble with: `nasm -g -felf64 output.asm`
- Link with: `gcc -g -no-pie -o output output.o`
  - GCC will also include the C standard library
- Run with: `./output`

In order to run the outputted assembly using Docker:

- Copy the asm file to the [./ubuntu_data](./ubuntu_data) folder, renaming it to `output.asm` if needed
- Run the `ubuntu.sh` script in the root of the project
  - This will open a Ubuntu shell 
- Run `./build.sh` to assemble and run the assembly file
- To exit the Ubuntu shell, run `exit`

### Window Keybindings

- q/Escape: Quit application
- Space bar: Start the visualisation
- Space bar: pause/unpause the visualisation
- +: Speed up visualisation
- -: Slow down the visualisation
- 0: Reset the speed of the visualisation
- Right arrow: Skip forward 1 step in the visualisation
- Down arrow: Skip to the next section of the visualisation

### Source Programming Language

The application accepts a custom C-style programming language. Below is an example source file that explains the various language features.

```
// This is a comment

/*
This is a multi-line comment.
Multi-line comments can also be nested as such:
/* Nested comment */
*/

/*
This is an external procedure. The C calling convention is used, so any C procedures in linked libraries can be used.
*/
extern void printf(void format, int a, int b, int c)

/*
This is a procedure that accepts two parameters.
All procedures should start with `void` because procedure return values are not supported.
*/
void printSum(int firstParam, int secondParam) {
  // Variable assignment and arithmetic operators (`+`, `-`, `/`, and `*` are supported)
  int sum = firstParam + secondParam;

  // This is a procedure call.
  // String literals are supported, but strings as a data type are not.
  printf("%d + %d is %d!", sum);
}

/*
The program entrypoint uses the following procedure signature.
*/
void main() {
  int counterA = 5;
  int counterB = 5;

  printf("Printing %d summed numbers using two counters (%d and %d):", counterA * counterB, counterA, counterB);

  // While statements execute their block while the conditional expression does not evaluate to zero.
  // Supported relational operators are: `>`, `>=`, `<`, `<=`, `==`, and `!=`.
  while (counterA > 0) {
    while (counterB > 0) {
      // If statements execute their block if the conditional expression does not evaluate to zero.
      // Note: `else` blocks are also supported in an if statement.
      if (counterA != 3) {
        printSum(counterA, counterB);
      }

      // Varaibles can be assigned after they are declared
      counterB = counterB - 1;
    }
    counterA = counterA - 1;
  }

  int counter = 0;
  while (1) {
    counter = counter + 1;

    if (counter >= 10) {
      // `break` will exit out of the current while loop.
      break;
    } else if (counter == 5) { // If statements can occur directly after the `else` keyword.
      // `continue` will skip to the next iteration of the while loop.
      continue;
    }

    print("The number is %d. %d is less than 10. %d is not 5.", counter, counter, counter);
  }
}
```

### Understanding the visualisation

The visualisation’s current section is displayed in the top left of the window. The visualisation’s current playback speed is displayed in the top right.

There are three sections to the visualisation: Lexer, Parser, and Code Generation.

#### Lexer

When lexing: the inputted source code is displayed on the left, with the currently observed characters highlighted; a checklist of the decision tree for the highlighted characters is displayed in the middle; and the outputted stream of tokens is displayed on the right.

Two colours are used when highlighting characters: magenta being a peeked character, and yellow being the current selection.

#### Parser

When parsing: the inputted stream of tokens is displayed on the left; and the Abstract Syntax Tree intermediate representation is displayed on the right. Each node in the tree displays the node type as the first line of text, followed by any parameters the node has. A parameter may either have a text value or a child node, in which case it will be colour coded. Child nodes are colour coded to the parent node’s parameter.

#### Code Generation

When in the code generation phase: the inputted Abstract Syntax Tree is displayed on the left; the register table is displayed in the top middle; the locations tables is displayed in the middle; the constants table is displayed in the bottom middle; and the outputted assembly instructions are displayed on the right. The register table is responsible for showing register availability, with the left-hand column listing the registers and the right-hand column listing the register’s contents. The locations table is responsible for showing all variables and intermediate values currently in use. The constants table is responsible for showing a list of all literal values (string literals or integer literals). Comments in the outputted assembly are coloured grey.

## Notices

Portions of this software are copyright © 2017 The FreeType Project (www.freetype.org). All rights reserved.
