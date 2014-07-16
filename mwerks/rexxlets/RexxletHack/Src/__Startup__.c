
// The CW linker wants a startup function.  
// This file provides a dummy one, which will never get used anyway,
// since we're writing a "hack."
 
void __Startup__(void);
void __Startup__(void) {}
