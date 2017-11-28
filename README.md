# CompilerInterpreter

Interpreter and Compiler: (concepts of programming languages class) Spring 2017

## Description:
- This is a class project that was created using C++.
- I created a base language that could solve basic algebraic equations. With each assignment, I added new functionality such as allowing variables, displaying data to the user, and the implementation IF and WHILE structures.
- The HLL1.cpp through HLL6.cpp is the progression of the compiler through the duration of the semester. The compiler creates 3 character long mnemonic codes which are inserted into a .txt file. This file can then be fed into the corresponding ILL1.cpp through ILL5.cpp interpreter to be interpreted. (The HLL6.cpp compiler uses the ILL5.cpp interpreter.)

## Difficulties Faced: 
- I struggled with creating an overall design that allows for future growth
  - Solution: By separating processes by functions, I was able to design functions that scaled well and would allow for simple additions of new symbols.  
- I was having troubles figuring out how to have the compiler recognize valid statements that were being input by the user.
  - Solution: I created a “parseSymbol()” function which determined what kind of symbol is being read in. The function determines what the symbol is by looking at what the single character (i.e. “a-z, 0-9, *,<,>,=” ect.)  is or what the characters in a row are. 
- How to handle errors, such as an incorrect symbol used or a missing symbol.
  - Solution: For this problem, I created an “error()” function that accepts an integer as an argument. In this function, I used a case statement to handle all the different errors I could come up with such as, “Identifier not declared" or "The END is expected". This way the user can receive helpful error messages. 

## New Skills Acquired: 
- I gained an understanding of how a language uses layers of abstraction to hide complexities.
- I learned how a programming language that uses a compiler works and what a compiler and interpreter combination can be used for.
- I also learned about the usefulness of a compiler and interpreter in the sense of sharing information. You can create a compile listing and send that listing to anyone who has your interpreter. The interpreter can then execute the compile listing that you have sent. This means that people can store interpreters on their computers to run many different programs.

## Final Thought:
- I made this project in a class before my summer break. Over my break, I decided to continue my learning by reading the book “Clean Code” by Robert Cecil Martin, from which I learned some valuable lessons for writing code in general. After going through this book, if I was to go back and rewrite this program, I would make my functions more clear and I would try my best to have my functions do only one thing. This increases the readability of the code and makes it easier to understand what the program is actually doing. 
