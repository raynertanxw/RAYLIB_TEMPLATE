#ifndef UTILS_H
#define UTILS_H

// NOTE: We can use this to utilities related functions
// We can't do a pragma ignore for clang / gcc because it requires at least a function outside else warnings appear.
#if defined(__clang__) || defined(__GNUC__)
#define MARK_IGNORE_UNUSED_FUNC __attribute__((unused))
#else
#define MARK_IGNORE_UNUSED_FUNC
#endif

#endif // !UTILS_H
