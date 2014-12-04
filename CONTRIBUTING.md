Contributing
============


Editing the code
----------------

1. Get a [GitHub account](https://github.com/signup/free)
2. [Fork this repository](https://github.com/gmcgoldr/RooFBU/fork)
3. Clone your fork:  
   `git clone htts://github.com/yourusername/RooFBU.git`
4. Make a branch in which you will work:  
   `git checkout -b name_describing_work master`
5. Modify your branch and make commits as you implement feautres:  
   `git commit -m "Describe what was accomplished"`
6. Push your branch:  
   `git push -u origin name_describing_work`
7. [Submit a pull request](https://github.com/gmcgoldr/RooFBU/compare)

Code Style
----------

### Formatting

* Use [Stroustrup](http://en.wikipedia.org/wiki/Indent_style#Variant:_Stroustrup) style braces and indentation. Do not indent namespaces.
* Indentations are made with 2 spaces, **no tabs**.
* Pointer, reference and const: `Type* name`, `Type& name`, `const Type& name`.
* Brace spacing: `Type function(type parameter)`, `keyword (statement)`
* Line splitting: double indent the following line. **Do not visually indent**.
* When splitting a parameter list, place **only one parameter per line**.
* Arithmetic spacing: use spaces except if trying convey order of operation; e.g. `1 + 1`, `1*2 + 1`.
* Do not declare or define multiple variables on the same line.
* Initialize all variables when they are defined.
* Keep lines to within 80 characters when possible. Do not allow lines to become much larger than this.

Examples:
```
Type function(
    Type parameter,
    Type parameter,
    ...) {
...
}

for (std::vector<Type*>::iterator it = var.begin();
    it != var.end(); ++it) {
...
}
```

### Naming

* Variables use camel case always starting with a **lower case** letter; e.g. `int someVariable = 0;`
* Members variables are like other variables but prefix with `m_`; e.g. `int m_someMember;`
* Static member variables are prefixed with `s_`; e.g. `static int s_someStaticMember;`
* All functions use camel case always starting with a **lower case** letter; e.g. `void someFunction()`
* Type names use camel case always starting with an **upper case** letter; e.g. `class SomeClass {};`
* Use descriptive names instead of short ones. Avoid abbreviations unless they are *very* obvious in context.

### Conventions

* Use `std::string` instead of c-style strings (`char*`).
* Use `std::vector` instead of dynamic arrays wherever possible.
* If an error can propagate and cause unexpected behaviour elsewhere in the program, it should throw an exception.
* Throw exceptions from those in `stdexcept` derived from `std::exception`.
* Exception messages start with `Namespace::memberName:`, `Namespace: functionName:` or `FileName: functionName:`.
* Exceptions can also be reported where they occur by a `std::cerr` statement if it will help identify its source.
* Use initialization lists in constructors as opposed to initializing members inside the constructor body.
* Use [doxygen](http://en.wikipedia.org/wiki/Doxygen) type comments to document header files.
* Avoid `/* */` style comments inside source files.
* List headers roughly in order of generality, starting with `iostream` which is useful to have in all source files.

Examples:
```
#include <iostream>
#include <stdexcept>
// more stl includes ...

#include <HeaderFromDependency.h>
// more includes from that dependency ...

#include "HeaderFromPackage.h"
// more headers from this package
#include "HeaderForSource.h"

void NameSpace::SomeFunction() {
   if (errorWillPropage) {
      std::cerr << "ERROR: message about the circumstances" << std::endl;
      throw std::runtime_exception("Namespace::functionName: message identifies the error");
   }
}
```

### Tips

* Use `const` wherever possible, it helps document the code and allows for some sanity-checking at compile time.
* Use `const` to declare your intent vis-a-vis parameters; e.g. `void function(const Type& name)` isn't going to modify `name`.
* Use pointers when `NULL` is a valid value (e.g. the object being passed might not exist), otherwise use references.
* If a member function doesn't need to access the state of its object, it can be made static.
* If a static member function isn't relevant outside its class, it can be a regular function in the source file.
* Functions that don't need to share state don't need to be in the same class. They can be grouped in a namespace.
* Member variables that are manipulated by the user but not by the class can be public; e.g. configuration values.
* Member variables which the user reads and which are not manipulated can be `const` and public.
* Getters and setters can be useful to maintain a constant interface to members which might change in future code iterations.
* Getters can be useful to provide constant access to a non-constant member; e.g. `Type getVar() const { return m_vars; }`).
* Use comments to explain the code, not describe it. Don't describe what can be read from the code.
* Import headers in every source and header file needing its declarations. This makes it easier to move, re-use and compartmentalize code.
* Use `<>` braces when including external headers. Use `""` quotes for package headers. This helps differentiate them, and allows for the package files to be extracted to a directory and compiled outside the build chain.
