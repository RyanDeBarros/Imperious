#pragma once

#define _IMP_UNPAREN_IMPL(...) __VA_ARGS__
#define IMP_UNPAREN(X) _IMP_UNPAREN_IMPL X
#define _IMP_CONCAT(...) __VA_ARGS__
#define IMP_FLATTEN(...) IMP_UNPAREN((_IMP_CONCAT(__VA_ARGS__)))
