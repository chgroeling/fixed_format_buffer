# fixed_format_buffer

Allocation-free fixed-capacity formatting buffer for embedded C++. Usable on the stack, as a class member, or statically allocated. Formatted string views are transient and valid only until the buffer is modified, reused, or destroyed.
